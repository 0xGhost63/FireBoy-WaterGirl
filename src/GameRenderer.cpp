#include "../include/GameRenderer.h"
#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <cmath>
using namespace std;

// ── GameRenderer Constructor ──────────────────────────────────
// This class handles all the drawing on the screen.
// We load all images (PNGs) once when the game starts to save memory.
GameRenderer::GameRenderer(GameEngine* e, QWidget* parent)
    : QWidget(parent), eng(e) 
{
    setMinimumSize(800, 640);
    setFocusPolicy(Qt::StrongFocus);

    // Load character sprites
    pmFireboy   = QPixmap("assets/images/fireboy.png");
    pmWatergirl = QPixmap("assets/images/watergirl.png");
    
    // Load item sprites
    pmGemFire   = QPixmap("assets/images/gem_fire.png");
    pmGemWater  = QPixmap("assets/images/gem_water.png");
    pmDoorFire  = QPixmap("assets/images/door_fire.png");
    pmDoorWater = QPixmap("assets/images/door_water.png");
    
    // Load environment tiles
    pmTile      = QPixmap("assets/images/tile_solid.png");
    pmBg[0]     = QPixmap("assets/images/bg_forest.png");
    pmBg[1]     = QPixmap("assets/images/bg_cave.png");
    pmBg[2]     = QPixmap("assets/images/bg_ruins.png");
    
    // Load interactables
    pmBtnBlue    = QPixmap("assets/images/blue_button.png");
    pmBtnOrange  = QPixmap("assets/images/orange_button.png");
    pmGateBlue   = QPixmap("assets/images/blue_gate.png");
    pmGateOrange = QPixmap("assets/images/orange_gate.png");
    pmConveyor   = QPixmap("assets/images/conveyor_small.png");
    
    // Load hazards (scale them up slightly to fit nicely)
    pmLava   = QPixmap("assets/images/lava.png").scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    pmWater  = QPixmap("assets/images/water.png").scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    pmPoison = QPixmap("assets/images/poison.png").scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    
    // Load UI and hint icons
    pmUndo       = QPixmap("assets/images/undo.png");
    pmRedo       = QPixmap("assets/images/redo.png");
    pmArrow      = QPixmap("assets/images/arrow.png");
    pmTeleport   = QPixmap("assets/images/teleport.png");
    pmFireArrow  = QPixmap("assets/images/fire_arrow.png");
    pmWaterArrow = QPixmap("assets/images/water_arrow.png");

    computeScale();
}

void GameRenderer::resizeEvent(QResizeEvent*) { computeScale(); }

// ── Screen Scaling Math ───────────────────────────────────────
// This keeps the game centered and aspect-ratio correct even if
// the window is resized.
void GameRenderer::computeScale() 
{
    sx = (float)width()  / MAP_W;
    sy = (float)height() / MAP_H;
    float s = qMin(sx, sy); // Pick the smaller scale so it fits on screen
    sx = sy = s;            // Keep it square
    ox = (width()  - MAP_W * sx) / 2; // Offset X (to center it)
    oy = (height() - MAP_H * sy) / 2; // Offset Y (to center it)
}

// Helpers to convert world coordinates (wx) to screen pixels (SX)
float GameRenderer::toSX(float wx) { return ox + wx * sx; }
float GameRenderer::toSY(float wy) { return oy + wy * sy; }
float GameRenderer::toSW(float ww) { return ww * sx; }
float GameRenderer::toSH(float wh) { return wh * sy; }

