#include "Levels.h"
#include "../include/DSA.h"
#include <cstring>
using namespace std;

// Tile shorthand macros — local to this .cpp ONLY (never in the header)
// to avoid colliding with Qt template parameter names.
#define E  TILE_EMPTY
#define S  TILE_SOLID
#define L  TILE_LAVA
#define W  TILE_WATER
#define P  TILE_POISON
#define R  TILE_CONVEYOR_R
#define Q  TILE_CONVEYOR_L

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
    int map[MAP_ROWS][MAP_COLS] = 
    {
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
    bstInit(&lv.tileTree);
    for(int r=0;r<MAP_ROWS;r++) for(int c=0;c<MAP_COLS;c++) bstInsert(&lv.tileTree, r, c, map[r][c]);

    // Players start at bottom left
    lv.fireboyStartX   = 1*TILE_SIZE;  lv.fireboyStartY   = 15*TILE_SIZE - PLAYER_H;
    lv.watergirlStartX = 3*TILE_SIZE; lv.watergirlStartY  = 15*TILE_SIZE - PLAYER_H;

    // Doors are at the top left
    lv.doors[0] = {6*TILE_SIZE, 3*TILE_SIZE - TILE_SIZE*2.0f, TILE_SIZE*2.0f, FIREBOY,   false};
    lv.doors[1] = {3*TILE_SIZE, 3*TILE_SIZE - TILE_SIZE*2.0f, TILE_SIZE*2.0f, WATERGIRL, false};

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
    lv.buttons[0] = {6*TILE_SIZE-6, 7*TILE_SIZE-20, TILE_SIZE+24, 20, 0, false};
    
    // Button 1: Row 3, col 10. Watergirl presses this to ALSO open Gate 0.
    lv.buttons[1] = {10*TILE_SIZE-6, 3*TILE_SIZE-20, TILE_SIZE+24, 20, 0, false};
    lv.buttonCount = 2;

    lv.platformCount = 0;
    lv.conveyorCount = 0;
    lv.teleportCount = 0;
    return lv;
}

