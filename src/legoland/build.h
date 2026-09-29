#pragma once

#include "legoland.h"

// Per-TU header for build.c — building/construction list helpers.

struct ObjClass;
struct Ride;

LEGO_EXPORT int AddObjectToBuildList(struct ObjClass *obj, TileId coords);
LEGO_EXPORT void ClearBuildObjList(void);
void FUN_00450c00(TileId coords);
LEGO_EXPORT void ProcessBuildingTimes(void);
LEGO_EXPORT void DoBuildEffects(struct Ride *ride, TileId coords);
LEGO_EXPORT int GetBuildAnimFrame(struct Ride *ride, TileId coords);
