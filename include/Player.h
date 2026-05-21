#pragma once
#include "../include/GameObjects.h"
#include "../include/DSA.h"

// ── Player struct ────────────────────────────────────────────
struct Player {
    float x, y;          // top-left world position
    float vx, vy;        // velocity
    float spawnX, spawnY;// saved spawn position (replaces Stack checkpoint)
    int   type;          // FIREBOY or WATERGIRL
    bool  dead;
    bool  onGround;
    bool  moveLeft;
    bool  moveRight;
    bool  jumpWanted;
    int   gemsCollected;
};

void  playerInit             (Player* p, int type, float x, float y);
void  playerReset            (Player* p, float x, float y);
void  playerUpdate           (Player* p, int map[MAP_ROWS][MAP_COLS]);
float playerCenterX          (Player* p);
float playerCenterY          (Player* p);

