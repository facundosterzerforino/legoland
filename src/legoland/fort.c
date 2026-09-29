#include <stdlib.h>
#include <string.h>
#include "legoland.h"

#include "bloke.h"
#include "fort.h"
#include "gamemap.h"
#include "globals.h"
#include "map_object.h"
#include "math.h"
#include "obj_instance.h"
#include "objclass.h"

#include "image_sprite.h"

// FUNCTION: LEGOLAND 0x00406240
void FUN_00406240(Element *elem) {
    struct Ride *ride = elem->ride;
    DAT_004c11dc = ride;
    if (ride != NULL) {
        ride->flags |= 0x20;
        if (DAT_004c11dc->layer != NULL) {
            DAT_004c11dc->layer->flags |= 0x2000;
            DAT_004c11d8 = DAT_004c11dc->layer;
        }
    }
    // STRING: LEGOLAND 0x004b4590
    DAT_004c11cc = LoadSprite("fortmask.lls", 1);
}

// FUNCTION: LEGOLAND 0x004062a0
void FUN_004062a0(void) {
    struct Sprite *sprite = DAT_004c11cc;
    if (sprite != NULL) {
        KillSprite(sprite);
    }
}

// FUNCTION: LEGOLAND 0x004062c0
void FUN_004062c0(void) { STUB(); }

// FUNCTION: LEGOLAND 0x004064d0
void FUN_004064d0(RideNode *node, Bloke *bloke) {
    int h, r;
    float fx, fy;
    char dir;

    switch ((short)bloke->field_40) {
    case 1:
        if (--bloke->field_58 <= 0) {
            bloke->field_40 = bloke->field_42;
        }
        break;
    case 2:
        r = (DAT_004b4580.right - DAT_004b4580.left) << 8;
        h = (DAT_004b4580.bottom - DAT_004b4580.top) << 8;
        fx = (rand() & 0xff) * 0.003921569f;
        fy = (rand() & 0xff) * 0.003921569f;
        bloke->dest.x = (int)(((node->tile.pos.x + DAT_004b4580.left) << 8) + r * fx);
        bloke->dest.y = (int)(((node->tile.pos.y + DAT_004b4580.top) << 8) + h * fy);
        dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
        bloke->field_e = 7;
        bloke->field_73 = dir + 0x10;
        NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
        bloke->field_40 = 3;
        break;
    case 3:
        r = rand() & 3;
        if (r == 0 || r == 1) {
            bloke->field_72 = rand() & 0xf;
            bloke->field_58 = (rand() & 0xf) + 3;
            bloke->field_40 = 1;
            bloke->field_42 = 3;
        }
        if (r == 2) {
            bloke->field_40 = r;
            return;
        }
        if (r == 3) {
            bloke->param_action++;
        }
        break;
    }
}

// FUNCTION: LEGOLAND 0x00406660
void FUN_00406660(void) { STUB(); }

// FUNCTION: LEGOLAND 0x00406820
void FUN_00406820(void) {
    EditMode.unk8 = DAT_004c11dc;
    EditMode.unk0 = 1;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint(&((struct EditCursorData *)EditMode.unk8)->field_3c);
}

// FUNCTION: LEGOLAND 0x00406860
unsigned int FUN_00406860(unsigned int param1, unsigned int param2) {
    return AddBasicObject(param1, param2);
}

// FUNCTION: LEGOLAND 0x00406880
void FUN_00406880(Element *elem, TileId tile, struct Cursor *cursor) {
    StandardRemoveObject(elem, tile, cursor);
    RemoveAllBlokesFromRide(elem->ride, tile);
}

// FUNCTION: LEGOLAND 0x004068b0
void FUN_004068b0(struct ClassNode *name, struct CallbackTable *ci) {
    // STRING: LEGOLAND 0x004b45a0
    if (_stricmp("FORT", name->name) != 0) {
        return;
    }
    ci->cb_a4 = FUN_00406240;
    ci->cb_ac = FUN_004062a0;
    ci->cb_8c = FUN_00406820;
    ci->cb_a8 = FUN_00406660;
    ci->cb_b0 = FUN_004062c0;
    ci->cb_9c = FUN_00406880;
    ci->cb_98 = FUN_00406860;
}
