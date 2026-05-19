#include "DSA.h"
#include <cstring>
#include <cstdlib>


// ════════════════════════════════════════════════════════════
// 1. DOUBLY LINKED LIST – Level catalog navigation
// ════════════════════════════════════════════════════════════

// Clears the level list to an empty state (head, tail, current = nullptr).
void listInit(LevelList* levelCatalog)
{
    levelCatalog->head    = nullptr;
    levelCatalog->tail    = nullptr;
    levelCatalog->current = nullptr;
    levelCatalog->count   = 0;
}

// Appends a new level node at the tail. Sets current to the first node if unset.
void listAppend(LevelList* levelCatalog, LevelData levelData)
{
    LevelNode* newLevelNode = new LevelNode();
    newLevelNode->data = levelData;
    newLevelNode->next = nullptr;
    newLevelNode->prev = levelCatalog->tail;
    if (levelCatalog->tail)
        levelCatalog->tail->next = newLevelNode;
    else
        levelCatalog->head = newLevelNode;
    levelCatalog->tail = newLevelNode;
    levelCatalog->count++;
    if (!levelCatalog->current)
        levelCatalog->current = newLevelNode;
}

// Advances current to the next node. Returns false if already at the last level.
bool listNext(LevelList* levelCatalog)
{
    if (!levelCatalog->current || !levelCatalog->current->next) return false;
    levelCatalog->current = levelCatalog->current->next;
    return true;
}

// Moves current to the previous node. Returns false if already at the first level.
bool listPrev(LevelList* levelCatalog)
{
    if (!levelCatalog->current || !levelCatalog->current->prev) return false;
    levelCatalog->current = levelCatalog->current->prev;
    return true;
}

// Returns true if there is a level after the current one.
bool listHasNext(LevelList* levelCatalog)
{
    return levelCatalog->current && levelCatalog->current->next;
}

// Frees every level node and its BST tile tree. Resets the list to empty.
void listFree(LevelList* levelCatalog)
{
    LevelNode* currentLevelNode = levelCatalog->head;
    while (currentLevelNode)
    {
        LevelNode* nextLevelNode = currentLevelNode->next;
        bstFree(currentLevelNode->data.tileTree.root);
        delete currentLevelNode;
        currentLevelNode = nextLevelNode;
    }
    levelCatalog->head    = nullptr;
    levelCatalog->tail    = nullptr;
    levelCatalog->current = nullptr;
    levelCatalog->count   = 0;
}

// ════════════════════════════════════════════════════════════
// 4. QUICK SORT  O(n log n) – Leaderboard score ordering
// ════════════════════════════════════════════════════════════

// Partitions arr[lo..hi] around the pivot score at arr[hi].
// Returns the final pivot index after partitioning.
static int partition(ScoreEntry arr[], int lo, int hi)
{
    int pivot = arr[hi].score;
    int i = lo - 1;
    for (int j = lo; j < hi; j++)
    {
        if (arr[j].score >= pivot)
        {
            i++;
            ScoreEntry tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
        }
    }
    ScoreEntry tmp = arr[i+1];
    arr[i+1] = arr[hi];
    arr[hi]  = tmp;
    return i + 1;
}

// Sorts score entries in descending order (highest score first).
// Used by the leaderboard for all table sizes.
void quickSort(ScoreEntry arr[], int lo, int hi)
{
    if (lo < hi)
    {
        int p = partition(arr, lo, hi);
        quickSort(arr, lo, p - 1);
        quickSort(arr, p + 1, hi);
    }
}

// ════════════════════════════════════════════════════════════
// 5. LINEAR SEARCH – Gem pickup detection
// ════════════════════════════════════════════════════════════

// Scans all gems for one within pickup radius of (px, py).
// Skips collected gems. Returns gem index, or -1 if none found.
int linearSearchGem(Gem gems[], int count, float px, float py)
{
    float radius = 22.0f;
    for (int i = 0; i < count; i++)
    {
        if (gems[i].collected) continue;
        float dx = gems[i].x - px;
        float dy = gems[i].y - py;
        if (dx*dx + dy*dy <= radius*radius)
            return i;
    }
    return -1;
}

