#include "../include/GameEngine.h"
#include "../levels/Levels.h"
#include <cmath>
#include <cstring>
#include <QUrl>
#include <QDebug>
using namespace std;

static const int TICK_MS = 16;

GameEngine::GameEngine(QObject* parent) : QObject(parent) {
    timer = new QTimer(this);
    timer->setInterval(TICK_MS);
    connect(timer, &QTimer::timeout, this, &GameEngine::tick);

    listInit(&levels);
    listAppend(&levels, makeLevel1());
    listAppend(&levels, makeLevel2());
    listAppend(&levels, makeLevel3());

    sndMaleJump = new QSoundEffect(this);
    sndMaleJump->setSource(QUrl("qrc:/sounds/male_jump.wav"));
    sndFemaleJump = new QSoundEffect(this);
    sndFemaleJump->setSource(QUrl("qrc:/sounds/female_jump.wav"));
    sndGemCollect = new QSoundEffect(this);
    sndGemCollect->setSource(QUrl("qrc:/sounds/gem_collect.wav"));
    sndDie = new QSoundEffect(this);
    sndDie->setSource(QUrl("qrc:/sounds/die.wav"));
    sndWin = new QSoundEffect(this);
    sndWin->setSource(QUrl("qrc:/sounds/win.wav"));

    sndLavaWalk = new QSoundEffect(this);
    sndLavaWalk->setSource(QUrl("qrc:/sounds/lava_walk.wav"));
    sndLavaWalk->setLoopCount(QSoundEffect::Infinite);
    sndLavaWalk->setVolume(1.0f);

    sndWaterWalk = new QSoundEffect(this);
    sndWaterWalk->setSource(QUrl("qrc:/sounds/water_walk.wav"));
    sndWaterWalk->setLoopCount(QSoundEffect::Infinite);
    sndWaterWalk->setVolume(1.0f);

    pqInit(&eventQueue);
    gateMapInit(&gateMap);
    cheatInit(&skipCheat, "SKIP");
    historyInit(&history);
    gemTrailInit(&gemTrail);
    nearestFbGem = -1; nearestWgGem = -1;
    snapTimer = 0; undoRedoFlash = 0; undoCooldown = 0; lastUndoWasUndo = true;
    state = STATE_MENU; score = 0; lives = 3; elapsed = 0; showHint = false;
    fireboyHint.len = 0; watergirlHint.len = 0;
}
GameEngine::~GameEngine() {
    listFree(&levels);
    historyFree(&history);
    gemTrailFree(&gemTrail);
}

LevelData* GameEngine::currentLevel() {
    return levels.current ? &levels.current->data : nullptr;
}

void GameEngine::rebuildGateMap() {
    gateMapInit(&gateMap);
    LevelData* lv = currentLevel(); if (!lv) return;
    for (int i = 0; i < lv->gateCount; i++)
        gateMapInsert(&gateMap, lv->gates[i].id, i);
}

// Build effective tilemap: base tiles + closed gates overlaid as SOLID
void GameEngine::buildEffectiveTileMap() {
    LevelData* lv = currentLevel(); if (!lv) return;
    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            effectiveTileMap[r][c] = bstGet(&lv->tileTree, r, c);
        }
    }
    for (int i = 0; i < lv->gateCount; i++) {
        Gate& g = lv->gates[i];
        if (g.open) continue; // open gates are passable, skip
        int c0 = (int)(g.x / TILE_SIZE);
        int r0 = (int)(g.y / TILE_SIZE);
        int c1 = (int)((g.x + g.w - 1) / TILE_SIZE);
        int r1 = (int)((g.y + g.h - 1) / TILE_SIZE);
        for (int r = r0; r <= r1 && r < MAP_ROWS; r++)
            for (int c = c0; c <= c1 && c < MAP_COLS; c++)
                effectiveTileMap[r][c] = TILE_SOLID;
    }
}

