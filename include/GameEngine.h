#pragma once
#include <QObject>
#include <QTimer>
#include <QSoundEffect>
#include "../include/GameObjects.h"
#include "../include/DSA.h"
#include "../include/Player.h"

class GameEngine : public QObject {
    Q_OBJECT
public:
    explicit GameEngine(QObject* parent = nullptr);
    ~GameEngine();

    void start();
    void pause();
    void resume();
    void resetLevel();
    void nextLevel();
    void keyPress(int key);
    void keyRelease(int key);

    // State
    int   state;
    int   score;
    int   lives;
    float elapsed;
    bool  showHint;

    // Players
    Player fireboy;
    Player watergirl;

    // DSA: Linked List of levels
    LevelList levels;

    // DSA: Priority Queue for events (death > win > gem)
    PriorityQueue eventQueue;

    // DSA: Gate Hash Map for O(1) button→gate lookup
    GateHashMap gateMap;

    // Dijkstra hint paths (via grid)
    PathResult fireboyHint;
    PathResult watergirlHint;

    // DSA: State History (Doubly Linked List – Undo / Redo)
    StateHistory history;
    float snapTimer;         // counts up; snapshot every 500ms
    float undoRedoFlash;     // > 0 shows flash overlay
    float undoCooldown;      // > 0 = block auto-snapshots (set after undo/redo)
    bool  lastUndoWasUndo;   // true=undo, false=redo (for flash label)

    // Cheat code tracker
    CheatTracker skipCheat;

    // Effective tilemap (base tiles + closed gates overlaid)
    int effectiveTileMap[MAP_ROWS][MAP_COLS];

    LevelData* currentLevel();

signals:
    void frameReady();
    void stateChanged(int s);
    void scoreChanged(int s);

private slots:
    void tick();

private:
    QTimer* timer;

    void buildEffectiveTileMap();
    void rebuildGateMap();
    void checkButtons();
    void checkHazards();
    void checkGems();
    void checkDoors();
    void updatePlatforms();
    void updateConveyors();
    void updateConveyorTiles();
    void processEvents();
    void computeHints();
    void handleDeath();
    void buildGrid(int who, int grid[MAP_ROWS][MAP_COLS]);

    QSoundEffect* sndMaleJump;
    QSoundEffect* sndFemaleJump;
    QSoundEffect* sndGemCollect;
    QSoundEffect* sndDie;
    QSoundEffect* sndWin;
    QSoundEffect* sndLavaWalk;
    QSoundEffect* sndWaterWalk;
};