// ── paintEvent (Main Drawing Loop) ───────────────────────────
// Called automatically by Qt every frame to draw everything.
void GameRenderer::paintEvent(QPaintEvent*) 
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing); // Make graphics smooth
    p.fillRect(rect(), QColor(10, 10, 20));  // Draw black borders

    // If we are on the main menu, only draw the overlay
    if (eng->state == STATE_MENU) { 
        drawOverlay(p); 
        return; 
    }

    LevelData* lv = eng->currentLevel();
    if (lv) 
    {
        // 1. Draw static background
        drawBackground(p);
        
        // 2. Draw level geometry
        drawTiles(p);
        drawGates(p);
        drawHazards(p);
        drawConveyors(p);
        
        // 3. Draw Teleport Pads
        for (int i = 0; i < lv->teleportCount; i++) {
            TeleportPad& pad = lv->pads[i];
            int px = toSX(pad.x);
            int py = toSY(pad.y);
            int pw = (int)(TILE_SIZE * sx);
            int ph = (int)(TILE_SIZE * sy);
            
            if (!pmTeleport.isNull()) {
                p.drawPixmap(QRect(px, py, pw, ph), pmTeleport);
            } else {
                p.setBrush(QColor(150, 0, 255, 150));
                p.setPen(Qt::white);
                p.drawEllipse(px, py, pw, ph);
            }
        }
        
        // 4. Draw interactables
        drawButtons(p);
        drawGems(p);
        drawDoors(p);
        
        // 5. Draw pathfinding hints if requested
        if (eng->showHint) 
            drawHints(p);

        // ── DSA: Min-Heap + Dijkstra Arrow ──────────────────
        // This points an arrow to the nearest gem the player can actually reach
        if (eng->showHint) {
            LevelData* lvA = eng->currentLevel();
            if (lvA) {
                int fbIdx = gemMinHeapFind(lvA->gems, lvA->gemCount,
                                eng->fireboy.x, eng->fireboy.y,
                                FIREBOY, eng->effectiveTileMap, eng->teleportEdges);
                                
                int wgIdx = gemMinHeapFind(lvA->gems, lvA->gemCount,
                                eng->watergirl.x, eng->watergirl.y,
                                WATERGIRL, eng->effectiveTileMap, eng->teleportEdges);

                auto drawArrow = [&](int gemIdx, QPixmap& pm) {
                    if (gemIdx < 0 || pm.isNull()) return;
                    Gem& g = lvA->gems[gemIdx];
                    if (g.collected) return;
                    
                    int aw = (int)(44 * sx);
                    int ah = (int)(44 * sy);
                    int ax = (int)toSX(g.x + 16.f) - aw / 2;
                    int ay = (int)toSY(g.y)        - ah - (int)(6 * sy);
                    p.drawPixmap(QRect(ax, ay, aw, ah), pm);
                };
                
                drawArrow(fbIdx, pmFireArrow);
                drawArrow(wgIdx, pmWaterArrow);
            }
        }

        // 6. Draw Characters
        drawPlayer(p, &eng->fireboy);
        drawPlayer(p, &eng->watergirl);
        
        // 7. Draw the user interface
        drawHUD(p);

        // 8. Draw Undo / Redo flash banner
        if (eng->undoRedoFlash > 0) {
            QPixmap& pm = eng->lastUndoWasUndo ? pmUndo : pmRedo;
            float alpha = qMin(1.0f, eng->undoRedoFlash); // fade out effect

            int bw, bh;
            if (!pm.isNull()) {
                bw = (int)(width() * 0.40f);
                bh = (int)(bw * ((float)pm.height() / pm.width()));
            } else {
                bw = (int)(width() * 0.38f); 
                bh = (int)(56 * sy);
            }
            int bx = (width()  - bw) / 2;
            int by = (int)(height() * 0.10f);
            QRect bannerRect(bx, by, bw, bh);

            p.setOpacity(alpha);
            if (!pm.isNull()) {
                p.drawPixmap(bannerRect, pm);
            } else {
                p.fillRect(bannerRect, QColor(60, 0, 120));
                p.setPen(Qt::white);
                p.setFont(QFont("Inter", 15 * sy, QFont::Bold));
                p.drawText(bannerRect, Qt::AlignCenter, eng->lastUndoWasUndo ? "UNDO" : "REDO");
            }
            p.setOpacity(1.0f); // restore full opacity
        }
    }
    
    // Draw pause screen / win screen if needed
    if (eng->state != STATE_PLAYING) 
        drawOverlay(p);
}

