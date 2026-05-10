# `Levels.cpp` Detailed Explanation

**File**: `levels/Levels.cpp`

This file is a hardcoded level designer. It explicitly builds out the data structs for each individual level in the game.

### Function-Level Breakdown

#### `LevelData makeLevel1()`, `makeLevel2()`, etc.
Every level generation function follows a strict, sequential blueprint:

1. **Initialization**: 
   - Creates a blank `LevelData lv;`.
   - Sets metadata like `lv.num = 1;` and `strcpy(lv.name, "Level 1");`.

2. **Map Construction**:
   - Defines a `20x16` local `int map[MAP_ROWS][MAP_COLS]` array.
   - Uses `#define` macros (`S` for Solid, `E` for Empty, `W` for Water, `L` for Lava) to physically draw the level in ASCII format inside the code.
   - Loops through this 2D array and inserts every single tile into the custom Binary Search Tree using `bstInsert(&lv.tileTree, r, c, map[r][c]);`.

3. **Spawn Points**:
   - Assigns pixel-perfect starting coordinates: `lv.fireboyStartX = 2 * TILE_SIZE;`

4. **Doors**:
   - Configures the end-level doors. Sets Fireboy's red door at a specific coordinate and Watergirl's blue door next to it.

5. **Gems Array**:
   - Sets `lv.gemCount`.
   - Hardcodes every single diamond in the level. Example: `lv.gems[0] = {X, Y, FIREBOY, false, 0.0f};` places a red gem at X,Y that is currently uncollected.

6. **Hazards & Interactions**:
   - Sets counts and arrays for `gates`, `buttons`, `teleporters`, `hazards`, and `conveyors`.
   - **Linking Mechanics**: When defining a `Button`, you pass it an ID like `btn.gateId = 0;`. Then you define a `Gate` with `gate.id = 0;`. The `GameEngine` uses Hash Maps later to magically connect these together so the button opens that specific gate.

7. **Return**:
   - `return lv;` passes the fully constructed level package out to the `GameEngine` to be pushed into the Doubly-Linked List.

#### Shorthand Cleanup (`#undef S`, etc.)
At the very bottom of the file, all the `#define` macros are explicitly undefined using `#undef`. This ensures that letters like `S` or `E` don't bleed into other files and break standard C++ compilation elsewhere in the project.
