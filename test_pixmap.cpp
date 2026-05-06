#include <QApplication>
#include <QPixmap>
#include <QDebug>
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QPixmap pm("assets/images/conveyor.png");
    qDebug() << "conveyor.png isNull:" << pm.isNull();
    QPixmap pmTile("assets/images/tile_solid.png");
    qDebug() << "tile_solid.png isNull:" << pmTile.isNull();
    return 0;
}
