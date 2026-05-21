/*
How rendering works:
    QTimer fires  ->  Game state updates  ->  update()  ->  paintEvent()  ->  Frame on screen
*/
#include "../include/GameRenderer.h"
#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <cmath>

// ══════════════════════════════════════════════════════════════
// Constructor — loads all PNG sprites once at startup
// ══════════════════════════════════════════════════════════════
GameRenderer::GameRenderer(GameEngine* e, QWidget* parent)
    : QWidget(parent), eng(e), conveyorScrollOffset(0.0f)
{
    setMinimumSize(800, 640);
    setFocusPolicy(Qt::StrongFocus);

    // Characters
    pmFireboy   = QPixmap("assets/images/fireboy.png");
    pmWatergirl = QPixmap("assets/images/watergirl.png");
    
    // Gems & Doors
    pmGemFire   = QPixmap("assets/images/gem_fire.png");
    pmGemWater  = QPixmap("assets/images/gem_water.png");
    pmDoorFire  = QPixmap("assets/images/door_fire.png");
    pmDoorWater = QPixmap("assets/images/door_water.png");
    
    // Environment
    pmTile      = QPixmap("assets/images/tile_solid.png");
    pmBg[0]     = QPixmap("assets/images/bg_forest.png");
    pmBg[1]     = QPixmap("assets/images/bg_cave.png");
    pmBg[2]     = QPixmap("assets/images/bg_ruins.png");
    
    // Buttons, Gates, Conveyors
    pmBtnBlue    = QPixmap("assets/images/blue_button.png");
    pmBtnOrange  = QPixmap("assets/images/orange_button.png");
    pmGateBlue   = QPixmap("assets/images/blue_gate.png");
    pmGateOrange = QPixmap("assets/images/orange_gate.png");
    pmConveyor   = QPixmap("assets/images/conveyor_small.png");
    
    // Hazards (pre-scaled to 64x64 for tiling)
    pmLava   = QPixmap("assets/images/lava.png").scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    pmWater  = QPixmap("assets/images/water.png").scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    pmPoison = QPixmap("assets/images/poison.png").scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    
    // UI icons and hint arrows
    pmUndo       = QPixmap("assets/images/undo.png");
    pmRedo       = QPixmap("assets/images/redo.png");
    pmTeleportFire  = QPixmap("assets/images/portal_fire.png");
    pmTeleportWater = QPixmap("assets/images/portal_water.png");
    pmFireArrow  = QPixmap("assets/images/fire_arrow.png");
    pmWaterArrow = QPixmap("assets/images/water_arrow.png");

    computeScale();
}

// Recalculate scale whenever the window is resized
void GameRenderer::resizeEvent(QResizeEvent*) 
{ 
    computeScale();
}


// ══════════════════════════════════════════════════════════════
// Screen Scaling — keeps game centered with correct aspect ratio
// ══════════════════════════════════════════════════════════════

void GameRenderer::computeScale() 
{
    // Figure out how much to scale the game world to fit the window
    float scaleX = (float)width()  / MAP_W;
    float scaleY = (float)height() / MAP_H;
    float scale  = qMin(scaleX, scaleY);  // use the smaller one so it fits

    sx = scale;  // horizontal scale
    sy = scale;  // vertical scale (same as horizontal to keep square pixels)

    // Center the game area in the window
    ox = (width()  - MAP_W * sx) / 2;
    oy = (height() - MAP_H * sy) / 2;
}

// These convert world coordinates to screen pixel positions
float GameRenderer::toSX(float wx) { return ox + wx * sx; }  // world X -> screen X
float GameRenderer::toSY(float wy) { return oy + wy * sy; }  // world Y -> screen Y
float GameRenderer::toSW(float ww) { return ww * sx; }       // world width  -> screen width
float GameRenderer::toSH(float wh) { return wh * sy; }       // world height -> screen height


// ══════════════════════════════════════════════════════════════
// paintEvent — Main drawing function, called every frame
// ══════════════════════════════════════════════════════════════