// ════════════════════════════════════════════════════════════
// 6. BINARY SEARCH – Sorted leaderboard lookup
// ════════════════════════════════════════════════════════════

// Binary-searches a descending-sorted score array for the given score.
// Returns the index where the score would be inserted (or exact match index).
int binarySearch(ScoreEntry arr[], int n, int score)
{
    int lo = 0, hi = n - 1;
    while (lo <= hi)
    {
        int mid = (lo + hi) / 2;
        if (arr[mid].score == score) return mid;
        else if (arr[mid].score > score) lo = mid + 1;
        else hi = mid - 1;
    }
    return lo;
}

// ════════════════════════════════════════════════════════════
// 7. DIJKSTRA – Grid pathfinding for hints and nearest gem
// ════════════════════════════════════════════════════════════

// Finds the shortest walkable path on grid from (srcCol,srcRow) to (dstCol,dstRow).
// Passability is set by GameEngine::buildGrid() (DIJKSTRA_PASSABLE / DIJKSTRA_BLOCKED).
// Edge weights: DIJKSTRA_STEP_COST per cardinal step, DIJKSTRA_TELEPORT_COST for warps.
// Returns a PathResult with tile coordinates from source to destination.
PathResult dijkstraGridFind(int grid[MAP_ROWS][MAP_COLS],
                            int srcCol, int srcRow,
                            int dstCol, int dstRow,
                            int teleportEdges[MAP_ROWS][MAP_COLS][2])
{
    PathResult res;
    res.len = 0;
    if (srcCol < 0 || srcRow < 0 || dstCol < 0 || dstRow < 0) return res;
    if (srcCol >= MAP_COLS || srcRow >= MAP_ROWS || dstCol >= MAP_COLS || dstRow >= MAP_ROWS) return res;

    float dist[MAP_ROWS][MAP_COLS];
    int parentX[MAP_ROWS][MAP_COLS];
    int parentY[MAP_ROWS][MAP_COLS];
    bool closed[MAP_ROWS][MAP_COLS];

    for (int r = 0; r < MAP_ROWS; r++)
    {
        for (int c = 0; c < MAP_COLS; c++)
        {
            dist[r][c]    = DIJKSTRA_UNREACHABLE;
            parentX[r][c] = -1;
            parentY[r][c] = -1;
            closed[r][c]  = false;
        }
    }

    dist[srcRow][srcCol] = 0;

    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};

    bool found = false;
    int nodes  = MAP_ROWS * MAP_COLS;

    for (int iter = 0; iter < nodes; iter++)
    {
        int cx = -1, cy = -1;
        float best = DIJKSTRA_UNREACHABLE;
        for (int r = 0; r < MAP_ROWS; r++)
        {
            for (int c = 0; c < MAP_COLS; c++)
            {
                if (!closed[r][c] && dist[r][c] < best)
                {
                    best = dist[r][c];
                    cx = c;
                    cy = r;
                }
            }
        }

        if (cx < 0 || best == DIJKSTRA_UNREACHABLE) break;
        if (cx == dstCol && cy == dstRow) { found = true; break; }
        closed[cy][cx] = true;

        for (int d = 0; d < 4; d++)
        {
            int nx = cx + dx[d];
            int ny = cy + dy[d];
            if (nx < 0 || ny < 0 || nx >= MAP_COLS || ny >= MAP_ROWS) continue;
            if (closed[ny][nx] || grid[ny][nx] != DIJKSTRA_PASSABLE) continue;
            float alt = dist[cy][cx] + DIJKSTRA_STEP_COST;
            if (alt < dist[ny][nx])
            {
                dist[ny][nx]    = alt;
                parentX[ny][nx] = cx;
                parentY[ny][nx] = cy;
            }
        }

        if (teleportEdges && teleportEdges[cy][cx][0] != -1)
        {
            int tx = teleportEdges[cy][cx][0];
            int ty = teleportEdges[cy][cx][1];
            if (!closed[ty][tx])
            {
                float alt = dist[cy][cx] + DIJKSTRA_TELEPORT_COST;
                if (alt < dist[ty][tx])
                {
                    dist[ty][tx]    = alt;
                    parentX[ty][tx] = cx;
                    parentY[ty][tx] = cy;
                }
            }
        }
    }

    if (!found) return res;

    int tmpX[MAX_HINT_PATH], tmpY[MAX_HINT_PATH], tLen = 0;
    int cx = dstCol, cy = dstRow;
    while (!(cx == srcCol && cy == srcRow) && tLen < MAX_HINT_PATH)
    {
        tmpX[tLen] = cx;
        tmpY[tLen] = cy;
        tLen++;
        int px = parentX[cy][cx];
        int py = parentY[cy][cx];
        cx = px;
        cy = py;
    }
    tmpX[tLen] = srcCol;
    tmpY[tLen] = srcRow;
    tLen++;

    for (int i = 0; i < tLen && i < MAX_HINT_PATH; i++)
    {
        res.px[i] = tmpX[tLen - 1 - i];
        res.py[i] = tmpY[tLen - 1 - i];
    }
    res.len = tLen;
    return res;
}

