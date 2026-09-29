#include "globals.h"
#include "legoland.h"

#include "clipping.h"
#include "icon.h"
#include "profile.h"
#include "profile_io.h"
#include "savegame_ui.h"
#include "sound_music.h"
#include "sound_sfx.h"
#include "string.h"
#include "text.h"
#include "title.h"

struct ProfileFlags {
    unsigned char pad_0[0x34];
    unsigned int var_34;
};

struct Profile {
    unsigned char pad_0[0x1c];
    unsigned char var_1c;
};

#include "image_sprite.h"
#include "stream.h"

// FUNCTION: LEGOLAND 0x0048c260
LEGO_EXPORT void InitListProfiles(void) {
    struct ProfileNode *node;
    struct IconNode *icon;
    char *str;

    UpdateSoundVols();
    DeleteProfileList();
    LoadProfilesFormDisk();
    node = (struct ProfileNode *)DAT_00798890;
    // STRING: LEGOLAND 0x004bf124
    SPRITE_TitleScreenBk = LoadSprite("Reg_ScreenBK.lls", 0);
    // STRING: LEGOLAND 0x004bf114
    DAT_0079868c = LoadSprite("RegDeleteOn.lls", 4);
    // STRING: LEGOLAND 0x004bf104
    DAT_00798690 = LoadSprite("RegDelete.lls", 4);
    // STRING: LEGOLAND 0x004bf0f0
    DAT_007986b4 = LoadSprite("RegProfileON.lls", 4);
    // STRING: LEGOLAND 0x004bf0dc
    DAT_00798694 = LoadSprite("RegProfileOff_1.lls", 4);
    // STRING: LEGOLAND 0x004bf0c8
    DAT_00798698 = LoadSprite("RegProfileOff_2.lls", 4);
    // STRING: LEGOLAND 0x004bf0b4
    DAT_0079869c = LoadSprite("RegProfileOff_3.lls", 4);
    // STRING: LEGOLAND 0x004bf0a0
    DAT_007986a0 = LoadSprite("RegProfileOff_4.lls", 4);
    // STRING: LEGOLAND 0x004bf08c
    DAT_007986a4 = LoadSprite("RegProfileOff_5.lls", 4);
    // STRING: LEGOLAND 0x004bf078
    DAT_007986a8 = LoadSprite("RegProfileOff_6.lls", 4);
    // STRING: LEGOLAND 0x004bf064
    DAT_007986ac = LoadSprite("RegProfileOff_7.lls", 4);
    // STRING: LEGOLAND 0x004bf050
    DAT_007986b0 = LoadSprite("RegProfileOff_8.lls", 4);
    // STRING: LEGOLAND 0x004bf038
    DAT_007986b8 = LoadSprite("Reg_Delete_PopUp.lls", 4);
    // STRING: LEGOLAND 0x004bf024
    DAT_007986bc = LoadSprite("Reg_Diff_PopUp.lls", 4);
    // STRING: LEGOLAND 0x004bf014
    DAT_007986c0 = LoadSprite("Reg_Easy_On.lls", 4);
    // STRING: LEGOLAND 0x004bf000
    DAT_007986c4 = LoadSprite("Reg_Easy_Off.lls", 4);
    // STRING: LEGOLAND 0x004beff0
    DAT_007986c8 = LoadSprite("Reg_Mid_On.lls", 4);
    // STRING: LEGOLAND 0x004befe0
    DAT_007986cc = LoadSprite("Reg_Mid_Off.lls", 4);
    // STRING: LEGOLAND 0x004befd0
    DAT_007986d0 = LoadSprite("Reg_Hard_On.lls", 4);
    // STRING: LEGOLAND 0x004befbc
    DAT_007986d4 = LoadSprite("Reg_Hard_Off.lls", 4);

    // STRING: LEGOLAND 0x004befa8
    DAT_007986e0 = (unsigned int)LoadSpriteIcon("Accept_On_Reg.lls", 4, 0x1ef, 0x14f, 7);
    ((struct IconNode *)DAT_007986e0)->string_id = 6;
    ((struct IconNode *)DAT_007986e0)->string = GetString(6);
    ((struct IconNode *)DAT_007986e0)->flags |= 0x2000;
    ((struct IconNode *)DAT_007986e0)->flags |= 0x4002;
    ((struct IconNode *)DAT_007986e0)->flags |= 0x400;
    ((struct IconNode *)DAT_007986e0)->event_handler = (void *)FUN_0048d300;
    DAT_006687bc = (unsigned int)FUN_0048d300;
    DAT_006687c0 = (unsigned int)FUN_004920a0;
    strcpy(DAT_007cb340, GetString(0x84));

    for (; node != NULL; node = node->next) {
        if (node->has_header) {
            icon = InsertIcon(0x80, node->slot * 0x26 + 0x86, 7, FUN_0048c5e0(node->slot));
            icon->string_id = 0;
            str = GetString(0);
            icon->flags |= 0x6002;
            icon->string = str;
            icon->event_handler = (void *)FUN_0048d390;
            icon->field_18p = &node->data;
            icon->slot = node->slot;
        } else {
            icon = InsertIcon(0x80, node->slot * 0x26 + 0x86, 7, FUN_0048c5e0(node->slot));
            icon->string_id = 1;
            str = GetString(1);
            icon->flags |= 0x6002;
            icon->string = str;
            icon->event_handler = (void *)FUN_0048d3c0;
            icon->field_18p = "EMPTY";
            icon->slot = node->slot;
        }
        icon->field_20b |= 1;
    }

    DAT_007cb360 = InsertIcon(0, 0, 7, DAT_00798690);
    DAT_007cb360->string_id = 2;
    DAT_007cb360->string = GetString(2);
    DAT_007cb360->flags |= 0x2000;
    DAT_007cb360->flags |= 0x4002;
    DAT_007cb360->flags |= 0x400;
    DAT_007cb360->event_handler = (void *)FUN_0048cc30;
}