void GameRenderer::paintEvent(QPaintEvent*) 
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor(10, 10, 20));  // black borders

    LevelData* lv = eng->currentLevel();
    if (lv) 
    {
        // Draw the level (back to front)
        drawBackground(p);
        drawTiles(p);
        drawGates(p);
        drawHazards(p);
        drawConveyors(p);
        drawTeleportPads(p);
        drawButtons(p);
        drawGems(p);
        drawDoors(p);

        // Draw hint paths and gem arrows if H key is held
        if (eng->showHint) {
            drawHints(p);
            drawGemArrows(p);
        }

        // Draw characters on top
        drawPlayer(p, &eng->fireboy);
        drawPlayer(p, &eng->watergirl);
        
        // Draw score bar and controls
        drawHUD(p);

        // Draw undo/redo flash if active
        drawUndoRedoFlash(p);
    }
    
    // Draw pause / win / death overlay if game is not playing
    if (eng->state != STATE_PLAYING) 
        drawOverlay(p);
}


// ══════════════════════════════════════════════════════════════
//  Drawing functions — each one draws a specific part of the game
// ══════════════════════════════════════════════════════════════

// ── Background ───────────────────────────────────────────────
void GameRenderer::drawBackground(QPainter& p) 
{
    /*
        0 = forest
        1 = cave
        2 = ruins
    */
    LevelData* lv = eng->currentLevel();
    if (!lv) return;
    
    int bg = lv->bgStyle;
    if (bg >= 0 && bg < 3) {
        QRect area(ox, oy, MAP_W * sx, MAP_H * sy);
        p.drawPixmap(area, pmBg[bg]);
    }
}

// ── Tiles (solid blocks) ─────────────────────────────────────
void GameRenderer::drawTiles(QPainter& p) 
{
    LevelData* lv = eng->currentLevel();
    if (!lv) return;
    
    for (int row = 0; row < MAP_ROWS; row++) {
        for (int col = 0; col < MAP_COLS; col++) {
            int tile = bstGet(&lv->tileTree, row, col);
            if (tile == TILE_EMPTY) continue;
            
            if (tile == TILE_SOLID) {
                int screenX = toSX(col * TILE_SIZE);
                int screenY = toSY(row * TILE_SIZE);
                int screenW = toSW(TILE_SIZE);
                int screenH = toSH(TILE_SIZE);
                p.drawPixmap(QRect(screenX, screenY, screenW, screenH), pmTile);
            }
        }
    }
}

// ── Hazards (lava, water, poison pools) ──────────────────────
void GameRenderer::drawHazards(QPainter& p) 
{
    LevelData* lv = eng->currentLevel();
    if (!lv) return;
    
    for (int i = 0; i < lv->hazardCount; i++) {
        HazardPool& h = lv->hazards[i];
        QRect area(toSX(h.x), toSY(h.y), toSW(h.w), toSH(h.h));

        // Pick the right texture based on hazard type
        if (h.type == TILE_LAVA)
            p.drawTiledPixmap(area, pmLava);
        else if (h.type == TILE_WATER)
            p.drawTiledPixmap(area, pmWater);
        else if (h.type == TILE_POISON)
            p.drawTiledPixmap(area, pmPoison);
    }
}

// ── Gates (open/close vertically) ────────────────────────────
void GameRenderer::drawGates(QPainter& p) 
{
    LevelData* lv = eng->currentLevel();
    if (!lv) return;
    
    for (int i = 0; i < lv->gateCount; i++) {
        Gate& g = lv->gates[i];
        
        // The gate shrinks upwards as it opens
        float visibleHeight = g.h * (1.0f - g.openAnim); 
        if (visibleHeight < 2.0f) continue;  // fully open, skip

        // Pick orange or blue gate sprite
        QPixmap& pm = (g.id % 2 == 0) ? pmGateOrange : pmGateBlue;

        // Draw only the visible (closed) portion of the gate
        QRectF destArea(toSX(g.x), toSY(g.y + g.h - visibleHeight), toSW(g.w), toSH(visibleHeight));
        float ratio = visibleHeight / g.h;
        QRect sourceArea(0, pm.height() * (1.0f - ratio), pm.width(), pm.height() * ratio);
        p.drawPixmap(destArea, pm, sourceArea);
    }
}