// ── Sub-drawing functions ─────────────────────────────────────

void GameRenderer::drawBackground(QPainter& p) 
{
    LevelData* lv = eng->currentLevel();
    if (!lv) return;
    
    QRectF area(ox, oy, MAP_W*sx, MAP_H*sy);
    int bg = lv->bgStyle;
    
    // Draw image if available, else draw a solid colour
    if (bg >= 0 && bg < 3 && !pmBg[bg].isNull()) {
        p.drawPixmap(area.toRect(), pmBg[bg]);
    } else {
        p.fillRect(area, QColor(20, 20, 30));
    }
}

void GameRenderer::drawTiles(QPainter& p) 
{
    LevelData* lv = eng->currentLevel();
    if (!lv) return;
    
    // Loop through the whole grid and draw solid blocks
    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            int t = bstGet(&lv->tileTree, r, c);
            if (t == TILE_EMPTY) continue;
            
            QRectF tileRect(toSX(c*TILE_SIZE), toSY(r*TILE_SIZE), toSW(TILE_SIZE), toSH(TILE_SIZE));
            if (t == TILE_SOLID) {
                if (!pmTile.isNull()) {
                    p.drawPixmap(tileRect.toRect(), pmTile);
                } else {
                    p.fillRect(tileRect, QColor(80,65,45));
                    p.setPen(QPen(QColor(110,90,60), 1));
                    p.drawRect(tileRect);
                }
            }
        }
    }
}

void GameRenderer::drawHazards(QPainter& p) 
{
    LevelData* lv = eng->currentLevel();
    if (!lv) return;
    
    // Animate the waves on the surface of the hazards
    static float wave = 0; 
    wave += 0.05f;
    
    for (int i = 0; i < lv->hazardCount; i++) {
        HazardPool& h = lv->hazards[i];
        QRectF area(toSX(h.x), toSY(h.y), toSW(h.w), toSH(h.h));

        QPixmap* pm = nullptr;
        if (h.type == TILE_LAVA)        pm = &pmLava;
        else if (h.type == TILE_WATER)  pm = &pmWater;
        else if (h.type == TILE_POISON) pm = &pmPoison;

        if (pm && !pm->isNull()) {
            p.drawTiledPixmap(area.toRect(), *pm);
        } else {
            // Simple flat colour fallback if image fails to load
            QColor col;
            if (h.type == TILE_LAVA) col = QColor(255, 80, 0);
            else if (h.type == TILE_WATER) col = QColor(0, 100, 255);
            else col = QColor(0, 200, 50);
            p.fillRect(area, col);
        }
    }
}

void GameRenderer::drawGates(QPainter& p) 
{
    LevelData* lv = eng->currentLevel();
    if (!lv) return;
    
    for (int i = 0; i < lv->gateCount; i++) {
        Gate& g = lv->gates[i];
        
        // The gate shrinks upwards as it opens
        float visibleHeight = g.h * (1.0f - g.openAnim); 
        if (visibleHeight < 2.0f) continue; // Fully open, don't draw
        
        QRectF area(toSX(g.x), toSY(g.y + g.h - visibleHeight), toSW(g.w), toSH(visibleHeight));
        QPixmap* pm = (g.id % 2 == 0) ? &pmGateOrange : &pmGateBlue;
        
        if (!pm->isNull()) {
            float ratio = visibleHeight / g.h;
            QRect sourceRect(0, pm->height() * (1.0f - ratio), pm->width(), pm->height() * ratio);
            p.drawPixmap(area, *pm, sourceRect);
        } else {
            QColor col = (g.id % 2 == 0) ? QColor(220, 120, 20) : QColor(40, 140, 220);
            p.fillRect(area, col);
        }
    }
}

