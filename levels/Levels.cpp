#include "Levels.h"
#include <cstring>
using namespace std;

// ============================================================
// LEVEL 1 – "The Crossing"
// Both players start on opposite sides of a central wall.
// Fireboy presses Button 0 → Gate 0 opens → Watergirl crosses
// Watergirl presses Button 1 → Gate 1 opens → Fireboy crosses
// Then both reach their doors on the opposite side. Classic!
// ============================================================
// ============================================================
// LEVEL 1 – "The Forest Temple"
// A classic zig-zag climb! 
// Bottom: Jump over Lava (Fireboy) and Water (Watergirl).
// Floor 2: Jump over Poison (both).
// Floor 3 & 4 Puzzle:
//   - Fireboy stands on Button 0 to open Gate 0.
//   - Watergirl jumps up to Floor 4 and stands on Button 1.
//   - Button 1 ALSO opens Gate 0, keeping it open!
//   - Fireboy can now safely pass Gate 0 and jump up to Floor 4.
// ============================================================
LevelData makeLevel1() {
    LevelData lv;
    lv.num = 1; lv.bgStyle = 0;
    strcpy(lv.name, "The Forest Temple");

    // E=empty S=solid L=lava W=water P=poison
    int map[MAP_ROWS][MAP_COLS] = {
        {S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S},  // 0
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 1
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 2
        {S,S,S,S,S,S,S,S,S,S,S,S,E,E,S,S,S,E,E,S},  // 3  Top floor (Doors left). Gaps at 12,13 and 17,18.
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 4
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 5
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 6  
        {S,S,S,E,E,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S},  // 7  3rd floor. Gap at 3,4. 
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 8
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 9
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 10 
        {S,S,S,S,S,S,S,S,S,S,S,S,S,P,P,P,S,E,E,S},  // 11 2nd floor. Gap at 17,18.
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 12 
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 13
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 14
        {S,S,S,S,S,L,L,S,S,S,W,W,S,S,S,S,S,S,S,S},  // 15 Bottom floor
    };
    for(int r=0;r<MAP_ROWS;r++) for(int c=0;c<MAP_COLS;c++) lv.tileMap[r][c]=map[r][c];

    // Players start at bottom left
    lv.fireboyStartX   = 1*TILE_SIZE;  lv.fireboyStartY   = 15*TILE_SIZE - PLAYER_H;
    lv.watergirlStartX = 3*TILE_SIZE; lv.watergirlStartY  = 15*TILE_SIZE - PLAYER_H;

    // Doors are at the top left
    lv.doors[0] = {6*TILE_SIZE, 3*TILE_SIZE - PLAYER_H*1.5f, FIREBOY,   false};
    lv.doors[1] = {3*TILE_SIZE, 3*TILE_SIZE - PLAYER_H*1.5f, WATERGIRL, false};

    // Gems (encourage exploration of the level)
    lv.gemCount = 8;
    lv.gems[0] = {4*TILE_SIZE,   14*TILE_SIZE, FIREBOY,   false, 0.0f};
    lv.gems[1] = {14*TILE_SIZE,  10*TILE_SIZE, FIREBOY,   false, 0.5f};
    lv.gems[2] = {6*TILE_SIZE,   6*TILE_SIZE,  FIREBOY,   false, 1.0f};
    lv.gems[3] = {16*TILE_SIZE,  2*TILE_SIZE,  FIREBOY,   false, 1.5f};
    
    lv.gems[4] = {8*TILE_SIZE,   14*TILE_SIZE, WATERGIRL, false, 0.0f};
    lv.gems[5] = {5*TILE_SIZE,   10*TILE_SIZE, WATERGIRL, false, 0.7f};
    lv.gems[6] = {16*TILE_SIZE,  6*TILE_SIZE,  WATERGIRL, false, 1.4f};
    lv.gems[7] = {10*TILE_SIZE,  2*TILE_SIZE,  WATERGIRL, false, 2.0f};

    lv.hazardCount = 3;
    lv.hazards[0] = {5*TILE_SIZE,  15*TILE_SIZE, 2*TILE_SIZE, TILE_SIZE, TILE_LAVA};
    lv.hazards[1] = {10*TILE_SIZE, 15*TILE_SIZE, 2*TILE_SIZE, TILE_SIZE, TILE_WATER};
    lv.hazards[2] = {13*TILE_SIZE,  11*TILE_SIZE, 3*TILE_SIZE, TILE_SIZE, TILE_POISON};

    // Gate 0: Row 7, col 10 (Blocks the right path on the 3rd floor)
    // Height is 3 tiles (Rows 4, 5, 6)
    lv.gates[0] = {0, 10*TILE_SIZE, 4*TILE_SIZE, TILE_SIZE, 3*TILE_SIZE, false, 0.0f};
    lv.gateCount = 1;

    // Button 0: Row 7, col 6. Fireboy presses this to open Gate 0.
    lv.buttons[0] = {6*TILE_SIZE, 7*TILE_SIZE-14, TILE_SIZE+12, 14, 0, false};
    
    // Button 1: Row 3, col 10. Watergirl presses this to ALSO open Gate 0.
    lv.buttons[1] = {10*TILE_SIZE, 3*TILE_SIZE-14, TILE_SIZE+12, 14, 0, false};
    lv.buttonCount = 2;

    lv.platformCount = 0;
    return lv;
}