// ── Buttons (press plates that open gates) ───────────────────
void GameRenderer::drawButtons(QPainter& p) 
{
    LevelData* lv = eng->currentLevel();
    if (!lv) return;
    
    for (int i = 0; i < lv->buttonCount; i++) {
        Button& btn = lv->buttons[i];
        QRectF area(toSX(btn.x), toSY(btn.y), toSW(btn.w), toSH(btn.h));
        
        // Pick orange or blue button sprite
        QPixmap& pm = (btn.gateId % 2 == 0) ? pmBtnOrange : pmBtnBlue;
        
        // Push the button down visually when pressed
        QRectF drawArea = area;
        if (btn.pressed)
            drawArea.setTop(area.top() + area.height() * 0.4f);
            
        p.drawPixmap(drawArea.toRect(), pm);
    }
}

// ── Conveyor Belts ───────────────────────────────────────────

// Draws a single conveyor belt with scrolling animation
void GameRenderer::drawOneConveyorBelt(QPainter& p, QRectF area, float speed)
{
    p.setClipRect(area);

    float direction = (speed > 0) ? 1.0f : -1.0f;
    float tileW = toSW(TILE_SIZE);
    float tileH = toSH(TILE_SIZE);
    float speedRatio = fabsf(speed) / CONVEYOR_BELT_SPEED;
    float offset = fmodf(conveyorScrollOffset * toSW(speedRatio) * direction, tileW);

    // Tile the conveyor sprite across the belt area
    for (float x = area.left() - tileW + offset; x < area.right() + tileW; x += tileW) {
        p.drawPixmap(QRectF(x, area.top(), tileW, tileH).toRect(), pmConveyor);
    }

    p.setClipping(false);
}

void GameRenderer::drawConveyors(QPainter& p) 
{
    LevelData* lv = eng->currentLevel();
    if (!lv) return;

    conveyorScrollOffset += CONVEYOR_BELT_SPEED;

    // Draw manually placed conveyor regions
    for (int i = 0; i < lv->conveyorCount; i++) {
        ConveyorBelt& belt = lv->conveyors[i];
        QRectF area(toSX(belt.x), toSY(belt.y), toSW(belt.w), toSH(belt.h));
        drawOneConveyorBelt(p, area, belt.speed);
    }

    // Draw tile-based conveyors from the grid
    for (int row = 0; row < MAP_ROWS; row++) {
        for (int col = 0; col < MAP_COLS; col++) {
            int tile = bstGet(&lv->tileTree, row, col);
            if (tile == TILE_CONVEYOR_R || tile == TILE_CONVEYOR_L) {
                QRectF area(toSX(col * TILE_SIZE), toSY(row * TILE_SIZE), toSW(TILE_SIZE), toSH(TILE_SIZE));
                float speed = (tile == TILE_CONVEYOR_R) ? CONVEYOR_BELT_SPEED : -CONVEYOR_BELT_SPEED;
                drawOneConveyorBelt(p, area, speed);
            }
        }
    }
}

// ── Teleport Pads ────────────────────────────────────────────
void GameRenderer::drawTeleportPads(QPainter& p)
{
    LevelData* lv = eng->currentLevel();
    if (!lv) return;

    for (int i = 0; i < lv->teleportCount; i++) {
        TeleportPad& pad = lv->pads[i];
        int screenX = toSX(pad.x);
        int screenY = toSY(pad.y);
        int screenW = toSW(TILE_SIZE);
        int screenH = toSH(TILE_SIZE);
        
        // Alternate fire/water portal sprite based on pad ID
        QPixmap& pm = (pad.id % 2 == 0) ? pmTeleportFire : pmTeleportWater;
        p.drawPixmap(QRect(screenX, screenY, screenW, screenH), pm);
    }
}

