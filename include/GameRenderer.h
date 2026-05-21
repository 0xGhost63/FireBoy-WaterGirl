#pragma once
#include <QWidget>
#include <QPixmap>
#include "../include/GameEngine.h"

class GameRenderer : public QWidget {
    Q_OBJECT
public:
    explicit GameRenderer(GameEngine* eng, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;

private:
    GameEngine* eng;

    // PNG sprites loaded once at startup
    QPixmap pmFireboy, pmWatergirl;
    QPixmap pmGemFire, pmGemWater;
    QPixmap pmDoorFire, pmDoorWater;
    QPixmap pmTile;
    QPixmap pmBg[3];    // 0=forest 1=cave 2=ruins
    
    QPixmap pmBtnBlue, pmBtnOrange;
    QPixmap pmGateBlue, pmGateOrange;
    QPixmap pmConveyor;
    QPixmap pmLava, pmWater, pmPoison;
    QPixmap pmUndo, pmRedo;
    QPixmap pmTeleportFire;
    QPixmap pmTeleportWater;
    QPixmap pmFireArrow;    // nearest-gem arrow for Fireboy
    QPixmap pmWaterArrow;   // nearest-gem arrow for Watergirl

    // Screen scaling values (set by computeScale)
    float sx, sy;       // scale factor (world units -> pixels)
    int   ox, oy;       // pixel offset to center the game area

    float conveyorScrollOffset;  // scroll position driven by CONVEYOR_BELT_SPEED

    void computeScale();

    // Convert world coordinates to screen pixel coordinates
    float toSX(float wx);   // world X  -> screen X
    float toSY(float wy);   // world Y  -> screen Y
    float toSW(float ww);   // world width  -> screen width
    float toSH(float wh);   // world height -> screen height

    // ── Drawing functions (called in order by paintEvent) ──
    void drawBackground   (QPainter& p);
    void drawTiles        (QPainter& p);
    void drawGates        (QPainter& p);
    void drawHazards      (QPainter& p);
    void drawConveyors    (QPainter& p);
    void drawTeleportPads (QPainter& p);
    void drawButtons      (QPainter& p);
    void drawGems         (QPainter& p);
    void drawDoors        (QPainter& p);
    void drawPlayer       (QPainter& p, Player* player);
    void drawHints        (QPainter& p);
    void drawGemArrows    (QPainter& p);
    void drawHUD          (QPainter& p);
    void drawUndoRedoFlash(QPainter& p);
    void drawOverlay      (QPainter& p);

    // Helper functions (extracted from lambdas)
    void drawOneConveyorBelt(QPainter& p, QRectF area, float speed);
    void drawOneHintPath   (QPainter& p, PathResult& path, QColor color);
    void drawOneGemArrow   (QPainter& p, int gemIdx, QPixmap& arrowPm);
};

