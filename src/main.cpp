// ============================================================
// main.cpp — Entry point of the Fireboy & Watergirl game
// ============================================================
// This file starts the Qt application and opens the game window.
// Qt apps always need a QApplication object before anything else.

#include <QApplication>   // Manages the whole Qt app lifecycle
#include <QFile>          // Used to read the stylesheet file
#include <QSplashScreen>
#include <QPixmap>
#include <QElapsedTimer>
#include <QThread>
#include "../include/GameWindow.h"  // Our main game window class

int main(int argc, char* argv[])
{
    // Step 1: Create the Qt Application object
    QApplication app(argc, argv);
    app.setApplicationName("FireboyWatergirl");

    // Step 2: Load a .qss stylesheet (like CSS but for Qt widgets)
    QFile qss(":/styles/game.qss");
    if (qss.open(QFile::ReadOnly | QFile::Text))
    {
        app.setStyleSheet(QLatin1String(qss.readAll()));
    }

    int splash_timer = 3000; // Time in milliseconds (3 seconds)

    QPixmap splashPix("assets/images/splash.png");
    splashPix = splashPix.scaled(800, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QSplashScreen* splash = new QSplashScreen(splashPix);
    splash->show();
    app.processEvents();

    QElapsedTimer timer;
    timer.start();

    // Initialize the game window while splash is shown
    GameWindow* win = new GameWindow();

    // Wait for the remaining time if initialization was faster than splash_timer
    qint64 elapsed = timer.elapsed();
    if (elapsed < splash_timer) {
        QThread::msleep(splash_timer - elapsed);
    }

    splash->finish(win);
    win->show();
    splash->deleteLater();

    // Step 4: Start the Qt event loop
    return app.exec();
}
