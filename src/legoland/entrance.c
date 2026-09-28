#include "globals.h"
#include "legoland.h"

#include "entrance.h"
#include "money.h"
#include "sound_music.h"

#include "image_sprite.h"
#include "llidb.h"
#include "map_object.h"
#include "obj_instance.h"

// FUNCTION: LEGOLAND 0x0042d970
void FUN_0042d970(TileId *tile, unsigned int arg) {
    struct SampleParams config;

    config.field_0 = 1;
    config.field_4 = arg;
    PlayInstanceOfSample(*(void **)(ENTRANCE_SFX + 8), 0, 1, &config);
    if (MapStats.field_170 != 0) {
        PlayMoneySFX(tile, 1, 0);
    }
}

// FUNCTION: LEGOLAND 0x0042d9c0
void FUN_0042d9c0(void) { STUB(); }

// FUNCTION: LEGOLAND 0x0042de50
void FUN_0042de50(Element *param) {
    Load_FXList(ENTRANCE_SFX, 1);
    LoadMoneySFX();
    DAT_006160f4 = param->ride;
    DAT_006160f4->flags |= 0x20;
    DAT_006160f0 = DAT_006160f4->layer;
    DAT_006160f0->flags |= 0x2000;
    // STRING: LEGOLAND 0x004b66cc
    DAT_006160fc = LoadSprite("entrance_matte1.lls", 1);
    // STRING: LEGOLAND 0x004b66b8
    DAT_00616100 = LoadSprite("entrance_matte2.lls", 1);
    // STRING: LEGOLAND 0x004b66a4
    DAT_00616104 = LoadSprite("entrance_matte3.lls", 1);
    // STRING: LEGOLAND 0x004b6690
    DAT_00616108 = LoadSprite("entrance_matte4.lls", 1);
    // STRING: LEGOLAND 0x004b6684
    DAT_0061610c = LoadSprite("booth1.lls", 1);
}

// FUNCTION: LEGOLAND 0x0042def0
void FUN_0042def0(Element *param) {
    Kill_FXList(ENTRANCE_SFX, 1);
    KillMoneySFX();
    DAT_006160f4 = param->ride;
    if (DAT_0061610c != 0) {
        KillSprite(DAT_0061610c);
    }
    if (DAT_006160fc != 0) {
        KillSprite(DAT_006160fc);
    }
    if (DAT_00616100 != 0) {
        KillSprite(DAT_00616100);
    }
    if (DAT_00616104 != 0) {
        KillSprite(DAT_00616104);
    }
    if (DAT_00616108 != 0) {
        KillSprite(DAT_00616108);
    }
}

// FUNCTION: LEGOLAND 0x0042df70
void FUN_0042df70(Element *obj, TileId tile, struct Cursor *cursor) {
    StandardRemoveObject(obj, tile, cursor);
    RemoveAllBlokesFromRide(obj->ride, tile);
}

// FUNCTION: LEGOLAND 0x0042dfa0
void FUN_0042dfa0(void) { STUB(); }
