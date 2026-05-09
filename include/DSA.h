#pragma once
#include "GameObjects.h"

// ================================================================
// DSA.h  –  All Data Structures & Algorithms used in the game
// ================================================================
//  1. Queue       – game event pipeline (FIFO / circular)
//  2. LinkedList  – level catalogue (doubly linked) + Undo/Redo
//  3. QuickSort   – leaderboard sort (all list sizes)
//  4. LinearSearch– gem proximity detection
//  5. BinarySearch– find player rank in leaderboard
//  6. Dijkstra    – grid-based hint pathfinding through gems
//  7. Priority Queue (Min-Heap) – events + nearest gem finder
//  8. Hash Map    – O(1) button→gate lookup
//  9. BST Map     – tile storage & lookup
// 10. State History (Doubly Linked List) – Undo/Redo
// 11. Singly Linked List – gem collection trail
// ================================================================

// ── Limits ───────────────────────────────────────────────────
#define QUEUE_MAX  128
#define MAX_HINT_PATH   400

// ================================================================
// 2. QUEUE  (FIFO – First In, First Out, circular array)
// Used to process game events (gem collect, death, win…)
// ================================================================
struct EventQueue {
    GameEvent items[QUEUE_MAX];
    int front, rear, count;
};

void      queueInit    (EventQueue* q);
bool      queueIsEmpty (EventQueue* q);
bool      queueIsFull  (EventQueue* q);
void      queueEnqueue (EventQueue* q, GameEvent e);
GameEvent queueDequeue (EventQueue* q);

// ================================================================
// 3. DOUBLY LINKED LIST
// Used to chain all game levels; easy forward/backward travel
// ================================================================
struct LevelNode {
    LevelData  data;
    LevelNode* next;
    LevelNode* prev;
};

struct LevelList {
    LevelNode* head;
    LevelNode* tail;
    LevelNode* current;
    int        count;
};

void listInit   (LevelList* lst);
void listAppend (LevelList* lst, LevelData d);
bool listNext   (LevelList* lst);
bool listPrev   (LevelList* lst);
bool listHasNext(LevelList* lst);
void listFree   (LevelList* lst);

// ================================================================
// 4. QUICK SORT  O(n log n)  – used for ALL leaderboard sorts
// ================================================================
void quickSort(ScoreEntry arr[], int lo, int hi);

// ================================================================
// 5. LINEAR SEARCH  O(n)
// ================================================================
int linearSearchGem(Gem gems[], int count, float px, float py);

// ================================================================
// 6. BINARY SEARCH  O(log n)
// ================================================================
int binarySearch(ScoreEntry arr[], int n, int score);

// ================================================================
// 7. DIJKSTRA GRID-BASED PATHFINDING (Hint system)
// ================================================================
struct PathResult {
    int px[MAX_HINT_PATH];
    int py[MAX_HINT_PATH];
    int len;
};

PathResult dijkstraGridFind(int grid[MAP_ROWS][MAP_COLS],
                            int srcCol, int srcRow,
                            int dstCol, int dstRow,
                            int teleportEdges[MAP_ROWS][MAP_COLS][2] = nullptr);

// ================================================================
// 8. MIN-HEAP – Nearest Gem Finder
// Pushes uncollected gems keyed by distance; pops the closest.
// Returns gem index, or -1 if none reachable.
// ================================================================
// grid = effectiveTileMap; Dijkstra path length is used as the heap key
int gemMinHeapFind(Gem gems[], int gemCount, float px, float py, int playerType,
                   int grid[MAP_ROWS][MAP_COLS],
                   int teleportEdges[MAP_ROWS][MAP_COLS][2] = nullptr);

// ================================================================
// 12. SINGLY LINKED LIST – Gem Collection Trail
// Appended every time a gem is collected; shown on win screen.
// ================================================================
struct GemTrailNode {
    int   gemIndex;   // index in lv->gems[]
    int   playerType; // FIREBOY or WATERGIRL
    GemTrailNode* next;
};

struct GemTrail {
    GemTrailNode* head;
    GemTrailNode* tail;
    int           count;
};

void gemTrailInit  (GemTrail* t);
void gemTrailAppend(GemTrail* t, int gemIndex, int playerType);
void gemTrailFree  (GemTrail* t);

