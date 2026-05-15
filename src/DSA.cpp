#include "DSA.h"
#include <cstring>
#include <cstdlib>


// ════════════════════════════════════════════════════════════
// 1. DOUBLY LINKED LIST
// ════════════════════════════════════════════════════════════
void listInit(LevelList* lst)
{
    lst->head    = nullptr;
    lst->tail    = nullptr;
    lst->current = nullptr;
    lst->count   = 0;
}

void listAppend(LevelList* lst, LevelData d)
{
    LevelNode* node = new LevelNode();
    node->data = d;
    node->next = nullptr;
    node->prev = lst->tail;
    if (lst->tail)
        lst->tail->next = node;
    else
        lst->head = node;
    lst->tail = node;
    lst->count++;
    if (!lst->current)
        lst->current = node;
}

bool listNext(LevelList* lst)
{
    if (!lst->current || !lst->current->next) return false;
    lst->current = lst->current->next;
    return true;
}

bool listPrev(LevelList* lst)
{
    if (!lst->current || !lst->current->prev) return false;
    lst->current = lst->current->prev;
    return true;
}

bool listHasNext(LevelList* lst)
{
    return lst->current && lst->current->next;
}

void listFree(LevelList* lst)
{
    LevelNode* cur = lst->head;
    while (cur)
    {
        LevelNode* nxt = cur->next;
        bstFree(cur->data.tileTree.root); // Free the BST to prevent memory leaks
        delete cur;
        cur = nxt;
    }
    lst->head    = nullptr;
    lst->tail    = nullptr;
    lst->current = nullptr;
    lst->count   = 0;
}

// ════════════════════════════════════════════════════════════
// 4. QUICK SORT  O(n log n) – used for ALL leaderboard sizes
// ════════════════════════════════════════════════════════════
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
// 5. LINEAR SEARCH
// ════════════════════════════════════════════════════════════
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
// 6. BINARY SEARCH
// ════════════════════════════════════════════════════════════
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
// 7. DIJKSTRA WEIGHTED PATHFINDING (Grid-Based)
// ════════════════════════════════════════════════════════════
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
            dist[r][c]    = 1e9f;
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
        // Find the unvisited node with the smallest distance
        int cx = -1, cy = -1;
        float best = 1e9f;
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

        if (cx < 0 || best == 1e9f) break;
        if (cx == dstCol && cy == dstRow) { found = true; break; }
        closed[cy][cx] = true;

        // Check all 4 neighbours
        for (int d = 0; d < 4; d++)
        {
            int nx = cx + dx[d];
            int ny = cy + dy[d];
            if (nx < 0 || ny < 0 || nx >= MAP_COLS || ny >= MAP_ROWS) continue;
            if (closed[ny][nx] || grid[ny][nx] != 0) continue;
            float alt = dist[cy][cx] + 1.0f;
            if (alt < dist[ny][nx])
            {
                dist[ny][nx]    = alt;
                parentX[ny][nx] = cx;
                parentY[ny][nx] = cy;
            }
        }

        // ── DSA: Zero-cost Teleport Edge ────────────────────────
        if (teleportEdges && teleportEdges[cy][cx][0] != -1)
        {
            int tx = teleportEdges[cy][cx][0];
            int ty = teleportEdges[cy][cx][1];
            if (!closed[ty][tx])
            {
                float alt = dist[cy][cx]; // zero cost to warp
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

    // Reconstruct path from destination back to source
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

    // Reverse the path so it goes from source to destination
    for (int i = 0; i < tLen && i < MAX_HINT_PATH; i++)
    {
        res.px[i] = tmpX[tLen - 1 - i];
        res.py[i] = tmpY[tLen - 1 - i];
    }
    res.len = tLen;
    return res;
}

// ════════════════════════════════════════════════════════════
// 8. MIN-HEAP – Nearest Gem Finder
// ════════════════════════════════════════════════════════════
struct HeapEntry { float dist; int idx; };

static void heapSwap(HeapEntry a[], int i, int j)
{
    HeapEntry tmp = a[i];
    a[i] = a[j];
    a[j] = tmp;
}

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

        // DSA: Dijkstra path length = actual traversal cost
        PathResult pr = dijkstraGridFind(grid, pCol, pRow, gCol, gRow, teleportEdges);
        float distVal = (pr.len > 0) ? (float)pr.len : 1e30f;
        heap[hn].dist = distVal;
        heap[hn].idx  = i;
        hn++;
    }

    if (hn == 0) return -1;
    // Build min-heap: root = gem with shortest actual path
    for (int i = hn/2 - 1; i >= 0; i--)
        heapifyDown(heap, hn, i);
    return heap[0].idx;
}

// ════════════════════════════════════════════════════════════
// 12. SINGLY LINKED LIST – Gem Collection Trail
// Each collected gem is appended; iterated on the win screen.
// ════════════════════════════════════════════════════════════
void gemTrailInit(GemTrail* t)
{
    t->head  = nullptr;
    t->tail  = nullptr;
    t->count = 0;
}

