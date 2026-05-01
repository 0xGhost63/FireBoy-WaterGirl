#include "DSA.h"
#include <cstring>
using namespace std;

// ════════════════════════════════════════════════════════════
// 1. STACK
// ════════════════════════════════════════════════════════════
void stackInit(Stack* s)                        { s->top = -1; }
bool stackIsEmpty(Stack* s)                     { return s->top == -1; }
bool stackIsFull(Stack* s)                      { return s->top == STACK_MAX - 1; }

void stackPush(Stack* s, float x, float y) {
    if (stackIsFull(s)) return;
    s->top++;
    s->xItems[s->top] = x;
    s->yItems[s->top] = y;
}
void stackPop(Stack* s, float* x, float* y) {
    if (stackIsEmpty(s)) return;
    *x = s->xItems[s->top];
    *y = s->yItems[s->top];
    s->top--;
}
void stackPeek(Stack* s, float* x, float* y) {
    if (stackIsEmpty(s)) return;
    *x = s->xItems[s->top];
    *y = s->yItems[s->top];
}

// ════════════════════════════════════════════════════════════
// 2. QUEUE (circular array)
// ════════════════════════════════════════════════════════════
void queueInit(EventQueue* q)  { q->front = 0; q->rear = -1; q->count = 0; }
bool queueIsEmpty(EventQueue* q) { return q->count == 0; }
bool queueIsFull(EventQueue* q)  { return q->count == QUEUE_MAX; }

void queueEnqueue(EventQueue* q, GameEvent e) {
    if (queueIsFull(q)) return;
    q->rear = (q->rear + 1) % QUEUE_MAX;
    q->items[q->rear] = e;
    q->count++;
}
GameEvent queueDequeue(EventQueue* q) {
    GameEvent e; e.type = 0;
    if (queueIsEmpty(q)) return e;
    e = q->items[q->front];
    q->front = (q->front + 1) % QUEUE_MAX;
    q->count--;
    return e;
}

// ════════════════════════════════════════════════════════════
// 3. DOUBLY LINKED LIST
// ════════════════════════════════════════════════════════════
void listInit(LevelList* lst) {
    lst->head = lst->tail = lst->current = nullptr;
    lst->count = 0;
}
void listAppend(LevelList* lst, LevelData d) {
    LevelNode* node = new LevelNode();
    node->data = d;
    node->next = nullptr;
    node->prev = lst->tail;
    if (lst->tail) lst->tail->next = node;
    else           lst->head = node;
    lst->tail = node;
    lst->count++;
    if (!lst->current) lst->current = node;
}
bool listNext(LevelList* lst) {
    if (!lst->current || !lst->current->next) return false;
    lst->current = lst->current->next;
    return true;
}
bool listPrev(LevelList* lst) {
    if (!lst->current || !lst->current->prev) return false;
    lst->current = lst->current->prev;
    return true;
}
bool listHasNext(LevelList* lst) {
    return lst->current && lst->current->next;
}
void listFree(LevelList* lst) {
    LevelNode* cur = lst->head;
    while (cur) { LevelNode* nxt = cur->next; delete cur; cur = nxt; }
    lst->head = lst->tail = lst->current = nullptr;
    lst->count = 0;
}

// ════════════════════════════════════════════════════════════
// 4. BUBBLE SORT
// ════════════════════════════════════════════════════════════
void bubbleSort(ScoreEntry arr[], int n) {
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (arr[j].score < arr[j+1].score) {
                ScoreEntry tmp = arr[j]; arr[j] = arr[j+1]; arr[j+1] = tmp;
            }
}

// ════════════════════════════════════════════════════════════
// 5. QUICK SORT
// ════════════════════════════════════════════════════════════
static int partition(ScoreEntry arr[], int lo, int hi) {
    int pivot = arr[hi].score, i = lo - 1;
    for (int j = lo; j < hi; j++)
        if (arr[j].score >= pivot) {
            i++;
            ScoreEntry tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
        }
    ScoreEntry tmp = arr[i+1]; arr[i+1] = arr[hi]; arr[hi] = tmp;
    return i + 1;
}
void quickSort(ScoreEntry arr[], int lo, int hi) {
    if (lo < hi) {
        int p = partition(arr, lo, hi);
        quickSort(arr, lo, p - 1);
        quickSort(arr, p + 1, hi);
    }
}

// ════════════════════════════════════════════════════════════
// 6. LINEAR SEARCH
// ════════════════════════════════════════════════════════════
int linearSearchGem(Gem gems[], int count, float px, float py) {
    float r = 22.0f;
    for (int i = 0; i < count; i++) {
        if (gems[i].collected) continue;
        float dx = gems[i].x - px, dy = gems[i].y - py;
        if (dx*dx + dy*dy <= r*r) return i;
    }
    return -1;
}

// ════════════════════════════════════════════════════════════
// 7. BINARY SEARCH
// ════════════════════════════════════════════════════════════
int binarySearch(ScoreEntry arr[], int n, int score) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (arr[mid].score == score) return mid;
        else if (arr[mid].score > score) lo = mid + 1;
        else hi = mid - 1;
    }
    return lo;
}

