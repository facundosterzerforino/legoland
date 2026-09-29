#include "legoland.h"

#include "gfx.h"
#include "globals.h"
#include "interface.h"
#include "map_object.h"
#include "mapscreen.h"
#include "print_sprite.h"
#include "render.h"
#include "tilemap.h"

struct MapPoint {
    int field0;
    int field1;
};

#include "image_sprite.h"

// FUNCTION: LEGOLAND 0x004562c0
void FUN_004562c0(void) {
    DAT_00667c10 = DAT_006687bc;
    DAT_00667c28 = DAT_006687c0;
}

// FUNCTION: LEGOLAND 0x004562e0
void FUN_004562e0(void) {
    DAT_006687bc = DAT_00667c10;
    DAT_006687c0 = DAT_00667c28;
}

// FUNCTION: LEGOLAND 0x00456300
LEGO_EXPORT void InitMapScreen(void) {
    if (DAT_00667c30 != 0) {
        return;
    }
    DAT_00667c2c = CreateFunctionBasedSprite((int (*)(struct Sprite *))RenderFullMap, 0x280, 0x154);
    if (DAT_00667c2c != NULL) {
        // STRING: LEGOLAND 0x004b90b4
        DAT_00667c34 = LoadSprite("mapSpanner.lls", 0);
        DAT_00667c2c->src_x = 0;
        DAT_00667c2c->src_y = 0;
    }
    FUN_004562c0();
    DAT_006687c0 = (unsigned int)FUN_00475080;
    DAT_006687bc = (unsigned int)FUN_00475080;
}

// FUNCTION: LEGOLAND 0x00456370
LEGO_EXPORT void KillMapScreen(void) {
    if (DAT_00667c2c != NULL) {
        KillSprite(DAT_00667c2c);
        DAT_00667c2c = NULL;
        if (DAT_00667c34 != 0) {
            KillSprite(DAT_00667c34);
            DAT_00667c34 = 0;
        }
    }
    DAT_00667c30 = 0;
}

// FUNCTION: LEGOLAND 0x004563b0
void FUN_004563b0(void) {
    int x = ((ScrollX >> 8) - DAT_00667c00) * DAT_008139c4 / DAT_00667c1c;
    int y = ((ScrollY >> 8) - DAT_00667c04) * DAT_008139c4 / DAT_00667c18 / 2 + DAT_00667c20;
    int w = DAT_008139c4 * DAT_008139c4 / DAT_00667c1c;
    int h = DAT_008139c0 * DAT_008139c4 / DAT_00667c18 / 2;

    RenderThickBox(DAT_008139c8 + x, DAT_008139cc + y, w, h, 2, GetNearestColour(255, 255, 255));
}

// FUNCTION: LEGOLAND 0x00456460
LEGO_EXPORT void RenderMouseBounds(void) {
    int in[2];
    int out[2];
    int x;
    int y;
    int w;
    int h;

    in[0] = (DAT_00813a44.x - DAT_008139c8) * DAT_00667c1c / DAT_008139c4 + DAT_00667c00;
    in[1] = (DAT_00813a44.y - DAT_008139cc - DAT_00667c20 + 1) * DAT_00667c18 * 2 / DAT_008139c4 + DAT_00667c04;
    PointToIsoPlane(in, out);
    if (out[0] >= 0 && out[1] >= 0 && out[0] < DAT_008139c4 && out[1] < DAT_008139c0) {
        x = DAT_00813a44.x - DAT_008139c4 / 2 * DAT_008139c4 / DAT_00667c1c - DAT_008139c8;
        y = DAT_00813a44.y - DAT_008139c0 / 2 * DAT_008139c4 / DAT_00667c18 / 2 - DAT_008139cc;
        w = DAT_008139c4 * DAT_008139c4 / DAT_00667c1c;
        h = DAT_008139c0 * DAT_008139c4 / (DAT_00667c18 * 2);
        RenderBox(DAT_008139c8 + x, DAT_008139cc + y, w, h, GetNearestColour(0, 255, 0));
    }
}

// FUNCTION: LEGOLAND 0x004565b0
LEGO_EXPORT void MapScreenSetScrollPos(struct Point *point) {
    int in[2];
    int out[2];

    in[0] = (DAT_00813a44.x - DAT_008139c8) * DAT_00667c1c / DAT_008139c4 + DAT_00667c00;
    in[1] = (DAT_00813a44.y - DAT_008139cc - DAT_00667c20 + 1) * DAT_00667c18 * 2 / DAT_008139c4 + DAT_00667c04;
    PointToIsoPlane(in, out);
    if (out[0] >= 0 && out[1] >= 0 && out[0] < DAT_008139c4 && out[1] < DAT_008139c0) {
        ScrollX = ((point->x - DAT_008139c8 + 1) * DAT_00667c1c / DAT_008139c4 - DAT_008139c4 / 2 + DAT_00667c00) << 8;
        ScrollY =
            ((point->y - DAT_008139cc - DAT_00667c20 + 1) * DAT_00667c18 * 2 / DAT_008139c4 - DAT_008139c0 / 2 + DAT_00667c04)
            << 8;
        FUN_00461290(lpConfig->field_10 << 8, lpConfig->field_12 << 8, 0, 0);
    }
}

// FUNCTION: LEGOLAND 0x004566f0
LEGO_EXPORT void DrawMapScreen(void) {
    struct MapMarker(*row)[32];
    struct MapMarker *m;
    int j;

    PrintSprite(DAT_00667c2c, DAT_008139c8, DAT_008139cc, 0, 0);
    if (DAT_008119a4 & 0x10) {
        for (row = DAT_008119c0; (int)row < (int)DAT_008138c0; row++) {
            m = *row;
            for (j = 0; j < 31; j++, m++) {
                if (m->x != 0 || m->y != 0) {
                    PrintSprite(DAT_00667c34, m->x, m->y, 0, 0);
                }
            }
        }
    }
    FUN_004563b0();
}

// FUNCTION: LEGOLAND 0x00456770
void FUN_00456770(struct MapPoint *arg) {
    int v0 = arg->field0;
    int v1 = arg->field1;

    if (v0 < 0) {
        v0 = -((-v0) >> 1);
    } else {
        v0 = v0 >> 1;
    }

    if (v1 < 0) {
        v1 = -((-v1) >> 1);
    } else {
        v1 = v1 >> 1;
    }

    arg->field0 = v0;
    arg->field1 = v1;
}

// FUNCTION: LEGOLAND 0x004567a0
LEGO_EXPORT void RenderFullMap(void) { STUB(); }
