# Project Progress — Fireboy & Watergirl (Forest Temple Edition)

## File Overview

| File | Purpose |
|---|---|
| `FireboyWatergirl.pro` | qmake project file; lists all sources, headers, and resources. Build artifacts go to `build/` and `bin/`. |
| `resources.qrc` | Qt resource manifest; bundles all PNGs and the QSS stylesheet into the executable. |
| `run_linux.sh` | One-click build & run script for Linux. Calls `qmake` → `make` → launches `bin/FireboyWatergirl`. |
| `run_windows.bat` | One-click build & run for Windows. Auto-detects Qt and compiler (mingw32-make/jom/nmake), friendly output. |
| `image_prompts.txt` | AI image generation prompts for every sprite (fireboy, watergirl, gems, doors, backgrounds, tiles). |
| `docs/proposal.md` | Project proposal: problem statement + feature descriptions with DSA explanations. |
| `docs/readme.md` | General project readme and background. |

### `include/` — Headers

| File | Purpose |
|---|---|
| `GameObjects.h` | All core structs: `Gem`, `Door`, `HazardPool`, `MovingPlatform`, `Button`, `Gate`, `GameEvent`, `ScoreEntry`, `LevelData`. No STL containers — fixed arrays only. |
| `DSA.h` | Declarations for all 10 DSA concepts: Stack, Queue, Linked List, BubbleSort, QuickSort, LinearSearch, BinarySearch, BFS, PriorityQueue (min-heap), GateHashMap (direct-address table). |
| `Player.h` | `Player` struct (position, velocity, flags, two DSA Stacks for history + checkpoints). Move and reset function declarations. |
| `GameEngine.h` | Qt `QObject` subclass. Owns the level list (LinkedList), priority event queue, gate hash map, and all game logic tick functions. |
| `GameRenderer.h` | Qt `QWidget` subclass. Loads all PNG sprites once; exposes `draw*` methods for every game element including buttons and gates. |
| `GameWindow.h` | Qt `QMainWindow`. Manages the stacked page UI (Menu, Game, Leaderboard), key routing, and leaderboard file I/O with sorting. |

### `src/` — Implementations

| File | Purpose |
|---|---|
| `main.cpp` | App entry point. Loads `game.qss` and shows `GameWindow`. |
| `DSA.cpp` | Full implementations of all 10 DSA structures. Every algorithm is written from scratch — no STL beyond `<cstring>`. |
| `Player.cpp` | Platformer physics: gravity, AABB tile collision, moving platform riding. Uses Stack to push position history every 4 frames. |
| `GameEngine.cpp` | Main game loop (16 ms timer). Runs: checkButtons → buildEffectiveTileMap → physics → checkHazards → checkGems → checkDoors → processEvents. Uses PriorityQueue for events and GateHashMap for O(1) button→gate lookup. |
| `GameRenderer.cpp` | Draws the game each frame using `QPainter`. PNGs used for all sprites; clean QPainter fallbacks if a PNG is missing. Draws gates with slide animation and buttons that glow when pressed. |
| `GameWindow.cpp` | Handles menu navigation, key events, leaderboard saving/loading (sorted with BubbleSort/QuickSort, searched with BinarySearch). |

### `levels/` — Level Data

| File | Purpose |
|---|---|
| `Levels.h` | Declares `makeLevel1()`, `makeLevel2()`, `makeLevel3()`. Edit only this folder to add or modify levels. |
| `Levels.cpp` | Full tile maps, gem positions, hazard bounds, button/gate definitions for all 3 levels. Each level has a clear cooperative puzzle description in its header comment. |

### `assets/` — Media

| Path | Purpose |
|---|---|
| `assets/images/*.png` | Placeholder sprites generated with ImageMagick. Replace with AI-generated PNGs using `image_prompts.txt`. |
| `assets/styles/game.qss` | Minimal dark-theme stylesheet applied only to UI widgets (menu buttons, leaderboard table). The game canvas is rendered entirely via QPainter + PNGs. |

---

## Update Log

### v1.0 — Initial Build
- Full project created: C++ Qt5 desktop game with 3 levels.
- DSA: Stack (player history), Queue (events), LinkedList (levels), BubbleSort, QuickSort, LinearSearch, BinarySearch, BFS (hint system).

### v1.1 — Refactor & Reorganization
- All files moved into `src/`, `include/`, `levels/`, `assets/`, `docs/` folders.
- `FireboyWatergirl.pro` updated with `OBJECTS_DIR`, `MOC_DIR`, `DESTDIR` to keep root clean.
- `run_linux.sh` and `run_windows.bat` build scripts created.
- `image_prompts.txt` created with AI generation prompts for all sprites.

### v1.2 — Code Simplification
- Removed all `std::vector` and template usage from DSA.
- All DSA structures now use plain C arrays and plain functions (beginner-friendly).
- `using namespace std;` added project-wide.
- `GameObjects.h` uses `#define` constants instead of enums.

### v1.3 — PNG Sprite System
- `GameRenderer` now loads PNG sprites via Qt resources.
- Graceful QPainter fallbacks if PNG not found.
- Placeholder PNGs auto-generated with ImageMagick at build time.

### v1.4 — Cooperative Level Redesign + New DSA
- **Levels completely redesigned** with button/gate cooperative mechanics.
  - Level 1 "The Crossing": Both players start on opposite sides; must press buttons to let each other cross.
  - Level 2 "The Crystal Bridge": Moving platform carries both over a poison pool; timed gate coordination.
  - Level 3 "The Ancient Trap": Chain-reaction puzzle — button 0 (Fireboy) opens path to a secret room where Watergirl finds button 1 which opens Fireboy's escape route.
- **New DSA — Priority Queue (Min-Heap)**: `pqPush/pqPop` used for game events. Death events (priority 0) are always processed before gem collects (priority 2).
- **New DSA — Gate Hash Map (Direct-Address Table)**: `gateMapInsert/gateMapGet` provides O(1) lookup of gate index from button's `gateId`. Rebuilt every level load.
- `GameObjects.h`: Added `Button` and `Gate` structs to `LevelData`.
- `GameEngine`: Added `checkButtons()`, `buildEffectiveTileMap()`, `rebuildGateMap()`.
- `GameRenderer`: Added animated gate slide and glowing button pressure plate rendering.
- `run_windows.bat`: Friendlier ASCII-art output, fallback launch of last good build.
