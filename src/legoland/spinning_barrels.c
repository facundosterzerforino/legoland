#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "legoland.h"

#include "binv.h"
#include "bloke.h"
#include "gamemap.h"
#include "llidb.h"
#include "man3d.h"
#include "map_object.h"
#include "obj_instance.h"
#include "objclass.h"
#include "print_sprite.h"
#include "render3d.h"
#include "spinning_barrels.h"

struct BarrelSource {
    unsigned short field_0;
};

struct BarrelCarNode {
    /* 0x00 */ unsigned char pad_0[0x10];
    /* 0x10 */ unsigned int field_10;
    /* 0x14 */ unsigned int field_14;
    /* 0x18 */ unsigned int field_18;
    /* 0x1c */ unsigned char pad_1c[0x48];
    /* 0x64 */ struct BarrelCarNode *next;
};

struct BarrelRoot {
    /* 0x00 */ unsigned char pad_0[0xc];
    /* 0x0c */ struct BarrelCarNode *car;
};

#include "image_sprite.h"

// FUNCTION: LEGOLAND 0x0043bdb0
void FUN_0043bdb0(void *param1) {
    struct BarrelNode *block = (struct BarrelNode *)malloc(0x34);
    if (block == NULL) {
        return;
    }
    memset(block, 0, 0x34);
    block->field_4 = ((struct BarrelSource *)param1)->field_0;
    block->field_8 = 0;
    block->next = DAT_0062fe08;
    DAT_0062fe08 = block;
    FUN_0043c2f0(block);
}

// FUNCTION: LEGOLAND 0x0043be00
void FUN_0043be00(struct BarrelNode *node) {
    struct BarrelNode *prev;
    struct BarrelNode *cur;

    if (DAT_0062fe08 == node) {
        DAT_0062fe08 = node->next;
    } else {
        cur = DAT_0062fe08->next;
        prev = DAT_0062fe08;
        while (cur != node) {
            prev = prev->next;
            if (prev == NULL) {
                break;
            }
            cur = prev->next;
        }
        if (prev != NULL) {
            prev->next = node->next;
        }
    }
    free(node);
}

// FUNCTION: LEGOLAND 0x0043be40
struct BarrelNode *FUN_0043be40(unsigned short *key) {
    struct BarrelNode *cur = DAT_0062fe08;