// ════════════════════════════════════════════════════════════
// 8. MIN-HEAP – Nearest reachable gem (by Dijkstra path length)
// ════════════════════════════════════════════════════════════

struct HeapEntry { float dist; int idx; };

// Swaps two heap entries in place.
static void heapSwap(HeapEntry a[], int i, int j)
{
    HeapEntry tmp = a[i];
    a[i] = a[j];
    a[j] = tmp;
}

// Restores min-heap property by sinking entry at index i toward the leaves.
static void heapifyDown(HeapEntry a[], int n, int i)
{
    int s = i, l = 2*i+1, r = 2*i+2;
    if (l < n && a[l].dist < a[s].dist) s = l;
    if (r < n && a[r].dist < a[s].dist) s = r;
    if (s != i)
    {
        heapSwap(a, i, s);
        heapifyDown(a, n, s);
    }
}

// Builds a min-heap of uncollected gems owned by playerType, keyed by
// Dijkstra path length from the player. Returns the closest gem index, or -1.
int gemMinHeapFind(Gem gems[], int gemCount, float px, float py, int playerType,
                   int grid[MAP_ROWS][MAP_COLS],
                   int teleportEdges[MAP_ROWS][MAP_COLS][2])
{
    int pCol = (int)(px / TILE_SIZE);
    int pRow = (int)(py / TILE_SIZE);
    if (pCol < 0) pCol = 0;
    if (pCol >= MAP_COLS) pCol = MAP_COLS-1;
    if (pRow < 0) pRow = 0;
    if (pRow >= MAP_ROWS) pRow = MAP_ROWS-1;

    HeapEntry heap[MAX_GEMS];
    int hn = 0;

    for (int i = 0; i < gemCount && i < MAX_GEMS; i++)
    {
        if (gems[i].collected)            continue;
        if (gems[i].owner != playerType)  continue;

        int gCol = (int)((gems[i].x + 16) / TILE_SIZE);
        int gRow = (int)((gems[i].y + 16) / TILE_SIZE);
        if (gCol < 0) gCol = 0;
        if (gCol >= MAP_COLS) gCol = MAP_COLS-1;
        if (gRow < 0) gRow = 0;
        if (gRow >= MAP_ROWS) gRow = MAP_ROWS-1;

        PathResult pr = dijkstraGridFind(grid, pCol, pRow, gCol, gRow, teleportEdges);
        float distVal = (pr.len > 0) ? (float)pr.len : 1e30f;
        heap[hn].dist = distVal;
        heap[hn].idx  = i;
        hn++;
    }

    if (hn == 0) return -1;
    for (int i = hn/2 - 1; i >= 0; i--)
        heapifyDown(heap, hn, i);
    return heap[0].idx;
}

