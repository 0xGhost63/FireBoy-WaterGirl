#pragma once
#include "../include/GameObjects.h"

// ============================================================
// Levels.h – All level data defined in one place
// Edit this file to add or modify levels easily.
// ============================================================

// Note: tile shorthand macros (E, S, L, W, P, R, Q) are defined
// locally in Levels.cpp only, to avoid colliding with Qt template
// parameter names in other translation units.

LevelData makeLevel1();
LevelData makeLevel2();
LevelData makeLevel3();
LevelData makeLevel4();
LevelData makeLevel5();
