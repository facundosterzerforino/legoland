#include "legoland.h"

#include "build.h"
#include "gfx.h"
#include "globals.h"
#include "image_sprite.h"
#include "llidb.h"
#include "map_object.h"
#include "obj_instance.h"
#include "objclass.h"
#include "render.h"
#include "render3d.h"
#include "tilemap.h"

// FUNCTION: LEGOLAND 0x00450b90
LEGO_EXPORT int AddObjectToBuildList(struct ObjClass *obj, TileId coords) {
    int i;

    if (DAT_006670f8 >= 256) {
        return 0;
    }
    for (i = 0; i < 256; i++) {
        if (DAT_006664f8[i].ride == NULL) {
            break;
        }
    }
    if (i >= 256) {
        return 0;
    }
    DAT_006664f8[i].ride = (struct Ride *)obj;
    DAT_006664f8[i].coords = coords;
    DAT_006664f8[i].elapsed = 0;
    DAT_006670f8++;
    return 1;
}

// FUNCTION: LEGOLAND 0x00450c00
void FUN_00450c00(TileId coords) {
    int i;
    BuildObj *b;

    i = 0;
    for (b = DAT_006664f8; (int)&b->coords < (int)&DAT_006670fc; b++, i++) {
        if (b->coords.id == coords.id) {
            DAT_006670f8--;
            DAT_006664f8[i].ride = NULL;
            return;
        }
    }
}

// FUNCTION: LEGOLAND 0x00450c40
LEGO_EXPORT int GetBuildTime(Ride *objClass) {
    int cost = GetObjCost(objClass);
    if (cost < 50) {
        return 50;
    }
    if (cost > 150) {
        return 150;
    }
    return cost;
}

// FUNCTION: LEGOLAND 0x00450c70
unsigned int FUN_00450c70(Ride *ride) {
    return 0;
}

// FUNCTION: LEGOLAND 0x00450c80
LEGO_EXPORT void ProcessBuildingTimes(void) {
    int i;

    for (i = 0; i < 256; i++) {
        if (DAT_006664f8[i].ride != NULL) {
            DAT_006664f8[i].elapsed++;
            if (DAT_006664f8[i].elapsed >= GetBuildTime(DAT_006664f8[i].ride)) {
                DAT_006670f8--;
                ObjectIsBuilt((struct ObjClass *)DAT_006664f8[i].ride, DAT_006664f8[i].coords);
                DAT_006664f8[i].ride = NULL;
            } else {
                ObjectIsBuilding((struct ObjClass *)DAT_006664f8[i].ride, DAT_006664f8[i].coords);
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x00450cf0
LEGO_EXPORT int GetBuildAnimFrame(Ride *ride, TileId coords) {
    int i;
    int frames;
    int n;
    int frame;
    int max;
    BuildObj *b;
    LLS *lls;

    i = 0;
    for (b = DAT_006664f8; (int)&b->coords < (int)&DAT_006670fc; b++, i++) {
        if (b->coords.id == coords.id) {
            break;
        }
    }
    if ((ride->anim->flags & 0x8000) != 0) {
        max = 0;
        for (n = 0; n < ride->anim->group->count; n++) {
            lls = GetLLSForLayer(ride->anim, n);
            if (lls != NULL && lls->frame_count > max) {
                max = lls->frame_count;
            }
        }
        frames = max;
    } else {
        lls = *ride->anim->lls;
        if (lls == NULL) {
            return 0;
        }
        frames = lls->frame_count;
    }
    frame = DAT_006664f8[i].elapsed * frames / GetBuildTime(ride);
    if (frame >= frames) {
        frame = frames - 1;
    }
    return frame;
}

// FUNCTION: LEGOLAND 0x00450d90
LEGO_EXPORT void DoBuildEffects(Ride *ride, TileId coords) {
    Point pt;
    int b[4];
    int i;
    int x;
    int y;
    int cx;
    int cy;
    int w;
    int h;
    int ty;
    int left;
    BuildObj *o;

    if (FUN_00450c70(ride) && DAT_0066895c == 0) {
        pt.x = ride->footprint.x0 + coords.pos.x;
        pt.y = ride->footprint.y1 + coords.pos.y;
        GetTileBounds(&pt, b);
        left = b[0];
        ty = b[1];
        pt.x = ride->footprint.x1 + coords.pos.x;
        pt.y = ride->footprint.y0 + coords.pos.y;
        GetTileBounds(&pt, b);
        cx = (b[2] + left) / 2;
        cy = (ty + b[3]) / 2;
        x = cx - 32;
        y = cy - 5;
        w = cx - x + 33;
        h = cy - y + 6;
        i = 0;
        for (o = DAT_006664f8; (int)&o->coords < (int)&DAT_006670fc; o++, i++) {
            if (o->coords.id == coords.id) {
                if (i < 256) {
                    RenderBlock(x, y, w, h, 0);
                    RenderBox(x, y, w, h, GetNearestColour(255, 255, 255));
                    RenderBlock(x + 1, y + 1, DAT_006664f8[i].elapsed * (w - 2) / GetBuildTime(ride), h - 2, GetNearestColour(0, 255, 0));
                }
                return;
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x00450f10
LEGO_EXPORT void ClearBuildObjList(void) {
    int i;

    for (i = 0; i < 256; i++) {
        DAT_006664f8[i].ride = NULL;
    }
}