// ════════════════════════════════════════════════════════════
// 9. SINGLY LINKED LIST – Gem collection trail (win screen)
// ════════════════════════════════════════════════════════════

// Initializes an empty gem trail list.
void gemTrailInit(GemTrail* trailCatalog)
{
    trailCatalog->head  = nullptr;
    trailCatalog->tail  = nullptr;
    trailCatalog->count = 0;
}

// Appends a collected gem (index + player type) to the trail in chronological order.
void gemTrailAppend(GemTrail* trailCatalog, int gemIndex, int playerType)
{
    GemTrailNode* newTrailNode = (GemTrailNode*)malloc(sizeof(GemTrailNode));
    newTrailNode->gemIndex   = gemIndex;
    newTrailNode->playerType = playerType;
    newTrailNode->next       = nullptr;
    if (!trailCatalog->head)
        trailCatalog->head = newTrailNode;
    else
        trailCatalog->tail->next = newTrailNode;
    trailCatalog->tail = newTrailNode;
    trailCatalog->count++;
}

// Frees all trail nodes and resets the list to empty.
void gemTrailFree(GemTrail* trailCatalog)
{
    GemTrailNode* currentTrailNode = trailCatalog->head;
    while (currentTrailNode)
    {
        GemTrailNode* nextTrailNode = currentTrailNode->next;
        free(currentTrailNode);
        currentTrailNode = nextTrailNode;
    }
    gemTrailInit(trailCatalog);
}

// ════════════════════════════════════════════════════════════
// 10. PRIORITY QUEUE (Min-Heap) – Game event ordering
// Lower priority number = processed first:
//   0 = death, 1 = teleport/win, 2 = gem collect
// ════════════════════════════════════════════════════════════

// Resets the event queue to empty.
void pqInit(PriorityQueue* eventPriorityQueue)
{
    eventPriorityQueue->size = 0;
}

// Returns true when no events are queued.
bool pqEmpty(PriorityQueue* eventPriorityQueue)
{
    return eventPriorityQueue->size == 0;
}

// Inserts an event and bubbles it up to maintain min-heap order by priority.
void pqPush(PriorityQueue* eventPriorityQueue, GameEvent gameEvent)
{
    if (eventPriorityQueue->size >= PQUEUE_MAX) return;
    int itemIndex = eventPriorityQueue->size;
    eventPriorityQueue->size++;
    eventPriorityQueue->items[itemIndex] = gameEvent;
    while (itemIndex > 0)
    {
        int parentIndex = (itemIndex - 1) / 2;
        if (eventPriorityQueue->items[parentIndex].priority <= eventPriorityQueue->items[itemIndex].priority) break;
        GameEvent tempEvent                         = eventPriorityQueue->items[parentIndex];
        eventPriorityQueue->items[parentIndex]      = eventPriorityQueue->items[itemIndex];
        eventPriorityQueue->items[itemIndex]         = tempEvent;
        itemIndex = parentIndex;
    }
}

// Removes and returns the highest-priority (lowest number) event from the heap.
GameEvent pqPop(PriorityQueue* eventPriorityQueue)
{
    GameEvent highestPriorityEvent = eventPriorityQueue->items[0];
    eventPriorityQueue->size--;
    eventPriorityQueue->items[0] = eventPriorityQueue->items[eventPriorityQueue->size];
    int itemIndex = 0;
    while (true)
    {
        int leftChildIndex    = 2*itemIndex + 1;
        int rightChildIndex   = 2*itemIndex + 2;
        int smallestChildIndex = itemIndex;
        if (leftChildIndex  < eventPriorityQueue->size && eventPriorityQueue->items[leftChildIndex].priority  < eventPriorityQueue->items[smallestChildIndex].priority) smallestChildIndex = leftChildIndex;
        if (rightChildIndex < eventPriorityQueue->size && eventPriorityQueue->items[rightChildIndex].priority < eventPriorityQueue->items[smallestChildIndex].priority) smallestChildIndex = rightChildIndex;
        if (smallestChildIndex == itemIndex) break;
        GameEvent tempEvent                          = eventPriorityQueue->items[itemIndex];
        eventPriorityQueue->items[itemIndex]          = eventPriorityQueue->items[smallestChildIndex];
        eventPriorityQueue->items[smallestChildIndex]   = tempEvent;
        itemIndex = smallestChildIndex;
    }
    return highestPriorityEvent;
}

