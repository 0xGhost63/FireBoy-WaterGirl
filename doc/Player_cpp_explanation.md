# `Player.cpp` Detailed Explanation

**File**: `src/Player.cpp`

Contains the granular physics and collision code for the player entities.

### Function-Level Breakdown

#### `void playerInit(PlayerState* p, int type, float startX, float startY)`
- Resets a player to standard conditions.
- Sets `p->type` to `FIREBOY` or `WATERGIRL`.
- Sets `p->x = startX` and `p->y = startY`.
- Resets velocity (`vx = 0`, `vy = 0`) and movement flags (`moveLeft`, `moveRight`).

#### `void playerUpdate(PlayerState* p, int map[MAP_ROWS][MAP_COLS])`
This function processes 1 frame of physics. It is mathematically rigid to prevent players from phasing through walls.

**1. Horizontal Movement (`vx`)**
- Checks input booleans. If `moveLeft`, it subtracts from `vx`. If `moveRight`, it adds to `vx`.
- Clamps velocity using `qBound(-MAX_SPEED, p->vx, MAX_SPEED)` so they don't run infinitely fast.
- Applies standard `FRICTION` if no keys are pressed, allowing a smooth slide to a halt.
- Updates position: `p->x += p->vx`.

**2. Horizontal Collision (`checkTileCollision`)**
- Calculates the player's bounding box using grid math `(int)(p->x / TILE_SIZE)`.
- If the player's new X overlaps a tile marked `TILE_SOLID` in the `map`:
  - It snaps the player's X coordinate to be exactly flush against the edge of the wall tile.
  - It sets `p->vx = 0` (momentum killed).

**3. Vertical Movement (`vy`)**
- Constantly applies downward force: `p->vy += GRAVITY`.
- Limits falling speed to `MAX_FALL_SPEED` (terminal velocity).
- Updates position: `p->y += p->vy`.

**4. Vertical Collision (`checkTileCollision`)**
- If the player's new Y overlaps a `TILE_SOLID`:
  - If they are moving down (`vy > 0`), it means they hit the floor. It snaps them exactly to the top of the tile, sets `vy = 0`, and sets `p->onGround = true`.
  - If they are moving up (`vy < 0`), it means they hit the ceiling. It snaps them below the tile and sets `vy = 0` (bonk their head).

**5. Jumping**
- If `jumpWanted` is true AND `onGround` is true, the engine applies massive negative velocity: `p->vy = JUMP_FORCE`.
- Sets `onGround = false` so they cannot jump repeatedly mid-air.