// FUNCTION: LEGOLAND 0x0048c5e0
struct Sprite *FUN_0048c5e0(signed char param_1) {
    switch (param_1) {
    case 1:
        return DAT_00798694;
    case 2:
        return DAT_00798698;
    case 3:
        return DAT_0079869c;
    case 4:
        return DAT_007986a0;
    case 5:
        return DAT_007986a4;
    case 6:
        return DAT_007986a8;
    case 7:
        return DAT_007986ac;
    case 8:
        return DAT_007986b0;
    default:
        return NULL;
    }
}

// FUNCTION: LEGOLAND 0x0048c650
LEGO_EXPORT void EnterNewProfileCheckBoxIcons(struct IconNode *param_1) {
    // STRING: LEGOLAND 0x004bf148
    DAT_0079867c = LoadSprite("RegClose.lls", 4);
    // STRING: LEGOLAND 0x004bf138
    DAT_00798680 = LoadSprite("RegCloseON.lls", 4);
    DAT_00798684 = LoadSprite("PU_ClosePopUp.lls", 4);
    DAT_00798688 = LoadSprite("PU_ClosePopUpON.lls", 4);

    DAT_007986d8 = 0;
    DAT_007986dc = InsertIcon(param_1->x + 0xe1, param_1->y + 0x1e, 0xe, DAT_00798684);
    DAT_007986dc->string_id = 4;
    DAT_007986dc->string = GetString(4);
    DAT_007986dc->flags |= 0x2000;
    DAT_007986dc->flags |= 0x4002;
    DAT_007986dc->event_handler = (void *)FUN_004920a0;
    DAT_006687c0 = (unsigned int)DAT_007986dc->event_handler;
}