// ════════════════════════════════════════════════════════════
// 10.a GATE HASH MAP (Direct-Address Table) – O(1) gate lookup
// ════════════════════════════════════════════════════════════

// Sets all gate slots to -1 (empty).
void gateMapInit(GateHashMap* gateMap)
{
    gateMap->size = 0;
    for (int i = 0; i < MAX_GATES; i++)
        gateMap->table[i] = -1;
}

// Maps gateIdKey -> index in lv->gates[]. O(1) insert.
void gateMapInsert(GateHashMap* gateMap, int gateIdKey, int index)
{
    if (gateIdKey >= 0 && gateIdKey < MAX_GATES)
    {
        gateMap->table[gateIdKey] = index;
        gateMap->size++;
    }
}

// Returns the gates[] index for gateIdKey, or -1 if not registered.
int gateMapGet(GateHashMap* gateMap, int gateIdKey)
{
    if (gateIdKey >= 0 && gateIdKey < MAX_GATES)
        return gateMap->table[gateIdKey];
    return -1;
}

// ════════════════════════════════════════════════════════════
// 10.b TELEPORT HASH MAP (Direct-Address Table) – O(1) pad lookup
// ════════════════════════════════════════════════════════════

// Sets all teleport pad slots to -1 (empty).
void teleportMapInit(TeleportHashMap* teleportMap)
{
    teleportMap->size = 0;
    for (int i = 0; i < MAX_TELEPORTS; i++)
        teleportMap->table[i] = -1;
}

// Maps padIdKey -> index in lv->pads[]. O(1) insert.
void teleportMapInsert(TeleportHashMap* teleportMap, int padIdKey, int index)
{
    if (padIdKey >= 0 && padIdKey < MAX_TELEPORTS)
    {
        teleportMap->table[padIdKey] = index;
        teleportMap->size++;
    }
}

// Returns the pads[] index for padIdKey, or -1 if not registered.
int teleportMapGet(TeleportHashMap* teleportMap, int padIdKey)
{
    if (padIdKey >= 0 && padIdKey < MAX_TELEPORTS)
        return teleportMap->table[padIdKey];
    return -1;
}

// ════════════════════════════════════════════════════════════
// 11. CONVEYOR QUEUE (Circular Array FIFO)
// Each tick: dequeue -> move X by belt speed -> enqueue back
// ════════════════════════════════════════════════════════════

// Resets the circular queue to empty (front=0, rear=-1, count=0).
void conveyorQueueInit(ConveyorQueue* conveyorQueue)
{
    conveyorQueue->front = 0;
    conveyorQueue->rear  = -1;
    conveyorQueue->count = 0;
}

// Returns true when the queue holds no items.
bool conveyorQueueEmpty(ConveyorQueue* conveyorQueue)
{
    return conveyorQueue->count == 0;
}

// Returns true when the queue cannot accept more items.
bool conveyorQueueFull(ConveyorQueue* conveyorQueue)
{
    return conveyorQueue->count == CONVEYOR_QUEUE_MAX;
}

