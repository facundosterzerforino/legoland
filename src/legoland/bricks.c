#include "legoland.h"

#include "bricks.h"
#include "globals.h"
#include "llidb.h"
#include "obj_instance.h"

// FUNCTION: LEGOLAND 0x00457870
void FUN_00457870(int param_1) {
    DAT_004b90fc = (param_1 == 0);
}

// FUNCTION: LEGOLAND 0x00457890
int FUN_00457890(void) {
    return DAT_004b90fc == 0;
}

// FUNCTION: LEGOLAND 0x004578a0
LEGO_EXPORT void AddBricks(unsigned int param_1) {
    if (DAT_004b90fc == 0) {
        DAT_004b90f8 += param_1;
    }
}

// FUNCTION: LEGOLAND 0x004578c0
LEGO_EXPORT void UseBricks(unsigned int param_1) {
    if (DAT_004b90fc == 0) {
        DAT_004b90f8 -= param_1;
    }
}

// FUNCTION: LEGOLAND 0x004578e0
LEGO_EXPORT int GetBrickCount(void) {
    if (DAT_004b90fc != 0) {
        return 0x7fffffff;
    }
    return DAT_004b90f8;
}

// FUNCTION: LEGOLAND 0x00457900
void FUN_00457900(unsigned int param_1) {
    DAT_004b90f8 = param_1;
}

// FUNCTION: LEGOLAND 0x00457910
int FUN_00457910(void) {
    if (SaveGameWrite(&DAT_004b90fc, 4) == 0) {
        return 0;
    }
    return SaveGameWrite(&DAT_004b90f8, 4) != 0;
}

// FUNCTION: LEGOLAND 0x00457940
int FUN_00457940(void) {
    if (SaveGameRead(&DAT_004b90fc, 4) == 0) {
        return 0;
    }
    return SaveGameRead(&DAT_004b90f8, 4) != 0;
}

// FUNCTION: LEGOLAND 0x00457970
int FUN_00457970(int dx, int dy) {
    struct Ride *ride = EditMode.unk8;
    int x, y;
    struct MapElement *elem;

    dx += ride->footprint.x0;
    dy += ride->footprint.y0;
    for (y = dy; y < dy + (int)DAT_00813a70; y++) {
        for (x = dx; x < dx + (int)DAT_00813a6c; x++) {
            if (x >= 0 && x < lpConfig->width && y >= 0 && y < lpConfig->height) {
                elem = &GameMap[y][x];
            } else {
                elem = NULL;
            }
            if (elem == NULL) return 0;
            if (elem->flags & 0x8f8) return 0;
            if (elem->field_12 != 0) {
                unsigned int idx;
                // STRING: LEGOLAND 0x004b8a70
                LLIDB_FindElement("PATH CONTROL", &idx, 0);
                if (EditMode.unk8->element != (struct Element *)idx) return 0;
            }
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00457a70
void FUN_00457a70(void) { STUB(); }
