#include "legoland.h"

#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include "bloke.h"
#include "bloke_ai.h"
#include "bricks.h"
#include "challenge.h"
#include "debug_alloc.h"
#include "draw.h"
#include "gamemap.h"
#include "globals.h"
#include "input.h"
#include "interface.h"
#include "llidb.h"
#include "man3d.h"
#include "map_object.h"
#include "nerps.h"
#include "obj_instance.h"
#include "pathfind.h"
#include "render3d.h"
#include "saveload.h"
#include "screens.h"
#include "timer.h"
#include "worker.h"

/* Abort a load: close the file and report failure. */
#pragma auto_inline(off)
static int LoadAbort(void) {
    _close(DAT_006691b0);
    DAT_00667ca0 = 0;
    return 0;
}
#pragma auto_inline(on)
#define LOAD_FAIL() return LoadAbort()

// FUNCTION: LEGOLAND 0x0047d790
LEGO_EXPORT int BeginMeasuredBlock(void) {
    int pos;
    int marker;

    pos = _tell(DAT_006691b0);
    marker = 0;
    if (pos == -1) {
        return 0;
    }
    DAT_006691bc[DAT_006691fc] = pos;
    DAT_006691fc = DAT_006691fc + 1;
    return SaveGameWrite(&marker, 4) != 0;
}

// FUNCTION: LEGOLAND 0x0047d7e0
int FUN_0047d7e0(void) {
    unsigned int buffer;

    return SaveGameRead(&buffer, 4);
}

// FUNCTION: LEGOLAND 0x0047d800
LEGO_EXPORT int EndMeasuredBlock(void) {
    int end_pos;

    end_pos = _tell(DAT_006691b0);
    if (end_pos == -1) {
        return 0;
    }
    DAT_006691fc = DAT_006691fc - 1;
    if (_lseek(DAT_006691b0, DAT_006691bc[DAT_006691fc], 0) == -1) {
        return 0;
    }
    if (SaveGameWrite(&end_pos, 4) == 0) {
        return 0;
    }
    return _lseek(DAT_006691b0, end_pos, 0) != -1;
}

// FUNCTION: LEGOLAND 0x0047d880
LEGO_EXPORT int FindeIneList(union SavedElement *handle) {
    int i;

    for (i = 0; i < DAT_006691b4; i++) {
        if (handle->element == DAT_00669200[i]) {
            handle->index = i;
            return 1;
        }
    }
    handle->index = -1;
    return 0;
}

// FUNCTION: LEGOLAND 0x0047d8c0
LEGO_EXPORT struct Element *GeteListPtr(int idx) {
    if (idx == -1) {
        return 0;
    }
    return DAT_00669200[idx];
}

// FUNCTION: LEGOLAND 0x0047d8e0
LEGO_EXPORT int SaveGame(char *filename) { STUB(); }