// FUNCTION: LEGOLAND 0x0048c720
LEGO_EXPORT void InitProfileCheckBoxIcons(struct IconNode *param_1) {
    DAT_00798678 = LoadSprite("PU_OK.lls", 4);
    DAT_00798674 = LoadSprite("PU_OKON.lls", 4);
    DAT_0079867c = LoadSprite("RegClose.lls", 4);
    DAT_00798680 = LoadSprite("RegCloseON.lls", 4);
    DAT_00798684 = LoadSprite("PU_ClosePopUp.lls", 4);
    DAT_00798688 = LoadSprite("PU_ClosePopUpON.lls", 4);

    DAT_007986d8 = InsertIcon(param_1->x - 0x24, param_1->y - 0x18, 0xe, DAT_00798678);
    DAT_007986d8->string_id = 0x2;
    DAT_007986d8->string = GetString(0x2);
    DAT_007986d8->flags |= 0x2000;
    DAT_007986d8->flags |= 0x4002;
    DAT_007986d8->event_handler = (void *)FUN_0048d400;

    DAT_007986dc = InsertIcon(DAT_007986d8->x + 0x24, DAT_007986d8->y, 0xe, DAT_0079867c);
    DAT_007986dc->string_id = 0x4;
    DAT_007986dc->string = GetString(0x4);
    DAT_007986dc->flags |= 0x2000;
    DAT_007986dc->flags |= 0x4002;
    DAT_007986dc->event_handler = (void *)FUN_0048d450;
}

// FUNCTION: LEGOLAND 0x0048c860
void FUN_0048c860(struct IconNode *param_1) {
    DAT_00798678 = LoadSprite("PU_OK.lls", 4);
    DAT_00798674 = LoadSprite("PU_OKON.lls", 4);
    DAT_0079867c = LoadSprite("RegClose.lls", 4);
    DAT_00798680 = LoadSprite("RegCloseON.lls", 4);
    DAT_00798684 = LoadSprite("PU_ClosePopUp.lls", 4);
    DAT_00798688 = LoadSprite("PU_ClosePopUpON.lls", 4);

    DAT_007986d8 = InsertIcon(param_1->x - 0x42, param_1->y - 0x18, 0xe, DAT_00798678);
    DAT_007986d8->string_id = 5;
    DAT_007986d8->string = GetString(5);
    DAT_007986d8->flags |= 0x2000;
    DAT_007986d8->flags |= 0x4002;
    DAT_007986d8->event_handler = (void *)FUN_0048e450;

    DAT_007986dc = InsertIcon(DAT_007986d8->x + 0x24, DAT_007986d8->y, 0xe, DAT_0079867c);
    DAT_007986dc->string_id = 4;
    DAT_007986dc->string = GetString(4);
    DAT_007986dc->flags |= 0x2000;
    DAT_007986dc->flags |= 0x4002;
    DAT_007986dc->event_handler = (void *)FUN_0048d450;
}

// FUNCTION: LEGOLAND 0x0048c9a0
LEGO_EXPORT void KillFrontEndCheckBoxSprite(void) {
    if (DAT_00798674 != NULL) {
        KillSprite(DAT_00798674);
        DAT_00798674 = NULL;
    }
    if (DAT_00798678 != NULL) {
        KillSprite(DAT_00798678);
        DAT_00798678 = NULL;
    }
    if (DAT_00798680 != NULL) {
        KillSprite(DAT_00798680);
        DAT_00798680 = NULL;
    }
    if (DAT_0079867c != NULL) {
        KillSprite(DAT_0079867c);
        DAT_0079867c = NULL;
    }
    if (DAT_00798684 != NULL) {
        KillSprite(DAT_00798684);
        DAT_00798684 = NULL;
    }
    if (DAT_00798688 != NULL) {
        KillSprite(DAT_00798688);
        DAT_00798688 = NULL;
    }
}