// ── Gems (collectable items) ─────────────────────────────────
void GameRenderer::drawGems(QPainter& p) 
{
    LevelData* lv = eng->currentLevel();
    if (!lv) return;
    
    for (int i = 0; i < lv->gemCount; i++) {
        Gem& g = lv->gems[i];
        if (g.collected) continue;
        
        // Make gems hover up and down with a sine wave
        float bounce = sinf(g.animPhase) * 3.0f;

        int screenX = toSX(g.x);
        int screenY = toSY(g.y - bounce);
        int screenW = toSW(20);
        int screenH = toSH(20);

        QPixmap& pm = (g.owner == FIREBOY) ? pmGemFire : pmGemWater;
        p.drawPixmap(QRect(screenX, screenY, screenW, screenH), pm);
    }
}

// ── Doors (exit doors for each player) ───────────────────────
void GameRenderer::drawDoors(QPainter& p) 
{
    LevelData* lv = eng->currentLevel();
    if (!lv) return;
    
    for (int i = 0; i < 2; i++) {
        Door& d = lv->doors[i];

        int screenX = toSX(d.x);
        int screenY = toSY(d.y);
        int screenW = toSW(TILE_SIZE);
        int screenH = toSH(d.h);

        QPixmap& pm = (d.owner == FIREBOY) ? pmDoorFire : pmDoorWater;
        p.drawPixmap(QRect(screenX, screenY, screenW, screenH), pm);
        
        // Bright glow when the door is open
        if (d.open)
            p.fillRect(QRect(screenX, screenY, screenW, screenH), QColor(255, 255, 255, 60));
    }
}

// ── Players (fireboy and watergirl) ──────────────────────────
void GameRenderer::drawPlayer(QPainter& p, Player* pl) 
{
    if (!pl || pl->dead) return;
    
    int screenX = toSX(pl->x);
    int screenY = toSY(pl->y);
    int screenW = toSW(PLAYER_W);
    int screenH = toSH(PLAYER_H);

    QPixmap& pm = (pl->type == FIREBOY) ? pmFireboy : pmWatergirl;
    p.drawPixmap(QRect(screenX, screenY, screenW, screenH), pm);
}


// ══════════════════════════════════════════════════════════════
//  Hint System (pathfinding visualization)
// ══════════════════════════════════════════════════════════════

// Draws a single dashed path line on the map
void GameRenderer::drawOneHintPath(QPainter& p, PathResult& path, QColor color)
{
    if (path.len < 2) return;

    p.setPen(QPen(color, 2 * sx, Qt::DashLine));
    p.setBrush(Qt::NoBrush);
    
    // Build a path from tile centers
    QPainterPath qp;
    float halfTile = TILE_SIZE / 2;
    qp.moveTo(toSX(path.px[0] * TILE_SIZE + halfTile),
              toSY(path.py[0] * TILE_SIZE + halfTile));
              
    for (int i = 1; i < path.len && i < MAX_HINT_PATH; i++) {
        qp.lineTo(toSX(path.px[i] * TILE_SIZE + halfTile),
                  toSY(path.py[i] * TILE_SIZE + halfTile));
    }
    p.drawPath(qp);
}

void GameRenderer::drawHints(QPainter& p) 
{
    drawOneHintPath(p, eng->fireboyHint,   QColor(255, 120, 0, 180));   // orange path
    drawOneHintPath(p, eng->watergirlHint, QColor(60, 180, 255, 180));  // blue path
}

// ── DSA: Min-Heap + Dijkstra Arrow ──────────────────────────
// Draws an arrow pointing down at the nearest reachable gem

// Draws one arrow above a specific gem
void GameRenderer::drawOneGemArrow(QPainter& p, int gemIdx, QPixmap& arrowPm)
{
    LevelData* lv = eng->currentLevel();
    if (!lv) return;
    if (gemIdx < 0 || arrowPm.isNull()) return;

    Gem& g = lv->gems[gemIdx];
    if (g.collected) return;
    
    int arrowW = 44 * sx;
    int arrowH = 44 * sy;
    int arrowX = toSX(g.x + 16.0f) - arrowW / 2;  // center above gem
    int arrowY = toSY(g.y) - arrowH - 6 * sy;      // float above gem
    p.drawPixmap(QRect(arrowX, arrowY, arrowW, arrowH), arrowPm);
}