// FUNCTION: LEGOLAND 0x0047e980
LEGO_EXPORT int LoadGame(char *path) {
    int i;
    int x;
    int y;
    struct Ride *ride;
    struct Element *e;
    struct MapElement *tile;
    struct Bloke *bloke;
    struct Anim3D *anim;
    struct ObjInstance *inst;
    struct ObjInstance *inst_prev;
    struct RideNode *rnode;
    struct RideNode *rnode_prev;
    struct Ride *entrance;
    struct MapElement *first;
    int n;
    unsigned int len;
    int m;
    char header[0x20];
    char name[0x200];

    DAT_006691b0 = _open(path, _O_BINARY);
    if (DAT_006691b0 == -1) {
        return 0;
    }
    DAT_006691fc = 0;
    DAT_00667ca0 = 1;
    for (;;) {
        if (SaveGameRead(header, 0x20) == 0) {
            break;
        }
        header[5] = 0;
        if (atoi(header) != 2) {
            return 0;
        }
        if (FUN_0047d7e0() == 0) {
            break;
        }
        if (FUN_0047d7e0() == 0) {
            break;
        }
        if (SaveGameRead(&DAT_006691b4, 4) == 0) {
            break;
        }
        if (DAT_00669200 != 0) {
            free(DAT_00669200);
            DAT_00669200 = 0;
        }
        DAT_00669200 = malloc(DAT_006691b4 * 4);
        for (i = 0; i < DAT_006691b4; i++) {
            FUN_004663f0();
            if (SaveGameRead(&len, 4) == 0) {
                LOAD_FAIL();
            }
            if (SaveGameRead(name, len) == 0) {
                LOAD_FAIL();
            }
            name[len] = 0;
            if (LLIDB_FindElement(name, (unsigned int *)&n, 0) != 0) {
                LOAD_FAIL();
            }
            DAT_00669200[i] = (struct Element *)n;
            if (SaveGameRead(&m, 4) == 0) {
                LOAD_FAIL();
            }
            LLIDB_LoadData((void *)n);
            ((struct Element *)n)->flags &= 0xfffcfff1;
            ((struct Element *)n)->flags |= m;
        }
        {
            unsigned short *tab[256];
            if (SaveGameRead(&DAT_007fdb84, 4) == 0) {
                break;
            }
            for (i = 0; i < DAT_007fdb84; i++) {
                FUN_004663f0();
                if (SaveGameRead(&len, 4) == 0) {
                    LOAD_FAIL();
                }
                if (SaveGameRead(name, len) == 0) {
                    LOAD_FAIL();
                }
                name[len] = 0;
                if (LLIDB_FindElement(name, (unsigned int *)&n, 0) != 0) {
                    LOAD_FAIL();
                }
                DAT_007fd660[i] = n;
                LLIDB_LoadData((void *)n);
                tab[i] = ((struct Element *)n)->data;
            }
            if (FUN_0047d7e0() == 0) {
                break;
            }
            if (SaveGameRead(lpConfig, 0x44) == 0) {
                break;
            }
            FUN_00463680();
            for (y = 0; y < lpConfig->height; y++) {
                FUN_004663f0();
                for (x = 0; x < lpConfig->width; x++) {
                    tile = &GameMap[y][x];
                    if (SaveGameRead(tile, 0x14) == 0) {
                        LOAD_FAIL();
                    }
                    if ((tile->flags & 0x8a8) != 0) {
                        tile->field_0 = DAT_00669200[(int)tile->field_0];
                    } else {
                        tile->field_0 = 0;
                    }
                    if (tile->field_8 != 0) {
                        tile->field_8 = *tab[(tile->field_8 >> 8) - 1] + (tile->field_8 & 0xff);
                    }
                    if (tile->field_a != 0) {
                        tile->field_a = *tab[(tile->field_a >> 8) - 1] + (tile->field_a & 0xff);
                    }
                }
            }
        }
        if (FUN_0047d7e0() == 0) {
            break;
        }
        if (SaveGameRead(&MapStats, 0x3f0) == 0) {
            break;
        }
        if (SaveGameRead(&ScrollX, 4) == 0) {
            break;
        }
        if (SaveGameRead(&ScrollY, 4) == 0) {
            break;
        }
        if (SaveGameRead(&EditMode, 0xc) == 0) {
            break;
        }
        EditMode.unk4 = 3;
        EditMode.unk8 = 0;
        FUN_004741c0();
        FUN_00474880();
        FUN_004663f0();
        if (FUN_0046cb60() == 0) {
            break;
        }
        if (FUN_00444260() == 0) {
            break;
        }
        if (FUN_00457940() == 0) {
            break;
        }
        if (SaveGameRead(DAT_007fdd00, 0x24) == 0) {
            break;
        }
        FUN_004663f0();
        DAT_00667d50 = 1;
        if (FUN_0047d7e0() == 0) {
            break;
        }
        while (FirstBloke != 0) {
            DestroyBloke(FirstBloke);
        }
        n = 0;
        FUN_004663f0();
        if (SaveGameRead(&n, 4) == 0) {
            break;
        }
        while (n-- != 0) {
            if (SaveGameRead(&m, 4) == 0) {
                LOAD_FAIL();
            }
            bloke = &DAT_0066b57c[m];
            bloke->next = FirstBloke;
            FirstBloke = bloke;
            if (SaveGameRead(&DAT_007fda60, sizeof(DAT_007fda60)) == 0) {
                LOAD_FAIL();
            }
            if (DAT_007fda60.block_34[8] != 0) {
                DAT_007fda60.block_34[8] = (unsigned int)malloc(0x48);
                if (SaveGameRead((void *)DAT_007fda60.block_34[8], 0x48) == 0) {
                    LOAD_FAIL();
                }
            }
            bloke->action = DAT_007fda60.action;
            bloke->field_e = DAT_007fda60.field_e;
            bloke->field_10 = DAT_007fda60.field_10;
            if (DAT_007fda60.target != -1) {
                bloke->target = DAT_00669200[DAT_007fda60.target];
            } else {
                bloke->target = 0;
            }
            if (DAT_007fda60.last_ride != -1) {
                bloke->last_ride = DAT_00669200[DAT_007fda60.last_ride];
            } else {
                bloke->last_ride = 0;
            }
            bloke->field_1c = DAT_007fda60.field_1c;
            bloke->field_20 = DAT_007fda60.field_20;
            bloke->dest.x = DAT_007fda60.dest.x;
            bloke->dest.y = DAT_007fda60.dest.y;
            bloke->goal.x = DAT_007fda60.goal.x;
            bloke->goal.y = DAT_007fda60.goal.y;
            memcpy(&bloke->field_34, DAT_007fda60.block_34, sizeof(DAT_007fda60.block_34));
            bloke->field_5c = DAT_007fda60.field_5c;
            bloke->param_action = DAT_007fda60.param_action;
            bloke->flags = DAT_007fda60.flags;
            bloke->field_64 = DAT_007fda60.field_64;
            bloke->field_78 = DAT_007fda60.field_78;
            bloke->field_7a = DAT_007fda60.field_7a;
            bloke->field_7c = DAT_007fda60.field_7c;
            bloke->field_7e = DAT_007fda60.field_7e;
            bloke->field_7f = DAT_007fda60.field_7f;
            bloke->field_80 = DAT_007fda60.field_80;
            bloke->field_81 = DAT_007fda60.field_81;
            bloke->field_82 = DAT_007fda60.field_82;
            if (DAT_007fda60.favourite[0] < DAT_006691b4) {
                bloke->favourite_attraction_0 = DAT_00669200[DAT_007fda60.favourite[0]];
            } else {
                bloke->favourite_attraction_0 = 0;
            }
            if (DAT_007fda60.favourite[1] < DAT_006691b4) {
                bloke->favourite_attraction_1 = DAT_00669200[DAT_007fda60.favourite[1]];
            } else {
                bloke->favourite_attraction_1 = 0;
            }
            if (DAT_007fda60.favourite[2] < DAT_006691b4) {
                bloke->favourite_attraction_2 = DAT_00669200[DAT_007fda60.favourite[2]];
            } else {
                bloke->favourite_attraction_2 = 0;
            }
            if (DAT_007fda60.favourite[3] < DAT_006691b4) {
                bloke->favourite_food = DAT_00669200[DAT_007fda60.favourite[3]];
            } else {
                bloke->favourite_food = 0;
            }
            bloke->pos.x = DAT_007fda60.pos.x;
            bloke->pos.y = DAT_007fda60.pos.y;
            bloke->field_70 = DAT_007fda60.field_70;
            bloke->field_72 = DAT_007fda60.field_72;
            bloke->field_73 = DAT_007fda60.field_73;
            bloke->field_74 = DAT_007fda60.field_74;
            bloke->field_75 = DAT_007fda60.field_75;
            bloke->nav = DAT_007fda60.nav;
            bloke->person = malloc(sizeof(Person));
            FUN_0043f810(bloke->person);
            bloke->person->bloke = bloke;
            bloke->person->field_8 = DAT_007fda60.person_8;
            bloke->person->scale = DAT_007fda60.scale;
            bloke->person->screen = DAT_007fda60.screen;
            bloke->person->offset = DAT_007fda60.offset;
            bloke->person->field_34 = DAT_007fda60.field_34;
            bloke->person->field_38 = DAT_007fda60.field_38;
            bloke->person->depth = DAT_007fda60.depth;
            bloke->person->rotation = DAT_007fda60.rotation;
            bloke->person->field_4c = DAT_007fda60.person_4c;
            bloke->person->field_88 = DAT_007fda60.anim;
            bloke->person->sort_id = DAT_007fda60.sort_id;
            bloke->person->field_38 = DAT_007fda60.field_38;
            memcpy(bloke->person->m, DAT_007fda60.m, sizeof(DAT_007fda60.m));
            bloke->person->field_7c = DAT_007fda60.field_7c_p;
            bloke->person->field_80 = DAT_007fda60.field_80_p;
            bloke->person->field_8c = DAT_007fda60.field_8c_p;
            bloke->person->field_90 = DAT_007fda60.field_90_p;
            bloke->person->random = DAT_007fda60.random;
            bloke->prev_param = DAT_007fda60.prev_param;
            bloke->prev_action = DAT_007fda60.prev_action;
            bloke->person->field_2c = 0;
            bloke->person->field_30 = DAT_007fda60.field_30;
            anim = GetBlokeAnim3DFromPerson(bloke->person);
            bloke->person->field_50 = FUN_00442580(bloke->person, DAT_0081c8c0, (unsigned int)anim->field_8, anim->elems->shared->count, bloke->person->random);
        }
        if (FUN_0047d7e0() == 0) {
            break;
        }
        FUN_0049c3c0();
        if (FUN_0047d7e0() == 0) {
            break;
        }
        FUN_0049c8b0();
        FUN_004663f0();
        if (FUN_0047d7e0() == 0) {
            break;
        }
        FUN_0049cc10();
        if (FUN_0047d7e0() == 0) {
            break;
        }
        FUN_0049ce00();
        FUN_004663f0();
        if (FUN_0047d7e0() == 0) {
            break;
        }
        if (SaveGameRead(&DAT_0079a8d0, 4) == 0) {
            break;
        }
        if (SaveGameRead(&DAT_006670f8, 4) == 0) {
            break;
        }
        if (SaveGameRead(DAT_006664f8, sizeof(DAT_006664f8)) == 0) {
            break;
        }
        for (i = 0; i < DAT_006691b4; i++) {
            FUN_004663f0();
            e = DAT_00669200[i];
            if ((e->flags & 0x10) != 0) {
                // STRING: LEGOLAND 0x004bcb94
                char label[9] = "xxxxxxxx";
                e->flags |= 4;
                ride = DAT_00669200[i]->ride;
                if (SaveGameRead(label, 8) == 0) {
                    LOAD_FAIL();
                }
                // STRING: LEGOLAND 0x004bcb90
                DBPrintf("%s\n", label);
                if (ride->type != 2 && ride->type != 0) {
                    ride->counters = malloc(lpConfig->field_1a);
                    if (SaveGameRead(ride->counters, lpConfig->field_1a) == 0) {
                        LOAD_FAIL();
                    }
                }
                if (SaveGameRead(&ride->field_8, 4) == 0) {
                    LOAD_FAIL();
                }
                inst_prev = 0;
                n = 0;
                if (SaveGameRead(&n, 4) == 0) {
                    LOAD_FAIL();
                }
                while (n-- != 0) {
                    inst = malloc(sizeof(struct ObjInstance));
                    inst->next = 0;
                    inst->field_8 = (unsigned int)ride;
                    if (inst_prev == 0) {
                        ride->instances = inst;
                        inst->field_4 = (unsigned int)inst_prev;
                    } else {
                        inst_prev->next = inst;
                        inst->field_4 = (unsigned int)inst_prev;
                    }
                    inst_prev = inst;
                    if (SaveGameRead(&inst->flags, 4) == 0) {
                        LOAD_FAIL();
                    }
                    if (SaveGameRead(&inst->uid, 4) == 0) {
                        LOAD_FAIL();
                    }
                    if (SaveGameRead(&inst->field_10, 4) == 0) {
                        LOAD_FAIL();
                    }
                }
                rnode_prev = 0;
                n = 0;
                if (SaveGameRead(&n, 4) == 0) {
                    LOAD_FAIL();
                }
                while (n-- != 0) {
                    rnode = malloc(sizeof(struct RideNode));
                    rnode->next = 0;
                    if (rnode_prev == 0) {
                        ride->riders = rnode;
                        rnode->prev = rnode_prev;
                    } else {
                        rnode_prev->next = rnode;
                        rnode->prev = rnode_prev;
                    }
                    rnode_prev = rnode;
                    if (SaveGameRead(&m, 4) == 0) {
                        LOAD_FAIL();
                    }
                    if (SaveGameRead(&rnode->tile, 2) == 0) {
                        LOAD_FAIL();
                    }
                    rnode->rider = GetBlokePtr(m);
                    rnode->person = rnode->rider->person;
                }
                if (ride->load_hook != 0) {
                    if (ride->load_hook(ride->element) == 0) {
                        LOAD_FAIL();
                    }
                }
            }
        }
        if (SaveGameRead(DAT_007cb3e0, sizeof(DAT_007cb3e0)) == 0) {
            break;
        }
        FUN_004663f0();
        if (FUN_0047d7e0() == 0) {
            break;
        }
        if (FUN_00482920() == 0) {
            break;
        }
        if (FUN_0047d7e0() == 0) {
            break;
        }
        FUN_00450b10();
        FUN_004663f0();
        if (FUN_0047d7e0() == 0) {
            break;
        }
        {
            char name2[0x200];
            struct OverlayParam ov;
            SaveGameRead(&n, 4);
            SaveGameRead(name2, n);
            name2[n] = 0;
            LLIDB_FindElement(name2, (unsigned int *)&DAT_00801410, 0);
            OverlayILF = (unsigned int)LLIDB_LoadData(DAT_00801410);
            SaveGameRead(&n, 4);
            if (n != 0) {
                SaveGameRead(name2, n);
                name2[n] = 0;
                LLIDB_FindElement(name2, (unsigned int *)&DAT_00801404, 0);
                DAT_00667cb0 = LLIDB_LoadData(DAT_00801404);
                FUN_004618d0(name2);
            } else {
                DAT_00801404 = 0;
                DAT_00667cb0 = 0;
            }
            SaveGameRead(&n, 4);
            for (i = 0; i < n; i++) {
                SaveGameRead(&ov, 0x14);
                AddOvSav(&ov);
            }
        }
        _close(DAT_006691b0);
        FUN_004663f0();
        DAT_00667ca0 = 0;
        EditMode.unk0 = 0;
        GamePad |= 0x20;
        CalculateMapRenderOrder();
        // STRING: LEGOLAND 0x004b83d0
        DAT_006661c4 = ElemID("ENTRANCE 1");
        entrance = DAT_006661c4->ride;
        first = GetFirstObjectMatching(DAT_006661c4);
        if (first != 0) {
            DAT_004b8320.x = (first->field_4 + entrance->footprint.v[0]) * 0x100 + -0x100;
            DAT_004b8320.y = (((entrance->footprint.v[3] - entrance->footprint.v[1]) << 7) & ~0xff) + ((first->field_5 + entrance->footprint.v[1]) << 8);
        }
        FUN_00475f10();
        FUN_00458bb0(1);
        DAT_00810140 = 1;
        DAT_00667c48 = 1;
        GamePad &= ~0x1000;
        FUN_0046b760();
        return 1;
    }
    return LoadAbort();
}

