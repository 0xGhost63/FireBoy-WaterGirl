#include <QApplication>
#include <QFile>
#include "../include/GameWindow.h"

#include <QSplashScreen>
#include <QPixmap>
#include <QPainter>
#include <QTimer>
#include <QFont>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("FireboyWatergirl");

    // Load minimal QSS (only for menu/UI widgets)
    QFile qss(":/styles/game.qss");
    if (qss.open(QFile::ReadOnly | QFile::Text))
        app.setStyleSheet(QLatin1String(qss.readAll()));

    int splashTimeMs = 3000;

    if (splashTimeMs > 0) 
    {

        
        splashPix = splashPix.scaled(800, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QSplashScreen* splash = new QSplashScreen(splashPix);
        splash->show();
        app.processEvents();

        GameWindow* win = new GameWindow();
        
        QTimer::singleShot(splashTimeMs, [splash, win]() {
            splash->finish(win);
            win->show();
            splash->deleteLater();
        });
    } else {
        GameWindow* win = new GameWindow();
        win->show();
    }

    return app.exec();
}