void GameEngine::start() {
    LevelData* lv = currentLevel(); if (!lv) return;
    playerInit(&fireboy,   FIREBOY,   lv->fireboyStartX,   lv->fireboyStartY);
    playerInit(&watergirl, WATERGIRL, lv->watergirlStartX, lv->watergirlStartY);
    for (int i = 0; i < lv->gemCount;    i++) lv->gems[i].collected = false;
    for (int i = 0; i < 2;               i++) lv->doors[i].open     = false;
    for (int i = 0; i < lv->gateCount;   i++) { lv->gates[i].open = false; lv->gates[i].openAnim = 0; }
    for (int i = 0; i < lv->buttonCount; i++) lv->buttons[i].pressed = false;
    for (int i = 0; i < lv->conveyorCount; i++) conveyorQueueInit(&lv->conveyors[i].queue);
    score = 0; lives = 3; elapsed = 0;
    rebuildGateMap();
    buildEffectiveTileMap();
    // Clear undo history at the start of each level
    historyFree(&history);
    gemTrailFree(&gemTrail);
    nearestFbGem = -1; nearestWgGem = -1;
    snapTimer = 0; undoRedoFlash = 0;
    state = STATE_PLAYING;
    emit stateChanged(state);
    timer->start();
}

void GameEngine::pause()  { 
    if(state==STATE_PLAYING){
        state=STATE_PAUSED;  
        timer->stop();  
        sndLavaWalk->stop();
        sndWaterWalk->stop();
        emit stateChanged(state);
    } 
}
void GameEngine::resume() { if(state==STATE_PAUSED) {state=STATE_PLAYING; timer->start(); emit stateChanged(state);} }

void GameEngine::resetLevel() {
    LevelData* lv = currentLevel(); if (!lv) return;
    playerReset(&fireboy,   lv->fireboyStartX,   lv->fireboyStartY);
    playerReset(&watergirl, lv->watergirlStartX, lv->watergirlStartY);
    for (int i = 0; i < lv->gemCount;    i++) lv->gems[i].collected = false;
    for (int i = 0; i < 2;               i++) lv->doors[i].open     = false;
    for (int i = 0; i < lv->gateCount;   i++) { lv->gates[i].open = false; lv->gates[i].openAnim = 0; }
    for (int i = 0; i < lv->buttonCount; i++) lv->buttons[i].pressed = false;
    for (int i = 0; i < lv->conveyorCount; i++) conveyorQueueInit(&lv->conveyors[i].queue);
    elapsed = 0;
    rebuildGateMap();
    buildEffectiveTileMap();
    state = STATE_PLAYING; emit stateChanged(state);
    if (!timer->isActive()) timer->start();
}

void GameEngine::nextLevel() {
    if (!listNext(&levels)) {
        state = STATE_WIN; timer->stop(); emit stateChanged(state); return;
    }
    LevelData* lv = currentLevel();
    playerReset(&fireboy,   lv->fireboyStartX,   lv->fireboyStartY);
    playerReset(&watergirl, lv->watergirlStartX, lv->watergirlStartY);
    for (int i = 0; i < lv->gemCount;    i++) lv->gems[i].collected = false;
    for (int i = 0; i < 2;               i++) lv->doors[i].open     = false;
    for (int i = 0; i < lv->gateCount;   i++) { lv->gates[i].open = false; lv->gates[i].openAnim = 0; }
    for (int i = 0; i < lv->buttonCount; i++) lv->buttons[i].pressed = false;
    for (int i = 0; i < lv->conveyorCount; i++) conveyorQueueInit(&lv->conveyors[i].queue);
    elapsed = 0;
    rebuildGateMap();
    buildEffectiveTileMap();
    state = STATE_PLAYING; emit stateChanged(state);
    if (!timer->isActive()) timer->start();
}

