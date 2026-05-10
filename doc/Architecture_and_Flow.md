# Game Architecture and Flow

## 1. High-Level Overview
This project is a 2D puzzle-platformer based on the classic "Fireboy & Watergirl". It is built entirely in C++ using the Qt5 framework. The architecture strictly separates the user interface and rendering from the core game logic and physics.

## 2. File Interlinking & Game Flow
The execution order and hierarchy of the files look like this:

1. **`main.cpp`** (Entry Point)
   - Starts the Qt Application (`QApplication`).
   - Displays a brief 3-second splash screen using `QElapsedTimer`.
   - Creates and shows the `GameWindow`.

2. **`GameWindow.cpp`** (UI & Application State)
   - Acts as the main application shell. It manages the `QStackedWidget` which holds the Menu, Leaderboard, Name Entry, and the Game Screen.
   - Handles the high-score text file reading and saving.
   - When "Play Game" is pressed, it initializes the `GameEngine` and focuses on the `GameRenderer`.

3. **`GameEngine.cpp`** (The Brain / Core Logic)
   - Handles everything gameplay related: physics, collision detection, object interactions (buttons, doors, teleporters), and score tracking.
   - Uses a `QTimer` to trigger a `tick()` every 16ms (~60 FPS).
   - Delegates player movement math to `Player.cpp`.
   - Uses Custom Data Structures from `DSA.cpp` to optimize pathfinding and object storage.
   - Loads the level maps from `Levels.cpp`.

4. **`GameRenderer.cpp`** (The Eyes / Graphics)
   - Doesn't perform any logic. It simply asks `GameEngine` where objects are located and draws them on the screen using `QPainter`.
   - Handles the drawing of the HUD, the pause screen, and the game over / win overlays.

5. **`Levels.cpp`**, **`Player.cpp`**, and **`DSA.cpp`** (The Utilities)
   - `Levels.cpp` constructs the hardcoded maps and places objects.
   - `Player.cpp` manages exactly how characters accelerate, jump, and collide with tiles.
   - `DSA.cpp` contains pure computer science algorithms (Linked Lists, Binary Search Trees, Min-Heaps) used by the engine to make the game run fast and cleanly.

## 3. Order of Functionality During Gameplay
1. **Input**: User presses a key. `GameWindow` catches it and forwards it to `GameEngine::keyPress()`.
2. **Tick Event**: Every 16ms, `GameEngine::tick()` fires.
3. **Logic Update**: `GameEngine` moves the players (`Player.cpp`), checks for button presses, updates gates, and handles collisions.
4. **Draw Event**: `GameEngine` emits a `frameReady()` signal.
5. **Render**: `GameWindow` catches the signal and tells `GameRenderer` to redraw the screen.
6. **Result**: The player sees the updated frame on their monitor.