// ============================================================
// ============================================================
// LEVEL 2 – "The Conveyor Gauntlet"
// Conveyor belts (Queue DSA) replace chains/see-saws!
// Players must fight the belt direction to avoid being
// pushed into hazards. Buttons open the central gate.
// Layout matches the classic Forest Temple Level 2 screenshot.
// ============================================================
LevelData makeLevel2() {
    LevelData lv;
    lv.num = 4; lv.bgStyle = 0;
    strcpy(lv.name, "Custom Level");

    int map[MAP_ROWS][MAP_COLS] = {
        {S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S},  // 0
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 1
        {S,S,S,S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 2
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S,S},  // 3
        {S,E,E,E,E,P,Q,Q,Q,Q,P,P,R,R,R,R,R,P,S,S},  // 4
        {S,E,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S},  // 5
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 6
        {S,S,S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 7
        {S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,E,E,E,S},  // 8
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S,E,E,S},  // 9
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S,E,S},  // 10
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 11
        {S,E,E,S,W,W,W,S,E,E,E,E,S,L,L,L,S,E,E,S},  // 12
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S,S},  // 13
        {S,E,E,E,L,L,L,E,E,E,E,E,E,W,W,W,E,E,S,S},  // 14
        {S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S}  // 15
    };
    bstInit(&lv.tileTree);
    for(int r=0;r<MAP_ROWS;r++) for(int c=0;c<MAP_COLS;c++) bstInsert(&lv.tileTree,r,c,map[r][c]);

    lv.fireboyStartX   = 2*TILE_SIZE; lv.fireboyStartY   = 14*TILE_SIZE - PLAYER_H;
    lv.watergirlStartX = 1*TILE_SIZE; lv.watergirlStartY = 14*TILE_SIZE - PLAYER_H;

    lv.doors[0] = {1*TILE_SIZE, (1+1)*TILE_SIZE - TILE_SIZE*2.0f, TILE_SIZE*2.0f, FIREBOY, false};
    lv.doors[1] = {3*TILE_SIZE, (1+1)*TILE_SIZE - TILE_SIZE*2.0f, TILE_SIZE*2.0f, WATERGIRL, false};

    lv.gemCount = 9;
    lv.gems[0] = {5*TILE_SIZE+8, 3*TILE_SIZE+8, WATERGIRL, false, 0.0f};
    lv.gems[1] = {16*TILE_SIZE+8, 3*TILE_SIZE+8, FIREBOY, false, 0.0f};
    lv.gems[2] = {1*TILE_SIZE+8, 6*TILE_SIZE+8, FIREBOY, false, 0.0f};
    lv.gems[3] = {5*TILE_SIZE+8, 7*TILE_SIZE+8, FIREBOY, false, 0.0f};
    lv.gems[4] = {11*TILE_SIZE+8, 7*TILE_SIZE+8, WATERGIRL, false, 0.0f};
    lv.gems[5] = {3*TILE_SIZE+8, 11*TILE_SIZE+8, FIREBOY, false, 0.0f};
    lv.gems[6] = {7*TILE_SIZE+8, 11*TILE_SIZE+8, FIREBOY, false, 0.0f};
    lv.gems[7] = {12*TILE_SIZE+8, 11*TILE_SIZE+8, WATERGIRL, false, 0.0f};
    lv.gems[8] = {16*TILE_SIZE+8, 11*TILE_SIZE+8, WATERGIRL, false, 0.0f};

    lv.conveyorCount = 0;
    lv.buttonCount = 2;
    lv.buttons[0] = {3*TILE_SIZE-6, (7+1)*TILE_SIZE-20, TILE_SIZE+24, 20, 0, false};
    lv.buttons[1] = {14*TILE_SIZE-6, (7+1)*TILE_SIZE-20, TILE_SIZE+24, 20, 0, false};

    lv.gateCount = 1;
    lv.gates[0] = {0, 8*TILE_SIZE, 6*TILE_SIZE,TILE_SIZE,2*TILE_SIZE, false, 0.0f};

    lv.hazardCount = 16;
    lv.hazards[0] = {5*TILE_SIZE, 4*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[1] = {10*TILE_SIZE, 4*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[2] = {11*TILE_SIZE, 4*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[3] = {17*TILE_SIZE, 4*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[4] = {4*TILE_SIZE, 12*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_WATER};
    lv.hazards[5] = {5*TILE_SIZE, 12*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_WATER};
    lv.hazards[6] = {6*TILE_SIZE, 12*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_WATER};
    lv.hazards[7] = {13*TILE_SIZE, 12*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_LAVA};
    lv.hazards[8] = {14*TILE_SIZE, 12*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_LAVA};
    lv.hazards[9] = {15*TILE_SIZE, 12*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_LAVA};
    lv.hazards[10] = {4*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_LAVA};
    lv.hazards[11] = {5*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_LAVA};
    lv.hazards[12] = {6*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_LAVA};
    lv.hazards[13] = {13*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_WATER};
    lv.hazards[14] = {14*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_WATER};
    lv.hazards[15] = {15*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_WATER};

    lv.platformCount = 0;
    lv.teleportCount = 0;
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
    lv.num = 4; lv.bgStyle = 0;
    strcpy(lv.name, "Custom Level");

    int map[MAP_ROWS][MAP_COLS] = {
        {S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S},  // 0
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 1
        {S,E,E,E,E,E,E,E,E,E,E,S,E,E,E,E,E,E,E,S},  // 2
        {S,E,E,E,E,E,E,E,E,E,E,S,E,E,S,S,S,S,S,S},  // 3
        {S,E,E,E,E,E,S,S,S,S,E,S,E,E,S,E,E,E,E,S},  // 4
        {S,E,E,E,W,W,E,S,E,E,E,S,E,E,S,E,E,E,E,S},  // 5
        {S,S,E,S,S,S,S,S,E,E,E,S,E,E,S,S,E,E,E,S},  // 6
        {S,E,E,S,E,E,E,E,E,E,E,S,E,E,S,E,E,S,S,S},  // 7
        {S,E,E,S,E,E,E,E,E,E,E,S,E,E,S,E,E,E,E,S},  // 8
        {S,E,S,S,E,E,E,E,E,E,E,E,E,E,S,S,S,S,E,S},  // 9
        {S,E,E,S,E,E,E,E,E,E,S,S,S,E,E,E,S,S,E,S},  // 10
        {S,E,E,S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 11
        {S,S,E,S,S,S,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 12
        {S,E,E,E,S,S,E,R,R,R,R,E,Q,Q,Q,Q,E,S,S,S},  // 13
        {S,E,E,E,S,S,P,P,P,P,P,P,P,P,P,P,P,S,S,S},  // 14
        {S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S}  // 15
    };
    bstInit(&lv.tileTree);
    for(int r=0;r<MAP_ROWS;r++) for(int c=0;c<MAP_COLS;c++) bstInsert(&lv.tileTree,r,c,map[r][c]);

    lv.fireboyStartX   = 3*TILE_SIZE; lv.fireboyStartY   = 14*TILE_SIZE - PLAYER_H;
    lv.watergirlStartX = 18*TILE_SIZE; lv.watergirlStartY = 2*TILE_SIZE - PLAYER_H;

    lv.doors[0] = {4*TILE_SIZE, (11+1)*TILE_SIZE - TILE_SIZE*2.0f, TILE_SIZE*2.0f, FIREBOY, false};
    lv.doors[1] = {5*TILE_SIZE, (11+1)*TILE_SIZE - TILE_SIZE*2.0f, TILE_SIZE*2.0f, WATERGIRL, false};

    lv.gemCount = 10;
    lv.gems[0] = {8*TILE_SIZE+8, 3*TILE_SIZE+8, FIREBOY, false, 0.0f};
    lv.gems[1] = {12*TILE_SIZE+8, 4*TILE_SIZE+8, WATERGIRL, false, 0.0f};
    lv.gems[2] = {1*TILE_SIZE+8, 5*TILE_SIZE+8, FIREBOY, false, 0.0f};
    lv.gems[3] = {18*TILE_SIZE+8, 6*TILE_SIZE+8, WATERGIRL, false, 0.0f};
    lv.gems[4] = {2*TILE_SIZE+8, 8*TILE_SIZE+8, FIREBOY, false, 0.0f};
    lv.gems[5] = {12*TILE_SIZE+8, 9*TILE_SIZE+8, WATERGIRL, false, 0.0f};
    lv.gems[6] = {1*TILE_SIZE+8, 11*TILE_SIZE+8, FIREBOY, false, 0.0f};
    lv.gems[7] = {7*TILE_SIZE+8, 12*TILE_SIZE+8, WATERGIRL, false, 0.0f};
    lv.gems[8] = {10*TILE_SIZE+8, 12*TILE_SIZE+8, FIREBOY, false, 0.0f};
    lv.gems[9] = {15*TILE_SIZE+8, 12*TILE_SIZE+8, WATERGIRL, false, 0.0f};

    lv.conveyorCount = 0;
    lv.buttonCount = 2;
    lv.buttons[0] = {7*TILE_SIZE-6, (3+1)*TILE_SIZE-20, TILE_SIZE+24, 20, 0, false};
    lv.buttons[1] = {15*TILE_SIZE-6, (5+1)*TILE_SIZE-20, TILE_SIZE+24, 20, 1, false};

    lv.gateCount = 2;
    lv.gates[0] = {1, 9*TILE_SIZE, 1*TILE_SIZE,TILE_SIZE,3*TILE_SIZE, false, 0.0f};
    lv.gates[1] = {0, 16*TILE_SIZE, 1*TILE_SIZE, TILE_SIZE, TILE_SIZE*2, false, 0.0f};

    lv.hazardCount = 13;
    lv.hazards[0] = {4*TILE_SIZE, 5*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_WATER};
    lv.hazards[1] = {5*TILE_SIZE, 5*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_WATER};
    lv.hazards[2] = {6*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[3] = {7*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[4] = {8*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[5] = {9*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[6] = {10*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[7] = {11*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[8] = {12*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[9] = {13*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[10] = {14*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[11] = {15*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[12] = {16*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};

    lv.platformCount = 0;
    lv.teleportCount = 0;
    return lv;
}
/*
O(1) Hash Map: TeleportHashMap matches pad A directly to pad B instantly.
Dijkstra Integration: dijkstraGridFind now views teleporter pairs as zero-cost graph edges, so the nearest-gem Hint Arrows will automatically point through the portals if the gem is closer on the other side!
Undo/Redo Stack: Integrated with the historyPush stack.
*/
LevelData makeLevel4() {
    LevelData lv;
    lv.num = 4;
    strcpy(lv.name, "Teleport Test Chamber");
    lv.bgStyle = 0; // Forest

    int layout[MAP_ROWS][MAP_COLS] = {
        {S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S},
        {S,E,E,E,E,E,E,E,E,S,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,S,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,S,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,S,E,E,E,E,E,E,E,E,E,S},
        {S,S,S,S,S,E,E,E,E,S,E,E,E,E,E,S,S,S,S,S},
        {S,E,E,E,E,E,E,E,E,S,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,S,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,S,E,E,E,E,E,E,E,E,E,S},
        {S,S,S,S,S,E,E,E,E,S,E,E,E,E,E,S,S,S,S,S},
        {S,E,E,E,E,E,E,E,E,S,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,S,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,S,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,S,E,E,E,E,E,E,E,E,E,S},
        {S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S},
        {S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S}
    };
    lv.tileTree.root = nullptr;
    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            if (layout[r][c] != E) {
                bstInsert(&lv.tileTree, r, c, layout[r][c]);
            }
        }
    }

    // Players start on left side
    lv.fireboyStartX   = 2*TILE_SIZE; lv.fireboyStartY   = 14*TILE_SIZE - PLAYER_H;
    lv.watergirlStartX = 4*TILE_SIZE; lv.watergirlStartY = 14*TILE_SIZE - PLAYER_H;

    // Doors on right side
    lv.doors[0] = {16*TILE_SIZE, 14*TILE_SIZE - TILE_SIZE*2.0f, TILE_SIZE*2.0f, FIREBOY, false};
    lv.doors[1] = {12*TILE_SIZE, 14*TILE_SIZE - TILE_SIZE*2.0f, TILE_SIZE*2.0f, WATERGIRL, false};

    lv.gemCount = 2;
    lv.gems[0] = {15*TILE_SIZE+8, 11*TILE_SIZE+8, FIREBOY, false, 0.0f};
    lv.gems[1] = {12*TILE_SIZE+8, 11*TILE_SIZE+8, WATERGIRL, false, 0.0f};

    // Teleporters! 
    // Pad A (Left) <-> Pad B (Right)
    lv.teleportCount = 2;
    lv.pads[0] = {6*TILE_SIZE, 13*TILE_SIZE, 0, 1, 0.0f}; // id=0, partner=1
    lv.pads[1] = {10*TILE_SIZE, 13*TILE_SIZE, 1, 0, 0.0f}; // id=1, partner=0

    lv.conveyorCount = 0;
    lv.buttonCount = 0;
    lv.gateCount = 0;
    lv.hazardCount = 0;
    lv.platformCount = 0;
    return lv;
}

// Clean up tile shorthand macros
#undef E
#undef S
#undef L
#undef W
#undef P
#undef R
#undef Q