void GameEngine::keyPress(int key) {
    if (state != STATE_PLAYING) return;
    
    // Check for "SKIP" cheat code
    if (key >= Qt::Key_A && key <= Qt::Key_Z) {
        if (cheatUpdate(&skipCheat, (char)key)) {
            skipCheat.isUnlocked = false; // Reset it immediately
            nextLevel();
            return;
        }
    }

    switch (key) {
    case Qt::Key_Left:   fireboy.moveLeft    = true; break;
    case Qt::Key_Right:  fireboy.moveRight   = true; break;
    case Qt::Key_Up:     fireboy.jumpWanted  = true; break;
    case Qt::Key_A:      watergirl.moveLeft  = true; break;
    case Qt::Key_D:      watergirl.moveRight = true; break;
    case Qt::Key_W:      watergirl.jumpWanted= true; break;
    case Qt::Key_H:      showHint = !showHint; computeHints(); break;
    case Qt::Key_Escape: pause(); break;
    case Qt::Key_U: {
        // UNDO – rewind 500ms
        GameSnapshot snap;
        if (historyUndo(&history, &snap)) {
            fireboy.x   = snap.fbX;  fireboy.y   = snap.fbY;
            fireboy.vx  = snap.fbVX; fireboy.vy  = snap.fbVY;
            fireboy.onGround = snap.fbOnGround;
            watergirl.x  = snap.wgX;  watergirl.y  = snap.wgY;
            watergirl.vx = snap.wgVX; watergirl.vy = snap.wgVY;
            watergirl.onGround = snap.wgOnGround;
            LevelData* lv = currentLevel();
            if (lv) for (int i = 0; i < snap.gemCount && i < lv->gemCount; i++)
                lv->gems[i].collected = snap.gemCollected[i];
            score = snap.score;
            emit scoreChanged(score);
            undoRedoFlash = 1.2f; lastUndoWasUndo = true;
            undoCooldown  = 1.5f;  // block auto-snapshots for 1.5s
            snapTimer     = 0;     // reset so next snap is a fresh 500ms
        }
        break;
    }
    case Qt::Key_R: {
        // REDO – forward one snapshot
        GameSnapshot snap;
        if (historyRedo(&history, &snap)) {
            fireboy.x   = snap.fbX;  fireboy.y   = snap.fbY;
            fireboy.vx  = snap.fbVX; fireboy.vy  = snap.fbVY;
            fireboy.onGround = snap.fbOnGround;
            watergirl.x  = snap.wgX;  watergirl.y  = snap.wgY;
            watergirl.vx = snap.wgVX; watergirl.vy = snap.wgVY;
            watergirl.onGround = snap.wgOnGround;
            LevelData* lv = currentLevel();
            if (lv) for (int i = 0; i < snap.gemCount && i < lv->gemCount; i++)
                lv->gems[i].collected = snap.gemCollected[i];
            score = snap.score;
            emit scoreChanged(score);
            undoRedoFlash = 1.2f; lastUndoWasUndo = false;
            undoCooldown  = 1.5f;
            snapTimer     = 0;
        }
        break;
    }
    case Qt::Key_P: {
        // Prev level (Doubly Linked List navigation)
        if (listPrev(&levels)) { start(); }
        break;
    }
    case Qt::Key_N: {
        // Next level (Doubly Linked List navigation)
        if (listNext(&levels)) { start(); }
        break;
    }
    default: break;
    }
}
void GameEngine::keyRelease(int key) {
    switch (key) {
    case Qt::Key_Left:  fireboy.moveLeft    = false; break;
    case Qt::Key_Right: fireboy.moveRight   = false; break;
    case Qt::Key_A:     watergirl.moveLeft  = false; break;
    case Qt::Key_D:     watergirl.moveRight = false; break;
    default: break;
    }
}

