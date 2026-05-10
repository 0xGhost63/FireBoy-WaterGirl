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
    bool  showHint;

    // Players
    Player fireboy;
    Player watergirl;

    // DSA: Linked List of levels
    LevelList levels;

    // DSA: Priority Queue for events (death > win > gem)
    PriorityQueue eventQueue;

    // DSA: Hash Maps (Direct Addressing)
    GateHashMap     gateMap;
    TeleportHashMap teleportMap;

    int effectiveTileMap[MAP_ROWS][MAP_COLS];
    int teleportEdges[MAP_ROWS][MAP_COLS][2]; // [0]=col, [1]=row of partner

    // Dijkstra hint paths (via grid)
    PathResult fireboyHint;
    PathResult watergirlHint;

    // DSA: State History (Doubly Linked List – Undo / Redo)
    StateHistory history;
    float snapTimer;
    float undoRedoFlash;
    float undoCooldown;
    bool  lastUndoWasUndo;

    // DSA: Singly Linked List – Gem Collection Trail (unified, chronological order)
    GemTrail gemTrail;

    // DSA: Min-Heap – nearest gem index per player (-1 = none)
    int nearestFbGem, nearestWgGem;

    // Cheat code tracker

    // Effective tilemap (base tiles + closed gates overlaid)


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
    void rebuildTeleportMap();
    void checkButtons();
    void checkConveyors();
    void checkTeleports(Player* p);
    void applyTeleport(int playerType, int destPadIndex);
    void checkHazards();
    void checkGems();
    void checkDoors();
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
