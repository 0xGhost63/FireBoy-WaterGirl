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
    
    pmBtnBlue    = QPixmap("assets/images/blue_button.png");
    pmBtnOrange  = QPixmap("assets/images/orange_button.png");
    pmGateBlue   = QPixmap("assets/images/blue_gate.png");
    pmGateOrange = QPixmap("assets/images/orange_gate.png");
    pmConveyor   = QPixmap("assets/images/conveyor_small.png");
    pmLava       = QPixmap("assets/images/lava.png").scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    pmWater      = QPixmap("assets/images/water.png").scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    pmPoison     = QPixmap("assets/images/poison.png").scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    pmUndo       = QPixmap("assets/images/undo.png");
    pmRedo       = QPixmap("assets/images/redo.png");
    pmArrow      = QPixmap("assets/images/arrow.png");
    pmTeleport   = QPixmap("assets/images/teleport.png");
    pmFireArrow  = QPixmap("assets/images/fire_arrow.png");
    pmWaterArrow = QPixmap("assets/images/water_arrow.png");

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
        drawConveyors(p);
        drawPlatforms(p);
        
        // Draw Teleport Pads
        for (int i = 0; i < lv->teleportCount; i++) {
            TeleportPad& pad = lv->pads[i];
            int px = toSX(pad.x), py = toSY(pad.y);
            int pw = (int)(TILE_SIZE * sx), ph = (int)(TILE_SIZE * sy);
            if (!pmTeleport.isNull()) {
                p.drawPixmap(QRect(px, py, pw, ph), pmTeleport);
            } else {
                p.setBrush(QColor(150, 0, 255, 150));
                p.setPen(Qt::white);
                p.drawEllipse(px, py, pw, ph);
            }
        }
        
        drawButtons(p);
        drawGems(p);
        drawDoors(p);
        if (eng->showHint) drawHints(p);

        // ── Min-Heap + Dijkstra Arrow: nearest REACHABLE gem ────────
        if (eng->showHint) {
            LevelData* lvA = eng->currentLevel();
            if (lvA) {
                // Dijkstra path length is the heap key – walls and platforms
                // are accounted for, so the arrow always points to the
                // nearest gem the player can actually REACH.
                int fbIdx = gemMinHeapFind(lvA->gems, lvA->gemCount,
                                eng->fireboy.x,   eng->fireboy.y,
                                FIREBOY,   eng->effectiveTileMap);
                int wgIdx = gemMinHeapFind(lvA->gems, lvA->gemCount,
                                eng->watergirl.x, eng->watergirl.y,
                                WATERGIRL, eng->effectiveTileMap);

                auto drawArrow = [&](int gemIdx, QPixmap& pm) {
                    if (gemIdx < 0 || pm.isNull()) return;
                    Gem& g = lvA->gems[gemIdx];
                    if (g.collected) return;
                    int aw = (int)(44 * sx), ah = (int)(44 * sy);
                    int ax = (int)toSX(g.x + 16.f) - aw / 2;
                    int ay = (int)toSY(g.y)         - ah - (int)(6 * sy);
                    p.drawPixmap(QRect(ax, ay, aw, ah), pm);
                };
                drawArrow(fbIdx, pmFireArrow);
                drawArrow(wgIdx, pmWaterArrow);
            }
        }

        drawPlayer(p, &eng->fireboy);
        drawPlayer(p, &eng->watergirl);
        drawHUD(p);

        // ── Undo / Redo flash banner (PNG) ──────────────────────────
        if (eng->undoRedoFlash > 0) {
            QPixmap& pm = eng->lastUndoWasUndo ? pmUndo : pmRedo;
            float alpha = qMin(1.0f, eng->undoRedoFlash); // 0.0 – 1.0

            // Size: 40% of game width, keep image aspect ratio
            int bw, bh;
            if (!pm.isNull()) {
                bw = (int)(width() * 0.40f);
                bh = pm.isNull() ? (int)(56 * sy)
                                 : (int)(bw * ((float)pm.height() / pm.width()));
            } else {
                bw = (int)(width() * 0.38f); bh = (int)(56 * sy);
            }
            int bx = (width()  - bw) / 2;
            int by = (int)(height() * 0.10f);
            QRect bannerRect(bx, by, bw, bh);

            p.setOpacity(alpha);
            if (!pm.isNull()) {
                p.drawPixmap(bannerRect, pm);
            } else {
                // Fallback text if PNG fails to load
                p.fillRect(bannerRect, QColor(60, 0, 120));
                p.setPen(Qt::white);
                QFont f("Inter", 15 * sy, QFont::Bold);
                p.setFont(f);
                p.drawText(bannerRect, Qt::AlignCenter,
                           eng->lastUndoWasUndo ? "UNDO" : "REDO");
            }
            p.setOpacity(1.0f); // restore full opacity
        }
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