void GameEngine::tick() {
    if (state != STATE_PLAYING) return;
    LevelData* lv = currentLevel(); if (!lv) return;
    elapsed += TICK_MS / 1000.0f;

    // Button → Gate logic must happen BEFORE physics so gates are correct
    checkButtons();
    buildEffectiveTileMap();

    updatePlatforms();
    updateConveyors();
    updateConveyorTiles();

    bool fbJumping = fireboy.jumpWanted && fireboy.onGround;
    bool wgJumping = watergirl.jumpWanted && watergirl.onGround;

    playerUpdate(&fireboy,   effectiveTileMap, lv->platforms, lv->platformCount);
    playerUpdate(&watergirl, effectiveTileMap, lv->platforms, lv->platformCount);

    if (fbJumping) sndMaleJump->play();
    if (wgJumping) sndFemaleJump->play();

    checkHazards();
    checkGems();
    checkDoors();
    processEvents();

    if (showHint) computeHints();

    // Animate gate open/close progress
    for (int i = 0; i < lv->gateCount; i++) {
        Gate& g = lv->gates[i];
        float target = g.open ? 1.0f : 0.0f;
        g.openAnim += (target - g.openAnim) * 0.15f;
    }

    // Animate gems
    for (int i = 0; i < lv->gemCount; i++) lv->gems[i].animPhase += 0.05f;

    // ── Snapshot every 500ms for Undo/Redo history ────────────────
    if (undoCooldown > 0) {
        undoCooldown -= TICK_MS / 1000.0f;  // tick down cooldown
    } else {
        snapTimer += TICK_MS / 1000.0f;
        if (snapTimer >= 0.5f) {
            snapTimer = 0;
            LevelData* lvSnap = currentLevel();
            if (lvSnap) {
                GameSnapshot snap;
                snap.fbX = fireboy.x;   snap.fbY = fireboy.y;
                snap.fbVX= fireboy.vx;  snap.fbVY= fireboy.vy;
                snap.fbOnGround = fireboy.onGround;
                snap.wgX = watergirl.x; snap.wgY = watergirl.y;
                snap.wgVX= watergirl.vx;snap.wgVY= watergirl.vy;
                snap.wgOnGround = watergirl.onGround;
                snap.gemCount = lvSnap->gemCount;
                for (int i = 0; i < lvSnap->gemCount && i < MAX_GEMS; i++)
                    snap.gemCollected[i] = lvSnap->gems[i].collected;
                snap.score = score;
                historyPush(&history, snap);
            }
        }
    }

    // ── Tick down undo/redo flash ──────────────────────────────
    if (undoRedoFlash > 0) undoRedoFlash -= TICK_MS / 1000.0f;

    // ── Min-Heap nearest gem update (when hint is on) ─────────
    if (showHint) {
        nearestFbGem = gemMinHeapFind(lv->gems, lv->gemCount,
                           fireboy.x,   fireboy.y,   FIREBOY,   effectiveTileMap);
        nearestWgGem = gemMinHeapFind(lv->gems, lv->gemCount,
                           watergirl.x, watergirl.y, WATERGIRL, effectiveTileMap);
    } else {
        nearestFbGem = nearestWgGem = -1;
    }

    emit frameReady();
}

// ── Button pressure detection ─────────────────────────────────
void GameEngine::checkButtons() {
    LevelData* lv = currentLevel(); if (!lv) return;

    // First, assume all gates are closed (unless they were already open? No, buttons hold them open)
    bool gateShouldBeOpen[10] = {false};

    for (int i = 0; i < lv->buttonCount; i++) {
        Button& btn = lv->buttons[i];
        btn.pressed = false;

        bool fbOn = (fireboy.x  < btn.x + btn.w) && (fireboy.x  + PLAYER_W > btn.x) &&
                    (fireboy.y  < btn.y + btn.h)  && (fireboy.y  + PLAYER_H > btn.y);
        bool wgOn = (watergirl.x < btn.x + btn.w) && (watergirl.x + PLAYER_W > btn.x) &&
                    (watergirl.y < btn.y + btn.h)  && (watergirl.y + PLAYER_H > btn.y);

        if (fbOn || wgOn) {
            btn.pressed = true;
            if (btn.gateId >= 0 && btn.gateId < 10) {
                gateShouldBeOpen[btn.gateId] = true;
            }
        }
    }

    // Apply the computed state to the actual gates
    for (int i = 0; i < lv->gateCount; i++) {
        int id = lv->gates[i].id;
        if (id >= 0 && id < 10) {
            lv->gates[i].open = gateShouldBeOpen[id];
        }
    }
}

