#pragma once
#include "../include/GameObjects.h"

// ============================================================
// Levels.h – All level data defined in one place
// Edit this file to add or modify levels easily.
// ============================================================

// Tile shorthand macros (used only in this file)
#define E  TILE_EMPTY
#define S  TILE_SOLID
#define L  TILE_LAVA
#define W  TILE_WATER
#define P  TILE_POISON
#define R  TILE_CONVEYOR_R
#define Q  TILE_CONVEYOR_L

LevelData makeLevel1();
LevelData makeLevel2();
LevelData makeLevel3();
LevelData makeLevel4();
