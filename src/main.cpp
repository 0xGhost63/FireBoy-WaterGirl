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

    // Create a 3-second Splash Screen with an Image
    QPixmap splashPix("assets/images/younoob.png");
    if (splashPix.isNull()) {
        // Fallback if the user hasn't saved the image yet
        splashPix = QPixmap(800, 600);
        splashPix.fill(Qt::black);
        QPainter painter(&splashPix);
        painter.setPen(QColor(0, 255, 0)); // Cyberpunk Green
        painter.setFont(QFont("Courier", 80, QFont::Bold));
        painter.drawText(splashPix.rect(), Qt::AlignCenter, "YouNoob");
        painter.end();
    } else {
        // Scale it to a reasonable size if it's too large/small
        splashPix = splashPix.scaled(800, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    QSplashScreen* splash = new QSplashScreen(splashPix);
    splash->show();
    app.processEvents();

    GameWindow* win = new GameWindow();
    
    // Close the splash screen instantly (0 ms delay) and show the main game window
    QTimer::singleShot(0, [splash, win]() {
        splash->finish(win);
        win->show();
        splash->deleteLater();
    });

    return app.exec();
}
