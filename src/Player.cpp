#include "../include/Player.h"
#include <cmath>
using namespace std;

static const float GRAVITY  = 0.55f;
static const float MAX_FALL = 16.0f;
static const float WALK_SPD = 4.5f;
static const float JUMP_VEL = -13.0f;

void playerInit(Player* p, int type, float x, float y) {
    p->type = type;
    stackInit(&p->history);
    stackInit(&p->checkpoints);
    playerReset(p, x, y);
}

void playerReset(Player* p, float x, float y) {
    p->x = x; p->y = y;
    p->vx = p->vy = 0;
    p->dead = false; p->onGround = false;
    p->moveLeft = p->moveRight = p->jumpWanted = false;
    p->gemsCollected = 0;
}

float playerCenterX(Player* p) { return p->x + PLAYER_W / 2.0f; }
float playerCenterY(Player* p) { return p->y + PLAYER_H / 2.0f; }

void playerSaveCheckpoint(Player* p) { stackPush(&p->checkpoints, p->x, p->y); }

bool playerRestoreCheckpoint(Player* p) {
    if (stackIsEmpty(&p->checkpoints)) return false;
    stackPop(&p->checkpoints, &p->x, &p->y);
    p->vx = p->vy = 0; p->dead = false;
    return true;
}

static bool isSolid(int t) { return t == TILE_SOLID || t == TILE_CONVEYOR_R || t == TILE_CONVEYOR_L; }

static int tileAt(int map[MAP_ROWS][MAP_COLS], float wx, float wy) {
    int c = (int)(wx / TILE_SIZE), r = (int)(wy / TILE_SIZE);
    if (c < 0 || r < 0 || c >= MAP_COLS || r >= MAP_ROWS) return TILE_SOLID;
    return map[r][c];
}

static void resolveCollisions(Player* p, int map[MAP_ROWS][MAP_COLS]) {
    float r = p->x + PLAYER_W, b = p->y + PLAYER_H, mx = p->x + PLAYER_W / 2.f;

    // Floor
    if (p->vy >= 0) {
        if (isSolid(tileAt(map, p->x+2, b)) || isSolid(tileAt(map, r-2, b)) || isSolid(tileAt(map, mx, b))) {
            p->y = (int)(b / TILE_SIZE) * TILE_SIZE - PLAYER_H;
            p->vy = 0; p->onGround = true;
        }
    }
    // Ceiling
    if (p->vy < 0) {
        if (isSolid(tileAt(map, p->x+2, p->y)) || isSolid(tileAt(map, r-2, p->y))) {
            p->y = ((int)(p->y / TILE_SIZE) + 1) * TILE_SIZE;
            p->vy = 0;
        }
    }
    float my = p->y + PLAYER_H / 2.f;
    // Left wall
    if (p->vx < 0 && (isSolid(tileAt(map, p->x, my)) || isSolid(tileAt(map, p->x, b-4)))) {
        p->x = ((int)(p->x / TILE_SIZE) + 1) * TILE_SIZE;
        p->vx = 0;
    }
    // Right wall
    if (p->vx > 0 && (isSolid(tileAt(map, r, my)) || isSolid(tileAt(map, r, b-4)))) {
        p->x = (int)(r / TILE_SIZE) * TILE_SIZE - PLAYER_W;
        p->vx = 0;
    }
}

static void resolvePlatforms(Player* p, MovingPlatform plats[], int count) {
    float b = p->y + PLAYER_H;
    for (int i = 0; i < count; i++) {
        MovingPlatform& mp = plats[i];
        if (!mp.active) continue;
        bool onTop = (p->x + PLAYER_W > mp.cx) && (p->x < mp.cx + 80) &&
                     (b >= mp.cy) && (b <= mp.cy + 20) && (p->vy >= 0);
        if (onTop) {
            p->y = mp.cy - PLAYER_H;
            p->vy = 0; p->onGround = true;
        }
    }
}

void playerUpdate(Player* p, int map[MAP_ROWS][MAP_COLS],
                  MovingPlatform plats[], int platCount) {
    if (p->dead) return;

    p->vx = p->moveLeft ? -WALK_SPD : p->moveRight ? WALK_SPD : 0;

    if (p->jumpWanted && p->onGround) { p->vy = JUMP_VEL; p->jumpWanted = false; }

    p->vy += GRAVITY;
    if (p->vy > MAX_FALL) p->vy = MAX_FALL;

    p->x += p->vx; p->y += p->vy;
    p->onGround = false;

    resolveCollisions(p, map);
    resolvePlatforms(p, plats, platCount);

    // DSA Stack: push position every 4 frames for history
    static int fc = 0;
    if (++fc % 4 == 0) stackPush(&p->history, p->x, p->y);
}
