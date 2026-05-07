#pragma once
#include "GameObjects.h"

// ================================================================
// DSA.h  –  All Data Structures & Algorithms used in the game
// ================================================================
//  1. Stack       – player position history (LIFO)
//  2. Queue       – game event pipeline (FIFO / circular)
//  3. LinkedList  – level catalogue (doubly linked)
//  4. BubbleSort  – leaderboard sort (small lists)
//  5. QuickSort   – leaderboard sort (larger lists)
//  6. LinearSearch – gem proximity detection
//  7. BinarySearch – find player rank in leaderboard
//  8. BFS          – shortest-path hint (legacy tile-grid)
//  9. Graph + Dijk – platform graph + Dijkstra hint pathfinding
// 13. StateHistory  – Doubly Linked List Undo/Redo (U = undo, R = redo)
// ================================================================

// ── Limits ───────────────────────────────────────────────────
#define STACK_MAX  256
#define QUEUE_MAX  128
#define MAX_HINT_PATH   400

// ================================================================
// 1. STACK  (LIFO – Last In, First Out)
// Used to save player positions for checkpoints / respawn
// ================================================================
struct Stack {
    float xItems[STACK_MAX];
    float yItems[STACK_MAX];
    int   top;              // -1 means empty
};

void  stackInit   (Stack* s);
bool  stackIsEmpty(Stack* s);
bool  stackIsFull (Stack* s);
void  stackPush   (Stack* s, float x, float y);
void  stackPop    (Stack* s, float* x, float* y);
void  stackPeek   (Stack* s, float* x, float* y);

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
// 4. BUBBLE SORT  O(n²)
// Sorts leaderboard entries highest-score first (small lists)
// ================================================================
void bubbleSort(ScoreEntry arr[], int n);

// ================================================================
// 5. QUICK SORT  O(n log n)
// Faster sort used when leaderboard has many entries
// ================================================================
void quickSort(ScoreEntry arr[], int lo, int hi);

// ================================================================
// 6. LINEAR SEARCH  O(n)
// Checks every gem to see if a player is close enough to collect
// ================================================================
int linearSearchGem(Gem gems[], int count, float px, float py);

// ================================================================
// 7. BINARY SEARCH  O(log n)
// Finds a player's rank in a sorted leaderboard quickly
// ================================================================
int binarySearch(ScoreEntry arr[], int n, int score);

// ================================================================
// 8. BFS PATHFINDING  (Breadth-First Search on tile grid)
// Returns the shortest safe path from src to dst for hint system
// ================================================================
struct PathResult {
    int px[MAX_HINT_PATH];
    int py[MAX_HINT_PATH];
    int len;
};

PathResult bfsFind(int grid[MAP_ROWS][MAP_COLS],
                   int sx, int sy, int gx, int gy);

// ================================================================
// 9. DIJKSTRA WEIGHTED PATHFINDING (Grid-Based)
// Replaces BFS and Graph A*. 
// Finds minimum-cost path on the tile grid. 
// Provides straight, orthogonal lines for the hint system.
// ================================================================
PathResult dijkstraGridFind(int grid[MAP_ROWS][MAP_COLS],
                            int srcCol, int srcRow,
                            int dstCol, int dstRow);

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
struct GateHashMap {
    int table[MAX_GATES];   // table[gateId] = index in lv->gates[]
    int size;
};

void gateMapInit  (GateHashMap* m);
void gateMapInsert(GateHashMap* m, int gateId, int index);
int  gateMapGet   (GateHashMap* m, int gateId); // returns -1 if not found

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
// 12. CHEAT TRACKER
// Used to track sequence of characters typed by user
// ================================================================
struct CheatTracker {
    char targetCode[16];
    int currentIndex;
    int codeLength;
    bool isUnlocked;
};

void cheatInit(CheatTracker* tracker, const char* code);
bool cheatUpdate(CheatTracker* tracker, char key);

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
