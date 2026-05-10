# `main.cpp` Detailed Explanation

**File**: `src/main.cpp`

This is the entry point of the game application. It sets up the Qt application environment, displays the splash screen, and launches the main game window.

### Function-Level Breakdown

#### `int main(int argc, char *argv[])`
- **Application Initialization**: `QApplication app(argc, argv);` initializes the entire Qt GUI subsystem. This must be the first Qt object created.
- **Splash Screen Setup**: 
  - `QSplashScreen splash(QPixmap(":/images/splash.png"));` creates a splash window using a pre-loaded image from the resource file.
  - `splash.show();` instantly renders the splash screen before any heavy lifting begins.
- **Synchronous Delay Loop**:
  - `QElapsedTimer timer; timer.start();` begins tracking milliseconds.
  - The loop checks `timer.elapsed()` and forces the thread to sleep using `QThread::msleep()` until exactly `splash_timer` (3000ms) has passed. This guarantees the splash screen stays visible for exactly 3 seconds, regardless of how fast the computer loads the game window.
- **Game Window Initialization**:
  - `GameWindow w;` instantiates the massive main window (which loads all UI components and scores in the background).
  - `w.show();` displays the game window.
  - `splash.finish(&w);` cleanly destroys the splash screen and transfers operating system focus to `GameWindow`.
- **Event Loop Execution**:
  - `return app.exec();` hands over the program's control to Qt. The program will now sit idly waiting for user inputs (mouse clicks, keyboard presses, timers) until the application is fully closed.
