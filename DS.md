# 📊 Data Structures in Fireboy & Watergirl

This document provides a concise overview of the custom data structures implemented in the game and their specific roles in the engine.

---

### 1. Stack (LIFO)
- **Functionality**: Stores a history of player coordinates $(x, y)$.
- **How it Works**: Uses a fixed-size array with a `top` pointer. New positions are "pushed" onto the stack and "popped" during respawns.
- **Used For**: Checkpoint systems and position rewinding.

### 2. Circular Queue (FIFO)
- **Functionality**: Manages a pipeline of game events (e.g., `GEM_COLLECT`, `LEVEL_WIN`).
- **How it Works**: Uses an array with `front` and `rear` pointers that wrap around using modulo arithmetic.
- **Used For**: Decoupling event triggers from the rendering loop.

### 3. Doubly Linked List
- **Functionality**: Connects all game levels into a linear sequence.
- **How it Works**: Each node contains `LevelData` and pointers to the `next` and `prev` nodes.
- **Used For**: Level selection and moving between stages (Back/Forward).

### 4. Priority Queue (Min-Heap)
- **Functionality**: Processes events based on importance rather than just time.
- **How it Works**: An array-based binary heap where the event with the lowest priority value (highest importance) is always at the root.
- **Used For**: Ensuring `DEATH` events are handled before `WIN` or `GEM` events.

### 5. Hash Map (Direct-Address Table)
- **Functionality**: Maps a unique Gate ID to its index in the game world.
- **How it Works**: An array where the index matches the `gateId`. Provides $O(1)$ lookup time.
- **Used For**: Instantaneous gate activation when a button is pressed.

### 6. Binary Search Tree (BST)
- **Functionality**: Stores level tiles as a sparse matrix.
- **How it Works**: A pointer-based tree where tiles are sorted by their flattened grid index $(row \times width + col)$.
- **Used For**: Efficiently storing and retrieving tiles in large, mostly empty levels.

### 7. Conveyor Queue
- **Functionality**: Specifically manages objects on moving belts.
- **How it Works**: A dedicated circular queue that dequeues items, updates their position based on belt speed, and enqueues them back.
- **Used For**: Moving platforms and item transportation mechanics.

### 8. Cheat Tracker (Sequence Matcher)
- **Functionality**: Monitors keyboard input for specific character strings.
- **How it Works**: A small state machine that tracks the current character index in a target "secret code."
- **Used For**: Unlocking cheats (e.g., "SKIP" to skip levels).

---

## ⚙️ Algorithms in Fireboy & Watergirl

In addition to data structures, the game utilizes several key algorithms to handle logic and performance.

### 1. Bubble Sort ($O(n^2)$)
- **Use Case**: Small Leaderboards.
- **Why**: Simple to implement and efficient enough for small lists (e.g., top 10 local scores) where the overhead of complex sorts isn't needed.

### 2. Quick Sort ($O(n \log n)$)
- **Use Case**: Large Leaderboards / Global Rankings.
- **Why**: A "Divide and Conquer" algorithm used when the score list grows large, ensuring the game doesn't lag while sorting thousands of entries.

### 3. Linear Search ($O(n)$)
- **Use Case**: Gem Proximity Detection.
- **Why**: Used to check if the player is close enough to any gem on the screen. Since levels have a limited number of gems, a simple loop is the most direct approach.

### 4. Binary Search ($O(\log n)$)
- **Use Case**: Rank Finding.
- **Why**: After sorting the leaderboard, the game uses Binary Search to instantly find where a player's score ranks among others.

### 5. BFS (Breadth-First Search)
- **Use Case**: Shortest-Path Hint System.
- **Why**: Explores the tile grid level-by-level to find the shortest possible safe path from the player to the exit, providing "hints" for stuck players.

---
*Generated for the Fireboy & Watergirl Project Documentation.*