// ============================================================
// ============================================================
// LEVEL 2 – "The Crystal Bridge"
// A more vertical level. Moving platform brings both players
// up. Two gates, four buttons — must coordinate carefully.
// Poison pool in center forces both to use the moving bridge.
// ============================================================
LevelData makeLevel2() {
    LevelData lv;
    lv.num = 2; lv.bgStyle = 1;
    strcpy(lv.name, "The Crystal Bridge");

    int map[MAP_ROWS][MAP_COLS] = {
        {S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S},  // 0
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 1
        {S,S,S,S,S,E,E,E,E,E,E,E,E,E,E,S,S,S,S,S},  // 2 Top platforms (Doors here)
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 3
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 4
        {S,E,E,E,E,E,E,S,S,S,S,S,S,E,E,E,E,E,E,S},  // 5 Middle platform
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 6
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 7
        {S,S,S,S,E,E,E,E,E,E,E,E,E,E,E,E,S,S,S,S},  // 8 Side platforms
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 9
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 10 (Moving platform goes here)
        {S,S,S,S,S,S,E,E,E,E,E,E,E,E,S,S,S,S,S,S},  // 11 Start platforms
        {S,E,E,E,E,E,E,E,E,P,P,E,E,E,E,E,E,E,E,S},  // 12
        {S,E,E,E,E,E,E,E,E,P,P,E,E,E,E,E,E,E,E,S},  // 13
        {S,S,L,L,S,E,E,E,E,P,P,E,E,E,E,S,W,W,S,S},  // 14
        {S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S},  // 15
    };
    for(int r=0;r<MAP_ROWS;r++) for(int c=0;c<MAP_COLS;c++) lv.tileMap[r][c]=map[r][c];

    lv.fireboyStartX   = 2*TILE_SIZE;  lv.fireboyStartY   = 11*TILE_SIZE - PLAYER_H;
    lv.watergirlStartX = 16*TILE_SIZE; lv.watergirlStartY = 11*TILE_SIZE - PLAYER_H;

    // Doors are at the TOP (both players must reach the upper area)
    lv.doors[0] = {2*TILE_SIZE,  0*TILE_SIZE, FIREBOY,   false};
    lv.doors[1] = {16*TILE_SIZE, 0*TILE_SIZE, WATERGIRL, false};

    lv.gemCount = 8;
    lv.gems[0] = {7*TILE_SIZE+8,  4*TILE_SIZE-10, FIREBOY,   false, 0.0f};
    lv.gems[1] = {8*TILE_SIZE+8,  4*TILE_SIZE-10, FIREBOY,   false, 0.5f};
    lv.gems[2] = {9*TILE_SIZE+8,  4*TILE_SIZE-10, FIREBOY,   false, 1.0f};
    lv.gems[3] = {2*TILE_SIZE+8,  7*TILE_SIZE-10, FIREBOY,   false, 1.5f};
    lv.gems[4] = {10*TILE_SIZE+8, 4*TILE_SIZE-10, WATERGIRL, false, 0.0f};
    lv.gems[5] = {11*TILE_SIZE+8, 4*TILE_SIZE-10, WATERGIRL, false, 0.7f};
    lv.gems[6] = {12*TILE_SIZE+8, 4*TILE_SIZE-10, WATERGIRL, false, 1.4f};
    lv.gems[7] = {16*TILE_SIZE+8, 7*TILE_SIZE-10, WATERGIRL, false, 2.0f};

    lv.hazardCount = 3;
    lv.hazards[0] = {2*TILE_SIZE,  14*TILE_SIZE, 2*TILE_SIZE, TILE_SIZE, TILE_LAVA};
    lv.hazards[1] = {16*TILE_SIZE, 14*TILE_SIZE, 2*TILE_SIZE, TILE_SIZE, TILE_WATER};
    lv.hazards[2] = {9*TILE_SIZE,  12*TILE_SIZE, 2*TILE_SIZE, 3*TILE_SIZE, TILE_POISON};

    // Gate 0: Row 10, col 5 (blocks left path on start floor)
    lv.gates[0] = {0, 5*TILE_SIZE, 10*TILE_SIZE, TILE_SIZE, TILE_SIZE, false, 0.0f};
    // Gate 1: Row 10, col 14 (blocks right path on start floor)
    lv.gates[1] = {1, 14*TILE_SIZE, 10*TILE_SIZE, TILE_SIZE, TILE_SIZE, false, 0.0f};
    lv.gateCount = 2;

    // Symmetry Puzzle: 4 buttons
    // Button 0 & 1 open Gate 1. Button 2 & 3 open Gate 0.
    lv.buttons[0] = {4*TILE_SIZE,  11*TILE_SIZE-14, TILE_SIZE+12, 14, 1, false}; // Left outer
    lv.buttons[1] = {6*TILE_SIZE,  11*TILE_SIZE-14, TILE_SIZE+12, 14, 1, false}; // Left inner
    lv.buttons[2] = {14*TILE_SIZE+8, 11*TILE_SIZE-14, TILE_SIZE+12, 14, 0, false}; // Right outer
    lv.buttons[3] = {12*TILE_SIZE+8, 11*TILE_SIZE-14, TILE_SIZE+12, 14, 0, false}; // Right inner
    lv.buttonCount = 4;

    // Moving platform: carries both players across the center gap at Row 10
    lv.platformCount = 1;
    lv.platforms[0] = {6*TILE_SIZE, 10*TILE_SIZE, 13*TILE_SIZE, 10*TILE_SIZE,
                        6*TILE_SIZE, 10*TILE_SIZE, 70.0f, true, true};
    return lv;
}

