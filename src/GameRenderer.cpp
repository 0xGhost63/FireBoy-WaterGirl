#include "../include/GameRenderer.h"
#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <cmath>
using namespace std;

GameRenderer::GameRenderer(GameEngine* e, QWidget* parent)
    : QWidget(parent), eng(e) {
    setMinimumSize(800, 640);
    setFocusPolicy(Qt::StrongFocus);

    // Load PNG sprites directly from file system (prevents large binary bloat/OOM)
    pmFireboy   = QPixmap("assets/images/fireboy.png");
    pmWatergirl = QPixmap("assets/images/watergirl.png");
    pmGemFire   = QPixmap("assets/images/gem_fire.png");
    pmGemWater  = QPixmap("assets/images/gem_water.png");
    pmDoorFire  = QPixmap("assets/images/door_fire.png");
    pmDoorWater = QPixmap("assets/images/door_water.png");
    pmTile      = QPixmap("assets/images/tile_solid.png");
    pmBg[0]     = QPixmap("assets/images/bg_forest.png");
    pmBg[1]     = QPixmap("assets/images/bg_cave.png");
    pmBg[2]     = QPixmap("assets/images/bg_ruins.png");

    computeScale();
}

void GameRenderer::resizeEvent(QResizeEvent*) { computeScale(); }

void GameRenderer::computeScale() {
    sx = (float)width()  / MAP_W;
    sy = (float)height() / MAP_H;
    float s = qMin(sx, sy);
    sx = sy = s;
    ox = (width()  - MAP_W * sx) / 2;
    oy = (height() - MAP_H * sy) / 2;
}

float GameRenderer::toSX(float wx) { return ox + wx * sx; }
float GameRenderer::toSY(float wy) { return oy + wy * sy; }
float GameRenderer::toSW(float ww) { return ww * sx; }
float GameRenderer::toSH(float wh) { return wh * sy; }

// ── Paint ─────────────────────────────────────────────────────
void GameRenderer::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor(10, 10, 20));

    if (eng->state == STATE_MENU) { drawOverlay(p); return; }

    LevelData* lv = eng->currentLevel();
    if (lv) {
        drawBackground(p);
        drawTiles(p);
        drawGates(p);
        drawHazards(p);
        drawPlatforms(p);
        drawButtons(p);
        drawGems(p);
        drawDoors(p);
        if (eng->showHint) drawHints(p);
        drawPlayer(p, &eng->fireboy);
        drawPlayer(p, &eng->watergirl);
        drawHUD(p);
    }
    if (eng->state != STATE_PLAYING) drawOverlay(p);
}

// ── Background (PNG) ──────────────────────────────────────────
void GameRenderer::drawBackground(QPainter& p) {
    LevelData* lv = eng->currentLevel(); if (!lv) return;
    QRectF area(ox, oy, MAP_W*sx, MAP_H*sy);
    int bg = lv->bgStyle;
    if (bg >= 0 && bg < 3 && !pmBg[bg].isNull()) {
        p.drawPixmap(area.toRect(), pmBg[bg]);
    } else {
        // Fallback gradient
        QLinearGradient g(area.topLeft(), area.bottomLeft());
        const char* cols[][2] = {{"#0d2010","#050a05"},{"#0a0820","#050510"},{"#201008","#0a0503"}};
        g.setColorAt(0, QColor(cols[bg][0]));
        g.setColorAt(1, QColor(cols[bg][1]));
        p.fillRect(area, g);
    }
}

// ── Tiles (PNG tile repeated, or fallback solid rect) ─────────
void GameRenderer::drawTiles(QPainter& p) {
    LevelData* lv = eng->currentLevel(); if (!lv) return;
    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            int t = bstGet(&lv->tileTree, r, c);
            if (t == TILE_EMPTY) continue;
            QRectF sr(toSX(c*TILE_SIZE), toSY(r*TILE_SIZE), toSW(TILE_SIZE), toSH(TILE_SIZE));
            if (t == TILE_SOLID) {
                if (!pmTile.isNull()) {
                    p.drawPixmap(sr.toRect(), pmTile);
                } else {
                    p.fillRect(sr, QColor(80,65,45));
                    p.setPen(QPen(QColor(110,90,60), 1));
                    p.drawRect(sr);
                }
            }
        }
    }
}

