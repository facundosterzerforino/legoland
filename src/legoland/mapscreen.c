#include <string.h>

#include "legoland.h"

#include "castle.h"
#include "clipping.h"
#include "draw.h"
#include "gamemap.h"
#include "gfx.h"
#include "globals.h"
#include "interface.h"
#include "llidb.h"
#include "map_object.h"
#include "mapscreen.h"
#include "obj_instance.h"
#include "print_sprite.h"
#include "render.h"
#include "ride_queue.h"
#include "tilemap.h"

struct MapPoint {
    int field0;
    int field1;
};

/* A sprite overlaid on the map (OverlayList). */
struct OverlayNode {
    unsigned char pad_0[0x14];
    int x;
    int y;
    struct OverlayNode *next;
    struct Sprite *sprite;
};

#define HALF(v) ((v) < 0 ? -(-(v) >> 1) : (v) >> 1)

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
LEGO_EXPORT void RenderFullMap(void) {
    struct Element *square_track;
    struct Element *square_track_height;
    struct Element *square_track_height_0;
    struct Element *square_track_height_path;
    struct Element *castle_dummy;
    struct Element *driving_school_roads;
    struct Sprite *blob;
    struct Sprite *stick;
    struct Sprite *lights;
    HPEN pen;

    // STRING: LEGOLAND 0x004b5bfc
    square_track = ElemID("SQUARE_TRACK");
    // STRING: LEGOLAND 0x004b5be8
    square_track_height = ElemID("SQUARE_TRACK_HEIGHT");
    // STRING: LEGOLAND 0x004b5bd0
    square_track_height_0 = ElemID("SQUARE_TRACK_HEIGHT_0");
    // STRING: LEGOLAND 0x004b5bb4
    square_track_height_path = ElemID("SQUARE_TRACK_HEIGHT_PATH");
    // STRING: LEGOLAND 0x004b5b7c
    castle_dummy = ElemID("CASTLE_DUMMY");
    // STRING: LEGOLAND 0x004b89cc
    driving_school_roads = ElemID("DRIVING SCHOOL ROADS");
    // STRING: LEGOLAND 0x004b90e4
    blob = LoadSprite("TRACKBLOB.LLS", 1);
    // STRING: LEGOLAND 0x004b90d4
    stick = LoadSprite("TRACKSTICK.LLS", 1);
    // STRING: LEGOLAND 0x004b90c4
    lights = LoadSprite("MAPLIGHTS.LLS", 1);
    pen = CreatePen(PS_SOLID, 2, 0xff4000);

    if (DAT_00667c30 == 0) {
        RECT clip;
        int tw;
        int th;
        int half;
        int sx;
        int sy;
        int saved_x;
        int saved_y;
        int x;
        int y;
        int base_x;
        int base_y;
        int bounds[4];
        int origin[4];
        struct Point pt;
        struct MapElement tile;
        struct MapElement *obj;
        struct OverlayNode *node;

        FUN_0045a660();
        memset(DAT_008119c0, 0, 0x2000);
        DAT_008139c8 = 0;
        DAT_008139cc = 0x20;
        DAT_008139c4 = 0x280;
        DAT_008139c0 = 0x154;
        StoreClipping();
        clip.left = 0;
        clip.right = DAT_008139c4;
        clip.top = 0;
        clip.bottom = DAT_008139c0;
        SetClipping(&clip);
        GetTileDimensions(&tw, &th);
        DAT_00667c04 = 0;
        DAT_00667c00 = -(lpConfig->height * tw / 2);
        DAT_00667c08 = lpConfig->width * tw / 2;
        DAT_00667c0c = (lpConfig->width + lpConfig->height) * th / 2;
        DAT_00667c1c = DAT_00667c08 - DAT_00667c00 + 1;
        DAT_00667c18 = DAT_00667c0c + 1;
        sx = (DAT_008139c4 << 16) / DAT_00667c1c;
        sy = (DAT_008139c0 << 16) / DAT_00667c18;
        DAT_00667c16 = (short)((double)DAT_008139c4 * tw / DAT_00667c1c + 0.5f);
        DAT_00667c14 = (short)((double)DAT_008139c0 * th / DAT_00667c18 + 0.5f);
        half = (th + 1) >> 1;
        RenderBlock(0, 0, DAT_008139c4, DAT_008139c0, GetNearestColour(0, 0, 0));
        saved_x = lpConfig->field_20;
        saved_y = lpConfig->field_22;
        lpConfig->field_20 = 0;
        lpConfig->field_22 = 0;
        DAT_00667c30 = 1;
        DAT_00667c20 = (DAT_008139c0 - ((lpConfig->width + lpConfig->height - 2) * half - DAT_00667c04) * DAT_008139c4 / DAT_00667c18 / 2) >> 1;

        for (y = 0; y < (int)lpConfig->height; y++) {
            for (x = 0; x < (int)lpConfig->width; x++) {
                if (x >= 0 && x < (int)lpConfig->width && y >= 0 && y < (int)lpConfig->height) {
                    tile = GameMap[y][x];
                } else {
                    tile.flags = 0x40;
                }
                if ((tile.flags & 8) == 0) {
                    struct FXSpriteList *src;
                    unsigned short id;
                    int px;
                    int py;

                    id = GameMap[y][x].field_8;
                    src = TileSpriteInfo[id].src;
                    pt.x = x;
                    pt.y = y;
                    GetTileBounds(&pt, bounds);
                    bounds[0] += ScrollX >> 8;
                    bounds[1] += ScrollY >> 8;
                    px = ((bounds[0] - DAT_00667c00) * sx) >> 16;
                    py = (((bounds[1] - DAT_00667c04) * sy) >> 16) + DAT_00667c20;
                    if ((tile.flags & 0x10) && (tile.flags & 0x80)) {
                        RenderBlock(px - 7, py - 2, DAT_00667c16 + 5, DAT_00667c14 + 4, GetNearestColour(0x80, 0x80, 0x80));
                    } else {
                        switch (src->sprite_ids[id - src->base] & 0x3f) {
                        case 0:
                        case 0x20:
                            RenderBlock(px - 7, py - 2, DAT_00667c16 + 5, DAT_00667c14 + 4, GetNearestColour(0, 0x8f, 0x4f));
                            break;
                        case 1:
                        case 0x21:
                            RenderBlock(px - 7, py - 2, DAT_00667c16 + 5, DAT_00667c14 + 4, GetNearestColour(0xff, 0xe0, 0x8f));
                            break;
                        case 0x30:
                            RenderBlock(px - 7, py - 2, DAT_00667c16 + 5, DAT_00667c14 + 4, GetNearestColour(0x5b, 0xbe, 2));
                            break;
                        }
                    }
                }
            }
        }

        PushRenderingStatusAndUnlockVideoSurface();
        for (y = 0; y < (int)lpConfig->height; y++) {
            for (x = 0; x < (int)lpConfig->width; x++) {
                if (x >= 0 && x < (int)lpConfig->width && y >= 0 && y < (int)lpConfig->height) {
                    tile = GameMap[y][x];
                } else {
                    tile.field_8 = 0;
                    tile.flags = 0x40;
                }
                if (tile.flags & 8) {
                    struct Ride *ride;
                    struct MapPoint off;
                    float sxf;
                    float syf;

                    sxf = (float)DAT_008139c4 / DAT_00667c1c;
                    syf = (float)DAT_008139c0 / DAT_00667c18;
                    ride = tile.field_0->ride;
                    pt.x = x;
                    pt.y = y;
                    GetTileBounds(&pt, bounds);
                    off.field0 = ride->field_14;
                    off.field1 = ride->field_18;
                    FUN_00456770(&off);
                    bounds[1] += off.field1;
                    bounds[0] += off.field0;
                    bounds[1] += ScrollY >> 8;
                    bounds[0] += ScrollX >> 8;
                    PrintScaledSprite(TileSpriteArray[tile.field_8], (int)((bounds[0] - DAT_00667c00) * sxf),
                        (int)((bounds[1] - DAT_00667c04) * syf) + DAT_00667c20, DAT_00667c16 + 1,
                        DAT_00667c14 + 1);
                }
            }
        }

        clip.left = 0;
        clip.right = DAT_008139c4;
        clip.top = 0;
        clip.bottom = DAT_008139c0;
        SetClipping(&clip);
        node = OverlayList;
        pt.x = 0;
        pt.y = 0;
        GetTileBounds(&pt, origin);
        origin[0] += ScrollX >> 8;
        origin[1] += ScrollY >> 8;
        base_x = ((origin[0] - DAT_00667c00) * sx) >> 16;
        base_y = (((origin[1] - DAT_00667c04) * sy) >> 16) + DAT_00667c20;
        for (; node != NULL; node = node->next) {
            PrintScaledSprite(node->sprite, (((tw * 2 / 3 + node->x) * sx) >> 16) + base_x,
                ((node->y * sy) >> 16) + base_y, ((short)node->sprite->width * sx) >> 16,
                ((short)node->sprite->height * sy) >> 16);
        }

        for (obj = GetFirstRenderObject(); obj != NULL; obj = GetNextRenderObject(obj)) {
            TileId uid;
            struct Ride *ride;

            uid.id = obj->anchor.id;
            tile = *obj;
            if ((tile.flags & 0x200) && (tile.flags & 4) == 0) {
                struct MapMarker *marker;

                pt.x = (tile.field_4 & ~7) + 4;
                pt.y = (tile.field_5 & ~7) + 4;
                GetTileBounds(&pt, bounds);
                bounds[0] += ScrollX >> 8;
                bounds[1] += ScrollY >> 8;
                marker = &DAT_008119c0[0][0] + (tile.field_5 >> 3) * 32 + (tile.field_4 >> 3);
                marker->x = ((bounds[0] - DAT_00667c00) * sx) >> 16;
                marker->y = (((bounds[1] - DAT_00667c04) * sy) >> 16) + DAT_00667c20;
            }
            ride = tile.field_0->ride;
            if ((ride->flags & 4) == 0 && (ride->flags & 0x400) == 0) {
                if (ride == driving_school_roads->ride) {
                    struct RideQueueEntry *entry;

                    pt.x = uid.pos.x;
                    pt.y = uid.pos.y;
                    entry = FUN_004125f0(pt.x, pt.y);
                    if (entry != NULL && (entry->field_14 & 0xf) == 5) {
                        GetTileBounds(&pt, bounds);
                        bounds[0] = bounds[0] - 0x33 + (ScrollX >> 8);
                        bounds[1] = bounds[1] - 0x2c + (ScrollY >> 8);
                        DAT_00667c16 = lights->width;
                        DAT_00667c14 = lights->height;
                        PrintScaledSprite(lights, ((bounds[0] - DAT_00667c00) * sx) >> 16,
                            (((bounds[1] - DAT_00667c04) * sy) >> 16) + DAT_00667c20,
                            ((short)lights->width * sx) >> 16, ((short)lights->height * sy) >> 16);
                    }
                }
            } else if (ride->element == square_track || ride->element == square_track_height ||
                ride->element == square_track_height_0 || ride->element == square_track_height_path ||
                ride->element == castle_dummy) {
                struct Point pos;
                struct Point dest;
                RideSpriteInfo loc;
                float f1;
                float f2;
                int flag;
                int t;

                pos.x = tile.field_4;
                pos.y = tile.field_5;
                if (FUN_00424050(&pos, &f1, &dest, &f2, &flag)) {
                    int x1;
                    int y1;
                    int x2;
                    int y2;
                    HDC hdc;
                    HGDIOBJ old;

                    if (f1 < 0.0f) {
                        f1 = 0.0f;
                    }
                    if (f2 < 0.0f) {
                        f2 = 0.0f;
                    }
                    pt.x = pos.x;
                    pt.y = pos.y;
                    GetTileBounds(&pt, bounds);
                    t = (int)f1;
                    bounds[1] += HALF(-t);
                    bounds[0] += ScrollX >> 8;
                    bounds[1] += ScrollY >> 8;
                    x1 = ((bounds[0] - DAT_00667c00) * sx) >> 16;
                    y1 = (((bounds[1] - DAT_00667c04) * sy) >> 16) + DAT_00667c20;
                    pt.x = dest.x;
                    pt.y = dest.y;
                    GetTileBounds(&pt, bounds);
                    t = (int)f2;
                    bounds[1] += HALF(-t);
                    bounds[0] += ScrollX >> 8;
                    bounds[1] += ScrollY >> 8;
                    x2 = ((bounds[0] - DAT_00667c00) * sx) >> 16;
                    y2 = (((bounds[1] - DAT_00667c04) * sy) >> 16) + DAT_00667c20;
                    if (ride->element != square_track_height_path) {
                        DAT_00667c16 = stick->width;
                        DAT_00667c14 = (short)(int)((float)sy * f1 * 7.62939453125e-06f);
                        if (DAT_00667c14 > 0) {
                            PrintScaledSprite(stick, x1 - (((short)DAT_00667c16 * sx) >> 17), y1,
                                ((short)DAT_00667c16 * sx) >> 16, DAT_00667c14);
                        }
                    }
                    PushRenderingStatusAndUnlockVideoSurface();
                    renderEngine->lpVtbl->GetDC(renderEngine, &hdc);
                    old = SelectObject(hdc, pen);
                    MoveToEx(hdc, x2, y2, NULL);
                    LineTo(hdc, x1, y1);
                    if (flag != 0) {
                        pt.x = pos.x - 10;
                        pt.y = pos.y;
                        GetTileBounds(&pt, bounds);
                        t = (int)f1;
                        bounds[1] += HALF(-t);
                        bounds[0] += ScrollX >> 8;
                        bounds[1] += ScrollY >> 8;
                        LineTo(hdc, ((bounds[0] - DAT_00667c00) * sx) >> 16,
                            (((bounds[1] - DAT_00667c04) * sy) >> 16) + DAT_00667c20);
                    }
                    SelectObject(hdc, old);
                    renderEngine->lpVtbl->ReleaseDC(renderEngine, hdc);
                    PopRenderingStatus();
                }
                loc.sprite = blob;
                loc.x = 0;
                loc.y = (int)-f1;
            } else {
                RideSpriteInfo *info;
                RideSpriteInfo loc;
                struct Sprite *spr;

                if (ride->flags & 0x400) {
                    if (ride->cb_sprite == NULL) {
                        continue;
                    }
                    info = ride->cb_sprite(ride->element, uid);
                } else {
                    loc.sprite = ride->layer;
                    loc.x = ride->field_14;
                    loc.y = ride->field_18;
                    info = &loc;
                }
                if (info == NULL || info->sprite == NULL) {
                    continue;
                }
                spr = (struct Sprite *)info->sprite;
                if ((spr->flags & 0x8000) == 0) {
                    pt.x = uid.pos.x;
                    pt.y = uid.pos.y;
                    GetTileBounds(&pt, bounds);
                    bounds[0] = bounds[0] + HALF(info->x) + (ScrollX >> 8);
                    bounds[1] = bounds[1] + HALF(info->y) + (ScrollY >> 8);
                    DAT_00667c16 = spr->width;
                    DAT_00667c14 = spr->height;
                    PrintScaledSprite(spr, ((bounds[0] - DAT_00667c00) * sx) >> 16,
                        (((bounds[1] - DAT_00667c04) * sy) >> 16) + DAT_00667c20,
                        ((short)spr->width * sx) >> 16, ((short)spr->height * sy) >> 16);
                } else {
                    int i;

                    for (i = 0; i < spr->group->count; i++) {
                        struct Sprite *sub;
                        int hx;
                        int hy;

                        sub = spr->group->subs[i];
                        hx = HALF(spr->group->xoffs[i]);
                        hy = HALF(spr->group->yoffs[i]);
                        pt.x = uid.pos.x;
                        pt.y = uid.pos.y;
                        GetTileBounds(&pt, bounds);
                        bounds[0] = bounds[0] + HALF(info->x) + hx + (ScrollX >> 8);
                        bounds[1] = bounds[1] + HALF(info->y) + hy + (ScrollY >> 8);
                        DAT_00667c16 = sub->width;
                        DAT_00667c14 = sub->height;
                        PrintScaledSprite(sub, ((bounds[0] - DAT_00667c00) * sx) >> 16,
                            (((bounds[1] - DAT_00667c04) * sy) >> 16) + DAT_00667c20,
                            ((short)sub->width * sx) >> 16, ((short)sub->height * sy) >> 16);
                    }
                }
            }
        }
        PopRenderingStatus();
        lpConfig->field_20 = saved_x;
        lpConfig->field_22 = saved_y;
        RestoreClipping();
        CommitCliprectToHardware();
        if (blob != NULL) {
            KillSprite(blob);
        }
        if (stick != NULL) {
            KillSprite(stick);
        }
        if (lights != NULL) {
            KillSprite(lights);
        }
        DeleteObject(pen);
        CalculateMapRenderOrder();
    }
}