// Adds an item at the rear. No-op if the queue is full.
void conveyorQueueEnqueue(ConveyorQueue* conveyorQueue, ConveyorItem conveyorItem)
{
    if (conveyorQueueFull(conveyorQueue)) return;
    conveyorQueue->rear = (conveyorQueue->rear + 1) % CONVEYOR_QUEUE_MAX;
    conveyorQueue->items[conveyorQueue->rear] = conveyorItem;
    conveyorQueue->count++;
}

// Removes and returns the front item. Returns id=-1 if the queue is empty.
ConveyorItem conveyorQueueDequeue(ConveyorQueue* conveyorQueue)
{
    ConveyorItem conveyorItem;
    conveyorItem.id = -1;
    conveyorItem.x  = 0;
    conveyorItem.y  = 0;
    if (conveyorQueueEmpty(conveyorQueue)) return conveyorItem;
    conveyorItem = conveyorQueue->items[conveyorQueue->front];
    conveyorQueue->front = (conveyorQueue->front + 1) % CONVEYOR_QUEUE_MAX;
    conveyorQueue->count--;
    return conveyorItem;
}

// ════════════════════════════════════════════════════════════
// 12. SPARSE MATRIX BST – Level tile map storage
// Key = row * MAP_COLS + col; only non-empty tiles are stored.
// ════════════════════════════════════════════════════════════

// Clears the BST root (does not free existing nodes; use bstFree first if needed).
void bstInit(BSTMap* tileTree)
{
    tileTree->root = nullptr;
}

// Recursive helper: inserts or updates a node with the given key and tile type.
static BSTNode* bstInsertNode(BSTNode* treeNode, int key, int type)
{
    if (!treeNode)
    {
        BSTNode* newBstNode  = new BSTNode();
        newBstNode->key      = key;
        newBstNode->type     = type;
        newBstNode->left     = nullptr;
        newBstNode->right    = nullptr;
        return newBstNode;
    }
    if (key < treeNode->key)
        treeNode->left  = bstInsertNode(treeNode->left,  key, type);
    else if (key > treeNode->key)
        treeNode->right = bstInsertNode(treeNode->right, key, type);
    else
        treeNode->type  = type;
    return treeNode;
}

// Inserts a tile at (row, col). TILE_EMPTY tiles are not stored.
void bstInsert(BSTMap* tileTree, int row, int col, int type)
{
    if (type == TILE_EMPTY) return;
    int key = row * MAP_COLS + col;
    tileTree->root = bstInsertNode(tileTree->root, key, type);
}

// Looks up the tile type at (row, col).
// Out-of-bounds returns TILE_SOLID; missing key returns TILE_EMPTY.
int bstGet(BSTMap* tileTree, int row, int col)
{
    if (row < 0 || row >= MAP_ROWS || col < 0 || col >= MAP_COLS)
        return TILE_SOLID;
    int key = row * MAP_COLS + col;
    BSTNode* currentBstNode = tileTree->root;
    while (currentBstNode)
    {
        if (key == currentBstNode->key) return currentBstNode->type;
        if (key < currentBstNode->key)  currentBstNode = currentBstNode->left;
        else                            currentBstNode = currentBstNode->right;
    }
    return TILE_EMPTY;
}

// Post-order recursive free of the entire BST subtree.
void bstFree(BSTNode* treeNode)
{
    if (!treeNode) return;
    bstFree(treeNode->left);
    bstFree(treeNode->right);
    delete treeNode;
}

// ════════════════════════════════════════════════════════════
// 13. STATE HISTORY (Doubly Linked List) – Undo / Redo
// ════════════════════════════════════════════════════════════

// Initializes an empty undo/redo history list.
void historyInit(StateHistory* gameHistory)
{
    gameHistory->head    = nullptr;
    gameHistory->tail    = nullptr;
    gameHistory->current = nullptr;
    gameHistory->count   = 0;
}

// Frees all history nodes and resets the list to empty.
void historyFree(StateHistory* gameHistory)
{
    HistoryNode* currentHistoryNode = gameHistory->head;
    while (currentHistoryNode)
    {
        HistoryNode* nextHistoryNode = currentHistoryNode->next;
        free(currentHistoryNode);
        currentHistoryNode = nextHistoryNode;
    }
    historyInit(gameHistory);
}

