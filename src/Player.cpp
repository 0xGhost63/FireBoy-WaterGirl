

#include "../include/Player.h"
#include <cmath>

// ── Physics constants ──────────────────────────────────────────
// These numbers control how the players feel to control.
// GRAVITY is added to vertical speed every frame (makes them fall).
// MAX_FALL limits how fast they fall (terminal velocity).
// WALK_SPD is how many pixels per frame they walk left/right.
// JUMP_VEL is the upward speed given when jump is pressed (negative = up).

static const float GRAVITY  = 0.55f;    // how fast they accelerate downward
static const float MAX_FALL = 16.0f;    // fastest they can fall (pixels/frame)
static const float WALK_SPD = 4.5f;     // walking speed (pixels/frame)
static const float JUMP_VEL = -13.0f;   // jump speed (negative = upward)

// ── playerInit ────────────────────────────────────────────────
// Sets up a brand new player at a given position.
// type = FIREBOY or WATERGIRL
// x, y = starting position in the world (pixels)
void playerInit(Player* p, int type, float x, float y)
{
    p->type   = type;     // store which character this is
    p->spawnX = x;        // save the starting X as the spawn point
    p->spawnY = y;        // save the starting Y as the spawn point
    playerReset(p, x, y); // reset everything else to defaults
}

// ── playerReset ───────────────────────────────────────────────
// Resets a player's position, velocity, and flags.
// Called when a level starts or player returns to spawn.
void playerReset(Player* p, float x, float y)
{
    p->x = x;            // set position
    p->y = y;

    p->vx = 0;           // clear horizontal speed
    p->vy = 0;           // clear vertical speed

    p->dead      = false; // player is alive
    p->onGround  = true; // not standing on anything yet

    p->moveLeft  = false; // no movement keys held
    p->moveRight = false;
    p->jumpWanted = false;

    p->gemsCollected = 0; // reset gem counter
}

// ── playerCenterX / playerCenterY ────────────────────────────
// Returns the center pixel of the player (used for collision checks)
float playerCenterX(Player* p) { return p->x + PLAYER_W / 2.0f; }
float playerCenterY(Player* p) { return p->y + PLAYER_H / 2.0f; }

// ── playerRestoreCheckpoint ──────────────────────────────────
// Teleports the player back to their saved spawn position.
// Clears velocity and marks them alive again.
bool playerRestoreCheckpoint(Player* p)
{
    p->x    = p->spawnX;
    p->y    = p->spawnY;
    p->vx   = 0;
    p->vy   = 0;
    p->dead = false;
    return true;
}

// ── isSolid ───────────────────────────────────────────────────
// Returns true if a tile type is solid (blocks movement).
// Conveyor belts are solid — you can stand on them.
static bool isSolid(int tileType)
{
    return tileType == TILE_SOLID
        || tileType == TILE_CONVEYOR_R
        || tileType == TILE_CONVEYOR_L;
}

// ── tileAt ────────────────────────────────────────────────────
// Looks up which tile is at a world pixel position (wx, wy).
// Converts pixel → tile index, then reads the tile map array.
// Returns TILE_SOLID if the position is outside the map boundary.
static int tileAt(int map[MAP_ROWS][MAP_COLS], float wx, float wy)
{
    int col = (int)(wx / TILE_SIZE);   // convert pixel X to tile column
    int row = (int)(wy / TILE_SIZE);   // convert pixel Y to tile row

    // If outside the map, treat as solid wall so player can't escape
    if (col < 0 || row < 0 || col >= MAP_COLS || row >= MAP_ROWS)
        return TILE_SOLID;

    return map[row][col]; // look up the tile type in the 2D array
}

