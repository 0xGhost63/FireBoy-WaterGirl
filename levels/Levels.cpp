#include "Levels.h"
#include "../include/DSA.h"
#include <cstring>

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
// LEVEL 1 – "The Forest Temple"
// A classic zig-zag climb! 
// Bottom: Jump over Lava (Fireboy) and Water (Watergirl).
// Floor 2: Jump over Poison (both).
// Floor 3 & 4 Puzzle:
//   - Fireboy stands on Button 0 to open Gate 0.
//   - Watergirl jumps up to Floor 4 and stands on Button 1.
//   - Button 1 ALSO opens Gate 0, keeping it open!
//   - Fireboy can now safely pass Gate 0 and jump up to Floor 4.
// 
// Tile Legend:
//   - S = Solid Tile
//   - E = Empty Space
//   - L = Lava (Lethal to Watergirl, safe for Fireboy)
//   - W = Water (Lethal to Fireboy, safe for Watergirl)
//   - P = Poison (Lethal to both)
//   - R = Conveyor Belt (Pushing Right)
//   - Q = Conveyor Belt (Pushing Left)
// ============================================================
LevelData makeLevel1() {
    LevelData lv;
    lv.num = 1; lv.bgStyle = 0;
    strcpy(lv.name, "The Forest Temple");

    int map[MAP_ROWS][MAP_COLS] = {
        {S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S},  // 0
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 1
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 2
        {S,S,S,S,S,S,S,S,S,S,S,S,E,E,S,S,S,E,E,S},  // 3
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 4
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 5
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 6
        {S,S,S,E,E,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S},  // 7
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 8
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 9
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 10
        {S,S,S,S,S,S,S,S,S,S,S,S,S,P,P,S,S,E,E,S},  // 11
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 12
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 13
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 14
        {S,S,S,S,S,L,L,S,S,S,W,W,S,S,S,S,S,S,S,S}  // 15
    };
    bstInit(&lv.tileTree);
    for(int r=0;r<MAP_ROWS;r++) for(int c=0;c<MAP_COLS;c++) bstInsert(&lv.tileTree,r,c,map[r][c]);

    lv.fireboyStartX   = 1*TILE_SIZE; lv.fireboyStartY   = 15*TILE_SIZE - PLAYER_H;
    lv.watergirlStartX = 3*TILE_SIZE; lv.watergirlStartY = 15*TILE_SIZE - PLAYER_H;

    lv.doors[0] = {2*TILE_SIZE, (2+1)*TILE_SIZE - TILE_SIZE*2.0f, TILE_SIZE*2.0f, FIREBOY, false};
    lv.doors[1] = {5*TILE_SIZE, (2+1)*TILE_SIZE - TILE_SIZE*2.0f, TILE_SIZE*2.0f, WATERGIRL, false};

    lv.gemCount = 7;
    lv.gems[0] = {16*TILE_SIZE+8, 2*TILE_SIZE+8, FIREBOY, false, 0.0f};
    lv.gems[1] = {1*TILE_SIZE+8, 6*TILE_SIZE+8, FIREBOY, false, 0.0f};
    lv.gems[2] = {16*TILE_SIZE+8, 6*TILE_SIZE+8, WATERGIRL, false, 0.0f};
    lv.gems[3] = {14*TILE_SIZE+8, 8*TILE_SIZE+8, FIREBOY, false, 0.0f};
    lv.gems[4] = {5*TILE_SIZE+8, 10*TILE_SIZE+8, WATERGIRL, false, 0.0f};
    lv.gems[5] = {4*TILE_SIZE+8, 14*TILE_SIZE+8, FIREBOY, false, 0.0f};
    lv.gems[6] = {8*TILE_SIZE+8, 14*TILE_SIZE+8, WATERGIRL, false, 0.0f};

    lv.conveyorCount = 0;
    lv.buttonCount = 2;
    lv.buttons[0] = {10*TILE_SIZE-6, (2+1)*TILE_SIZE-20, TILE_SIZE+24, 20, 0, false};
    lv.buttons[1] = {7*TILE_SIZE-6, (6+1)*TILE_SIZE-20, TILE_SIZE+24, 20, 0, false};

    lv.gateCount = 1;
    lv.gates[0] = {0, 10*TILE_SIZE, 4*TILE_SIZE, TILE_SIZE, TILE_SIZE*3, false, 0.0f};

    lv.teleportCount = 0;

    lv.hazardCount = 6;
    lv.hazards[0] = {13*TILE_SIZE, 11*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[1] = {14*TILE_SIZE, 11*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[2] = {5*TILE_SIZE, 15*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_LAVA};
    lv.hazards[3] = {6*TILE_SIZE, 15*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_LAVA};
    lv.hazards[4] = {10*TILE_SIZE, 15*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_WATER};
    lv.hazards[5] = {11*TILE_SIZE, 15*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_WATER};

    
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
    lv.num = 2; lv.bgStyle = 0;
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
        {S,E,E,S,P,P,P,S,E,E,E,E,S,P,P,P,S,E,E,S},  // 12
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
    lv.gates[0] = {0, 8*TILE_SIZE, 6*TILE_SIZE, TILE_SIZE, TILE_SIZE*2, false, 0.0f};

    lv.teleportCount = 0;

    lv.hazardCount = 16;
    lv.hazards[0] = {5*TILE_SIZE, 4*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[1] = {10*TILE_SIZE, 4*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[2] = {11*TILE_SIZE, 4*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[3] = {17*TILE_SIZE, 4*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[4] = {4*TILE_SIZE, 12*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[5] = {5*TILE_SIZE, 12*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[6] = {6*TILE_SIZE, 12*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[7] = {13*TILE_SIZE, 12*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[8] = {14*TILE_SIZE, 12*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[9] = {15*TILE_SIZE, 12*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[10] = {4*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_LAVA};
    lv.hazards[11] = {5*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_LAVA};
    lv.hazards[12] = {6*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_LAVA};
    lv.hazards[13] = {13*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_WATER};
    lv.hazards[14] = {14*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_WATER};
    lv.hazards[15] = {15*TILE_SIZE, 14*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_WATER};

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
    lv.num = 3; lv.bgStyle = 1;
    strcpy(lv.name, "The Ancient Trap");

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
    lv.num = 4; lv.bgStyle = 0;
    strcpy(lv.name, "Teleportation Level");

    int map[MAP_ROWS][MAP_COLS] = {
        {S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S},  // 0
        {S,E,E,E,E,E,E,E,E,E,S,E,E,E,E,E,E,E,E,S},  // 1
        {S,E,E,E,E,E,E,E,E,E,S,E,E,E,E,E,E,E,E,S},  // 2
        {S,E,E,E,E,E,E,E,E,E,S,E,E,E,E,E,E,E,E,S},  // 3
        {S,E,E,E,E,E,E,E,E,E,S,E,E,E,E,E,E,E,E,S},  // 4
        {S,S,S,W,W,Q,Q,Q,Q,Q,S,S,L,L,S,S,S,S,S,S},  // 5
        {S,E,E,E,E,E,E,E,E,E,E,E,S,S,E,E,E,E,E,S},  // 6
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 7
        {S,S,S,S,S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 8
        {S,E,E,S,S,S,S,S,S,S,S,S,S,S,S,E,E,E,E,S},  // 9
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,S,P,P,S,S,S},  // 10
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},  // 11
        {S,S,S,S,S,S,S,S,S,S,P,S,S,E,E,E,E,E,E,S},  // 12
        {S,E,E,E,E,E,E,E,E,E,S,E,S,S,S,E,E,E,E,S},  // 13
        {S,E,E,E,E,E,E,E,E,E,S,E,E,E,S,E,E,E,E,S},  // 14
        {S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S}  // 15
    };
    bstInit(&lv.tileTree);
    for(int r=0;r<MAP_ROWS;r++) for(int c=0;c<MAP_COLS;c++) bstInsert(&lv.tileTree,r,c,map[r][c]);

    lv.fireboyStartX   = 3*TILE_SIZE; lv.fireboyStartY   = 14*TILE_SIZE - PLAYER_H;
    lv.watergirlStartX = 4*TILE_SIZE; lv.watergirlStartY = 14*TILE_SIZE - PLAYER_H;

    lv.doors[0] = {1*TILE_SIZE, (4+1)*TILE_SIZE - TILE_SIZE*2.0f, TILE_SIZE*2.0f, FIREBOY, false};
    lv.doors[1] = {14*TILE_SIZE, (4+1)*TILE_SIZE - TILE_SIZE*2.0f, TILE_SIZE*2.0f, WATERGIRL, false};

    lv.gemCount = 8;
    lv.gems[0] = {5*TILE_SIZE+8, 4*TILE_SIZE+8, FIREBOY, false, 0.0f};
    lv.gems[1] = {17*TILE_SIZE+8, 4*TILE_SIZE+8, FIREBOY, false, 0.0f};
    lv.gems[2] = {3*TILE_SIZE+8, 7*TILE_SIZE+8, FIREBOY, false, 0.0f};
    lv.gems[3] = {7*TILE_SIZE+8, 8*TILE_SIZE+8, WATERGIRL, false, 0.0f};
    lv.gems[4] = {11*TILE_SIZE+8, 8*TILE_SIZE+8, WATERGIRL, false, 0.0f};
    lv.gems[5] = {10*TILE_SIZE+8, 10*TILE_SIZE+8, WATERGIRL, false, 0.0f};
    lv.gems[6] = {7*TILE_SIZE+8, 13*TILE_SIZE+8, WATERGIRL, false, 0.0f};
    lv.gems[7] = {18*TILE_SIZE+8, 14*TILE_SIZE+8, FIREBOY, false, 0.0f};

    lv.conveyorCount = 0;
    lv.buttonCount = 4;
    lv.buttons[0] = {5*TILE_SIZE-6, (8+1)*TILE_SIZE-20, TILE_SIZE+24, 20, 0, false};
    lv.buttons[1] = {13*TILE_SIZE-6, (8+1)*TILE_SIZE-20, TILE_SIZE+24, 20, 0, false};
    lv.buttons[2] = {7*TILE_SIZE-6, (14+1)*TILE_SIZE-20, TILE_SIZE+24, 20, 1, false};
    lv.buttons[3] = {16*TILE_SIZE-6, (14+1)*TILE_SIZE-20, TILE_SIZE+24, 20, 1, false};

    lv.gateCount = 2;
    lv.gates[0] = {0, 9*TILE_SIZE, 6*TILE_SIZE, TILE_SIZE, TILE_SIZE*3, false, 0.0f};
    lv.gates[1] = {1, 14*TILE_SIZE, 11*TILE_SIZE, TILE_SIZE, TILE_SIZE*2, false, 0.0f};

    lv.teleportCount = 8;
    lv.pads[0] = {9*TILE_SIZE, 1*TILE_SIZE, 1, 0, 0.0f};
    lv.pads[1] = {11*TILE_SIZE, 1*TILE_SIZE, 3, 2, 0.0f};
    lv.pads[2] = {18*TILE_SIZE, 2*TILE_SIZE, 0, 1, 0.0f};
    lv.pads[3] = {1*TILE_SIZE, 6*TILE_SIZE, 5, 4, 0.0f};
    lv.pads[4] = {18*TILE_SIZE, 9*TILE_SIZE, 2, 3, 0.0f};
    lv.pads[5] = {1*TILE_SIZE, 10*TILE_SIZE, 7, 6, 0.0f};
    lv.pads[6] = {18*TILE_SIZE, 12*TILE_SIZE, 4, 5, 0.0f};
    lv.pads[7] = {1*TILE_SIZE, 14*TILE_SIZE, 6, 7, 0.0f};

    lv.hazardCount = 7;
    lv.hazards[0] = {3*TILE_SIZE, 5*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_WATER};
    lv.hazards[1] = {4*TILE_SIZE, 5*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_WATER};
    lv.hazards[2] = {12*TILE_SIZE, 5*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_LAVA};
    lv.hazards[3] = {13*TILE_SIZE, 5*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_LAVA};
    lv.hazards[4] = {15*TILE_SIZE, 10*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[5] = {16*TILE_SIZE, 10*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};
    lv.hazards[6] = {10*TILE_SIZE, 12*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_POISON};

    return lv;
}
// Generated by FBW Level Editor template
// Note: This level is currently a blank template (no gems, hazards, or buttons).
LevelData makeLevel5() {
    LevelData lv;
    lv.num = 5; lv.bgStyle = 0;
    strcpy(lv.name, "Custom Level 5 (Empty Template)");

    int map[MAP_ROWS][MAP_COLS] = {
        {S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S},
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},
        {S,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,S},
        {S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S,S}
    };
    bstInit(&lv.tileTree);
    for(int r=0;r<MAP_ROWS;r++) for(int c=0;c<MAP_COLS;c++) bstInsert(&lv.tileTree,r,c,map[r][c]);

    lv.fireboyStartX   = 2*TILE_SIZE; lv.fireboyStartY   = 14*TILE_SIZE - PLAYER_H;
    lv.watergirlStartX = 4*TILE_SIZE; lv.watergirlStartY = 14*TILE_SIZE - PLAYER_H;

    lv.doors[0] = {16*TILE_SIZE, (14+1)*TILE_SIZE - TILE_SIZE*2.0f, TILE_SIZE*2.0f, FIREBOY, false};
    lv.doors[1] = {18*TILE_SIZE, (14+1)*TILE_SIZE - TILE_SIZE*2.0f, TILE_SIZE*2.0f, WATERGIRL, false};

    lv.gemCount = 0;
    lv.conveyorCount = 0;
    lv.buttonCount = 0;
    lv.gateCount = 0;
    lv.teleportCount = 0;
    lv.hazardCount = 0;

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

