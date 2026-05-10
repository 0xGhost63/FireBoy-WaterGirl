# `GameWindow.cpp` Detailed Explanation

**File**: `src/GameWindow.cpp`

This file handles the main UI application shell. It acts as the navigation router between menus and the actual game.

### Function-Level Breakdown

#### `GameWindow::GameWindow(QWidget *parent)` (Constructor)
- Calls `buildUI()` to construct the UI elements and menus.
- Calls `loadScores()` to parse `scores.txt` from disk to populate the leaderboard in memory.

#### `void GameWindow::buildUI()`
- Constructs a `QStackedWidget` which acts as a "deck of cards" to switch between screens.
- **Page 0 (Game)**: Creates a blank layout and embeds `GameRenderer`, which is where the game graphics are drawn.
- **Page 1 (Main Menu)**: Creates title labels, "Play Game", "Leaderboard", and "Quit" buttons.
- **Page 2 (Leaderboard)**: Creates a `QTableWidget` to display ranks, names, and scores.
- **Page 3 (Name Entry)**: Creates a text box (`QLineEdit`) for the user's name and a "Continue" button.
- **Connections**: Uses `connect(...)` to map UI button clicks to functions like `startGame()` or `showLeaderboard()`.

#### `void GameWindow::submitName()`
- Grabs the text from the `nameInput` line edit. If it's not empty, it stores it in `currentPlayerName`.
- Transitions the stack index to `1` (Main Menu).

#### `void GameWindow::startGame()`, `showMenu()`, `showLeaderboard()`
- These are transition helper functions.
- `startGame()`: Resets `eng->levels.current` to `levels.head`, switches to Page 0, forces keyboard focus to the game canvas, and calls `eng->start()`.
- `showLeaderboard()`: Calls `refreshLeaderboard()` to update the table data, then switches to Page 2.

#### `void GameWindow::onStateChanged(int s)`
- A slot that listens to the `GameEngine`.
- If the state changes to `STATE_WIN` or `STATE_GAMEOVER`, it calculates the current level and calls `saveScore()` to automatically record the run to the leaderboard.

#### `void GameWindow::saveScore(const QString& name, int score, int level)`
- Inserts a new `ScoreEntry` into the `scores` array.
- If the array is full (`MAX_SCORES`), it replaces the absolute lowest score if the new score is higher.
- Calls `quickSort()` to organize the array from highest to lowest.
- Opens `scores.txt` and rewrites it sequentially with the updated array data.

#### `void GameWindow::loadScores()`
- Opens `scores.txt`, reads line-by-line, splitting on commas to extract `name`, `score`, and `level`.
- Pushes them into the `scores` array and runs `quickSort()` to ensure they are sorted in memory on boot.

#### `void GameWindow::refreshLeaderboard()`
- Iterates through the sorted `scores` array.
- Uses `binarySearch()` to determine the exact rank (allowing for ties).
- Populates the visual `QTableWidget` rows and centers the text.

#### `void GameWindow::keyPressEvent(QKeyEvent* e)` & `keyReleaseEvent()`
- Intercepts physical keyboard inputs.
- Handles global shortcuts (like hitting 'Enter' to start, or 'Esc' to pause).
- If the game is actively playing, it routes the raw keys to `eng->keyPress()` and `eng->keyRelease()`.
