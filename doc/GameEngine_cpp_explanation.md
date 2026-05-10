# `GameEngine.cpp` Detailed Explanation

**File**: `src/GameEngine.cpp`

This is the brain of the game. It controls physics updates, logic, collisions, and state tracking.

### Function-Level Breakdown

#### `GameEngine::GameEngine()` (Constructor)
- Loads all sound effects (`.wav` files) into `QSoundEffect` objects.
- Calls `listAppend()` repeatedly to push all generated levels (`makeLevel1`, etc.) into the Doubly-Linked list.
- Creates the main 16ms `QTimer` (`timer`) and connects its timeout to the `tick()` function.

#### `void GameEngine::start()`, `pause()`, `resume()`, `resetLevel()`, `nextLevel()`
- `start()`: Prepares the current level. Places Fireboy and Watergirl at their `StartX`/`StartY`. Resets gems, doors, and gates to their default states. Clears the undo history. Rebuilds the custom tile maps, and starts the timer.
- `pause() / resume()`: Simply stops or starts the 16ms timer and mutes looping walk audio.
- `resetLevel()`: Reloads the current level from scratch (used when the player dies).
- `nextLevel()`: Uses `listNext()` to move the linked list pointer forward. If it hits the end, it triggers `STATE_WIN` for beating the game.

#### `void GameEngine::tick()`
- The most important function. Fires ~60 times a second.
- **`checkButtons()`**: Determines if players are standing on buttons.
- **`buildEffectiveTileMap()`**: Overlays closed gates as solid tiles over the static map.
- **`playerUpdate()`**: Calls out to `Player.cpp` to move the characters.
- **`checkTeleports()` / `checkHazards()` / `checkDoors()` / `checkGems()`**: Compares character rectangles against map objects to trigger deaths, teleportations, or score increases.
- **Snapshot Logic**: Every 0.5 seconds, pushes the current `GameSnapshot` to the `history` Stack.
- **Event Queue Loop**: Pops all events from the custom `EventQueue` (like `EVT_PLAYER_DEAD`) and processes them to ensure things like death sounds and state changes happen in a thread-safe order.

#### `void GameEngine::checkHazards()`, `checkTeleports()`, `checkDoors()`, `checkGems()`
- These functions run mathematical bounding-box collision detection. 
- Example (`checkGems`): If the player intersects a gem box, and the gem matches their character type, the gem is marked `collected = true` and `score += 10`.

#### `void GameEngine::buildEffectiveTileMap()`
- The base map contains purely solid tiles or empty tiles. 
- This function loops over `gates`. If a gate is closed, it converts those exact tile coordinates into `TILE_SOLID` on a temporary map matrix (`effectiveTileMap`), ensuring the `playerUpdate` physics engine treats closed doors as brick walls.

#### `void GameEngine::undo()` and `redo()`
- `undo()`: Pops the top `GameSnapshot` off the `history` stack, applies the saved X/Y coordinates to both players, restores the `collected` states of the gems, and pushes the current state to the `redoStack`.
- `redo()`: Pops from the `redoStack` and pushes back to `history`.

#### `void GameEngine::keyPress(int key)` & `keyRelease(int key)`
- Receives the keyboard input and sets boolean flags on the characters (e.g., `fireboy.moveLeft = true`, `watergirl.jumpWanted = true`).