// ================================================================
// 9. PRIORITY QUEUE  (Min-Heap, array-based)
// Game events are processed by priority: death > win > gem
// This ensures critical events are never delayed by lower-priority ones
// ================================================================
#define PQUEUE_MAX 64

struct PriorityQueue {
    GameEvent items[PQUEUE_MAX];
    int size;
};

void      pqInit   (PriorityQueue* pq);
bool      pqEmpty  (PriorityQueue* pq);
void      pqPush   (PriorityQueue* pq, GameEvent e);
GameEvent pqPop    (PriorityQueue* pq);  // returns lowest-priority-value first

// ================================================================
// 10. GATE HASH MAP  (Direct-Address Table)
// Maps gate ID → gate index in O(1). Faster than linear scan
// when a button is pressed and we need to find the right gate.
// ================================================================
// ================================================================
struct GateHashMap {
    int table[MAX_GATES];   // table[gateId] = index in lv->gates[]
    int size;
};

void gateMapInit  (GateHashMap* m);
void gateMapInsert(GateHashMap* m, int gateId, int index);
int  gateMapGet   (GateHashMap* m, int gateId); // returns -1 if not found

// ================================================================
// 10.b HASH MAP (Direct Addressing) – Teleport Pad pairs
// Fast O(1) lookup: teleport pad ID -> index in lv->pads[] array.
// ================================================================
struct TeleportHashMap {
    int table[MAX_TELEPORTS]; // table[padId] = index in lv->pads[]
    int size;
};

void teleportMapInit  (TeleportHashMap* m);
void teleportMapInsert(TeleportHashMap* m, int padId, int index);
int  teleportMapGet   (TeleportHashMap* m, int padId); // returns -1 if not found

// ================================================================
// 11. CONVEYOR QUEUE (FIFO – Circular Array for Conveyor Belt)
// Items on the belt are enqueued/dequeued each tick.
// Dequeue → move X by belt speed → Enqueue back.
// ================================================================
void         conveyorQueueInit   (ConveyorQueue* q);
bool         conveyorQueueEmpty  (ConveyorQueue* q);
bool         conveyorQueueFull   (ConveyorQueue* q);
void         conveyorQueueEnqueue(ConveyorQueue* q, ConveyorItem item);
ConveyorItem conveyorQueueDequeue(ConveyorQueue* q);

void bstInit(BSTMap* tree);
void bstInsert(BSTMap* tree, int r, int c, int type);
int  bstGet(BSTMap* tree, int r, int c); // Returns TILE_EMPTY if not found
void bstFree(BSTNode* node);

// ================================================================
// 13. STATE HISTORY  (Doubly Linked List – Undo / Redo)
// A snapshot is saved every 500 ms while the game is playing.
// U  = undo  → restore the previous node (current = current->prev)
// R  = redo  → restore the next     node (current = current->next)
// Max 20 nodes  (~10 seconds of history). Oldest node is discarded
// when the list exceeds the limit (like a deque with a cap).
// ================================================================
#define HISTORY_MAX  20

struct GameSnapshot {
    // Player state
    float fbX,  fbY,  fbVX,  fbVY;
    float wgX,  wgY,  wgVX,  wgVY;
    bool  fbOnGround,  wgOnGround;
    // Gem collected flags (mirrors lv->gems[].collected)
    bool  gemCollected[MAX_GEMS];
    int   gemCount;
    // Score at snapshot time
    int   score;
};

struct HistoryNode {
    GameSnapshot  snap;
    HistoryNode*  next;
    HistoryNode*  prev;
};

struct StateHistory {
    HistoryNode*  head;     // oldest
    HistoryNode*  tail;     // newest
    HistoryNode*  current;  // where we are right now
    int           count;
};

void historyInit  (StateHistory* h);
void historyFree  (StateHistory* h);
void historyPush  (StateHistory* h, const GameSnapshot& snap);
// Returns true and fills *out if there is a previous state
bool historyUndo  (StateHistory* h, GameSnapshot* out);
// Returns true and fills *out if there is a next state
bool historyRedo  (StateHistory* h, GameSnapshot* out);
