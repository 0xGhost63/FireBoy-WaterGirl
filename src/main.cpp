#include <QApplication>
#include <QFile>
#include "../include/GameWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("FireboyWatergirl");

    // Load minimal QSS (only for menu/UI widgets)
    QFile qss(":/styles/game.qss");
    if (qss.open(QFile::ReadOnly | QFile::Text))
        app.setStyleSheet(QLatin1String(qss.readAll()));

    GameWindow win;
    win.show();
    return app.exec();
}