// ── Hazard pools (animated colour fill) ──────────────────────
void GameRenderer::drawHazards(QPainter& p) {
    LevelData* lv = eng->currentLevel(); if (!lv) return;
    static float wave = 0; wave += 0.05f;
    for (int i = 0; i < lv->hazardCount; i++) {
        HazardPool& h = lv->hazards[i];
        QRectF sr(toSX(h.x), toSY(h.y), toSW(h.w), toSH(h.h));
        QColor top, bot;
        if      (h.type == TILE_LAVA)   { top=QColor(255,80,0,220);  bot=QColor(180,20,0,255); }
        else if (h.type == TILE_WATER)  { top=QColor(30,120,255,200);bot=QColor(0,60,180,255); }
        else                             { top=QColor(80,220,80,200); bot=QColor(30,140,30,255); }
        QLinearGradient g(sr.topLeft(), sr.bottomLeft());
        g.setColorAt(0, top); g.setColorAt(1, bot);
        p.fillRect(sr, g);
        // Wave lines
        p.setPen(QPen(top.lighter(140), 1.5f));
        for (int w = 0; w < 3; w++) {
            float yy = sr.top() + sr.height() * (0.2f + 0.3f*w);
            QPainterPath wp; wp.moveTo(sr.left(), yy);
            for (float x = sr.left(); x < sr.right(); x += 8)
                wp.lineTo(x, yy + sinf((x-sr.left())*0.1f + wave + w)*3*sy);
            p.drawPath(wp);
        }
    }
}

// ── Gates (sliding barriers, animate open/close) ─────────────
void GameRenderer::drawGates(QPainter& p) {
    LevelData* lv = eng->currentLevel(); if (!lv) return;
    for (int i = 0; i < lv->gateCount; i++) {
        Gate& g = lv->gates[i];
        // Gate slides upward as it opens (openAnim 0=closed, 1=open)
        float visH = g.h * (1.0f - g.openAnim); // visible height shrinks
        if (visH < 2.0f) continue; // fully open — nothing to draw
        QRectF sr(toSX(g.x), toSY(g.y + g.h - visH), toSW(g.w), toSH(visH));
        // Alternating orange/blue based on which button controls it
        QColor col = (g.id % 2 == 0) ? QColor(220, 120, 20) : QColor(40, 140, 220);
        QLinearGradient grad(sr.topLeft(), sr.bottomLeft());
        grad.setColorAt(0, col.lighter(130));
        grad.setColorAt(1, col.darker(130));
        p.fillRect(sr, grad);
        // Draw bars on the gate for a grate look
        p.setPen(QPen(col.lighter(170), 1.5f * sy));
        float barH = toSH(8);
        for (float yy = sr.top(); yy < sr.bottom(); yy += barH * 2)
            p.drawLine(QPointF(sr.left(), yy), QPointF(sr.right(), yy));
        p.setPen(QPen(col.darker(150), 2.0f));
        p.drawRect(sr);
    }
}

// ── Buttons (pressure plates on the floor) ───────────────────
void GameRenderer::drawButtons(QPainter& p) {
    LevelData* lv = eng->currentLevel(); if (!lv) return;
    for (int i = 0; i < lv->buttonCount; i++) {
        Button& btn = lv->buttons[i];
        QRectF sr(toSX(btn.x), toSY(btn.y), toSW(btn.w), toSH(btn.h));
        // Orange buttons open gate 1, blue buttons open gate 0
        QColor col = (btn.gateId % 2 == 0) ? QColor(220, 120, 20) : QColor(40, 140, 220);
        if (btn.pressed) col = col.lighter(160); // bright when pressed
        p.fillRect(sr, col);
        // Recessed look when pressed
        p.setPen(QPen(btn.pressed ? col.darker(130) : col.lighter(160), 1.5f));
        p.drawRect(sr);
        // Small arrow pointing up to hint the player
        p.setPen(QPen(Qt::white, 1.2f));
        float mx = sr.center().x(), my = sr.center().y();
        p.drawLine(QPointF(mx, my+2), QPointF(mx, my-4));
        p.drawLine(QPointF(mx-3, my-1), QPointF(mx, my-4));
        p.drawLine(QPointF(mx+3, my-1), QPointF(mx, my-4));
    }
}

// ── Moving platforms ──────────────────────────────────────────
void GameRenderer::drawPlatforms(QPainter& p) {
    LevelData* lv = eng->currentLevel(); if (!lv) return;
    for (int i = 0; i < lv->platformCount; i++) {
        MovingPlatform& mp = lv->platforms[i];
        if (!mp.active) continue;
        QRectF sr(toSX(mp.cx), toSY(mp.cy), toSW(80), toSH(16));
        QColor col(100, 200, 100);
        QLinearGradient g(sr.topLeft(), sr.bottomLeft());
        g.setColorAt(0, col.lighter(130)); g.setColorAt(1, col);
        p.fillRect(sr, g);
        p.setPen(QPen(col.lighter(160), 1.5f)); p.drawRect(sr);
    }
}