// FUNCTION: LEGOLAND 0x0048ca40
LEGO_EXPORT void KillListProfileSprite(void) {
    struct Sprite *sprite;
    sprite = DAT_0079868c;
    if (sprite != NULL) {
        KillSprite(sprite);
        DAT_0079868c = NULL;
    }
    sprite = DAT_00798690;
    if (sprite != NULL) {
        KillSprite(sprite);
        DAT_00798690 = NULL;
    }
    sprite = DAT_00798694;
    if (sprite != NULL) {
        KillSprite(sprite);
        DAT_00798694 = NULL;
    }
    sprite = DAT_00798698;
    if (sprite != NULL) {
        KillSprite(sprite);
        DAT_00798698 = NULL;
    }
    sprite = DAT_0079869c;
    if (sprite != NULL) {
        KillSprite(sprite);
        DAT_0079869c = NULL;
    }
    sprite = DAT_007986a0;
    if (sprite != NULL) {
        KillSprite(sprite);
        DAT_007986a0 = NULL;
    }
    sprite = DAT_007986a4;
    if (sprite != NULL) {
        KillSprite(sprite);
        DAT_007986a4 = NULL;
    }
    sprite = DAT_007986a8;
    if (sprite != NULL) {
        KillSprite(sprite);
        DAT_007986a8 = NULL;
    }
    sprite = DAT_007986ac;
    if (sprite != NULL) {
        KillSprite(sprite);
        DAT_007986ac = NULL;
    }
    sprite = DAT_007986b0;
    if (sprite != NULL) {
        KillSprite(sprite);
        DAT_007986b0 = NULL;
    }
    sprite = DAT_007986b4;
    if (sprite != NULL) {
        KillSprite(sprite);
        DAT_007986b4 = NULL;
    }
    sprite = DAT_007986b8;
    if (sprite != NULL) {
        KillSprite(sprite);
        DAT_007986b8 = 0;
    }
    sprite = DAT_007986bc;
    if (sprite != NULL) {
        KillSprite(sprite);
        DAT_007986bc = NULL;
    }
    sprite = DAT_007986c0;
    if (sprite != NULL) {
        KillSprite(sprite);
        DAT_007986c0 = NULL;
    }
    sprite = DAT_007986c4;
    if (sprite != NULL) {
        KillSprite(sprite);
        DAT_007986c4 = NULL;
    }
    sprite = DAT_007986c8;
    if (sprite != NULL) {
        KillSprite(sprite);
        DAT_007986c8 = NULL;
    }
    sprite = DAT_007986cc;
    if (sprite != NULL) {
        KillSprite(sprite);
        DAT_007986cc = NULL;
    }
    sprite = DAT_007986d0;
    if (sprite != NULL) {
        KillSprite(sprite);
        DAT_007986d0 = NULL;
    }
    sprite = DAT_007986d4;
    if (sprite != NULL) {
        KillSprite(sprite);
        DAT_007986d4 = NULL;
    }
}

// FUNCTION: LEGOLAND 0x0048cc10
LEGO_EXPORT void CloseFontEndCheckBox(void) {
    RemoveIconGroup(0xE);
    KillFrontEndCheckBoxSprite();
    DAT_004bef9c = 1;
}