void gemTrailAppend(GemTrail* t, int gemIndex, int playerType)
{
    GemTrailNode* node = (GemTrailNode*)malloc(sizeof(GemTrailNode));
    node->gemIndex   = gemIndex;
    node->playerType = playerType;
    node->next       = nullptr;
    if (!t->head)
        t->head = node;
    else
        t->tail->next = node;
    t->tail = node;
    t->count++;
}

void gemTrailFree(GemTrail* t)
{
    GemTrailNode* n = t->head;
    while (n)
    {
        GemTrailNode* nx = n->next;
        free(n);
        n = nx;
    }
    gemTrailInit(t);
}

// ════════════════════════════════════════════════════════════
// 10. PRIORITY QUEUE (Min-Heap)
// Ensures death events are processed before gem events, etc.
// ════════════════════════════════════════════════════════════
void pqInit(PriorityQueue* pq)
{
    pq->size = 0;
}

bool pqEmpty(PriorityQueue* pq)
{
    return pq->size == 0;
}

// Heapify-up: after inserting at end, bubble item up to keep min-heap
void pqPush(PriorityQueue* pq, GameEvent e)
{
    if (pq->size >= PQUEUE_MAX) return;
    int i = pq->size;
    pq->size++;
    pq->items[i] = e;
    // Bubble up
    while (i > 0)
    {
        int parent = (i - 1) / 2;
        if (pq->items[parent].priority <= pq->items[i].priority) break;
        GameEvent tmp        = pq->items[parent];
        pq->items[parent]    = pq->items[i];
        pq->items[i]         = tmp;
        i = parent;
    }
}

// Heapify-down: after removing root, restore heap property
GameEvent pqPop(PriorityQueue* pq)
{
    GameEvent result = pq->items[0];
    pq->size--;
    pq->items[0] = pq->items[pq->size];
    int i = 0;
    while (true)
    {
        int left    = 2*i + 1;
        int right   = 2*i + 2;
        int smallest = i;
        if (left  < pq->size && pq->items[left].priority  < pq->items[smallest].priority) smallest = left;
        if (right < pq->size && pq->items[right].priority < pq->items[smallest].priority) smallest = right;
        if (smallest == i) break;
        GameEvent tmp         = pq->items[i];
        pq->items[i]          = pq->items[smallest];
        pq->items[smallest]   = tmp;
        i = smallest;
    }
    return result;
}

// ════════════════════════════════════════════════════════════
// 10. GATE HASH MAP (Direct-Address Table)
// O(1) gate lookup by ID — no need to loop through all gates
// ════════════════════════════════════════════════════════════
void gateMapInit(GateHashMap* m)
{
    m->size = 0;
    for (int i = 0; i < MAX_GATES; i++)
        m->table[i] = -1;
}

void gateMapInsert(GateHashMap* m, int gateId, int index)
{
    if (gateId >= 0 && gateId < MAX_GATES)
    {
        m->table[gateId] = index;
        m->size++;
    }
}

int gateMapGet(GateHashMap* m, int gateId)
{
    if (gateId >= 0 && gateId < MAX_GATES)
        return m->table[gateId];
    return -1;
}

// ════════════════════════════════════════════════════════════
// 10.b TELEPORT HASH MAP (Direct-Address Table)
// Fast lookup of teleport pads by ID.
// ════════════════════════════════════════════════════════════
void teleportMapInit(TeleportHashMap* m)
{
    m->size = 0;
    for (int i = 0; i < MAX_TELEPORTS; i++)
        m->table[i] = -1;
}

void teleportMapInsert(TeleportHashMap* m, int padId, int index)
{
    if (padId >= 0 && padId < MAX_TELEPORTS)
    {
        m->table[padId] = index;
        m->size++;
    }
}

int teleportMapGet(TeleportHashMap* m, int padId)
{
    if (padId >= 0 && padId < MAX_TELEPORTS)
        return m->table[padId];
    return -1;
}

// ════════════════════════════════════════════════════════════
// 11. CONVEYOR QUEUE (Circular Array FIFO for Conveyor Belt)
// Each tick: dequeue item -> modify X by belt speed -> enqueue back
// ════════════════════════════════════════════════════════════
void conveyorQueueInit(ConveyorQueue* q)
{
    q->front = 0;
    q->rear  = -1;
    q->count = 0;
}

bool conveyorQueueEmpty(ConveyorQueue* q)
{
    return q->count == 0;
}

bool conveyorQueueFull(ConveyorQueue* q)
{
    return q->count == CONVEYOR_QUEUE_MAX;
}

void conveyorQueueEnqueue(ConveyorQueue* q, ConveyorItem item)
{
    if (conveyorQueueFull(q)) return;
    q->rear = (q->rear + 1) % CONVEYOR_QUEUE_MAX;
    q->items[q->rear] = item;
    q->count++;
}

ConveyorItem conveyorQueueDequeue(ConveyorQueue* q)
{
    ConveyorItem item;
    item.id = -1;
    item.x  = 0;
    item.y  = 0;
    if (conveyorQueueEmpty(q)) return item;
    item = q->items[q->front];
    q->front = (q->front + 1) % CONVEYOR_QUEUE_MAX;
    q->count--;
    return item;
}

