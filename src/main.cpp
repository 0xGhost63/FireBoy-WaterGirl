// ============================================================
// main.cpp — Entry point of the Fireboy & Watergirl game
// ============================================================
// This file starts the Qt application and opens the game window.
// Qt apps always need a QApplication object before anything else.

#include <QApplication>   // Manages the whole Qt app lifecycle
#include <QFile>          // Used to read the stylesheet file
#include "../include/GameWindow.h"  // Our main game window class

int main(int argc, char* argv[])
{
    // Step 1: Create the Qt Application object
    // Every Qt program needs this to handle events, fonts, etc.
    QApplication app(argc, argv);
    app.setApplicationName("FireboyWatergirl");

    // Step 2: Load a .qss stylesheet (like CSS but for Qt widgets)
    // This makes the menus and buttons look styled
    QFile qss(":/styles/game.qss");
    if (qss.open(QFile::ReadOnly | QFile::Text))
    {
        // Read the file and apply it to the whole app
        app.setStyleSheet(QLatin1String(qss.readAll()));
    }

    // Step 3: Create and show the game window
    GameWindow* win = new GameWindow();
    win->show();

    // Step 4: Start the Qt event loop
    // This keeps the program running and responds to keyboard/mouse events
    return app.exec();
}