// ── Gems (PNG or diamond fallback) ───────────────────────────
void GameRenderer::drawGems(QPainter& p) {
    LevelData* lv = eng->currentLevel(); if (!lv) return;
    for (int i = 0; i < lv->gemCount; i++) {
        Gem& g = lv->gems[i];
        if (g.collected) continue;
        float bounce = sinf(g.animPhase) * 3.0f;
        QRectF sr(toSX(g.x), toSY(g.y - bounce), toSW(20), toSH(20));
        QPixmap& pm = (g.owner == FIREBOY) ? pmGemFire : pmGemWater;
        if (!pm.isNull()) {
            p.drawPixmap(sr.toRect(), pm);
        } else {
            // Fallback diamond
            QColor col = (g.owner == FIREBOY) ? QColor(255,80,30) : QColor(40,150,255);
            QPolygonF dia;
            dia << QPointF(sr.center().x(), sr.top())
                << QPointF(sr.right(), sr.center().y())
                << QPointF(sr.center().x(), sr.bottom())
                << QPointF(sr.left(), sr.center().y());
            p.setBrush(col); p.setPen(Qt::NoPen); p.drawPolygon(dia);
        }
    }
}

// ── Doors (PNG or rect fallback) ──────────────────────────────
void GameRenderer::drawDoors(QPainter& p) {
    LevelData* lv = eng->currentLevel(); if (!lv) return;
    for (int i = 0; i < 2; i++) {
        Door& d = lv->doors[i];
        QRectF sr(toSX(d.x), toSY(d.y), toSW(TILE_SIZE), toSH(TILE_SIZE*2));
        QPixmap& pm = (d.owner == FIREBOY) ? pmDoorFire : pmDoorWater;
        if (!pm.isNull()) {
            p.drawPixmap(sr.toRect(), pm);
            if (d.open) {
                p.fillRect(sr, QColor(255,255,255,60));
            }
        } else {
            QColor col = (d.owner == FIREBOY) ? QColor(220,80,20) : QColor(30,120,220);
            p.setBrush(d.open ? col.lighter(180) : QColor(40,40,40));
            p.setPen(QPen(col, 3)); p.drawRoundedRect(sr, 4, 4);
        }
    }
}

// ── Player (PNG or drawn fallback) ───────────────────────────
void GameRenderer::drawPlayer(QPainter& p, Player* pl) {
    if (!pl || pl->dead) return;
    float bounce = pl->onGround ? 0 : 0;
    QRectF sr(toSX(pl->x), toSY(pl->y + bounce), toSW(PLAYER_W), toSH(PLAYER_H));
    QPixmap& pm = (pl->type == FIREBOY) ? pmFireboy : pmWatergirl;
    if (!pm.isNull()) {
        p.drawPixmap(sr.toRect(), pm);
    } else {
        // Fallback: coloured oval + flame/wave
        QColor body = (pl->type == FIREBOY) ? QColor(255,80,0) : QColor(40,140,255);
        QRadialGradient g(sr.center(), sr.width()*0.5f);
        g.setColorAt(0, body.lighter(150)); g.setColorAt(1, body);
        p.setBrush(g); p.setPen(Qt::NoPen); p.drawEllipse(sr.adjusted(2,6,-2,-2));
        // Head
        QRectF head(sr.left()+sr.width()*0.2f, sr.top(), sr.width()*0.6f, sr.width()*0.6f);
        p.setBrush(body.lighter(130)); p.drawEllipse(head);
        // Eyes
        p.setBrush(Qt::white);
        p.drawEllipse(QRectF(head.left()+head.width()*0.18f, head.top()+head.height()*0.38f, head.width()*0.22f, head.height()*0.22f));
        p.drawEllipse(QRectF(head.left()+head.width()*0.58f, head.top()+head.height()*0.38f, head.width()*0.22f, head.height()*0.22f));
        p.setBrush(Qt::black);
        p.drawEllipse(QRectF(head.left()+head.width()*0.22f, head.top()+head.height()*0.42f, head.width()*0.1f, head.height()*0.1f));
        p.drawEllipse(QRectF(head.left()+head.width()*0.62f, head.top()+head.height()*0.42f, head.width()*0.1f, head.height()*0.1f));
    }
}