// Appends a game snapshot at the tail. Discards any redo branch if the player
// had undone and then acted again. Evicts the oldest node when count > HISTORY_MAX.
void historyPush(StateHistory* gameHistory, const GameSnapshot& snap)
{
    if (gameHistory->current && gameHistory->current != gameHistory->tail)
    {
        HistoryNode* nodeToDelete = gameHistory->current->next;
        gameHistory->current->next = nullptr;
        gameHistory->tail = gameHistory->current;
        while (nodeToDelete)
        {
            HistoryNode* nextHistoryNode = nodeToDelete->next;
            free(nodeToDelete);
            gameHistory->count--;
            nodeToDelete = nextHistoryNode;
        }
    }

    HistoryNode* newHistoryNode = (HistoryNode*)malloc(sizeof(HistoryNode));
    newHistoryNode->snap = snap;
    newHistoryNode->next = nullptr;
    newHistoryNode->prev = gameHistory->tail;

    if (gameHistory->tail)  gameHistory->tail->next = newHistoryNode;
    else                    gameHistory->head = newHistoryNode;
    gameHistory->tail    = newHistoryNode;
    gameHistory->current = newHistoryNode;
    gameHistory->count++;

    if (gameHistory->count > HISTORY_MAX)
    {
        HistoryNode* evictedHistoryNode = gameHistory->head;
        gameHistory->head = evictedHistoryNode->next;
        if (gameHistory->head) gameHistory->head->prev = nullptr;
        free(evictedHistoryNode);
        gameHistory->count--;
    }
}

// Moves current one step backward and copies that snapshot into restoredSnapshot.
// Returns false if already at the oldest state.
bool historyUndo(StateHistory* gameHistory, GameSnapshot* restoredSnapshot)
{
    if (!gameHistory->current || !gameHistory->current->prev) return false;
    gameHistory->current = gameHistory->current->prev;
    *restoredSnapshot = gameHistory->current->snap;
    return true;
}

// Moves current one step forward and copies that snapshot into restoredSnapshot.
// Returns false if already at the newest state.
bool historyRedo(StateHistory* gameHistory, GameSnapshot* restoredSnapshot)
{
    if (!gameHistory->current || !gameHistory->current->next) return false;
    gameHistory->current = gameHistory->current->next;
    *restoredSnapshot = gameHistory->current->snap;
    return true;
}

// ════════════════════════════════════════════════════════════
// 14. SCREEN STACK (Array-Based) – UI navigation
// ════════════════════════════════════════════════════════════

// Resets the stack to empty (top = -1).
void screenStackInit(ScreenStack* navigationStack)
{
    navigationStack->top = -1;
}

// Returns true when the stack has no screens.
bool screenStackEmpty(ScreenStack* navigationStack)
{
    return navigationStack->top == -1;
}

// Returns true when the stack cannot accept another push.
bool screenStackFull(ScreenStack* navigationStack)
{
    return navigationStack->top == SCREEN_STACK_MAX - 1;
}

// Pushes screenIndex onto the stack. No-op if full.
void screenStackPush(ScreenStack* navigationStack, int screenIndex)
{
    if (screenStackFull(navigationStack)) return;
    navigationStack->top++;
    navigationStack->items[navigationStack->top] = screenIndex;
}

// Pops and returns the top screen index, or -1 if empty.
int screenStackPop(ScreenStack* navigationStack)
{
    if (screenStackEmpty(navigationStack)) return -1;
    int poppedScreen = navigationStack->items[navigationStack->top];
    navigationStack->top--;
    return poppedScreen;
}

// Returns the top screen index without removing it, or -1 if empty.
int screenStackPeek(ScreenStack* navigationStack)
{
    if (screenStackEmpty(navigationStack)) return -1;
    return navigationStack->items[navigationStack->top];
}