void GameRenderer::drawGemArrows(QPainter& p)
{
    LevelData* lv = eng->currentLevel();
    if (!lv) return;

    // Use Dijkstra + Min-Heap to find nearest reachable gem for each player
    int fbIdx = gemMinHeapFind(lv->gems, lv->gemCount,
                    eng->fireboy.x, eng->fireboy.y,
                    FIREBOY, eng->effectiveTileMap, eng->teleportEdges);
                    
    int wgIdx = gemMinHeapFind(lv->gems, lv->gemCount,
                    eng->watergirl.x, eng->watergirl.y,
                    WATERGIRL, eng->effectiveTileMap, eng->teleportEdges);

    drawOneGemArrow(p, fbIdx, pmFireArrow);
    drawOneGemArrow(p, wgIdx, pmWaterArrow);
}


// ══════════════════════════════════════════════════════════════
//  HUD (score bar, lives, controls)
// ══════════════════════════════════════════════════════════════

void GameRenderer::drawHUD(QPainter& p) 
{
    // ── Top bar (score, lives, level) ──
    QRectF bar(ox, oy, MAP_W * sx, 34 * sy);
    p.fillRect(bar, QColor(0, 0, 0, 130));
    
    QFont f("Arial", qMax(8, (int)(11 * sy)), QFont::Bold);
    p.setFont(f);
    
    // Score on the left
    p.setPen(QColor(255, 210, 50));
    p.drawText(bar.adjusted(8, 2, 0, 0), Qt::AlignVCenter | Qt::AlignLeft,
               QString("Score: %1").arg(eng->score));
    
    // Hearts in the center
    QString hearts; 
    for (int i = 0; i < eng->lives; i++) hearts += "♥ ";
    p.setPen(QColor(255, 60, 80));
    p.drawText(bar, Qt::AlignVCenter | Qt::AlignHCenter, hearts);
    
    // Level number on the right
    p.setPen(QColor(160, 210, 255));
    LevelData* lv = eng->currentLevel();
    p.drawText(bar.adjusted(0, 2, -8, 0), Qt::AlignVCenter | Qt::AlignRight,
               QString("Lvl %1").arg(lv ? lv->num : 0));
               
    // ── Bottom bar (keyboard controls) ──
    QFont sf("Arial", qMax(6, (int)(8 * sy)));
    p.setFont(sf); 
    p.setPen(QColor(160, 160, 160, 170));
    QRectF ctrl(ox, oy + MAP_H * sy - 16 * sy, MAP_W * sx, 16 * sy);
    p.fillRect(ctrl, QColor(0, 0, 0, 100));
    p.drawText(ctrl, Qt::AlignCenter,
               "Fireboy: \u2190 \u2191 \u2192    Watergirl: A W D    Hint: H    Undo: U    Redo: R    Pause: Esc");
}


// ══════════════════════════════════════════════════════════════
//  Undo/Redo Flash Banner
// ══════════════════════════════════════════════════════════════

void GameRenderer::drawUndoRedoFlash(QPainter& p)
{
    if (eng->undoRedoFlash <= 0) return;

    QPixmap& pm = eng->lastUndoWasUndo ? pmUndo : pmRedo;
    float alpha = qMin(1.0f, eng->undoRedoFlash);

    // Draw the undo/redo icon centered near the top
    int bannerW = width() * 0.40f;
    int bannerH = bannerW * ((float)pm.height() / pm.width());
    int bannerX = (width()  - bannerW) / 2;
    int bannerY = height() * 0.10f;

    p.setOpacity(alpha);
    p.drawPixmap(QRect(bannerX, bannerY, bannerW, bannerH), pm);
    p.setOpacity(1.0f);
}


// ══════════════════════════════════════════════════════════════
//  Game State Overlay (pause, win, death, game over)
// ══════════════════════════════════════════════════════════════