// ── BFS Hint paths ────────────────────────────────────────────
void GameRenderer::drawHints(QPainter& p) {
    auto draw = [&](PathResult& path, QColor col) {
        if (path.len < 2) return;
        p.setPen(QPen(col, 2*sx, Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        QPainterPath qp;
        qp.moveTo(toSX(path.px[0]*TILE_SIZE + TILE_SIZE/2),
                  toSY(path.py[0]*TILE_SIZE + TILE_SIZE/2));
        for (int i = 1; i < path.len && i < MAX_HINT_PATH; i++)
            qp.lineTo(toSX(path.px[i]*TILE_SIZE + TILE_SIZE/2),
                      toSY(path.py[i]*TILE_SIZE + TILE_SIZE/2));
        p.drawPath(qp);
    };
    draw(eng->fireboyHint,  QColor(255,120,0,180));
    draw(eng->watergirlHint,QColor(60,180,255,180));
}

// ── HUD ───────────────────────────────────────────────────────
void GameRenderer::drawHUD(QPainter& p) {
    QRectF bar(ox, oy, MAP_W*sx, 34*sy);
    p.fillRect(bar, QColor(0,0,0,130));
    QFont f("Arial", qMax(8,(int)(11*sy)), QFont::Bold);
    p.setFont(f);
    p.setPen(QColor(255,210,50));
    p.drawText(bar.adjusted(8,2,0,0), Qt::AlignVCenter|Qt::AlignLeft,
               QString("Score: %1").arg(eng->score));
    QString hearts; for (int i=0;i<eng->lives;i++) hearts += "♥ ";
    p.setPen(QColor(255,60,80));
    p.drawText(bar, Qt::AlignVCenter|Qt::AlignHCenter, hearts);
    p.setPen(QColor(160,210,255));
    LevelData* lv = eng->currentLevel();
    p.drawText(bar.adjusted(0,2,-8,0), Qt::AlignVCenter|Qt::AlignRight,
               QString("Lvl %1  |  %2s").arg(lv ? lv->num : 0).arg((int)eng->elapsed));
    // Controls bar at bottom
    QFont sf("Arial", qMax(6,(int)(8*sy)));
    p.setFont(sf); p.setPen(QColor(160,160,160,170));
    QRectF ctrl(ox, oy+MAP_H*sy-16*sy, MAP_W*sx, 16*sy);
    p.fillRect(ctrl, QColor(0,0,0,100));
    p.drawText(ctrl, Qt::AlignCenter,
               "Fireboy: ← ↑ →    Watergirl: A W D    Hint: H    Pause: Esc");
}

// ── Overlay (menu / pause / win / gameover) ───────────────────
void GameRenderer::drawOverlay(QPainter& p) {
    p.fillRect(rect(), QColor(0,0,0,155));
    QFont big("Arial", qMax(14,(int)(26*sy)), QFont::Bold);
    QFont med("Arial", qMax(9,(int)(13*sy)));
    QString title, sub; QColor col;
    switch (eng->state) {
    case STATE_MENU:
        title="Fireboy &  Watergirl"; col=QColor(255,200,50);
        sub="Press Enter to Start\n\nFireboy: ← → ↑\nWatergirl: A D W\nHint: H"; break;
    case STATE_PAUSED:
        title="PAUSED"; col=QColor(180,220,255);
        sub="Press Esc or Enter to Resume"; break;
    case STATE_WIN:
        if (eng->levels.current && eng->levels.current->next)
             { title="FIN ! "; sub=QString("Score: %1\nEnter → Next Level").arg(eng->score); }
        else { title="You Win! ";        sub=QString("Final Score: %1\nR → Restart").arg(eng->score); }
        col=QColor(80,255,120); break;
    case STATE_DEAD:
        title="Oh No!"; col=QColor(255,80,80);
        sub=QString("Lives Left: %1\nR → Retry").arg(eng->lives); break;
    case STATE_GAMEOVER:
        title="Game Over"; col=QColor(255,60,60);
        sub=QString("Final Score: %1\nR → Restart").arg(eng->score); break;
    default: return;
    }
    QRectF area(ox, oy, MAP_W*sx, MAP_H*sy);
    p.setFont(big); p.setPen(col);
    p.drawText(QRectF(area.left(), area.top()+area.height()*0.25, area.width(), area.height()*0.2),
               Qt::AlignCenter, title);
    p.setFont(med); p.setPen(Qt::white);
    p.drawText(QRectF(area.left(), area.top()+area.height()*0.48, area.width(), area.height()*0.4),
               Qt::AlignHCenter|Qt::AlignTop, sub);
}