// FUNCTION: LEGOLAND 0x0047f760
LEGO_EXPORT void UnloadSaveGameMap(void) {
    int i;

    if (DAT_00669200 != 0) {
        for (i = 0; i < DAT_006691b4; i++) {
            LLIDB_UnLoadData(DAT_00669200[i]);
        }
        free(DAT_00669200);
        DAT_00669200 = 0;
    }

    for (i = 0; i < DAT_007fdb84; i++) {
        LLIDB_UnLoadData(DAT_007fd660[i]);
    }

    FUN_00463680();
    LLIDB_UnLoadData((unsigned int)DAT_00801410);
    if (DAT_00801404 != 0) {
        LLIDB_UnLoadData((unsigned int)DAT_00801404);
    }
    ClearOverlays();
    FUN_004828f0();
    DAT_00667d50 = 0;
}

// FUNCTION: LEGOLAND 0x0047f810
void FUN_0047f810(void) {
    DAT_00669204 = GetGameTimer();
}

// FUNCTION: LEGOLAND 0x0047f820
int FUN_0047f820(void) {
    return GetGameTimer() - DAT_00669204;
}

// FUNCTION: LEGOLAND 0x0047f830
unsigned int FUN_0047f830(const char *path) {
    return 1;
}

// FUNCTION: LEGOLAND 0x0047f840
int FUN_0047f840(void) {
    return 1;
}

// FUNCTION: LEGOLAND 0x0047f850
void FUN_0047f850(void) {
    return;
}
