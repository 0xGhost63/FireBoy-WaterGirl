#pragma once
#include "GameObjects.h"

// ================================================================
// DSA.h  –  All Data Structures & Algorithms used in the game
// ================================================================
//  1. Doubly Linked List  – level catalogue + Undo/Redo history
//  2. QuickSort           – leaderboard sort
//  3. Linear Search       – gem proximity detection
//  4. Binary Search       – player rank in leaderboard
//  5. Dijkstra            – grid-based hint pathfinding
//  6. Priority Queue (Min-Heap) – event processing by urgency
//  7. Gate Hash Map       – O(1) button→gate lookup
//  8. Teleport Hash Map   – O(1) pad ID→index lookup
//  9. BST Map             – sparse tile storage & lookup
// 10. State History       – Undo/Redo snapshots
// 11. Singly Linked List  – gem collection trail
// 12. Screen Stack        – UI screen navigation history
// ================================================================

// ── Limits ───────────────────────────────────────────────────
#define MAX_HINT_PATH   400

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

void listInit   (LevelList* levelCatalog);
void listAppend (LevelList* levelCatalog, LevelData levelData);
bool listNext   (LevelList* levelCatalog);
bool listPrev   (LevelList* levelCatalog);
bool listHasNext(LevelList* levelCatalog);
void listFree   (LevelList* levelCatalog);

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
                            int dstCol, int dstRow);

// ================================================================
// 8. MIN-HEAP – Nearest Gem Finder
// Pushes uncollected gems keyed by distance; pops the closest.
// Returns gem index, or -1 if none reachable.
// ================================================================
// grid = effectiveTileMap; path length is used as the heap key
int gemMinHeapFind(Gem gems[], int gemCount, float px, float py, int playerType,
                   int grid[MAP_ROWS][MAP_COLS]);

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

void gemTrailInit  (GemTrail* trailCatalog);
void gemTrailAppend(GemTrail* trailCatalog, int gemIndex, int playerType);
void gemTrailFree  (GemTrail* trailCatalog);

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

void      pqInit   (PriorityQueue* eventPriorityQueue);
bool      pqEmpty  (PriorityQueue* eventPriorityQueue);
void      pqPush   (PriorityQueue* eventPriorityQueue, GameEvent gameEvent);
GameEvent pqPop    (PriorityQueue* eventPriorityQueue);  // returns lowest-priority-value first

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

void gateMapInit  (GateHashMap* gateMap);
void gateMapInsert(GateHashMap* gateMap, int gateIdKey, int index);
int  gateMapGet   (GateHashMap* gateMap, int gateIdKey); // returns -1 if not found

// ================================================================
// 10.b HASH MAP (Direct-Address Table) – Teleport Pad pairs
// Key = padId used directly as index. O(1) insert and lookup.
// ================================================================
struct TeleportHashMap {
    int table[MAX_TELEPORTS]; // table[padId] = index in lv->pads[] (-1 = empty)
    int size;
};

void teleportMapInit  (TeleportHashMap* teleportMap);
void teleportMapInsert(TeleportHashMap* teleportMap, int padIdKey, int index);
int  teleportMapGet   (TeleportHashMap* teleportMap, int padIdKey); // returns -1 if not found

// ================================================================
// 11. CONVEYOR QUEUE (FIFO – Circular Array for Conveyor Belt)
// Items on the belt are enqueued/dequeued each tick.
// Dequeue → move X by belt speed → Enqueue back.
// ================================================================
void         conveyorQueueInit   (ConveyorQueue* conveyorQueue);
bool         conveyorQueueEmpty  (ConveyorQueue* conveyorQueue);
bool         conveyorQueueFull   (ConveyorQueue* conveyorQueue);
void         conveyorQueueEnqueue(ConveyorQueue* conveyorQueue, ConveyorItem item);
ConveyorItem conveyorQueueDequeue(ConveyorQueue* conveyorQueue);

void bstInit(BSTMap* tileTree);
void bstInsert(BSTMap* tileTree, int row, int col, int type);
int  bstGet(BSTMap* tileTree, int row, int col); // Returns TILE_EMPTY if not found
void bstFree(BSTNode* treeNode);

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

void historyInit  (StateHistory* gameHistory);
void historyFree  (StateHistory* gameHistory);
void historyPush  (StateHistory* gameHistory, const GameSnapshot& snap);
// Returns true and fills *restoredSnapshot if there is a previous state
bool historyUndo  (StateHistory* gameHistory, GameSnapshot* restoredSnapshot);
// Returns true and fills *restoredSnapshot if there is a next state
bool historyRedo  (StateHistory* gameHistory, GameSnapshot* restoredSnapshot);

// ================================================================
// 12. SCREEN STACK (Array-Based Stack)
// Tracks which UI screens the user has visited.
// Push a screen index when navigating forward;
// pop to go back to the previous screen.
// Works like the browser Back button.
// ================================================================
#define SCREEN_STACK_MAX 10

struct ScreenStack {
    int items[SCREEN_STACK_MAX];
    int top;  // index of the top element (-1 = empty)
};

void screenStackInit (ScreenStack* navigationStack);
bool screenStackEmpty(ScreenStack* navigationStack);
bool screenStackFull (ScreenStack* navigationStack);
void screenStackPush (ScreenStack* navigationStack, int screenIndex);
int  screenStackPop  (ScreenStack* navigationStack);  // returns -1 if empty
int  screenStackPeek (ScreenStack* navigationStack);  // returns top without removing, -1 if empty

