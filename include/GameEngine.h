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

    // BFS hint paths
    PathResult fireboyHint;
    PathResult watergirlHint;

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