void GameEngine::updatePlatforms() {
    LevelData* lv = currentLevel(); if (!lv) return;
    for (int i = 0; i < lv->platformCount; i++) {
        MovingPlatform& mp = lv->platforms[i];
        if (!mp.active) continue;
        float tx = mp.towardsB ? mp.bx : mp.ax;
        float ty = mp.towardsB ? mp.by : mp.ay;
        float dx = tx - mp.cx, dy = ty - mp.cy;
        float dist = sqrtf(dx*dx + dy*dy);
        if (dist < 2.0f) mp.towardsB = !mp.towardsB;
        else { mp.cx += dx/dist * mp.speed * 0.016f; mp.cy += dy/dist * mp.speed * 0.016f; }
    }
}

// ── Conveyor Belt update (Queue DSA) ──────────────────────────
void GameEngine::updateConveyors() {
    LevelData* lv = currentLevel(); if (!lv) return;
    for (int i = 0; i < lv->conveyorCount; i++) {
        ConveyorBelt& belt = lv->conveyors[i];

        // Clear the queue each tick and re-detect who's on the belt
        conveyorQueueInit(&belt.queue);

        // Check if Fireboy is on this conveyor belt
        if (!fireboy.dead) {
            bool fbOn = (fireboy.x + PLAYER_W > belt.x) && (fireboy.x < belt.x + belt.w) &&
                        (fireboy.y + PLAYER_H > belt.y) && (fireboy.y < belt.y + belt.h);
            if (fbOn) {
                ConveyorItem item; item.id = 0; item.x = fireboy.x; item.y = fireboy.y;
                conveyorQueueEnqueue(&belt.queue, item);
            }
        }

        // Check if Watergirl is on this conveyor belt
        if (!watergirl.dead) {
            bool wgOn = (watergirl.x + PLAYER_W > belt.x) && (watergirl.x < belt.x + belt.w) &&
                        (watergirl.y + PLAYER_H > belt.y) && (watergirl.y < belt.y + belt.h);
            if (wgOn) {
                ConveyorItem item; item.id = 1; item.x = watergirl.x; item.y = watergirl.y;
                conveyorQueueEnqueue(&belt.queue, item);
            }
        }

        // Process the queue: dequeue → move → enqueue back
        int count = belt.queue.count;
        for (int j = 0; j < count; j++) {
            ConveyorItem item = conveyorQueueDequeue(&belt.queue);
            if (item.id < 0) continue;

            // Apply conveyor speed to the player's X position
            float newX = item.x + belt.speed * 0.016f;
            item.x = newX;

            // Update the actual player position
            if (item.id == 0) fireboy.x = newX;
            else              watergirl.x = newX;

        conveyorQueueEnqueue(&belt.queue, item);
        }
    }
}

// ── Conveyor tiles: push players standing on TILE_CONVEYOR_R/L ─
void GameEngine::updateConveyorTiles() {
    LevelData* lv = currentLevel(); if (!lv) return;
    auto push = [&](Player& pl) {
        if (!pl.onGround || pl.dead) return;
        int footRow = (int)((pl.y + PLAYER_H + 1) / TILE_SIZE);
        int colL    = (int)(pl.x / TILE_SIZE);
        int colR    = (int)((pl.x + PLAYER_W - 1) / TILE_SIZE);
        for (int c = colL; c <= colR; c++) {
            int tile = bstGet(&lv->tileTree, footRow, c);
            if      (tile == TILE_CONVEYOR_R) { pl.x += 1.8f; break; }
            else if (tile == TILE_CONVEYOR_L) { pl.x -= 1.8f; break; }
        }
    };
    push(fireboy);
    push(watergirl);
}

