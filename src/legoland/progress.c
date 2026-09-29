#include "progress.h"
#include "draw.h"
#include "freeplay.h"
#include "globals.h"
#include "icon.h"
#include "interface.h"
#include "legoland.h"
#include "options.h"
#include "screens.h"
#include "sound_music.h"

#include "image_sprite.h"
#include "stream.h"
#include "timer.h"

// FUNCTION: LEGOLAND 0x0048b7e0
LEGO_EXPORT void InitProgressScreen(void) { STUB(); }

// FUNCTION: LEGOLAND 0x0048bb60
unsigned char FUN_0048bb60(unsigned char *arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3) {
    DAT_00798660 = 1;
    if ((arg1 & 2) != 0) {
        if ((int)(GetTicks() - DAT_0079866c) < 500 && arg0[0x18] == DAT_004bec98) {
            if (DAT_00798664 != 0) {
                return FUN_0048bf90(arg0, arg1, arg2, arg3);
            }
            return FUN_0048bc20(arg0, arg1, arg2, arg3);
        }
        DAT_0079866c = GetTicks();
        DAT_004bec98 = arg0[0x18];
        PlayInstanceOfSample(PTR_004b92c0, 0, 1, 0);
        lpConfig->field_28 = arg0[0x18] + 1;
        DAT_0080ff80.unk4 = 0xffffffff;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048bc20
unsigned char FUN_0048bc20(unsigned char *arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3) {
    if ((arg1 & 2) != 0) {
        DAT_006687c0 = 0;
        DAT_006687bc = 0;
        FUN_00498920();
        DAT_006687b0 = 4;
        PlayInstanceOfSample(PTR_004b92c0, 0, 1, 0);
        FUN_0048b770();
        if ((int)lpConfig->field_28 <= 0xf) {
            FUN_00466360(0xfa, 0x181);
            DAT_00668e38 = 0;
            InitGameInterface(1);
            EditMode.unk4 = 3;
            FUN_00474880();
            FUN_00458a50();
            FUN_004663c0();
            DAT_00798660 = 0;
            DAT_00798668 = 0;
            return 1;
        }
        DAT_00798660 = 0;
        GamePad &= ~0x20;
        DAT_00798668 = 0;
        EditMode.unk4 = 2;
        DAT_0080ff80.unk4 = 0xffffffff;
        DAT_0080ff80.unk8 = 8;
        DAT_00668e38 = 1;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048bd00
void FUN_0048bd00(void) {
    struct FreePlaySpriteSlot *slot;

    for (slot = DAT_004becb4; (int)slot < (int)&DAT_004bed40; slot++) {
        slot->field_0 = LoadSprite(((const char **)slot)[-5], 4);
        slot->field_4 = LoadSprite(((const char **)slot)[-4], 4);
    }
}
// FUNCTION: LEGOLAND 0x0048bd40
void FUN_0048bd40(void) {
    int *esi;

    esi = (int *)&DAT_004becb4[0].field_4;
    do {
        ReferenceSprite((struct Sprite *)esi[-1]);
        ReferenceSprite((struct Sprite *)esi[0]);
        esi += 7;
    } while ((int)esi < (int)&DAT_004bed44);
}
// FUNCTION: LEGOLAND 0x0048bd70
void FUN_0048bd70(void) {
    struct FreePlaySpriteSlot *slot;

    RemoveIconGroup(0x1c);
    RemoveIconGroup(0x23);
    slot = DAT_004becb4;
    while ((int)slot < (int)&DAT_004bed40) {
        while (KillSprite(slot->field_0) == 0) {
        }
        while (KillSprite(slot->field_4) == 0) {
        }
        slot->field_4 = NULL;
        slot->field_0 = NULL;
        slot++;
    }
}

// FUNCTION: LEGOLAND 0x0048bde0
void FUN_0048bde0(void) { STUB(); }

// FUNCTION: LEGOLAND 0x0048bf90
unsigned char FUN_0048bf90(unsigned char *arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3) {
    if ((arg1 & 2) != 0) {
        DAT_006687c0 = 0;
        DAT_006687bc = 0;
        FUN_00498920();
        DAT_006687b0 = 4;
        PlayInstanceOfSample(PTR_004b92c0, 0, 1, 0);
        FUN_0048bd70();
        FUN_00466360(0x186, 0x18b);
        DAT_00668e38 = 0;
        InitGameInterface(1);
        EditMode.unk4 = 3;
        FUN_00474880();
        FUN_00458a50();
        FUN_004663c0();
        DAT_00798660 = 0;
        DAT_00798668 = 0;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048c020
unsigned char FUN_0048c020(unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4) {
    unsigned int temp;

    if ((a2 & 2) != 0) {
        DAT_006687c0 = 0;
        DAT_006687bc = 0;
        PlayInstanceOfSample(PTR_004b92c0, 0, 1, 0);
        temp = DAT_00798664;
        DAT_00798660 = 0;
        DAT_00798668 = 0;
        if (temp != 0) {
            FUN_0048bd70();
        } else {
            FUN_0048b770();
        }
        return FUN_0048fb80(a1, a2, a3, a4);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048c090
unsigned char FUN_0048c090(void *param1, unsigned char param2) {
    if (param2 & 2) {
        DAT_00798660 = 0;
        DAT_00798668 = 1;
        DAT_0080ff80.unk4 = 0xffffffff;
        PlayInstanceOfSample(PTR_004b92c0, 0, 1, 0);
        if (DAT_00798664 != 0) {
            FUN_0048bd70();
            lpConfig->field_28 = 6;
        } else {
            FUN_0048b770();
            lpConfig->field_28 = 1;
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048c100
void FUN_0048c100(void) {
    RECT rc;
    unsigned int *mapping;
    int *entry;
    int i;
    char *text;

    if (DAT_00798664 != 0) {
        rc.top = 0x45;
        rc.bottom = 0x6c;
        rc.left = 10;
        rc.right = 0x1d6;
        NewPrintCent(GetString(0x28a), 3, rc, 0);
        i = 0;
        mapping = (unsigned int *)0x7cb380;
        entry = (int *)&DAT_004becac;
        do {
            text = GetString(entry[-1]);
            rc.left = entry[0] + 0x32;
            rc.top = entry[1] + 6;
            rc.right = rc.left + 0x190;
            rc.bottom = rc.top + 0x16;
            if (i == (int)lpConfig->field_28 - 1) {
                FUN_00454d80(text, 2, rc, 0);
            } else if (DAT_0080ffd4[i] == 1) {
                FUN_00454d80(text, 2, rc, 0x323232);
            } else {
                FUN_00454d80(text, 2, rc, 0xa0a0a0);
            }
            if (DAT_0080ffd4[i] == 1 && DAT_00813a44.x >= rc.left && DAT_00813a44.x < rc.right && DAT_00813a44.y >= rc.top && DAT_00813a44.y < rc.bottom) {
                DAT_004bdd00 = 2;
                DAT_004bdd04 = (struct Bloke *)*mapping;
            }
            i++;
            entry += 7;
            mapping++;
        } while ((int)entry < (int)&DAT_004becb4[4].pad_8[12]);
    }
}