// FUNCTION: LEGOLAND 0x0048cc30
unsigned char FUN_0048cc30(void *param_1, unsigned int param_2) {
    if (DAT_004bef9c != 0 && (param_2 & 2) != 0) {
        if (DAT_0080ff80.unk8 == 0) {
            InitProfileCheckBoxIcons(param_1);
        }
        if (DAT_0080ff80.unk8 == 4) {
            FUN_0048c860(param_1);
        }
        DAT_007986e4 = 1;
        DAT_004bef9c = 0;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048cd50
LEGO_EXPORT void LightUpthisDeleteIcon(struct IconNode *icon, int param_2) {
    DAT_007cb360->flags &= ~0x400;
    if (param_2 != 0) {
        DAT_007cb360->y = icon->y + 0x1b;
        DAT_007cb360->x = icon->x + 0xe1;
    } else {
        DAT_007cb360->y = icon->y + (DAT_007986e4 != 0 ? 0x1b : 0);
        DAT_007cb360->x = icon->x + 0xff;
    }
    FUN_0046d680(DAT_007cb360, DAT_00798690);
    if (DAT_00813a44.x < DAT_007cb360->x + 0x24 && DAT_007cb360->x < DAT_00813a44.x &&
        DAT_00813a44.y < DAT_007cb360->y + 0x1b && DAT_007cb360->y < DAT_00813a44.y) {
        FUN_0046d680(DAT_007cb360, DAT_0079868c);
    }
}

// FUNCTION: LEGOLAND 0x0048ce20
LEGO_EXPORT void UpdateProfileCheckBoxIcons(void) {
    struct IconNode *a;
    struct IconNode *b;
    int x;
    int y;

    if (DAT_007986d8) {
        FUN_0046d680(DAT_007986d8, DAT_00798678);
    }
    if (DAT_007986e8) {
        FUN_0046d680(DAT_007986dc, DAT_00798684);
    } else {
        FUN_0046d680(DAT_007986dc, DAT_0079867c);
    }
    a = DAT_007986d8;
    if (a) {
        x = a->x;
        y = a->y;
        if (DAT_00813a44.x < x + 0x24 && x < DAT_00813a44.x && DAT_00813a44.y < y + 0x1b && y < DAT_00813a44.y) {
            FUN_0046d680(a, DAT_00798674);
        }
    }
    b = DAT_007986dc;
    x = b->x;
    y = b->y;
    if (DAT_00813a44.x < x + 0x24 && x < DAT_00813a44.x && DAT_00813a44.y < y + 0x1b && y < DAT_00813a44.y) {
        if (DAT_007986e8) {
            FUN_0046d680(b, DAT_00798688);
            return;
        }
        FUN_0046d680(b, DAT_00798680);
    }
}

// FUNCTION: LEGOLAND 0x0048cf10
LEGO_EXPORT void PrintProfileDetails(void) {
    struct IconNode *icon;
    struct IconNode *last;
    char *name;
    int y;
    int x;
    unsigned char sel;
    int show;

    y = 0x72;
    DAT_007cb360->flags |= 0x400;
    icon = DAT_006687c8;
    FUN_00455e50(DAT_007cb340, 0x8d, y, 0xf0, 0x2e, 3, 0x25, 0xffffff, 0);
    while (icon != NULL) {
        sel = 1;
        show = 1;
        if ((icon->flags & 0x400) == 0 && (icon->field_20b & 1)) {
            if (DAT_00813a44.x >= icon->x - 0x18 && DAT_00813a44.x < icon->x && DAT_00813a44.y >= icon->slot * 0x26 + 0x86 &&
                DAT_00813a44.y < icon->field_10 + icon->y) {
                DAT_004bdd00 = 2;
                DAT_004bdd04 = (struct Bloke *)icon;
            }
            if (DAT_0080ffe3 == icon->slot) {
                if (DAT_007986e4 != 0) {
                    FUN_0046d680(icon, DAT_007986b8);
                    last = icon;
                    icon->y = icon->slot * 0x26 + 0x6b;
                    y = icon->y + 0x22;
                } else if (DAT_007986e8 != 0) {
                    EnterNewProfile(icon);
                    show = 0;
                    last = icon;
                } else {
                    FUN_0046d680(icon, DAT_007986bc);
                    icon->y = icon->slot * 0x26 + 0x6b;
                    y = icon->y + 0x22;
                    LightUpthisDeleteIcon(icon, 1);
                    last = icon;
                }
            } else {
                FUN_0046d680(icon, FUN_0048c5e0(icon->slot));
                sel = 0;
                icon->y = icon->slot * 0x26 + 0x86;
                y = icon->y + 7;
            }
            x = icon->x + 0x14;
            name = (char *)icon->field_18p;
            if (name != NULL && show) {
                if (DAT_0080ffe3 - 1 != icon->slot || (DAT_007986e4 == 0 && DAT_007986e8 == 0)) {
                    if (sel) {
                        FUN_00455e50(name, x, y, 0xe0, 0x13, 2, 0x25, 0, 0xffffff);
                    } else {
                        FUN_00455e50(name, x, y, 0xe0, 0x13, 2, 0x25, 0xffffff, 0);
                    }
                }
            }
        }
        icon = icon->next;
    }
    if (DAT_007986e4 != 0) {
        FUN_00455e50(GetString(0x85), last->x + 0x14, last->y + 7, 0x9b, 0x13, 2, 0x25, 0, 0xffffff);
    } else if (DAT_007986e8 != 0) {
        FUN_00455e50(GetString(0x86), last->x + 0x14, last->y - 0x14, 0xe0, 0x13, 2, 0x25, 0, 0xffffff);
        UpdateProfileCheckBoxIcons();
    }
    if (DAT_0080ffe3 != 0 && DAT_007986e4 == 0) {
        if (DAT_007986e8 != 0) {
            if (FUN_00491540()) {
                ((struct IconNode *)DAT_007986e0)->flags &= ~0x400;
                return;
            }
        } else {
            ((struct IconNode *)DAT_007986e0)->flags &= ~0x400;
            return;
        }
    }
    ((struct IconNode *)DAT_007986e0)->flags |= 0x400;
}

// FUNCTION: LEGOLAND 0x0048d230
void FUN_0048d230(void) {
    struct ProfileNode *node = (struct ProfileNode *)DAT_00798890;

    while (node != NULL) {
        if (node->slot == DAT_0080ffe3) {
            strcpy((char *)&DAT_0080ffa0, node->data.name);
            DAT_0080ffc0 = node->data.field_20;
            DAT_0080ffe4 = 0;
            DAT_0080ffc4 = node->data.field_28;
            DAT_0080ffc8 = node->data.field_2c;
            DAT_0080ffcc = node->data.field_30;
            DAT_0080ffe5 = 0;
            memcpy(&DAT_0080ffa0.field_34, &node->data.field_34, 15);
            memcpy(DAT_0080ffe6, node->data.field_43, 200);
            *(int *)&DAT_0080ffa0.field_30 = *(int *)&node->data.field_10b;
            return;
        }
        node = node->next;
    }
}

// FUNCTION: LEGOLAND 0x0048d300
unsigned char FUN_0048d300(unsigned int dummy, unsigned char arg_0) {
    if (DAT_007986e4 == 0 && (arg_0 & 0x2) != 0 && ((((struct ProfileFlags *)DAT_007986e0)->var_34 >> 8) & 0x4) == 0 && DAT_0080ffe3 != 0) {
        if (DAT_007986e8 != 0) {
            SaveProfileToDisk();
            DeleteProfileList();
            LoadProfilesFormDisk();
            RemoveIconGroup(0x15);
            CloseFontEndCheckBox();
            DAT_007986e8 = 0;
        }
        FUN_00498920();
        DAT_006687b0 = 4;
        PlayInstanceOfSample(PTR_004b92c0, 0, 1, 0);
        DAT_0080ff80.unk8 = 1;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048d390
unsigned char FUN_0048d390(struct Profile *profile, unsigned char param_2) {
    if (DAT_004bef9c != 0 && (param_2 & 0x2) != 0) {
        DAT_0080ffe3 = profile->var_1c;
        FUN_0048a800();
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048d3c0
unsigned char FUN_0048d3c0(struct Profile *profile, unsigned int param_2) {
    if (DAT_004bef9c != 0) {
        if (param_2 & 0x2) {
            DAT_0080ffe3 = profile->var_1c;
            DAT_007986e8 = 1;
            InitNewProfilePoPUp(profile);
            DAT_004bef9c = 0;
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048d400
unsigned char FUN_0048d400(unsigned int arg0, unsigned int arg1) {
    if (arg1 & 0x2) {
        if (DAT_0080ffe3) {
            CloseFontEndCheckBox();
            DAT_007986e4 = 0;
            RemoveProfile(DAT_0080ffe3);
            DAT_0080ffe3 = 0;
            DAT_0080ff80.unk4 = 0xffffffff;
            DAT_0080ff80.unk8 = 0;
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048d450
unsigned char FUN_0048d450(unsigned int param_1, unsigned int param_2) {
    if ((param_2 & 2) != 0) {
        DAT_007986e4 = 0;
        CloseFontEndCheckBox();
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048d470
void FUN_0048d470(void) {
    DAT_007986f8 = DAT_006687bc;
    DAT_007986f4 = DAT_006687c0;
}

// FUNCTION: LEGOLAND 0x0048d490
void FUN_0048d490(void) {
    DAT_006687bc = DAT_007986f8;
    DAT_006687c0 = DAT_007986f4;
}
