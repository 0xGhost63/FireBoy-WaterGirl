#pragma once
using namespace std;

// ── Tile types ──────────────────────────────────────────────
#define TILE_EMPTY    0
#define TILE_SOLID    1
#define TILE_LAVA     2   // kills Watergirl, safe for Fireboy
#define TILE_WATER    3   // kills Fireboy, safe for Watergirl
#define TILE_POISON   4   // kills both

// ── Character types ─────────────────────────────────────────
#define FIREBOY   0
#define WATERGIRL 1

// ── Game states ─────────────────────────────────────────────
#define STATE_MENU     0
#define STATE_PLAYING  1
#define STATE_PAUSED   2
#define STATE_DEAD     3
#define STATE_WIN      4
#define STATE_GAMEOVER 5

// ── Event types ─────────────────────────────────────────────
#define EVT_GEM_COLLECT    1
#define EVT_PLAYER_DEAD    2
#define EVT_LEVEL_COMPLETE 3
#define EVT_GATE_TOGGLE    4

// ── Map & size constants ─────────────────────────────────────
#define TILE_SIZE     40
#define MAP_ROWS      16
#define MAP_COLS      20
#define MAP_W         (MAP_COLS * TILE_SIZE)
#define MAP_H         (MAP_ROWS * TILE_SIZE)
#define MAX_GEMS      20
#define MAX_HAZARDS   8
#define MAX_PLATFORMS 4
#define MAX_BUTTONS   6
#define MAX_GATES     6
#define MAX_SCORES    10
#define PLAYER_W      28
#define PLAYER_H      40

// ── Core Structs ─────────────────────────────────────────────

struct Gem {
    float x, y;
    int   owner;
    bool  collected;
    float animPhase;
};

struct Door {
    float x, y;
    int   owner;
    bool  open;
};

struct HazardPool {
    float x, y, w, h;
    int   type;
};

// ── Cooperative mechanic: Buttons open/close Gates ───────────

// A pressure plate that a player stands on.
// When pressed it opens the gate with matching gateId.
struct Button {
    float x, y, w, h;   // position and footprint
    int   gateId;        // which gate this controls (0-based ID)
    bool  pressed;       // is a player currently standing on it?
};

// A sliding barrier. Closed = solid wall. Open = passable.
struct Gate {
    int   id;            // unique ID used for DSA gate lookup table
    float x, y, w, h;   // occupies tile-aligned rectangle
    bool  open;          // current state
    float openAnim;      // 0.0 – 1.0 slide animation progress
};

struct MovingPlatform {
    float ax, ay, bx, by;   // waypoints
    float cx, cy;            // current position
    float speed;
    bool  active;
    bool  towardsB;
};

struct GameEvent {
    int   type;
    int   priority;   // lower = more urgent (for priority queue)
    float x, y;
    int   intData;
};

struct ScoreEntry {
    char  name[32];
    int   score;
    int   level;
    float timeSec;
};

// ── Sparse Matrix Map Structures ──────────────────────────────
struct BSTNode {
    int key;    // row * MAP_COLS + col
    int type;   // TILE_SOLID, TILE_LAVA, etc.
    BSTNode* left;
    BSTNode* right;
};

struct BSTMap {
    BSTNode* root;
};

// ── Level (self-contained, no STL containers) ────────────────
struct LevelData {
    int  num;
    char name[64];
    BSTMap tileTree;

    float fireboyStartX,   fireboyStartY;
    float watergirlStartX, watergirlStartY;

    Gem            gems[MAX_GEMS];
    int            gemCount;
    Door           doors[2];
    HazardPool     hazards[MAX_HAZARDS];
    int            hazardCount;
    MovingPlatform platforms[MAX_PLATFORMS];
    int            platformCount;

    // Cooperative interactive elements
    Button         buttons[MAX_BUTTONS];
    int            buttonCount;
    Gate           gates[MAX_GATES];
    int            gateCount;

    int bgStyle;  // 0=forest  1=cave  2=ruins
};