// ── resolveCollisions ─────────────────────────────────────────
// Pushes the player out of any solid tile they overlap with.
// Checks all four sides: floor, ceiling, left wall, right wall.
// This is the main function that prevents players from falling
// through the ground or walking through walls.
static void resolveCollisions(Player* p, int map[MAP_ROWS][MAP_COLS])
{
    // Calculate useful edge positions
    float rightEdge  = p->x + PLAYER_W;      // right side of player
    float bottomEdge = p->y + PLAYER_H;      // bottom (feet) of player
    float midX       = p->x + PLAYER_W / 2.f; // horizontal center

    // ── Floor collision ─────────────────────────────────────
    // Check if there's a solid tile at the player's feet (only when falling down)
    if (p->vy >= 0)
    {
        bool leftFootSolid  = isSolid(tileAt(map, p->x + 2,    bottomEdge));
        bool rightFootSolid = isSolid(tileAt(map, rightEdge - 2, bottomEdge));
        bool midFootSolid   = isSolid(tileAt(map, midX,         bottomEdge));

        if (leftFootSolid || rightFootSolid || midFootSolid)
        {
            // Snap the player's Y to sit exactly on top of the tile below
            p->y        = (int)(bottomEdge / TILE_SIZE) * TILE_SIZE - PLAYER_H;
            p->vy       = 0;     // stop downward movement
            p->onGround = true;  // mark that they are on the ground (can jump)
        }
    }

    // ── Ceiling collision ────────────────────────────────────
    // Check if the player's head hits a solid tile (only when moving up)
    if (p->vy < 0)
    {
        bool leftHeadSolid  = isSolid(tileAt(map, p->x + 2,    p->y));
        bool rightHeadSolid = isSolid(tileAt(map, rightEdge - 2, p->y));

        if (leftHeadSolid || rightHeadSolid)
        {
            // Snap the player's Y down to just below the ceiling tile
            p->y  = ((int)(p->y / TILE_SIZE) + 1) * TILE_SIZE;
            p->vy = 0; // stop upward movement (head hit the ceiling)
        }
    }

    // ── Left wall collision ──────────────────────────────────
    // Check if the player's left side runs into a wall (only when moving left)
    float midY = p->y + PLAYER_H / 2.f; // vertical midpoint of player
    if (p->vx < 0)
    {
        bool topLeftSolid = isSolid(tileAt(map, p->x, midY));
        bool botLeftSolid = isSolid(tileAt(map, p->x, bottomEdge - 4));

        if (topLeftSolid || botLeftSolid)
        {
            // Snap the player's X so they are flush against the wall's right edge
            p->x  = ((int)(p->x / TILE_SIZE) + 1) * TILE_SIZE;
            p->vx = 0; // stop horizontal movement
        }
    }

    // ── Right wall collision ─────────────────────────────────
    // Check if the player's right side runs into a wall (only when moving right)
    if (p->vx > 0)
    {
        bool topRightSolid = isSolid(tileAt(map, rightEdge, midY));
        bool botRightSolid = isSolid(tileAt(map, rightEdge, bottomEdge - 4));

        if (topRightSolid || botRightSolid)
        {
            // Snap the player's X so they are flush against the wall's left edge
            p->x  = (int)(rightEdge / TILE_SIZE) * TILE_SIZE - PLAYER_W;
            p->vx = 0;
        }
    }
}


// ── playerUpdate ─────────────────────────────────────────────
// The main physics update called every game frame (60 times/sec).
// 1. Apply movement from key presses
// 2. Apply gravity
// 3. Move the player
// 4. Resolve collisions with tiles and platforms
void playerUpdate(Player* p, int map[MAP_ROWS][MAP_COLS])
{
    // If dead, do nothing
    if (p->dead) return;

    // Step 1: Set horizontal speed based on which keys are held
    if (p->moveLeft)
        p->vx = -WALK_SPD;     // move left
    else if (p->moveRight)
        p->vx = WALK_SPD;      // move right
    else
        p->vx = 0;             // no key pressed = stop

    // Step 2: Handle jumping
    // Only jump if jump key is pressed AND player is on the ground
    if (p->jumpWanted && p->onGround)
    {
        p->vy         = JUMP_VEL;  // launch upward
        p->jumpWanted = false;     // consume the jump request
    }

    // Step 3: Apply gravity (add to downward speed each frame)
    p->vy += GRAVITY;

    // Clamp fall speed so player doesn't fall infinitely fast
    if (p->vy > MAX_FALL)
        p->vy = MAX_FALL;

    // Step 4: Move the player by their current velocity
    p->x += p->vx;
    p->y += p->vy;

    // Step 5: Reset onGround — will be set again if collision is found
    p->onGround = false;

    // Step 6: Check and fix collisions with solid tiles
    resolveCollisions(p, map);
}
