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
    QPixmap pmArrow;        // (kept for compatibility)
    QPixmap pmFireArrow;    // nearest-gem arrow for Fireboy
    QPixmap pmWaterArrow;   // nearest-gem arrow for Watergirl

    // Viewport scale & offset (letterbox fit)
    float sx, sy;
    int   ox, oy;

    void computeScale();

    // World→screen helpers
    float toSX(float wx);
    float toSY(float wy);
    float toSW(float ww);
    float toSH(float wh);

    void drawBackground(QPainter& p);
    void drawTiles     (QPainter& p);
    void drawGates     (QPainter& p);
    void drawButtons   (QPainter& p);
    void drawHazards   (QPainter& p);
    void drawConveyors (QPainter& p);
    void drawPlatforms (QPainter& p);
    void drawGems      (QPainter& p);
    void drawDoors     (QPainter& p);
    void drawPlayer    (QPainter& p, Player* player);
    void drawHints     (QPainter& p);
    void drawHUD       (QPainter& p);
    void drawOverlay   (QPainter& p);
};
