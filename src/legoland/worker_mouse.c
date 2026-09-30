#include "worker_mouse.h"
#include "bloke.h"
#include "bloke_ai.h"
#include "debug_alloc.h"
#include "globals.h"
#include "icon.h"
#include "legoland.h"
#include "man3d.h"
#include "math.h"
#include "render3d.h"
#include "sound_music.h"
#include "string.h"
#include "tilemap.h"
#include "worker.h"

struct WorkerInner {
    unsigned char pad_0[0x1c];
    struct Point pos;
};

struct WorkerOuter {
    unsigned char pad_0[4];
    struct WorkerInner *inner;
    unsigned char pad_8[6];
    unsigned short var_e;
    unsigned char pad_10[0x58];
    unsigned int var_68;
    unsigned int var_6c;
};

#include "image_sprite.h"

// FUNCTION: LEGOLAND 0x004700c0
unsigned int FUN_004700c0(void *object) {
    if (DAT_007fdf9c == 0x306) {
        if (DAT_007fdf8c == object) {
            return 1;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x004700f0
void *FUN_004700f0(void) {
    return DAT_007fdff0;
}

// FUNCTION: LEGOLAND 0x00470100
void FUN_00470100(unsigned int type, Bloke *worker) {
    // STRING: LEGOLAND 0x004ba9ec
    DBPrintf("Picking up worker (%x) Workorder = %x\n", worker, worker->order);
    DAT_007fdff0 = worker;
    DAT_007fdffc = type;
    worker->field_e = 0xd;
    DAT_007fdff4 = DAT_007fdff0->pos.x;
    DAT_007fdff8 = DAT_007fdff0->pos.y;
    DAT_007fdff0->field_72 = 5;
    DAT_00668954 = 1;
    if (DAT_007fdffc == 0x307) {
        ClearAGardenersWorkList(DAT_007fdff0);
        NewLongTermAction(DAT_007fdff0, 0x18);
        ClearAGardenersWorkList(DAT_007fdff0);
    } else {
        ClearAMechanicsWorkList(DAT_007fdff0);
        NewLongTermAction(DAT_007fdff0, 0x19);
        ClearAMechanicsWorkList(DAT_007fdff0);
    }
    worker->order = 0;
    DAT_007fdff0->order = 0;
    if (DAT_007fdffc == 0x307) {
        NewLongTermAction(DAT_007fdff0, 0x18);
    } else {
        NewLongTermAction(DAT_007fdff0, 0x19);
    }
}

// FUNCTION: LEGOLAND 0x004701f0
LEGO_EXPORT void SetWorkersPositionAtMouse(void) {
    struct WorkerOuter *worker;
    struct WorkerInner *inner;
    unsigned int pt[2];

    worker = DAT_007fdff0;
    worker->var_e = 13;
    worker = DAT_007fdff0;
    inner = worker->inner;
    inner->pos.x = DAT_00813a44.x;
    worker = DAT_007fdff0;
    inner = worker->inner;
    inner->pos.y = DAT_00813a44.y;
    worker = DAT_007fdff0;
    inner = worker->inner;
    AdjustBlokePosition(&inner->pos);
    ScreenToMapRef((unsigned int)&DAT_00813a44, pt, 0);
    worker = DAT_007fdff0;
    worker->var_68 = pt[0] << 8;
    worker = DAT_007fdff0;
    worker->var_6c = pt[1] << 8;
}

// FUNCTION: LEGOLAND 0x00470270
int FUN_00470270(void) {
    unsigned int v = DAT_004bdd08 & 0xffff;
    int x = v & 0xff;
    int y = v >> 8;
    MapElement *elem;
    Ride *ride;

    if (x >= 0 && x < lpConfig->width && y >= 0 && y < lpConfig->height) {
        elem = &GameMap[y][x];
    } else {
        elem = NULL;
    }
    ride = elem->field_0->ride;
    if (ride->element == (Element *)DAT_007fdfb0 && DAT_007fdffc == 0x307) {
        PutWorkerOnRide(DAT_007fdff0, elem);
        DAT_007fdff0->pos.x = (ride->x + x) << 8;
        DAT_007fdff0->dest.x = DAT_007fdff0->pos.x;
        DAT_007fdff0->pos.y = (ride->y + y) << 8;
        DAT_007fdff0->dest.y = DAT_007fdff0->pos.y;
        DAT_007fdff0->action = 5;
        DAT_007fdff0->param_action = 100;
        PlayInstanceOfSample(DAT_004b9320, 0, 1, 0);
        return 1;
    }
    if (ride->element == (Element *)DAT_007fdfb4 && DAT_007fdffc == 0x308) {
        PutWorkerOnRide(DAT_007fdff0, elem);
        DAT_007fdff0->pos.x = ((ride->x + x) << 8) + 0x80;
        DAT_007fdff0->dest.x = DAT_007fdff0->pos.x;
        DAT_007fdff0->pos.y = (ride->y + y) << 8;
        DAT_007fdff0->dest.y = DAT_007fdff0->pos.y;
        DAT_007fdff0->action = 5;
        DAT_007fdff0->param_action = 100;
        PlayInstanceOfSample(DAT_004b932c, 0, 1, 0);
        return 1;
    }
    return ride->element == (Element *)DAT_007fdfb8;
}

// FUNCTION: LEGOLAND 0x00470410
WorkOrder *FUN_00470410(Point *out) {
    WorkOrder *order;
    unsigned int v = DAT_004bdd08 & 0xffff;
    int x = v & 0xff;
    int y = v >> 8;

    if (DAT_007fdffc == 0x307) {
        order = GetGardenerWorkOrderAt(x, y);
    } else {
        order = GetMechanicWorkOrderAt(x, y);
    }
    if (order != NULL) {
        if (out != NULL) {
            out->x = order->pos.x + order->footprints->x0;
            out->y = order->footprints->y1 + order->pos.y;
        }
        if (DAT_007fdffc == 0x307) {
            PlayInstanceOfSample(DAT_004b9320, 0, 1, 0);
        } else {
            PlayInstanceOfSample(DAT_004b932c, 0, 1, 0);
        }
        return order;
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x004704b0
WorkOrder *FUN_004704b0(Point *out) {
    unsigned int v = DAT_004bdd08 & 0xffff;
    struct Point pos;
    MapElement *elem;
    Ride *ride;
    unsigned short flags;
    WorkOrder *order = NULL;

    pos.x = v & 0xff;
    pos.y = v >> 8;
    if (pos.x >= 0 && pos.x < lpConfig->width && pos.y >= 0 && pos.y < lpConfig->height) {
        elem = &GameMap[pos.y][pos.x];
    } else {
        elem = NULL;
    }
    flags = elem->flags;
    if (flags & 0x88) {
        ride = elem->field_0->ride;
        if (ride->durability != 0) {
            if ((ride->flags & 0x200000) && DAT_007fdffc == 0x307 && lpConfig->field_38 != 0) {
                if (!(0x4000 & flags)) {
                    order = AddRepairOrderForObject(ride, pos);
                }
            } else if ((ride->flags & 0x400000) && DAT_007fdffc == 0x308 && lpConfig->field_34 != 0) {
                if (!(0x4000 & flags)) {
                    order = AddRepairOrderForObject(ride, pos);
                }
            }
            if (order != NULL) {
                elem->flags |= 0x4000;
                if (out != NULL) {
                    out->x = order->pos.x + order->footprints->x0;
                    out->y = order->footprints->y1 + order->pos.y;
                }
                if (DAT_007fdffc == 0x307) {
                    PlayInstanceOfSample(DAT_004b9320, 0, 1, 0);
                } else {
                    PlayInstanceOfSample(DAT_004b932c, 0, 1, 0);
                }
            }
        }
        return order;
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x00470620
LEGO_EXPORT void CheckWorkerOnMouseStatus(int a) {
    int result = 0;
    int isOrder = 0;
    int pt[2];
    int x;
    WorkOrder *order;
    MapElement *elem;

    if (!(DAT_00813a60 & 2) && a == 0) {
        if (DAT_00813a50 & 2) {
            DAT_00667c48 = 1;
            DAT_00668954 = 0;
            for (;;) {
                if (DAT_004bdd00 == 0x103) {
                    if (FUN_00470270()) {
                        DAT_00668954 = 0;
                        return;
                    }
                    order = FUN_00470410((Point *)pt);
                    if (order != NULL) {
                        x = pt[0];
                        DAT_00668954 = 0;
                        isOrder = 1;
                    } else {
                        order = FUN_004704b0((Point *)pt);
                        if (order == NULL) {
                            break;
                        }
                        x = pt[0];
                        DAT_00668954 = 0;
                        isOrder = 1;
                    }
                } else {
                    if (DAT_004bdd00 == 0x10a || DAT_004bdd00 == 2 || DAT_00813a44.y < 0x20 || DAT_00813a44.y >= 0x174 || DAT_00813a44.x < lpConfig->field_20 + 9) {
                        break;
                    }
                    ScreenToMapRef(&DAT_00813a44.x, pt, 0);
                    x = pt[0];
                    if (x < 0 || x >= lpConfig->width || pt[1] < 0) {
                        break;
                    }
                    if (pt[1] < lpConfig->height) {
                        elem = &GameMap[pt[1]][x];
                        if (elem != NULL && (elem->field_10 & 2)) {
                            DAT_00668954 = 1;
                            if (DAT_007fdffc == 0x307 && (elem->flags & 0x800)) {
                                DAT_00668954 = 0;
                            } else {
                                SetWorkersPositionAtMouse();
                                return;
                            }
                        } else if (DAT_00668954 != 0) {
                            SetWorkersPositionAtMouse();
                            return;
                        }
                    } else {
                        DAT_00668954 = 1;
                        SetWorkersPositionAtMouse();
                        return;
                    }
                }
                if (DAT_007fdffc == 0x307) {
                    DAT_007fdff0->pos.x = (x << 8) + 0x80;
                    DAT_007fdff0->pos.y = (pt[1] << 8) + 0x80;
                    result = SetGardenerWorkOrderAtPostion(DAT_007fdff0, pt[0], pt[1]);
                } else {
                    if (isOrder) {
                        DAT_007fdff0->pos.x = ((order->step_x + x) << 8) + 0x80;
                        DAT_007fdff0->pos.y = ((pt[1] - order->step_y) << 8) + 0x80;
                    } else {
                        DAT_007fdff0->pos.x = (x << 8) + 0x80;
                        DAT_007fdff0->pos.y = (pt[1] << 8) + 0x80;
                    }
                    result = SetMechanicsOrderAtPostion(DAT_007fdff0, pt[0], pt[1]);
                }
                if (result == 0) {
                    break;
                }
                if (DAT_00668954 != 0) {
                    SetWorkersPositionAtMouse();
                }
                return;
            }
            DAT_00668954 = 1;
            SetWorkersPositionAtMouse();
            return;
        }
    } else {
        ResetWorkersOldCoords();
    }
    if (DAT_00668954 != 0) {
        SetWorkersPositionAtMouse();
    }
}

// FUNCTION: LEGOLAND 0x004708c0
LEGO_EXPORT void RenderWorkerOnMouse(void) {
    RenderBlokeIn3D((struct Bloke *)DAT_007fdff0);
}

// FUNCTION: LEGOLAND 0x004708d0
LEGO_EXPORT void ResetWorkersOldCoords(void) {
    struct Bloke *bloke;

    bloke = DAT_007fdff0;
    if (bloke != NULL) {
        bloke->pos.x = DAT_007fdff4;
        bloke = DAT_007fdff0;
        bloke->pos.y = DAT_007fdff8;
        bloke = DAT_007fdff0;
        bloke->field_50 = 0;
        if (DAT_007fdffc == 0x307) {
            NewLongTermAction(DAT_007fdff0, 0x10);
        } else {
            NewLongTermAction(DAT_007fdff0, 0x11);
        }
        ResetMoveAWorkerStruct();
    }
}

// FUNCTION: LEGOLAND 0x00470930
LEGO_EXPORT void ResetMoveAWorkerStruct(void) {
    DAT_00668954 = 0;
    DAT_007fdff0 = NULL;
    DAT_007fdffc = 0;
}

// FUNCTION: LEGOLAND 0x00470950
void FUN_00470950(void *a, void *b) {
    unsigned int temp_val, temp_val2;

    if (!DAT_00668938) {
        // STRING: LEGOLAND 0x004baa70
        DAT_00668938 = LoadSprite("PU_OK.lls", 4);
    }
    if (!DAT_00668934) {
        // STRING: LEGOLAND 0x004baa64
        DAT_00668934 = LoadSprite("PU_OKON.lls", 4);
    }
    if (!DAT_0066893c) {
        // STRING: LEGOLAND 0x004baa54
        DAT_0066893c = LoadSprite("CB_Close.lls", 4);
    }
    if (!DAT_00668940) {
        // STRING: LEGOLAND 0x004baa44
        DAT_00668940 = LoadSprite("CB_CloseON.lls", 4);
    }
    if (!DAT_00668904) {
        // STRING: LEGOLAND 0x004baa34
        DAT_00668904 = LoadSprite("CB_BGleft.lls", 4);
    }
    if (!DAT_00668908) {
        // STRING: LEGOLAND 0x004baa24
        DAT_00668908 = LoadSprite("CB_BGCentre.lls", 4);
    }
    if (!DAT_0066890c) {
        // STRING: LEGOLAND 0x004baa14
        DAT_0066890c = LoadSprite("CB_BGRight.lls", 4);
    }

    DAT_007fdea8 = InsertIcon(0, 0, 0x2c3, DAT_00668938);
    DAT_007fdea8->string_id = 0x74;
    DAT_007fdea8->string = GetString(0x74);
    DAT_007fdea8->flags |= 0x2000;
    DAT_007fdea8->flags |= 0x4002;
    temp_val = DAT_007fdea8->flags;
    temp_val2 = temp_val | 0x400;
    DAT_007fdea8->flags = temp_val2;
    DAT_007fdea8->event_handler = a;

    DAT_007fe000 = InsertIcon(0, 0, 0x2c3, DAT_0066893c);
    DAT_007fe000->string_id = 0x75;
    DAT_007fe000->string = GetString(0x75);
    DAT_007fe000->flags |= 0x2000;
    DAT_007fe000->flags |= 0x4002;
    temp_val = DAT_007fe000->flags;
    temp_val2 = temp_val | 0x400;
    DAT_007fe000->flags = temp_val2;
    DAT_007fe000->event_handler = b;
}

// FUNCTION: LEGOLAND 0x00470b00
void FUN_00470b00(void) {
    if (DAT_00668938) {
        KillSprite(DAT_00668938);
        DAT_00668938 = NULL;
    }
    if (DAT_00668934) {
        KillSprite(DAT_00668934);
        DAT_00668934 = NULL;
    }
    if (DAT_00668940) {
        KillSprite(DAT_00668940);
        DAT_00668940 = NULL;
    }
    if (DAT_0066893c) {
        KillSprite(DAT_0066893c);
        DAT_0066893c = NULL;
    }
    if (DAT_00668904) {
        KillSprite(DAT_00668904);
        DAT_00668904 = NULL;
    }
    if (DAT_00668908) {
        KillSprite(DAT_00668908);
        DAT_00668908 = NULL;
    }
    if (DAT_0066890c) {
        KillSprite(DAT_0066890c);
        DAT_0066890c = NULL;
    }
}