// ════════════════════════════════════════════════════════════
// 11. SPARSE MATRIX BST (Map Layout)
// ════════════════════════════════════════════════════════════
void bstInit(BSTMap* tree)
{
    tree->root = nullptr;
}

static BSTNode* bstInsertNode(BSTNode* node, int key, int type)
{
    if (!node)
    {
        BSTNode* n  = new BSTNode();
        n->key      = key;
        n->type     = type;
        n->left     = nullptr;
        n->right    = nullptr;
        return n;
    }
    if (key < node->key)
        node->left  = bstInsertNode(node->left,  key, type);
    else if (key > node->key)
        node->right = bstInsertNode(node->right, key, type);
    else
        node->type  = type; // update existing
    return node;
}

void bstInsert(BSTMap* tree, int r, int c, int type)
{
    if (type == TILE_EMPTY) return; // Don't store empty space
    int key = r * MAP_COLS + c;
    tree->root = bstInsertNode(tree->root, key, type);
}

int bstGet(BSTMap* tree, int r, int c)
{
    if (r < 0 || r >= MAP_ROWS || c < 0 || c >= MAP_COLS)
        return TILE_SOLID; // out of bounds
    int key = r * MAP_COLS + c;
    BSTNode* curr = tree->root;
    while (curr)
    {
        if (key == curr->key) return curr->type;
        if (key < curr->key)  curr = curr->left;
        else                  curr = curr->right;
    }
    return TILE_EMPTY; // not found means empty space
}

void bstFree(BSTNode* node)
{
    if (!node) return;
    bstFree(node->left);
    bstFree(node->right);
    delete node;
}

// ================================================================

// ════════════════════════════════════════════════════════════
// 13. STATE HISTORY  (Doubly Linked List – Undo / Redo)
// ════════════════════════════════════════════════════════════
void historyInit(StateHistory* h)
{
    h->head    = nullptr;
    h->tail    = nullptr;
    h->current = nullptr;
    h->count   = 0;
}

void historyFree(StateHistory* h)
{
    HistoryNode* n = h->head;
    while (n)
    {
        HistoryNode* next = n->next;
        free(n);
        n = next;
    }
    historyInit(h);
}

// Push a new snapshot.
// If we are NOT at the tail (i.e. the player undid some steps and then
// continued playing), discard everything after current — just like any
// real undo system (branching history is not supported).
// If count reaches HISTORY_MAX, evict the oldest node from the head.
void historyPush(StateHistory* h, const GameSnapshot& snap)
{
    // Discard future (forward) nodes when new action is taken after undo
    if (h->current && h->current != h->tail)
    {
        HistoryNode* toDelete = h->current->next;
        h->current->next = nullptr;
        h->tail = h->current;
        while (toDelete)
        {
            HistoryNode* nx = toDelete->next;
            free(toDelete);
            h->count--;
            toDelete = nx;
        }
    }

    // Allocate new node
    HistoryNode* node = (HistoryNode*)malloc(sizeof(HistoryNode));
    node->snap = snap;
    node->next = nullptr;
    node->prev = h->tail;

    if (h->tail)  h->tail->next = node;
    else          h->head = node; // first ever node
    h->tail    = node;
    h->current = node;
    h->count++;

    // Evict oldest if over cap
    if (h->count > HISTORY_MAX)
    {
        HistoryNode* old = h->head;
        h->head = old->next;
        if (h->head) h->head->prev = nullptr;
        free(old);
        h->count--;
    }
}

// Move current backward (undo). Returns false if already at beginning.
bool historyUndo(StateHistory* h, GameSnapshot* out)
{
    if (!h->current || !h->current->prev) return false;
    h->current = h->current->prev;
    *out = h->current->snap;
    return true;
}

// Move current forward (redo). Returns false if already at most recent.
bool historyRedo(StateHistory* h, GameSnapshot* out)
{
    if (!h->current || !h->current->next) return false;
    h->current = h->current->next;
    *out = h->current->snap;
    return true;
}

// ════════════════════════════════════════════════════════════
// 12. SCREEN STACK (Array-Based)
// ════════════════════════════════════════════════════════════
void screenStackInit(ScreenStack* s)
{
    s->top = -1;
}

bool screenStackEmpty(ScreenStack* s)
{
    return s->top == -1;
}

bool screenStackFull(ScreenStack* s)
{
    return s->top == SCREEN_STACK_MAX - 1;
}

void screenStackPush(ScreenStack* s, int screenIndex)
{
    if (screenStackFull(s)) return;
    s->top++;
    s->items[s->top] = screenIndex;
}

int screenStackPop(ScreenStack* s)
{
    if (screenStackEmpty(s)) return -1;
    int val = s->items[s->top];
    s->top--;
    return val;
}

int screenStackPeek(ScreenStack* s)
{
    if (screenStackEmpty(s)) return -1;
    return s->items[s->top];
}