void GameRenderer::drawButtons(QPainter& p) 
{
    LevelData* lv = eng->currentLevel();
    if (!lv) return;
    
    for (int i = 0; i < lv->buttonCount; i++) {
        Button& btn = lv->buttons[i];
        QRectF area(toSX(btn.x), toSY(btn.y), toSW(btn.w), toSH(btn.h));
        QPixmap* pm = (btn.gateId % 2 == 0) ? &pmBtnOrange : &pmBtnBlue;
        
        QRectF drawArea = area;
        if (btn.pressed) {
            // Push button down visually
            drawArea.setTop(area.top() + area.height() * 0.4f);
        }
        
        if (!pm->isNull()) {
            p.drawPixmap(drawArea.toRect(), *pm);
        } else {
            QColor col = (btn.gateId % 2 == 0) ? QColor(220, 120, 20) : QColor(40, 140, 220);
            p.fillRect(drawArea, col);
        }
    }
}


void GameRenderer::drawConveyors(QPainter& p) 
{
    LevelData* lv = eng->currentLevel();
    if (!lv) return;
    static float anim = 0; 
    anim += 0.08f;

    auto drawBelt = [&](QRectF area, float speed) {
        if (!pmConveyor.isNull()) {
            p.setClipRect(area);
            float dir = (speed > 0) ? 1.0f : -1.0f;
            float offset = fmodf(anim * toSW(40) * dir, toSW(TILE_SIZE));
            
            // Draw looping belt texture
            for (float x = area.left() - toSW(TILE_SIZE) + offset; x < area.right() + toSW(TILE_SIZE); x += toSW(TILE_SIZE)) {
                QRectF tileRect(x, area.top(), toSW(TILE_SIZE), toSH(TILE_SIZE));
                p.drawPixmap(tileRect.toRect(), pmConveyor, pmConveyor.rect());
            }
            p.setClipping(false);
        } else {
            p.fillRect(area, QColor(80, 80, 90)); // Fallback grey belt
        }
    };

    // Draw manual conveyor regions
    for (int i = 0; i < lv->conveyorCount; i++) {
        ConveyorBelt& belt = lv->conveyors[i];
        drawBelt(QRectF(toSX(belt.x), toSY(belt.y), toSW(belt.w), toSH(belt.h)), belt.speed);
    }

    // Draw tile-based conveyors
    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            int t = bstGet(&lv->tileTree, r, c);
            if (t == TILE_CONVEYOR_R) drawBelt(QRectF(toSX(c*TILE_SIZE), toSY(r*TILE_SIZE), toSW(TILE_SIZE), toSH(TILE_SIZE)), 60.0f);
            if (t == TILE_CONVEYOR_L) drawBelt(QRectF(toSX(c*TILE_SIZE), toSY(r*TILE_SIZE), toSW(TILE_SIZE), toSH(TILE_SIZE)), -60.0f);
        }
    }
}

void GameRenderer::drawGems(QPainter& p) 
{
    LevelData* lv = eng->currentLevel();
    if (!lv) return;
    
    for (int i = 0; i < lv->gemCount; i++) {
        Gem& g = lv->gems[i];
        if (g.collected) continue;
        
        // Make the gems hover up and down smoothly
        float bounce = sinf(g.animPhase) * 3.0f;
        QRectF area(toSX(g.x), toSY(g.y - bounce), toSW(20), toSH(20));
        QPixmap& pm = (g.owner == FIREBOY) ? pmGemFire : pmGemWater;
        
        if (!pm.isNull()) {
            p.drawPixmap(area.toRect(), pm);
        } else {
            QColor col = (g.owner == FIREBOY) ? QColor(255,80,30) : QColor(40,150,255);
            p.fillRect(area, col); // simple square fallback
        }
    }
}

void GameRenderer::drawDoors(QPainter& p) 
{
    LevelData* lv = eng->currentLevel();
    if (!lv) return;
    
    for (int i = 0; i < 2; i++) {
        Door& d = lv->doors[i];
        QRectF area(toSX(d.x), toSY(d.y), toSW(TILE_SIZE), toSH(d.h));
        QPixmap& pm = (d.owner == FIREBOY) ? pmDoorFire : pmDoorWater;
        
        if (!pm.isNull()) {
            p.drawPixmap(area.toRect(), pm, pm.rect());
            if (d.open) {
                // If open, draw a bright white glow over the door
                p.fillRect(area, QColor(255, 255, 255, 60));
            }
        } else {
            QColor col = (d.owner == FIREBOY) ? QColor(220,80,20) : QColor(30,120,220);
            p.setBrush(d.open ? col.lighter(180) : QColor(40,40,40));
            p.setPen(QPen(col, 3)); 
            p.drawRoundedRect(area, 4, 4);
        }
    }
}