// ── Hazard pools (PNG or animated colour fill fallback) ───────
void GameRenderer::drawHazards(QPainter& p) {
    LevelData* lv = eng->currentLevel(); if (!lv) return;
    static float wave = 0; wave += 0.05f;
    for (int i = 0; i < lv->hazardCount; i++) {
        HazardPool& h = lv->hazards[i];
        QRectF sr(toSX(h.x), toSY(h.y), toSW(h.w), toSH(h.h));

        QPixmap* pm = nullptr;
        if (h.type == TILE_LAVA) pm = &pmLava;
        else if (h.type == TILE_WATER) pm = &pmWater;
        else if (h.type == TILE_POISON) pm = &pmPoison;

        if (pm && !pm->isNull()) {
            p.drawTiledPixmap(sr.toRect(), *pm);
        } else {
            // Fallback gradient and waves
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
        QPixmap* pm = (g.id % 2 == 0) ? &pmGateOrange : &pmGateBlue;
        if (!pm->isNull()) {
            float ratio = visH / g.h;
            QRect sourceRect(0, pm->height() * (1.0f - ratio), pm->width(), pm->height() * ratio);
            p.drawPixmap(sr, *pm, sourceRect);
        } else {
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
}

// ── Buttons (pressure plates on the floor) ───────────────────
void GameRenderer::drawButtons(QPainter& p) {
    LevelData* lv = eng->currentLevel(); if (!lv) return;
    for (int i = 0; i < lv->buttonCount; i++) {
        Button& btn = lv->buttons[i];
        QRectF sr(toSX(btn.x), toSY(btn.y), toSW(btn.w), toSH(btn.h));
        QPixmap* pm = (btn.gateId % 2 == 0) ? &pmBtnOrange : &pmBtnBlue;
        if (!pm->isNull()) {
            QRectF drawRect = sr;
            if (btn.pressed) {
                float pressOffset = sr.height() * 0.4f;
                drawRect.setTop(sr.top() + pressOffset);
            }
            p.drawPixmap(drawRect.toRect(), *pm);
        } else {
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

// ── Conveyor Belts (animated directional arrows) ─────────────
void GameRenderer::drawConveyors(QPainter& p) {
    LevelData* lv = eng->currentLevel(); if (!lv) return;
    static float conveyorAnim = 0; conveyorAnim += 0.08f;

    // Helper lambda to draw one belt region
    auto drawBelt = [&](QRectF sr, float speed) {
        if (!pmConveyor.isNull()) {
            p.setClipRect(sr);
            float dir = (speed > 0) ? 1.0f : -1.0f;
            float offset = fmodf(conveyorAnim * toSW(40) * dir, toSW(TILE_SIZE));
            
            // Loop the drawn pixmap across the conveyor width
            for (float xx = sr.left() - toSW(TILE_SIZE) + offset; xx < sr.right() + toSW(TILE_SIZE); xx += toSW(TILE_SIZE)) {
                QRectF tileRect(xx, sr.top(), toSW(TILE_SIZE), toSH(TILE_SIZE));
                p.drawPixmap(tileRect.toRect(), pmConveyor, pmConveyor.rect());
            }
            p.setClipping(false);
        } else {
            QLinearGradient bg(sr.topLeft(), sr.bottomLeft());
            bg.setColorAt(0, QColor(60, 60, 70));
            bg.setColorAt(0.5, QColor(80, 80, 90));
            bg.setColorAt(1, QColor(50, 50, 60));
            p.fillRect(sr, bg);
            p.setPen(QPen(QColor(200, 180, 50, 180), 2.0f * sx));
            float dir = (speed > 0) ? 1.0f : -1.0f;
            float spacing = toSW(20);
            float offset = fmodf(conveyorAnim * toSW(40) * dir, spacing);
            for (float xx = sr.left() - spacing + offset; xx < sr.right() + spacing; xx += spacing) {
                if (xx < sr.left() || xx > sr.right()) continue;
                float cy = sr.center().y();
                float arrowW = toSW(6) * dir;
                float arrowH = toSH(5);
                p.drawLine(QPointF(xx - arrowW, cy - arrowH), QPointF(xx, cy));
                p.drawLine(QPointF(xx - arrowW, cy + arrowH), QPointF(xx, cy));
            }
            p.setPen(QPen(QColor(100, 100, 110), 1.5f));
            p.setBrush(Qt::NoBrush);
            p.drawRect(sr);
        }
    };

    // Draw conveyors[] array (backward compat with old levels)
    for (int i = 0; i < lv->conveyorCount; i++) {
        ConveyorBelt& belt = lv->conveyors[i];
        drawBelt(QRectF(toSX(belt.x), toSY(belt.y), toSW(belt.w), toSH(belt.h)), belt.speed);
    }

    // Draw tilemap-based conveyor tiles (TILE_CONVEYOR_R / TILE_CONVEYOR_L)
    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            int t = bstGet(&lv->tileTree, r, c);
            if (t != TILE_CONVEYOR_R && t != TILE_CONVEYOR_L) continue;
            float spd = (t == TILE_CONVEYOR_R) ? 60.0f : -60.0f;
            drawBelt(QRectF(toSX(c*TILE_SIZE), toSY(r*TILE_SIZE), toSW(TILE_SIZE), toSH(TILE_SIZE)), spd);
        }
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
        QRectF sr(toSX(d.x), toSY(d.y), toSW(TILE_SIZE), toSH(d.h));
        QPixmap& pm = (d.owner == FIREBOY) ? pmDoorFire : pmDoorWater;
        if (!pm.isNull()) {
            p.drawPixmap(sr.toRect(), pm, pm.rect());
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
               "Fireboy: ← ↑ →    Watergirl: A W D    Hint: H    Undo: U    Redo: R    Pause: Esc");
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
        sub="Press Enter to Start\n\nFireboy: \u2190 \u2192 \u2191\nWatergirl: A D W\nHint: H    Undo: U    Redo: R"; break;
    case STATE_PAUSED:
        title="PAUSED"; col=QColor(180,220,255);
        sub="Press Esc or Enter to Resume"; break;
    case STATE_WIN: {
        if (eng->levels.current && eng->levels.current->next)
             { title="Level Clear!"; sub=QString("Score: %1\nEnter \u2192 Next Level   P/N \u2192 Prev/Next").arg(eng->score); }
        else { title="You Win! ";    sub=QString("Final Score: %1\nR \u2192 Restart").arg(eng->score); }
        col=QColor(80,255,120); break;
    }
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
    p.drawText(QRectF(area.left(), area.top()+area.height()*0.48, area.width(), area.height()*0.3),
               Qt::AlignHCenter|Qt::AlignTop, sub);

    // ── Gem trail: draw actual PNG icons on win screen (chronological) ──
    if (eng->state == STATE_WIN) {
        int totalGems = eng->gemTrail.count;
        if (totalGems > 0) {
            int iconW = (int)(24 * sx), iconH = (int)(24 * sy);
            int arrowW = (int)(14 * sx);
            int totalW = totalGems * iconW + (totalGems - 1) * arrowW;
            int startX = (int)(area.left() + (area.width() - totalW) / 2);
            int rowY   = (int)(area.top()  + area.height() * 0.72f);

            QFont arrowFont("Arial", qMax(7, (int)(9 * sy)));
            p.setFont(arrowFont);

            int drawX = startX;
            bool first = true;
            GemTrailNode* n = eng->gemTrail.head;
            while (n) {
                if (!first) {
                    p.setPen(QColor(200, 200, 200, 200));
                    p.drawText(QRect(drawX, rowY, arrowW, iconH),
                               Qt::AlignCenter, "\u2192");
                    drawX += arrowW;
                }
                // Pick the correct gem PNG based on who collected it
                QPixmap& gemPm = (n->playerType == FIREBOY) ? pmGemFire : pmGemWater;
                if (!gemPm.isNull())
                    p.drawPixmap(QRect(drawX, rowY, iconW, iconH), gemPm);
                else {
                    p.setPen(n->playerType == FIREBOY ? QColor(255,80,30) : QColor(30,140,255));
                    p.drawText(QRect(drawX, rowY, iconW, iconH), Qt::AlignCenter, "G");
                }
                drawX += iconW;
                first = false;
                n = n->next;
            }
            // Final → door symbol
            p.setPen(QColor(200, 200, 200, 180));
            p.drawText(QRect(drawX, rowY, arrowW + iconW, iconH),
                       Qt::AlignVCenter | Qt::AlignLeft, " \u2192 \U0001F6AA");
        }
    }
}