// ============================================================
// LEVEL 3 – "The Ancient Trap"
// Chain-reaction puzzle: Gate 0 hides Button 1. Fireboy must
// hold Button 0 (opens Gate 0) while Watergirl runs in to
// press Button 1 (opens Gate 2 for Fireboy's path). Then both
// race to their doors before a moving platform disappears.
// ============================================================
LevelData makeLevel3() {
    LevelData lv;
    lv.num = 3; lv.bgStyle = 2;
    strcpy(lv.name, "The Ancient Trap");

    int map[MAP_ROWS][MAP_COLS] = {
        {S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S},  // 0
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 1
        {S,E,E,S,S,S,E,E,E,E,E,E,E,E,S,S,S,E,E,S},  // 2
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 3
        {S,E,E,E,E,E,E,E,S,S,S,S,E,E,E,E,E,E,E,S},  // 4 center bridge
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 5
        {S,S,S,S,E,E,E,E,E,E,E,E,E,E,E,E,S,S,S,S},  // 6 side barriers
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 7
        {S,E,E,E,E,S,S,S,E,E,E,E,S,S,S,E,E,E,E,S},  // 8 platforms
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 9
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 10
        {S,S,S,S,S,S,S,E,E,E,E,E,E,S,S,S,S,S,S,S},  // 11 walls (gate gaps at 6,12)
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 12 secret room (between gates)
        {S,S,S,S,S,S,S,E,E,E,E,E,E,S,S,S,S,S,S,S},  // 13 walls again
        {S,S,L,L,L,S,E,E,E,P,P,E,E,E,S,W,W,W,S,S},  // 14 hazards
        {S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S},  // 15
    };
    for(int r=0;r<MAP_ROWS;r++) for(int c=0;c<MAP_COLS;c++) lv.tileMap[r][c]=map[r][c];

    lv.fireboyStartX   = 3*TILE_SIZE;  lv.fireboyStartY   = 10*TILE_SIZE - PLAYER_H;
    lv.watergirlStartX = 15*TILE_SIZE; lv.watergirlStartY = 10*TILE_SIZE - PLAYER_H;

    // Doors are at the TOP
    lv.doors[0] = {3*TILE_SIZE,  1*TILE_SIZE, FIREBOY,   false};
    lv.doors[1] = {15*TILE_SIZE, 1*TILE_SIZE, WATERGIRL, false};

    lv.gemCount = 8;
    lv.gems[0] = {2*TILE_SIZE+8,  2*TILE_SIZE-10, FIREBOY,   false, 0.0f};
    lv.gems[1] = {3*TILE_SIZE+8,  2*TILE_SIZE-10, FIREBOY,   false, 0.5f};
    lv.gems[2] = {4*TILE_SIZE+8,  2*TILE_SIZE-10, FIREBOY,   false, 1.0f};
    lv.gems[3] = {9*TILE_SIZE+8, 12*TILE_SIZE-10, FIREBOY,   false, 1.5f}; // in secret room
    lv.gems[4] = {15*TILE_SIZE+8, 2*TILE_SIZE-10, WATERGIRL, false, 0.0f};
    lv.gems[5] = {16*TILE_SIZE+8, 2*TILE_SIZE-10, WATERGIRL, false, 0.7f};
    lv.gems[6] = {14*TILE_SIZE+8, 2*TILE_SIZE-10, WATERGIRL, false, 1.4f};
    lv.gems[7] = {10*TILE_SIZE+8,12*TILE_SIZE-10, WATERGIRL, false, 2.0f}; // in secret room

    lv.hazardCount = 3;
    lv.hazards[0] = {2*TILE_SIZE,  14*TILE_SIZE, 3*TILE_SIZE, TILE_SIZE, TILE_LAVA};
    lv.hazards[1] = {9*TILE_SIZE,  14*TILE_SIZE, 2*TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[2] = {15*TILE_SIZE, 14*TILE_SIZE, 3*TILE_SIZE, TILE_SIZE, TILE_WATER};

    // Gate 0: left wall gap (row 11, col 6) — Fireboy's path down
    //         Opened by Button 0 (Fireboy holds at col 4, row 10)
    lv.gates[0] = {0, 6*TILE_SIZE, 11*TILE_SIZE, TILE_SIZE, TILE_SIZE, false, 0.0f};
    // Gate 1: same left gap but lower (row 13, col 6) — opened same button
    lv.gates[1] = {1, 6*TILE_SIZE, 13*TILE_SIZE, TILE_SIZE, TILE_SIZE, false, 0.0f};
    // Gate 2: right wall gap (row 11+13, col 12) — Watergirl's path, opened by Button 1
    lv.gates[2] = {2, 12*TILE_SIZE, 11*TILE_SIZE, TILE_SIZE, TILE_SIZE, false, 0.0f};
    lv.gates[3] = {3, 12*TILE_SIZE, 13*TILE_SIZE, TILE_SIZE, TILE_SIZE, false, 0.0f};
    lv.gateCount = 4;

    // Button 0: Fireboy holds (row 10 left) → opens Gate 0+1 (left path for Watergirl!)
    lv.buttons[0] = {4*TILE_SIZE+6, 10*TILE_SIZE-14, TILE_SIZE+12, 14, 0, false};
    lv.buttons[1] = {4*TILE_SIZE+6, 10*TILE_SIZE-14, TILE_SIZE+12, 14, 1, false}; // same button, two gates
    // Button 1: Watergirl finds in secret room → opens Gate 2+3 (right path for Fireboy)
    lv.buttons[2] = {9*TILE_SIZE+6, 12*TILE_SIZE-14, TILE_SIZE+12, 14, 2, false};
    lv.buttons[3] = {9*TILE_SIZE+6, 12*TILE_SIZE-14, TILE_SIZE+12, 14, 3, false}; // same button
    lv.buttonCount = 4;

    // Two moving platforms help players reach upper gem platforms
    lv.platformCount = 2;
    lv.platforms[0] = {1*TILE_SIZE, 7*TILE_SIZE, 4*TILE_SIZE, 7*TILE_SIZE,
                        1*TILE_SIZE, 7*TILE_SIZE, 60.0f, true, true};
    lv.platforms[1] = {15*TILE_SIZE, 7*TILE_SIZE, 17*TILE_SIZE, 7*TILE_SIZE,
                        15*TILE_SIZE, 7*TILE_SIZE, 60.0f, true, true};
    return lv;
}