    if (cur != NULL) {
        do {
            if (memcmp(&cur->field_4, key, 2) == 0) {
                return cur;
            }
            cur = cur->next;
        } while (cur != NULL);
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x0043be70
void FUN_0043be70(Element *obj, void *param_2, void *param_3, TileId *tile) {
    Ride *ride = obj->ride;
    RideNode *elem = ride->riders;
    RideNode *riders;
    char count = 0;
    Bloke *blokes[16] = {0};
    Bloke *bloke;
    Person *person;
    struct BarrelNode *state;
    Point screen;
    Point off;
    Point off2;
    Point seat;
    LayerResult layer;
    char i;

    state = FUN_0043be40(&tile->id);
    if (state == NULL) {
        return;
    }
    screen = GetScreenCoordsForObject(tile, ride);
    GetLayer(ride->layer, &layer, 3);
    layer.field_10 = 0;
    for (; elem != NULL; elem = elem->next) {
        if (tile->id == elem->tile.id) {
            blokes[count++] = elem->rider;
        }
    }
    if (count != 0) {
        LLSSetFrame(GetLLSForLayer(DAT_0062fde0, 3), state->field_8);
        off = GetRenderOffsetForLayer(DAT_0062fde0, 3);
        AdjustOffsetForViewMode(&off);
        PrintSprite(GetSpriteForLayer(DAT_0062fde0, 3), screen.x + off.x, screen.y + off.y, 0, 0);
        riders = ride->riders;
        (*((struct Sprite *)DAT_0062fe00[0])->lls)->frame = state->field_8;
        for (; riders != NULL; riders = riders->next) {
            if (tile->id == riders->tile.id && (riders->rider->flags & 0x80) != 0) {
                bloke = riders->rider;
                person = bloke->person;
                seat.x = DAT_0062fdd8;
                seat.y = DAT_0062fddc;
                person->offset.x = bloke->screen_x;
                person->offset.y = bloke->screen_y;
                AdjustBlokePosition(&person->offset);
                AdjustOffsetForViewMode(&seat);
                person->screen.x = bloke->screen_x + seat.x + screen.x;
                person->screen.y = bloke->screen_y + seat.y + screen.y;
                AdjustBlokePosition(&person->screen);
                IP_RenderBlokeIn3DNow(riders->rider);
            }
        }
        for (i = 0; i < count; i++) {
            if (blokes[i]->param_action == 0) {
                IP_RenderBlokeIn3DNow(blokes[i]);
            }
        }
        for (i = 0; i < count; i++) {
            if (blokes[i]->param_action == 1) {
                IP_RenderBlokeIn3DNow(blokes[i]);
            }
        }
        for (i = 0; i < count; i++) {
            if (blokes[i]->param_action == 15) {
                IP_RenderBlokeIn3DNow(blokes[i]);
            }
        }
        off2 = GetRenderOffsetForLayer(DAT_0062fde0, 1);
        AdjustOffsetForViewMode(&off2);
        PrintSprite(DAT_0062fdd0, screen.x + off2.x, screen.y + off2.y, 0, 0);
        LLSSetFrame(GetLLSForLayer(DAT_0062fde0, 2), state->field_20);
        off2 = GetRenderOffsetForLayer(DAT_0062fde0, 2);
        AdjustOffsetForViewMode(&off2);
        PrintSprite(GetSpriteForLayer(DAT_0062fde0, 2), screen.x + off2.x, screen.y + off2.y, 0, 0);
        for (i = 0; i < count; i++) {
            if (blokes[i]->param_action == 16) {
                IP_RenderBlokeIn3DNow(blokes[i]);
            }
        }
        for (i = 0; i < count; i++) {
            if (blokes[i]->param_action == 17) {
                IP_RenderBlokeIn3DNow(blokes[i]);
            }
        }
        off2 = GetRenderOffsetForLayer(DAT_0062fde0, 1);
        AdjustOffsetForViewMode(&off2);
        PrintSprite(DAT_0062fdcc, screen.x + off2.x, screen.y + off2.y, 0, 0);
    } else {
        LLSSetFrame(GetLLSForLayer(DAT_0062fde0, 3), state->field_8);
        off = GetRenderOffsetForLayer(DAT_0062fde0, 3);
        AdjustOffsetForViewMode(&off);
        PrintSprite(GetSpriteForLayer(DAT_0062fde0, 3), screen.x + off.x, screen.y + off.y, 0, 0);
        LLSSetFrame(GetLLSForLayer(DAT_0062fde0, 2), state->field_20);
        off = GetRenderOffsetForLayer(DAT_0062fde0, 2);
        AdjustOffsetForViewMode(&off);
        PrintSprite(GetSpriteForLayer(DAT_0062fde0, 2), screen.x + off.x, screen.y + off.y, 0, 0);
    }
}

// FUNCTION: LEGOLAND 0x0043c2f0
void FUN_0043c2f0(struct BarrelNode *node) {
    node->field_14 = 0;
    node->field_8 = 0;
    node->field_18 = 0;
    node->field_6 = 0;
    node->field_c = node->field_c & 0xffffbffe;
    node->field_10 = 3;
}

// FUNCTION: LEGOLAND 0x0043c320
void FUN_0043c320(struct BarrelNode *node) {
    unsigned int packed = node->field_c;
    unsigned char prev = node->field_6;
    packed &= 0xffffbfff;
    node->field_7 = prev;
    packed |= 0x1;
    node->field_6 = 0;
    node->field_c = packed;
}

// FUNCTION: LEGOLAND 0x0043c340
void FUN_0043c340(struct Element *elem) {
    struct LayerResult layer;

    DAT_0062fde4 = elem->ride;
    DAT_0062fde4->flags |= 0x420;
    DAT_0062fde0 = DAT_0062fde4->layer;
    DAT_0062fde0->flags |= 0x2000;
    // STRING: LEGOLAND 0x004b7960
    DAT_0062fe04 = DAT_0062fe00[0] = LoadSprite("z_SpinningBarrels.lls", 1);
    // STRING: LEGOLAND 0x004b7944
    DAT_0062fdfc = DAT_0062fdf0[0] = LoadBinV("Zbuffers\\BoxBlokesOn1.bnv");
    // STRING: LEGOLAND 0x004b7924
    DAT_0062fde8 = DAT_0062fdf0[1] = LoadBinV("Zbuffers\\SpinningBarrels.bnv");
    // STRING: LEGOLAND 0x004b7908
    DAT_0062fdc8 = DAT_0062fdf0[2] = LoadBinV("Zbuffers\\BoxBlokesOff.bnv");
    HideLayer(DAT_0062fde0, 3);
    StopLayerPlaying(DAT_0062fde0, 3);
    LLSSetFrame(GetLLSForLayer(DAT_0062fde0, 3), 0);
    HideLayer(DAT_0062fde0, 2);
    StopLayerPlaying(DAT_0062fde0, 2);
    LLSSetFrame(GetLLSForLayer(DAT_0062fde0, 2), 0);
    GetLayer(DAT_0062fde4->layer, &layer, 3);
    DAT_0062fdd8 = layer.x - 103;
    DAT_0062fddc = layer.y - 55;
    // STRING: LEGOLAND 0x004b78e4
    DAT_0062fdd0 = LoadSprite("SpinningBarrelsEntranceMatte.lls", 1);
    // STRING: LEGOLAND 0x004b78c0
    DAT_0062fdcc = LoadSprite("SpinningBarrelsEntranceMatte2.lls", 1);
}

// FUNCTION: LEGOLAND 0x0043c490
void FUN_0043c490(void) {
    EditMode.unk0 = 1;
    EditMode.unk8 = DAT_0062fde4;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint((unsigned int *)((unsigned int)EditMode.unk8 + 0x3c));
}

// FUNCTION: LEGOLAND 0x0043c4d0
void FUN_0043c4d0(void) {
    while (DAT_0062fe08 != NULL) {
        FUN_0043be00(DAT_0062fe08);
    }
}

// FUNCTION: LEGOLAND 0x0043c4f0
void FUN_0043c4f0(Element *editObj, TileId tile, struct Cursor *cursor) {
    struct BarrelNode *node = FUN_0043be40(&tile.id);
    if (node != NULL) {
        FUN_0043be00(node);
    }
    StandardRemoveObject(editObj, tile, cursor);
    RemoveAllBlokesFromRide(editObj->ride, tile);
}

// FUNCTION: LEGOLAND 0x0043c540
void FUN_0043c540(Element *editObj, int *coords) {
    TileId tile;
    tile.pos.x = (unsigned char)coords[0];
    tile.pos.y = (unsigned char)coords[1];
    AddBasicObject(editObj, coords);
    FUN_0043bdb0(&tile);
}

// FUNCTION: LEGOLAND 0x0043c570
unsigned int *FUN_0043c570(struct BarrelRoot *ride, unsigned short param2) {
    struct BarrelCarNode *target = ride->car;

    DAT_0062fdb0 = (unsigned int)target->next;
    DAT_0062fdb4 = target->field_14;
    DAT_0062fdb8 = target->field_18;
    DAT_0062fdbc = param2;

    target = target->next;
    target->field_10 |= 0x2000;
    return &DAT_0062fdb0;
}

// FUNCTION: LEGOLAND 0x0043c5b0
void FUN_0043c5b0(void) {
    KillSprite(DAT_0062fdd0);
    KillSprite(DAT_0062fdcc);
    KillSprite(DAT_0062fe04);
    if (DAT_0062fde8 != NULL) {
        FreeBinV(DAT_0062fde8);
    }
    if (DAT_0062fdfc != NULL) {
        FreeBinV(DAT_0062fdfc);
    }
    if (DAT_0062fdc8 != NULL) {
        FreeBinV(DAT_0062fdc8);
    }
    FUN_0043c4d0();
}

// FUNCTION: LEGOLAND 0x0043c620
LEGO_EXPORT int SaveSBarrel(void) {
    struct BarrelNode *node = DAT_0062fe08;
    unsigned int marker = 1;
    unsigned int terminator = 0;

    if (node != NULL) {
        do {
            if (SaveGameWrite(&marker, 4)) {
                if (SaveGameWrite(node, 0x34)) {
                    node = node->next;
                } else {
                    return 0;
                }
            } else {
                return 0;
            }
        } while (node != NULL);
    }

    return SaveGameWrite(&terminator, 4) != 0;
}

struct BarrelTypeC {
    void *field_0;
    unsigned int field_4;
};

struct BarrelData {
    unsigned char pad_0[0x54];
    struct BarrelTypeC *field_54;
};

struct BarrelCar {
    unsigned char pad_0[0x2c];
    void *field_2c;
    unsigned int field_30;
};

struct BarrelListNode {
    struct BarrelListNode *next;
    unsigned char pad_4[4];
    struct BarrelData *field_8;
    unsigned char pad_c[4];
    struct BarrelCar *field_10;
};

struct BarrelGameObject {
    unsigned char pad_0[0xcc];
    struct BarrelListNode *field_cc;
};

struct BarrelLoadArg {
    unsigned char pad_0[0xc];
    struct BarrelGameObject *field_c;
};

// FUNCTION: LEGOLAND 0x0043c690
LEGO_EXPORT int LoadSBarrel(struct BarrelLoadArg *arg) {
    struct BarrelGameObject *obj = arg->field_c;
    struct BarrelNode *prev = NULL;
    struct BarrelListNode *list;
    struct BarrelCar *car;
    struct BarrelData *data;
    struct BarrelTypeC *tc;
    unsigned int marker;

    if (!SaveGameRead(&marker, 4)) {
        return 0;
    }
    while (marker != 0) {
        struct BarrelNode *node = (struct BarrelNode *)malloc(0x34);
        if (!SaveGameRead(node, 0x34)) {
            return 0;
        }
        node->next = NULL;
        if (prev != NULL) {
            prev->next = node;
        } else {
            DAT_0062fe08 = node;
        }
        prev = node;
        if (!SaveGameRead(&marker, 4)) {
            return 0;
        }
    }

    list = obj->field_cc;
    while (list != NULL) {
        car = list->field_10;
        if (car->field_30 != 0) {
            car->field_2c = DAT_0062fe00[car->field_30];
        } else {
            car->field_2c = NULL;
            list->field_10->field_30 = 0;
        }
        data = list->field_8;
        tc = data->field_54;
        if (tc != NULL) {
            tc->field_0 = DAT_0062fdf0[tc->field_4];
        }
        list = list->next;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0043c760
void FUN_0043c760(struct ClassNode *str, struct CallbackTable *ride) {
    // STRING: LEGOLAND 0x004b7978
    if (_stricmp("SPINNING BARRELS RIDE", str->name) == 0) {
        ride->cb_a4 = FUN_0043c340;
        ride->cb_8c = FUN_0043c490;
        ride->cb_a8 = FUN_0043c950;
        ride->cb_b0 = FUN_0043be70;
        ride->cb_9c = FUN_0043c4f0;
        ride->cb_98 = FUN_0043c540;
        ride->cb_ac = FUN_0043c5b0;
        ride->cb_a0 = FUN_0043c570;
        ride->cb_bc = SaveSBarrel;
        ride->cb_b8 = LoadSBarrel;
    }
}

// FUNCTION: LEGOLAND 0x0043c7f0
void FUN_0043c7f0(struct BarrelNode *node) {
    struct RideNode *r = DAT_0062fde4->riders;
    unsigned int flags;

    node->field_20++;
    if (node->field_20 >= 0x20) {
        node->field_20 = 0;
    }
    flags = node->field_c;
    if (flags & 1) {
        unsigned char c;
        int v = ++node->field_14;
        c = node->field_10;
        if (c == 0) {
            if (GetAllBlokesOffRide(DAT_0062fde4, node->field_4) == 0) {
                return;
            }
            FUN_0043c2f0(node);
            return;
        }
        if (v > 2) {
            node->field_14 = 0;
            node->field_8++;
            if (node->field_8 >= 0x40) {
                node->field_8 = 0;
                node->field_10 = c - 1;
            }
        }
    } else if (flags & 0x4000) {
        if (node->field_6 == node->field_18) {
            node->field_c = flags & 0xffffbfff;
            FUN_0043c320(node);
            return;
        }
    } else if (node->field_6 != 0) {
        if (node->field_1c == 0) {
            node->field_c = flags | 0x4000;
            Ride_SetFlagToNotLetAnyoneOn(&node->field_4);
        } else {
            node->field_1c--;
        }
    }
    for (; r != NULL; r = r->next) {
        if (node->field_4 == r->tile.id && r->rider->field_35 == 1) {
            sprintf(&DAT_004b78b4[8], "%02d", r->rider->field_36);
            SetBlokePositionFromBNV(DAT_0062fde8, r->rider, DAT_004b78b4, node->field_8, -1617922.25f, -1618065.75f, 0);
        }
    }
    *(short *)*((struct Sprite *)DAT_0062fe00[0])->lls = (short)node->field_8;
}

// FUNCTION: LEGOLAND 0x0043c930
void FUN_0043c930(void) {
    struct BarrelNode *node = DAT_0062fe08;
    while (node != NULL) {
        FUN_0043c7f0(node);
        node = node->next;
    }
}

// FUNCTION: LEGOLAND 0x0043c950
void FUN_0043c950(void) { STUB(); }

// FUNCTION: LEGOLAND 0x0043ce10
unsigned char FUN_0043ce10(int param1, int param2, signed char param3) {
    int rnd;
    int idx;

    rnd = rand();
    idx = rnd % param3;

    while (((unsigned char *)param2)[idx + 0x21] != 0) {
        idx += 1;
        if (idx >= param3) {
            idx = 0;
        }
    }

    ((unsigned char *)param2)[idx + 0x21] = 1;
    *(unsigned char *)(*(int *)((unsigned char *)param1 + 0x8) + 0x36) = (unsigned char)(idx + 1);
    return (unsigned char)(idx + 1);
}
