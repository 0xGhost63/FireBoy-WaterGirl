# Header Files Explanation

This document explains the role of each `.h` (header) file in the project. In C++, header files act as the "table of contents" for the codebase, defining structures, variables, and function prototypes so that different files can communicate with each other.

### 1. `include/GameObjects.h`
**Purpose**: Defines the raw data structures used throughout the entire game.
- Contains the `PlayerState` struct (position, velocity, jumping state).
- Contains structures for all interactive objects like `Gem`, `Door`, `Button`, `Gate`, `TeleportPad`, and `Conveyor`.
- Contains `LevelData` which holds the map and all objects in a level.
- Contains the `ScoreEntry` struct for the leaderboard.

### 2. `include/DSA.h`
**Purpose**: Defines the custom Data Structures and Algorithms.
- `LevelList` & `LevelNode`: A Doubly-Linked List used to chain levels together.
- `BSTNode`: A Binary Search Tree used to store and quickly lookup the 2D tile map.
- `MinHeap`: Used to find the nearest gem for the hint system.
- `EventQueue`: A circular queue to handle game events (like deaths or level completes) in order.
- `quickSort` & `binarySearch`: Used for sorting and querying the leaderboard.

### 3. `include/GameEngine.h`
**Purpose**: The blueprint for the core game logic engine.
- Declares the `GameEngine` class which inherits from `QObject`.
- Holds all the variables for the current game state (score, lives, elapsed time, players).
- Declares the `tick()` function loop and input handlers (`keyPress`, `keyRelease`).
- Declares signals like `scoreChanged` and `stateChanged` to communicate with the UI.

### 4. `include/GameRenderer.h`
**Purpose**: The blueprint for the visual drawing component.
- Declares the `GameRenderer` class which inherits from `QWidget`.
- Holds references to the `GameEngine` so it can read coordinates.
- Declares the `paintEvent()` function, which is the Qt standard for drawing graphics on screen.
- Pre-loads and stores all the `QPixmap` image sprites (fireboy, watergirl, tiles).

### 5. `include/GameWindow.h`
**Purpose**: The blueprint for the main application window.
- Declares the `GameWindow` class (inherits from `QMainWindow`).
- Manages the `QStackedWidget` for UI navigation (Menu -> Name Entry -> Game -> Leaderboard).
- Declares the UI building function `buildUI()` and the leaderboard functions (`loadScores`, `saveScore`).

### 6. `levels/Levels.h`
**Purpose**: Exposes the map generation functions.
- Simply provides the function declarations (e.g., `makeLevel1()`, `makeLevel2()`) so the `GameEngine` can call them without needing to know exactly how the maps are built.