void GameRenderer::drawOverlay(QPainter& p) 
{
    // Dim the entire screen
    p.fillRect(rect(), QColor(0, 0, 0, 155));
    
    // Pick the title text and color based on current state
    QString title, sub; 
    QColor col;
    
    switch (eng->state) {
    case STATE_PAUSED:
        title = "PAUSED"; 
        col   = QColor(180, 220, 255);
        sub   = "Press Esc or Enter to Resume"; 
        break;
        
    case STATE_WIN:
        if (eng->levels.current && eng->levels.current->next) { 
            title = "FIN !"; 
            sub   = QString("Total Score: %1\nEnter \u2192 Next Level   P/N \u2192 Prev/Next").arg(eng->score); 
        } else { 
            title = "You Win! ";    
            sub   = QString("Total Score: %1\nEnter \u2192 Continue").arg(eng->score); 
        }
        col = QColor(80, 255, 120); 
        break;
        
    case STATE_DEAD:
        title = "Oh No!"; 
        col   = QColor(255, 80, 80);
        sub   = QString("Lives Left: %1\nR \u2192 Retry").arg(eng->lives); 
        break;
        
    case STATE_GAMEOVER:
        title = "Game Over"; 
        col   = QColor(255, 60, 60);
        sub   = QString("Final Score: %1\nR \u2192 Restart").arg(eng->score); 
        break;
        
    default: 
        return;  // STATE_PLAYING or unknown — don't draw overlay
    }
    
    // Draw title text (big, colored)
    QFont big("Arial", qMax(14, (int)(26 * sy)), QFont::Bold);
    QFont med("Arial", qMax(9, (int)(13 * sy)));
    QRectF area(ox, oy, MAP_W * sx, MAP_H * sy);
    
    p.setFont(big); 
    p.setPen(col);
    QRectF titleArea(area.left(), area.top() + area.height() * 0.25, area.width(), area.height() * 0.2);
    p.drawText(titleArea, Qt::AlignCenter, title);
    
    // Draw subtitle text (smaller, white)
    p.setFont(med); 
    p.setPen(Qt::white);
    QRectF subArea(area.left(), area.top() + area.height() * 0.48, area.width(), area.height() * 0.3);
    p.drawText(subArea, Qt::AlignHCenter | Qt::AlignTop, sub);

    // ── DSA: Singly Linked List — show gem collection trail on win screen ──
    if (eng->state == STATE_WIN) {
        int totalGems = eng->gemTrail.count;
        if (totalGems > 0) {
            int iconW  = 24 * sx;
            int iconH  = 24 * sy;
            int arrowW = 14 * sx;
            int totalW = totalGems * iconW + (totalGems - 1) * arrowW;
            int startX = area.left() + (area.width() - totalW) / 2;
            int rowY   = area.top()  + area.height() * 0.72f;

            p.setFont(QFont("Arial", qMax(7, (int)(9 * sy))));

            int drawX = startX;
            bool first = true;
            
            // Walk the linked list of collected gems
            GemTrailNode* node = eng->gemTrail.head;
            while (node) {
                // Draw arrow between gems
                if (!first) {
                    p.setPen(QColor(200, 200, 200, 200));
                    p.drawText(QRect(drawX, rowY, arrowW, iconH), Qt::AlignCenter, "\u2192");
                    drawX += arrowW;
                }
                
                // Draw gem icon
                QPixmap& gemPm = (node->playerType == FIREBOY) ? pmGemFire : pmGemWater;
                p.drawPixmap(QRect(drawX, rowY, iconW, iconH), gemPm);
                drawX += iconW;
                first = false;
                node = node->next;  // move to next node in linked list
            }
            
            // Draw door icon at the end of the trail
            p.setPen(QColor(200, 200, 200, 180));
            p.drawText(QRect(drawX, rowY, arrowW + iconW, iconH), Qt::AlignVCenter | Qt::AlignLeft, " \u2192 \U0001F6AA");
        }
    }
}

