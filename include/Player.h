#pragma once
#include "../include/GameObjects.h"
#include "../include/DSA.h"

// ── Player struct ────────────────────────────────────────────
struct Player {
    float x, y;          // top-left world position
    float vx, vy;        // velocity
    int   type;          // FIREBOY or WATERGIRL
    bool  dead;
    bool  onGround;
    bool  moveLeft;
    bool  moveRight;
    bool  jumpWanted;
    int   gemsCollected;

    // DSA: Stack – saves positions for checkpoints / respawn
    Stack history;
    Stack checkpoints;
};

void  playerInit             (Player* p, int type, float x, float y);
void  playerReset            (Player* p, float x, float y);
void  playerUpdate           (Player* p, int map[MAP_ROWS][MAP_COLS],
                               MovingPlatform plats[], int platCount);
void  playerSaveCheckpoint   (Player* p);
bool  playerRestoreCheckpoint(Player* p);
float playerCenterX          (Player* p);
float playerCenterY          (Player* p);