void GameEngine::checkHazards() {
    LevelData* lv = currentLevel(); if (!lv) return;
    bool fbInLava = false;
    bool wgInWater = false;

    auto check = [&](Player* p) {
        if (p->dead) return;
        for (int i = 0; i < lv->hazardCount; i++) {
            HazardPool& h = lv->hazards[i];
            if (p->x+PLAYER_W <= h.x || p->x >= h.x+h.w) continue;
            if (p->y+PLAYER_H <= h.y || p->y >= h.y+h.h) continue;
            bool dies = (h.type == TILE_POISON) ||
                        (h.type == TILE_LAVA  && p->type == WATERGIRL) ||
                        (h.type == TILE_WATER && p->type == FIREBOY);
            if (dies) {
                p->dead = true;
                // DSA: PriorityQueue — death events have priority 0 (highest)
                GameEvent e; e.type = EVT_PLAYER_DEAD; e.priority = 0;
                e.x = p->x; e.y = p->y; e.intData = p->type;
                pqPush(&eventQueue, e);
            } else if (p->type == FIREBOY && h.type == TILE_LAVA) {
                fbInLava = true;
            } else if (p->type == WATERGIRL && h.type == TILE_WATER) {
                wgInWater = true;
            }
        }
    };
    check(&fireboy); check(&watergirl);

    if (fbInLava) {
        if (!sndLavaWalk->isPlaying()) sndLavaWalk->play();
    } else {
        if (sndLavaWalk->isPlaying()) sndLavaWalk->stop();
    }

    if (wgInWater) {
        if (!sndWaterWalk->isPlaying()) sndWaterWalk->play();
    } else {
        if (sndWaterWalk->isPlaying()) sndWaterWalk->stop();
    }
}

void GameEngine::checkGems() {
    LevelData* lv = currentLevel(); if (!lv) return;
    auto collect = [&](Player* p) {
        if (p->dead) return;
        float cx = p->x + PLAYER_W/2.f, cy = p->y + PLAYER_H/2.f;
        // DSA: Linear Search for nearby gem
        int idx = linearSearchGem(lv->gems, lv->gemCount, cx, cy);
        if (idx >= 0 && lv->gems[idx].owner == p->type) {
            lv->gems[idx].collected = true;
            p->gemsCollected++;
            score += 100;
            // Append to unified gem trail (chronological order)
            gemTrailAppend(&gemTrail, idx, p->type);
            // Gem events have priority 2 (lowest urgency)
            GameEvent e; e.type = EVT_GEM_COLLECT; e.priority = 2;
            e.x = lv->gems[idx].x; e.y = lv->gems[idx].y; e.intData = p->type;
            pqPush(&eventQueue, e);
            emit scoreChanged(score);
        }
    };
    collect(&fireboy); collect(&watergirl);
}

void GameEngine::checkDoors() {
    LevelData* lv = currentLevel(); if (!lv) return;
    auto check = [&](Player* p) {
        if (p->dead) return;
        for (int i = 0; i < 2; i++) {
            Door& d = lv->doors[i];
            if (d.owner != p->type) continue;
            bool inX = p->x < d.x+TILE_SIZE   && p->x+PLAYER_W > d.x;
            bool inY = p->y < d.y+d.h  && p->y+PLAYER_H > d.y;
            if (inX && inY) d.open = true;
        }
    };
    check(&fireboy); check(&watergirl);
    if (lv->doors[0].open && lv->doors[1].open) {
        score += qMax(0, 500 - (int)(elapsed * 5));
        emit scoreChanged(score);
        // Win event has priority 1
        GameEvent e; e.type = EVT_LEVEL_COMPLETE; e.priority = 1; e.x=e.y=0; e.intData=0;
        pqPush(&eventQueue, e);
    }
}

void GameEngine::processEvents() {
    // DSA: PriorityQueue — process highest-priority events first
    while (!pqEmpty(&eventQueue)) {
        GameEvent e = pqPop(&eventQueue);
        if (e.type == EVT_PLAYER_DEAD) {
            sndDie->play();
            handleDeath(); return; // stop processing after death
        }
        if (e.type == EVT_LEVEL_COMPLETE) {
            sndWin->play();
            state = STATE_WIN; timer->stop(); emit stateChanged(state); return;
        }
        if (e.type == EVT_GEM_COLLECT) {
            sndGemCollect->play();
        }
    }
}

void GameEngine::handleDeath() {
    sndLavaWalk->stop();
    sndWaterWalk->stop();
    lives--;
    if (lives <= 0) { state = STATE_GAMEOVER; timer->stop(); emit stateChanged(state); return; }
    bool fb = playerRestoreCheckpoint(&fireboy);
    bool wg = playerRestoreCheckpoint(&watergirl);
    if (!fb || !wg) resetLevel();
    else { fireboy.dead = false; watergirl.dead = false; }
}

