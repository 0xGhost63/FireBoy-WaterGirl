#pragma once

// ── Tile types ──────────────────────────────────────────────
#define TILE_EMPTY    0
#define TILE_SOLID    1
#define TILE_LAVA     2   // kills Watergirl, safe for Fireboy
#define TILE_WATER    3   // kills Fireboy, safe for Watergirl
#define TILE_POISON   4   // kills both
#define TILE_CONVEYOR_R 5 // conveyor pushing right (solid + pushes player)
#define TILE_CONVEYOR_L 6 // conveyor pushing left  (solid + pushes player)

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
#define EVT_GEM_COLLECT    1   // player picked up a gem
#define EVT_PLAYER_DEAD    2   // player touched a deadly hazard
#define EVT_LEVEL_COMPLETE 3   // both doors opened at the same time
#define EVT_TELEPORT       4   // player stepped on a teleport pad

// ── Map & size constants ─────────────────────────────────────
#define TILE_SIZE     40
#define MAP_ROWS      16
#define MAP_COLS      20
#define MAP_W         (MAP_COLS * TILE_SIZE)
#define MAP_H         (MAP_ROWS * TILE_SIZE)
#define MAX_GEMS      50
#define MAX_HAZARDS   50
#define MAX_BUTTONS   20
#define MAX_CONVEYORS 50
#define CONVEYOR_QUEUE_MAX 8
#define MAX_GATES     20
#define MAX_TELEPORTS  8   // max teleport pads per level (pairs)
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
    float x, y, h;
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

// ── DSA: TeleportPad (Hash Map lookup by id) ─────────────────
// Pairs of pads warp a player from one to its partner.
// Uses TeleportMap (direct-address hash) for O(1) partner lookup.
struct TeleportPad {
    float x, y;          // world-pixel top-left (tile-aligned)
    int   id;            // unique pad ID (used as hash key)
    int   partnerId;     // ID of the destination pad
    float cooldown;      // seconds before re-trigger (anti-bounce)
};

// ── Conveyor Belt (Queue-based DSA) ──────────────────────────
// Items on the belt are stored in a Queue. Every tick, dequeue
// the item, add speed to its X, and enqueue it back.
struct ConveyorItem {
    int   id;         // 0=fireboy, 1=watergirl
    float x, y;
};

struct ConveyorQueue {
    ConveyorItem items[CONVEYOR_QUEUE_MAX];
    int front, rear, count;
};

struct ConveyorBelt {
    float x, y, w, h;   // belt region (world coordinates)
    float speed;         // positive = push right, negative = push left
    ConveyorQueue queue; // queue tracking items on this belt
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

    // Conveyor belts (Queue DSA)
    ConveyorBelt   conveyors[MAX_CONVEYORS];
    int            conveyorCount;

    // Cooperative interactive elements
    Button         buttons[MAX_BUTTONS];
    int            buttonCount;
    Gate           gates[MAX_GATES];
    int            gateCount;

    // DSA: TeleportPad – Hash Map-based portal pairs
    TeleportPad    pads[MAX_TELEPORTS];
    int            teleportCount;

    int bgStyle;  // 0=forest  1=cave  2=ruins
};