void GameRenderer::drawPlayer(QPainter& p, Player* pl) 
{
    if (!pl || pl->dead) return;
    
    QRectF area(toSX(pl->x), toSY(pl->y), toSW(PLAYER_W), toSH(PLAYER_H));
    QPixmap& pm = (pl->type == FIREBOY) ? pmFireboy : pmWatergirl;
    
    if (!pm.isNull()) {
        p.drawPixmap(area.toRect(), pm);
    } else {
        // Simple oval fallback if the PNG didn't load
        QColor col = (pl->type == FIREBOY) ? QColor(255,80,0) : QColor(40,140,255);
        p.setBrush(col);
        p.setPen(Qt::NoPen); 
        p.drawEllipse(area);
    }
}

void GameRenderer::drawHints(QPainter& p) 
{
    auto drawLinePath = [&](PathResult& path, QColor col) {
        if (path.len < 2) return;
        p.setPen(QPen(col, 2*sx, Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        
        QPainterPath qp;
        qp.moveTo(toSX(path.px[0]*TILE_SIZE + TILE_SIZE/2),
                  toSY(path.py[0]*TILE_SIZE + TILE_SIZE/2));
                  
        for (int i = 1; i < path.len && i < MAX_HINT_PATH; i++) {
            qp.lineTo(toSX(path.px[i]*TILE_SIZE + TILE_SIZE/2),
                      toSY(path.py[i]*TILE_SIZE + TILE_SIZE/2));
        }
        p.drawPath(qp);
    };
    
    drawLinePath(eng->fireboyHint,   QColor(255, 120, 0, 180));
    drawLinePath(eng->watergirlHint, QColor(60, 180, 255, 180));
}

void GameRenderer::drawHUD(QPainter& p) 
{
    // Draw top bar
    QRectF bar(ox, oy, MAP_W*sx, 34*sy);
    p.fillRect(bar, QColor(0,0,0,130));
    
    QFont f("Arial", qMax(8,(int)(11*sy)), QFont::Bold);
    p.setFont(f);
    
    // Score
    p.setPen(QColor(255,210,50));
    p.drawText(bar.adjusted(8,2,0,0), Qt::AlignVCenter|Qt::AlignLeft, QString("Score: %1").arg(eng->score));
    
    // Lives (Hearts)
    QString hearts; 
    for (int i=0; i<eng->lives; i++) hearts += "♥ ";
    p.setPen(QColor(255,60,80));
    p.drawText(bar, Qt::AlignVCenter|Qt::AlignHCenter, hearts);
    
    // Level & Time
    p.setPen(QColor(160,210,255));
    LevelData* lv = eng->currentLevel();
    p.drawText(bar.adjusted(0,2,-8,0), Qt::AlignVCenter|Qt::AlignRight,
               QString("Lvl %1  |  %2s").arg(lv ? lv->num : 0).arg((int)eng->elapsed));
               
    // Controls bar at bottom
    QFont sf("Arial", qMax(6,(int)(8*sy)));
    p.setFont(sf); 
    p.setPen(QColor(160,160,160,170));
    QRectF ctrl(ox, oy+MAP_H*sy-16*sy, MAP_W*sx, 16*sy);
    p.fillRect(ctrl, QColor(0,0,0,100));
    p.drawText(ctrl, Qt::AlignCenter,
               "Fireboy: ← ↑ →    Watergirl: A W D    Hint: H    Undo: U    Redo: R    Pause: Esc");
}

void GameRenderer::drawOverlay(QPainter& p) 
{
    // Dim the screen behind the text
    p.fillRect(rect(), QColor(0,0,0,155));
    
    QFont big("Arial", qMax(14,(int)(26*sy)), QFont::Bold);
    QFont med("Arial", qMax(9,(int)(13*sy)));
    QString title, sub; 
    QColor col;
    
    switch (eng->state) {
    case STATE_MENU:
        title = "Fireboy &  Watergirl"; 
        col   = QColor(255,200,50);
        sub   = "Press Enter to Start\n\nFireboy: \u2190 \u2192 \u2191\nWatergirl: A D W\nHint: H    Undo: U    Redo: R"; 
        break;
    case STATE_PAUSED:
        title = "PAUSED"; 
        col   = QColor(180,220,255);
        sub   = "Press Esc or Enter to Resume"; 
        break;
    case STATE_WIN:
        if (eng->levels.current && eng->levels.current->next) { 
            title = "Level Clear!"; 
            sub   = QString("Score: %1\nEnter \u2192 Next Level   P/N \u2192 Prev/Next").arg(eng->score); 
        } else { 
            title = "You Win! ";    
            sub   = QString("Final Score: %1\nR \u2192 Restart").arg(eng->score); 
        }
        col = QColor(80,255,120); 
        break;
    case STATE_DEAD:
        title = "Oh No!"; 
        col   = QColor(255,80,80);
        sub   = QString("Lives Left: %1\nR → Retry").arg(eng->lives); 
        break;
    case STATE_GAMEOVER:
        title = "Game Over"; 
        col   = QColor(255,60,60);
        sub   = QString("Final Score: %1\nR → Restart").arg(eng->score); 
        break;
    default: 
        return;
    }
    
    QRectF area(ox, oy, MAP_W*sx, MAP_H*sy);
    p.setFont(big); 
    p.setPen(col);
    p.drawText(QRectF(area.left(), area.top()+area.height()*0.25, area.width(), area.height()*0.2), Qt::AlignCenter, title);
    
    p.setFont(med); 
    p.setPen(Qt::white);
    p.drawText(QRectF(area.left(), area.top()+area.height()*0.48, area.width(), area.height()*0.3), Qt::AlignHCenter|Qt::AlignTop, sub);

    // ── DSA: Singly Linked List - Draw the trail of collected gems in chronological order ──
    if (eng->state == STATE_WIN) {
        int totalGems = eng->gemTrail.count;
        if (totalGems > 0) {
            int iconW = (int)(24 * sx), iconH = (int)(24 * sy);
            int arrowW = (int)(14 * sx);
            int totalW = totalGems * iconW + (totalGems - 1) * arrowW;
            int startX = (int)(area.left() + (area.width() - totalW) / 2);
            int rowY   = (int)(area.top()  + area.height() * 0.72f);

            p.setFont(QFont("Arial", qMax(7, (int)(9 * sy))));

            int drawX = startX;
            bool first = true;
            
            GemTrailNode* n = eng->gemTrail.head;
            while (n) {
                if (!first) {
                    p.setPen(QColor(200, 200, 200, 200));
                    p.drawText(QRect(drawX, rowY, arrowW, iconH), Qt::AlignCenter, "\u2192");
                    drawX += arrowW;
                }
                
                QPixmap& gemPm = (n->playerType == FIREBOY) ? pmGemFire : pmGemWater;
                if (!gemPm.isNull()) {
                    p.drawPixmap(QRect(drawX, rowY, iconW, iconH), gemPm);
                } else {
                    p.setPen(n->playerType == FIREBOY ? QColor(255,80,30) : QColor(30,140,255));
                    p.drawText(QRect(drawX, rowY, iconW, iconH), Qt::AlignCenter, "G");
                }
                drawX += iconW;
                first = false;
                n = n->next; // Move to next node in the linked list
            }
            
            p.setPen(QColor(200, 200, 200, 180));
            p.drawText(QRect(drawX, rowY, arrowW + iconW, iconH), Qt::AlignVCenter | Qt::AlignLeft, " \u2192 \U0001F6AA");
        }
    }
}