// ════════════════════════════════════════════════════════════
// 8. BFS PATHFINDING
// ════════════════════════════════════════════════════════════
PathResult bfsFind(int grid[MAP_ROWS][MAP_COLS], int sx, int sy, int gx, int gy) {
    PathResult res; res.len = 0;
    if (sx < 0 || sy < 0 || gx < 0 || gy < 0) return res;

    bool  visited[MAP_ROWS][MAP_COLS];
    int   parentX[MAP_ROWS][MAP_COLS];
    int   parentY[MAP_ROWS][MAP_COLS];
    int   qx[MAP_ROWS * MAP_COLS], qy[MAP_ROWS * MAP_COLS];
    int   qFront = 0, qRear = -1, qCount = 0;

    for (int r = 0; r < MAP_ROWS; r++)
        for (int c = 0; c < MAP_COLS; c++) {
            visited[r][c] = false; parentX[r][c] = -1; parentY[r][c] = -1;
        }

    int cap = MAP_ROWS * MAP_COLS;
    qRear = (qRear + 1) % cap; qx[qRear] = sx; qy[qRear] = sy; qCount++;
    visited[sy][sx] = true;

    int dx[] = {0,0,1,-1}, dy[] = {1,-1,0,0};
    bool found = false;
    while (qCount > 0) {
        int cx = qx[qFront], cy = qy[qFront];
        qFront = (qFront + 1) % cap; qCount--;
        if (cx == gx && cy == gy) { found = true; break; }
        for (int d = 0; d < 4; d++) {
            int nx = cx+dx[d], ny = cy+dy[d];
            if (nx<0||ny<0||nx>=MAP_COLS||ny>=MAP_ROWS) continue;
            if (visited[ny][nx] || grid[ny][nx] != 0) continue;
            visited[ny][nx] = true;
            parentX[ny][nx] = cx; parentY[ny][nx] = cy;
            qRear = (qRear+1)%cap; qx[qRear]=nx; qy[qRear]=ny; qCount++;
        }
    }
    if (!found) return res;

    int tmpX[MAX_HINT_PATH], tmpY[MAX_HINT_PATH], tLen = 0;
    int cx = gx, cy = gy;
    while (!(cx==sx && cy==sy) && tLen < MAX_HINT_PATH) {
        tmpX[tLen]=cx; tmpY[tLen]=cy; tLen++;
        int px = parentX[cy][cx], py = parentY[cy][cx];
        cx=px; cy=py;
    }
    tmpX[tLen]=sx; tmpY[tLen]=sy; tLen++;
    for (int i = 0; i < tLen && i < MAX_HINT_PATH; i++) {
        res.px[i] = tmpX[tLen-1-i];
        res.py[i] = tmpY[tLen-1-i];
    }
    res.len = tLen;
    return res;
}

// ════════════════════════════════════════════════════════════
// 9. PRIORITY QUEUE (Min-Heap)
// Ensures death events are processed before gem events, etc.
// ════════════════════════════════════════════════════════════
void pqInit(PriorityQueue* pq) { pq->size = 0; }
bool pqEmpty(PriorityQueue* pq) { return pq->size == 0; }

// Heapify-up: after inserting at end, bubble item up to keep min-heap
void pqPush(PriorityQueue* pq, GameEvent e) {
    if (pq->size >= PQUEUE_MAX) return;
    int i = pq->size++;
    pq->items[i] = e;
    // Bubble up
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (pq->items[parent].priority <= pq->items[i].priority) break;
        GameEvent tmp = pq->items[parent];
        pq->items[parent] = pq->items[i];
        pq->items[i] = tmp;
        i = parent;
    }
}

// Heapify-down: after removing root, restore heap property
GameEvent pqPop(PriorityQueue* pq) {
    GameEvent result = pq->items[0];
    pq->items[0] = pq->items[--pq->size];
    int i = 0;
    while (true) {
        int left = 2*i+1, right = 2*i+2, smallest = i;
        if (left  < pq->size && pq->items[left].priority  < pq->items[smallest].priority) smallest = left;
        if (right < pq->size && pq->items[right].priority < pq->items[smallest].priority) smallest = right;
        if (smallest == i) break;
        GameEvent tmp = pq->items[i]; pq->items[i] = pq->items[smallest]; pq->items[smallest] = tmp;
        i = smallest;
    }
    return result;
}

// ════════════════════════════════════════════════════════════
// 10. GATE HASH MAP (Direct-Address Table)
// O(1) gate lookup by ID — no need to loop through all gates
// ════════════════════════════════════════════════════════════
void gateMapInit(GateHashMap* m) {
    m->size = 0;
    for (int i = 0; i < MAX_GATES; i++) m->table[i] = -1;
}

void gateMapInsert(GateHashMap* m, int gateId, int index) {
    if (gateId >= 0 && gateId < MAX_GATES) {
        m->table[gateId] = index;
        m->size++;
    }
}

int gateMapGet(GateHashMap* m, int gateId) {
    if (gateId >= 0 && gateId < MAX_GATES) return m->table[gateId];
    return -1;
}

