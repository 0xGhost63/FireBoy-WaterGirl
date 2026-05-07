# 🎮 DSA Ideas for Fireboy & Watergirl
> Simple, implementable ideas that use real data structures inside actual gameplay.

---

## ✅ Already Implemented
| # | DSA | Used For |
|---|-----|----------|
| 1 | Stack | Player checkpoint / respawn history |
| 2 | Circular Queue | Game event pipeline |
| 3 | Doubly Linked List | Level catalogue + **Undo/Redo system** |
| 4 | Bubble Sort | Leaderboard (small lists) |
| 5 | Quick Sort | Leaderboard (larger lists) |
| 6 | Linear Search | Gem proximity detection |
| 7 | Binary Search | Player rank lookup |
| 8 | BFS | Legacy tile-grid hint (kept for reference) |
| 9 | Dijkstra + Grid | Hint path through gems → door |
| 10 | Priority Queue (Min-Heap) | Event processing (death > win > gem) |
| 11 | Hash Map (Direct Address) | O(1) button → gate lookup |
| 12 | BST Map | Tile storage & lookup |
| 13 | Doubly Linked List | State History (Undo / Redo) |

---0

## 💡 New Ideas to Implement

---

### 14. 🗺️ Flood Fill (DFS / BFS on Grid)
**Idea:** When the player falls into lava/water, visually "flood" the hazard tiles outward from the contact point using BFS.  
**What it does:** Starting from the tile the player touched, expand outward tile-by-tile, briefly flashing each tile brighter — a ripple / explosion effect.  
**DSA:** BFS queue over the tile grid, up to radius 3 tiles.  
**Difficulty:** ⭐⭐

---

### 15. 🏆 Circular Buffer — Replay Last 3 Seconds
**Idea:** A fixed-size circular buffer stores the last ~3 seconds of `GameSnapshot`s (60 frames). On death, play back those frames visually before respawning — a "instant replay" ghost.  
**What it does:** Shows a semi-transparent ghost of both characters replaying their final moments.  
**DSA:** Circular array (ring buffer), same struct as the Undo history.  
**Difficulty:** ⭐⭐⭐

---

### 16. ⚖️ Min-Heap — Nearest Gem Finder
**Idea:** When hint mode is on, push all uncollected gems into a Min-Heap keyed by distance from each player. Pop the top to show which gem is closest and draw an arrow pointing to it.  
**What it does:** A small glowing arrow above each player points toward their nearest uncollected gem in real-time.  
**DSA:** Min-Heap / Priority Queue, same struct as the event queue.  
**Difficulty:** ⭐⭐

---

### 17. 📊 Frequency Counter (Array Hashing)
**Idea:** Track how many times each tile row/column has been visited by each player during a session using a simple `int visitCount[MAP_ROWS][MAP_COLS]` array. Draw a subtle heatmap overlay when hint mode is on.  
**What it does:** Tiles the player has visited glow slightly green, unexplored tiles stay dark — helps players see where they haven't been yet.  
**DSA:** Direct-address frequency table (array hashing).  
**Difficulty:** ⭐

---

### 18. 🔗 Singly Linked List — Gem Trail
**Idea:** Each gem the player collects gets added to a singly linked list as a "collection log". After winning, display the order gems were collected as a trophy trail.  
**What it does:** End screen shows "You collected: gem1 → gem2 → gem3 → door" with icons and arrows.  
**DSA:** Singly linked list, appended on each `EVT_GEM_COLLECT` event.  
**Difficulty:** ⭐

---

### 19. 🗃️ Stack — Tile Painter Undo (Level Editor)
**Idea:** In the level editor, every tile paint/erase action pushes the old tile type onto a `Stack`. Ctrl+Z pops and restores the previous tile.  
**What it does:** Full undo system inside the level editor, completely separate from the game's undo.  
**DSA:** Stack (already implemented — reuse the same struct).  
**Difficulty:** ⭐⭐

---

### 20. 🔄 Topological Sort — Puzzle Dependency Checker
**Idea:** When loading a level, build a small dependency graph:  
- Button B must be pressed → Gate G opens → Player can reach Door.  
Run a topological sort to verify the level is actually solvable (no circular dependencies or disconnected gates).  
**What it does:** Level editor shows a green ✅ or red ❌ "Solvable?" badge after generating code.  
**DSA:** Directed Acyclic Graph + Kahn's Algorithm (topological sort).  
**Difficulty:** ⭐⭐⭐

---

### 21. 📐 Sliding Window — Danger Zone Detection
**Idea:** As players move, scan a 5×5 sliding window of tiles around each player every tick. If the window contains more than 2 hazard tiles, increase the background music tempo (faster = more danger).  
**What it does:** Dynamic audio feedback based on local environment using a sliding window over the tile array.  
**DSA:** Sliding window sum over a 2D subarray.  
**Difficulty:** ⭐⭐

---

## 📝 Pick Order Suggestion
If you want quick wins: **17 → 16 → 18 → 19 → 14**  
If you want DSA depth: **20 → 15 → 21**
