# `GameRenderer.cpp` Detailed Explanation

**File**: `src/GameRenderer.cpp`

This is the visual engine. It translates the abstract math inside `GameEngine` into visual `QPixmap` sprites on your screen.

### Function-Level Breakdown

#### `GameRenderer::GameRenderer(...)` (Constructor)
- Initializes raw UI offsets and scales.
- Uses `QPixmap(":/assets/images/...")` to load all image assets from the compiled `.qrc` resource file directly into memory. 
- Scales down giant sprite sheets or cuts them into exact frames (like splitting the walking animation strips into single frames).

#### `void GameRenderer::paintEvent(QPaintEvent*)`
- The core drawing function automatically called by Qt whenever the screen needs to redraw.
- **Scaling Math**: Calculates `sx` (scale X) and `sy` (scale Y) by dividing the current window width/height by the target `MAP_W`/`MAP_H`. This makes the game perfectly responsive if you stretch the window.
- **Painter Stack**:
  1. `p.drawPixmap(...)` draws the parallax forest background.
  2. `drawTiles(p)`: Loops over the 2D grid and draws wall/floor blocks.
  3. `drawHazards(p)` & `drawGates(p)`: Renders lava, water, poison, and the mechanical elements.
  4. `drawDoors(p)` & `drawGems(p)`: Renders the exit doors and diamonds.
  5. `drawPlayers(p)`: Draws Fireboy and Watergirl on top of everything.
  6. `drawOverlay(p)`: Draws the top HUD (Score, Level number) and bottom controls bar.

#### `void GameRenderer::drawPlayers(QPainter& p)`
- Uses logic to decide *which* sprite frame to draw. 
- If `vy != 0` (player is falling or jumping), it draws the jump sprite.
- If `vx != 0` (player is moving), it cycles through a walk animation array using a math modulo based on time/ticks.
- If the player is moving left (`vx < 0`), it uses `p.scale(-1, 1)` to flip the sprite image horizontally so the character faces the correct direction.

#### `void GameRenderer::drawTiles(QPainter& p)`
- A double `for` loop (rows and columns) that queries the `GameEngine`'s tile tree. 
- Determines what type of block it is (Solid, Empty) and maps it to the specific grass/stone tile sprite.

#### `void GameRenderer::drawOverlay(QPainter& p)`
- Called last. If `eng->state != STATE_PLAYING`, it draws a semi-transparent black rectangle `p.fillRect()` over the entire game to darken it.
- Uses a `switch(eng->state)` to decide whether to draw "PAUSED", "Game Over", or "You Win!" using high-resolution font rendering. It also draws the final score and instructions.