void GameEngine::buildGrid(int who, int grid[MAP_ROWS][MAP_COLS]) {
    LevelData* lv = currentLevel(); if (!lv) return;
    for (int r = 0; r < MAP_ROWS; r++)
        for (int c = 0; c < MAP_COLS; c++) {
            // Use effectiveTileMap so closed gates count as solid
            int t = effectiveTileMap[r][c];
            bool blocked = (t == TILE_SOLID) ||
                           (t == TILE_CONVEYOR_R) ||    // conveyors are walkable surfaces
                           (t == TILE_CONVEYOR_L) ||
                           (t == TILE_LAVA   && who == WATERGIRL) ||
                           (t == TILE_WATER  && who == FIREBOY)  ||
                           (t == TILE_POISON);
            grid[r][c] = blocked ? 1 : 0;
        }
}

void GameEngine::computeHints() {
    if (!showHint) { fireboyHint.len = 0; watergirlHint.len = 0; return; }
    LevelData* lv = currentLevel(); if (!lv) return;

    int fbGrid[MAP_ROWS][MAP_COLS], wgGrid[MAP_ROWS][MAP_COLS];
    buildGrid(FIREBOY,   fbGrid);
    buildGrid(WATERGIRL, wgGrid);

    auto getPath = [&](int playerType, float startX, float startY, int doorX, int doorY, int grid[MAP_ROWS][MAP_COLS]) -> PathResult {
        PathResult fullPath; fullPath.len = 0;
        int curX = (int)(startX / TILE_SIZE);
        int curY = (int)(startY / TILE_SIZE);
        
        bool usedGems[32] = {false};
        
        // Find path to all uncollected gems (greedy TSP)
        while (true) {
            int bestGem = -1;
            int bestDist = 999999;
            PathResult bestPath; bestPath.len = 0;
            
            for (int i = 0; i < lv->gemCount; i++) {
                if (usedGems[i] || lv->gems[i].collected) continue;
                if (lv->gems[i].owner != playerType) continue;
                
                int gx = (int)(lv->gems[i].x / TILE_SIZE);
                int gy = (int)(lv->gems[i].y / TILE_SIZE);
                
                PathResult p = dijkstraGridFind(grid, curX, curY, gx, gy);
                if (p.len > 0 && p.len < bestDist) {
                    bestDist = p.len;
                    bestGem = i;
                    bestPath = p;
                }
            }
            
            if (bestGem == -1) break; // no more reachable gems
            usedGems[bestGem] = true;
            
            // Append path to fullPath
            for (int i = 0; i < bestPath.len; i++) {
                if (fullPath.len > 0 && i == 0) continue; // skip duplicate start node
                if (fullPath.len < MAX_HINT_PATH) {
                    fullPath.px[fullPath.len] = bestPath.px[i];
                    fullPath.py[fullPath.len] = bestPath.py[i];
                    fullPath.len++;
                }
            }
            curX = (int)(lv->gems[bestGem].x / TILE_SIZE);
            curY = (int)(lv->gems[bestGem].y / TILE_SIZE);
        }
        
        // Finally path to door
        PathResult doorPath = dijkstraGridFind(grid, curX, curY, doorX, doorY);
        for (int i = 0; i < doorPath.len; i++) {
            if (fullPath.len > 0 && i == 0) continue;
            if (fullPath.len < MAX_HINT_PATH) {
                fullPath.px[fullPath.len] = doorPath.px[i];
                fullPath.py[fullPath.len] = doorPath.py[i];
                fullPath.len++;
            }
        }
        
        if (fullPath.len == 0) return doorPath;
        return fullPath;
    };

    fireboyHint = getPath(FIREBOY, fireboy.x, fireboy.y, 
        (int)(lv->doors[0].x / TILE_SIZE), (int)(lv->doors[0].y / TILE_SIZE), fbGrid);
    
    watergirlHint = getPath(WATERGIRL, watergirl.x, watergirl.y, 
        (int)(lv->doors[1].x / TILE_SIZE), (int)(lv->doors[1].y / TILE_SIZE), wgGrid);
}
