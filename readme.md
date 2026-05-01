# Project Proposal: Fireboy & Watergirl (Forest Temple Edition)

## Problem Statement
Modern computer science students often learn Data Structures and Algorithms (DSA) in isolation through abstract console applications, making it difficult to visualize how these concepts apply to complex, real-time software. Furthermore, rebuilding classic browser-based games (like Fireboy and Watergirl) as standalone desktop applications presents challenges in physics simulation, state management, and performant rendering. This project solves both problems by rebuilding the classic game as a highly polished C++ Qt desktop application that deeply integrates core DSA concepts (Stacks, Queues, Linked Lists, Graphs, Sorting, and Searching) directly into its core engine and gameplay mechanics.

## Features & DSA Integration

- **Event Processing Engine (Queue)**
  Game events such as gem collection, player deaths, and level completion are pushed into a custom FIFO `GameQueue`. This ensures that simultaneous actions are processed safely and sequentially during the game's update tick.

- **Checkpoint & Replay System (Stack)**
  A custom `GameStack` records the players' movement histories and checkpoint states. This LIFO structure allows the game to safely pop the most recent safe position when a player falls into a hazard, providing instant respawns.

- **Dynamic Level Management (Linked List)**
  Levels are stored sequentially using a custom Doubly `LinkedList`. This allows the `LevelManager` to easily stream the next or previous levels in memory without needing fixed-size arrays, facilitating infinite level expansion.

- **Leaderboard Ranking System (Sorting & Searching)**
  The game features a competitive leaderboard that utilizes `BubbleSort` for small datasets and `QuickSort` for performance on larger datasets. Finding a specific player's rank relies on an optimized `BinarySearch` tree logic.

- **In-Game Intelligent Hint System (BFS Graph Traversal)**
  Pressing the 'H' key activates a smart hint path. The game treats the tile map as an unweighted graph and runs a Breadth-First Search (BFS) algorithm to calculate and draw the shortest safe path for both Fireboy and Watergirl to their respective exit doors.

