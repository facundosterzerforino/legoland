#include <windows.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "legoland.h"

#include <float.h>
#include <math.h>
#include "bloke.h"
#include "castle.h"
#include "debug_alloc.h"
#include "draw.h"
#include "gamemap.h"
#include "interface.h"
#include "llidb.h"
#include "man3d.h"
#include "map_object.h"
#include "obj_instance.h"
#include "objclass.h"
#include "path_control.h"
#include "print_sprite.h"
#include "render3d.h"
#include "screens.h"
#include "tilemap.h"
#include "timer.h"

struct ClearRect {
    int left;
    int top;
    int right;
    int bottom;
};

struct EdgeVert {
    int f[5];
};

struct EdgePair {
    int a;
    int b;
};

struct EdgeObj {
    unsigned short color;
    unsigned short pad_2;
    int npts;
    int nedges;
    struct EdgePair edges[12];
    float pts[8][3];
};

struct EdgeSet {
    unsigned char pad_0[8];
    int count;
    unsigned char pad_c[4];
    struct EdgeVert *base;
    struct EdgePair *pairs;
};

#include "image_sprite.h"
// FUNCTION: LEGOLAND 0x0041cc50
unsigned int GetOppositeDirection(unsigned int dir) {
    if (dir == 1) return 4;
    if (dir == 4) return 1;
    if (dir == 2) return 8;
    if (dir == 8) return 2;
    if (dir == 0xffffffff) return 1;
    return 0;
}

// FUNCTION: LEGOLAND 0x0041cc90
unsigned int BitFromIndex(unsigned int shift) {
    return 1U << shift;
}

// FUNCTION: LEGOLAND 0x0041cca0
unsigned int DirMaskToIndex(unsigned int dir) {
    if (dir == 1) return 0;
    if (dir == 4) return 2;
    if (dir == 2) return 1;
    if (dir == 8) return 3;
    return 0;
}

// FUNCTION: LEGOLAND 0x0041cce0
void GetFootprintRectAt(short *pt, struct Ride *ride, int *out) {
    int *rect = ride->footprint.v;
    out[0] = pt[0] + rect[0];
    out[2] = pt[0] + rect[2];
    out[1] = pt[1] + rect[1];
    out[3] = pt[1] + rect[3];
}

// FUNCTION: LEGOLAND 0x0041cd20
unsigned int IsAreaListClearAt(short *a, struct AreaNode *b) {
    return IsAreaListClear(a, b);
}

// FUNCTION: LEGOLAND 0x0041cd40
int FindDoorPointIndex(unsigned int *val, unsigned int *rec) {
    int i;
    unsigned int bit = 1;

    for (i = 0; i <= 3; i++) {
        if ((bit & rec[0]) != 0 && *val == rec[i + 1]) {
            return i;
        }
        bit <<= 1;
    }
    return -1;
}

// FUNCTION: LEGOLAND 0x0041cd80
void FindOpenDoorPoints(short *pos, struct DirPoints *out, unsigned int mask) {
    int i;
    unsigned int bit = 1;

    out->mask = 0;
    for (i = 0; i * 4 + 4 <= 16; i++) {
        if ((mask & bit) != 0) {
            out->pt[i][0] = pos[0] + DAT_004b5584[i][0];
            out->pt[i][1] = pos[1] + DAT_004b5584[i][1];
            if (IsAreaListClearAt(&out->pt[i][0], &DAT_004b5570) == 1) {
                out->mask |= bit;
            }
        }
        bit <<= 1;
    }
}

// FUNCTION: LEGOLAND 0x0041ce10
void ResetAnim(struct PieceEnd *anim) {
    anim->link = 0;
    anim->dir = 0xffffffff;
    anim->height = 0;
}

// FUNCTION: LEGOLAND 0x0041ce30
void InitPieceHead(struct PieceHead *pair) {
    pair->flags = 0;
    pair->x = 0;
    pair->y = 0;
    pair->cls = 0;
    pair->owner = 0;
    ResetAnim(&pair->end_b);
    ResetAnim(&pair->end_a);
}

// FUNCTION: LEGOLAND 0x0041ce60
unsigned int FUN_0041ce60(struct PieceHead *pair) {
    unsigned int a = pair->end_a.dir;
    unsigned int lo;
    unsigned int b;

    if (a == 0xffffffff) {
        a = GetOppositeDirection(pair->end_b.dir);
    }
    lo = DirMaskToIndex(a);
    b = pair->end_b.dir;
    if (b == 0xffffffff) {
        b = GetOppositeDirection(pair->end_a.dir);
    }
    return (DirMaskToIndex(b) << 2) | lo;
}

// FUNCTION: LEGOLAND 0x0041ceb0
void AddPathSegMapObject(struct PathSeg *obj) {
    int pt[2];

    pt[0] = obj->x;
    pt[1] = obj->y;
    AddMapObjectIfEnabled(obj->ride->element, pt);
}

// FUNCTION: LEGOLAND 0x0041cee0
unsigned int CountPrevSegs(struct PathSeg *node) {
    unsigned int count = 0;
    while (node != NULL) {
        if (node->flags & 0x6) break;
        node = node->prev;
        count++;
    }
    return count;
}

// FUNCTION: LEGOLAND 0x0041cf00
unsigned int CountNextSegs(struct PathSeg *node) {
    unsigned int count = 0;
    while (node != NULL) {
        if (node->flags & 0x6) break;
        node = node->next;
        count++;
    }
    return count;
}

// FUNCTION: LEGOLAND 0x0041cf20
void FUN_0041cf20(struct PathSeg *src, unsigned int *out_b, unsigned int *out_a) {
    *out_a = 0xffffffff;
    *out_b = 0xffffffff;
    if (src->prev != NULL) {
        *out_a = CountPrevSegs(src->prev);
    }
    if (src->next != NULL) {
        *out_b = CountNextSegs(src->next);
    }
}

// FUNCTION: LEGOLAND 0x0041cf70
void FUN_0041cf70(struct CastleState *src, int *out_a, int *out_b) {
    *out_a = 0;
    *out_b = 0;
    if (src->flags == 2) return;
    *out_b = CountNextSegs(src->f_c0);
    *out_a = CountPrevSegs(src->f_a8);
}

// FUNCTION: LEGOLAND 0x0041cfc0
void FUN_0041cfc0(struct PathSeg *host) {
    host->cls->method_1c(host);
}

// FUNCTION: LEGOLAND 0x0041cfd0
unsigned int FUN_0041cfd0(struct PathSeg *host, float arg) {
    return host->cls->method_20(host, arg);
}

// FUNCTION: LEGOLAND 0x0041cff0
struct SegBlob *FUN_0041cff0(struct PathSeg *obj, struct FVec3 *pos) {
    if (obj->flags & 1) {
        *pos = obj->vec;
        return &obj->blob;
    }
    MapPosToWorld(&obj->pos, obj->f18, pos);
    return obj->cls->method_24(obj);
}

// FUNCTION: LEGOLAND 0x0041d060
struct PathSeg *FindPathSegByXY(struct CastleState *host, unsigned int *key) {
    if (host->flags & 0x2) {
        struct PathSeg *current = (struct PathSeg *)&host->piece;
        unsigned int search = *key;
        if (current->xy == search) {
            return current;
        }
        while (1) {
            current = current->next;
            if (current == (struct PathSeg *)&host->piece) {
                break;
            }
            if (current->xy == search) {
                return current;
            }
        }
    } else {
        struct PathSeg *current = host->f_c0;
        unsigned int search = *key;
        if (current->xy == search) {
            return current;
        }
        while (1) {
            current = current->next;
            if (current == NULL) {
                break;
            }
            if (current->xy == search) {
                return current;
            }
        }
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x0041d0b0
int IsPointInPathSegFootprint(struct PathSeg *obj, short *pt) {
    int r[5];

    GetFootprintRectAt(&obj->x, obj->ride, r);
    if (pt[0] >= r[0] && pt[0] <= r[2] && pt[1] >= r[1] && pt[1] <= r[3]) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041d100
struct PathSeg *FindPathSegAt(struct CastleState *host, struct Ride *ride, short *pt) {
    struct PathSeg *node;

    if (host->flags & 2) {
        node = (struct PathSeg *)&host->piece;
        do {
            if (node->ride == ride && IsPointInPathSegFootprint(node, pt)) {
                return node;
            }
            node = node->next;
        } while (node != (struct PathSeg *)&host->piece);
        return 0;
    }
    node = host->f_c0;
    do {
        if (node->ride == ride && IsPointInPathSegFootprint(node, pt)) {
            return node;
        }
        node = node->next;
    } while (node);
    return 0;
}

// FUNCTION: LEGOLAND 0x0041d170
unsigned int FindPathSegDoorAPoints(struct PathSeg *obj, struct DirPoints *pts) {
    return FindPathSegDoorPoints(obj, &obj->cls->door_a, pts);
}

// FUNCTION: LEGOLAND 0x0041d190
unsigned int FindPathSegDoorBPoints(struct PathSeg *obj, struct DirPoints *pts) {
    return FindPathSegDoorPoints(obj, &obj->cls->door_b, pts);
}

// FUNCTION: LEGOLAND 0x0041d1b0
void ClearDirPoints(struct DirPoints *pts) {
    pts->mask = 0;
}

// FUNCTION: LEGOLAND 0x0041d1c0
void FUN_0041d1c0(void *unused) {
}

// FUNCTION: LEGOLAND 0x0041d1d0
unsigned int FindPathSegDoorPoints(struct PathSeg *obj, struct PieceDoor *door, struct DirPoints *pts) {
    short sum[2];

    ClearDirPoints(pts);
    sum[0] = obj->x + door->x;
    sum[1] = obj->y + door->y;
    FindOpenDoorPoints(sum, pts, door->mask);
}

// FUNCTION: LEGOLAND 0x0041d210
int FUN_0041d210(short *pos, struct SprEnt *ent, struct PieceClass *info) {
    short p[2];
    int r;

    if (CastleCoaster.flags != 2) {
        ent->owner = (struct CastleState *)&CastleCoaster;
        p[0] = info->door_b.x + pos[0];
        p[1] = info->door_b.y + pos[1];
        r = FindDoorPointIndex((unsigned int *)p, &CastleCoaster.pts_a.mask);
        ent->id_a = r;
        if (r != -1) {
            CastleCoaster.f_a8->dir_b = BitFromIndex(r);
        }
        p[0] = info->door_a.x + pos[0];
        p[1] = info->door_a.y + pos[1];
        r = FindDoorPointIndex((unsigned int *)p, &CastleCoaster.pts_b.mask);
        ent->id_b = r;
        if (r != -1) {
            CastleCoaster.f_c0->dir_a = BitFromIndex(r);
        }
        if (ent->id_a != -1 || ent->id_b != -1) {
            return 1;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041d2e0
int FUN_0041d2e0(struct PieceClass *info, short *pos, struct SprEnt *ent) {
    if (FUN_0041d210(pos, ent, info)) {
        if (ent->id_a != -1) {
            ent->h_a = FUN_00429840(ent->owner->f_a8, info->f8);
            if (!ent->h_a) {
                return 0;
            }
        }
        if (ent->id_b != -1) {
            ent->h_b = FUN_00429840(ent->owner->f_c0, info->f4);
            if (!ent->h_b) {
                return 0;
            }
        }
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041d350
int FUN_0041d350(struct PieceClass *info, short *pos, struct SprEnt *ent) {
    if (FUN_0041d210(pos, ent, info)) {
        if (ent->id_a == -1 || ent->id_b == -1) {
            return 1;
        }
        return FUN_004298a0(info, ent->owner, &ent->h_a, &ent->h_b) != 0;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041d3b0
struct SprEnt *CheckCoasterPiecePlacement(struct PieceClass *cls, struct Int16Pair *pos) {
    int r;

    if (cls->flags & 1) {
        r = FUN_0041d2e0(cls, &pos->x, &CoasterPlacement);
    } else {
        r = FUN_0041d350(cls, &pos->x, &CoasterPlacement);
    }
    CoasterPlacement.field_0 = 0;
    if (r) {
        if (CoasterPlacement.id_a != -1) {
            CoasterPlacement.field_0 = 1;
        }
        if (CoasterPlacement.id_b != -1) {
            CoasterPlacement.field_0 |= 2;
        }
        return &CoasterPlacement;
    }
    CoasterPlacement.field_0 = 0;
    return &CoasterPlacement;
}

// FUNCTION: LEGOLAND 0x0041d430
void LinkPathSegs(struct PathSeg *a, struct PathSeg *b) {
    a->next = b;
    b->prev = a;
}

// FUNCTION: LEGOLAND 0x0041d440
void AttachPathSegToCoaster(struct PathSeg *obj, struct SprEnt *ent) {
    struct CastleState *owner = ent->owner;
    int *r;

    obj->owner = owner;
    if (ent->id_a != -1) {
        LinkPathSegs(owner->f_a8, obj);
        owner->f_a8->dir_b = BitFromIndex(ent->id_a);
        obj->dir_a = GetOppositeDirection(owner->f_a8->dir_b);
        FUN_0041cfd0(obj, owner->f_a8->f24);
        owner->f_a8 = obj;
        FindPathSegDoorAPoints(obj, &owner->pts_a);
    }
    if (ent->id_b != -1) {
        LinkPathSegs(obj, owner->f_c0);
        owner->f_c0->dir_a = BitFromIndex(ent->id_b);
        obj->dir_b = GetOppositeDirection(owner->f_c0->dir_a);
        FUN_0041cfd0(obj, owner->f_c0->f18);
        owner->f_c0 = obj;
        FindPathSegDoorBPoints(obj, &owner->pts_b);
    }
    if (ent->id_a != -1 && ent->id_b != -1) {
        owner->flags = 2;
        FUN_00424e60(owner);
    } else {
        FUN_0041cfc0((struct PathSeg *)obj);
        FUN_0041d1c0(&owner->pts_a);
        FUN_0041d1c0(&owner->pts_b);
    }
    AddPathSegMapObject(obj);
    r = obj->ride->footprint.v;
    obj->fp.v[0] = r[0] - owner->piece.x + obj->x;
    obj->fp.v[2] = r[2] - owner->piece.x + obj->x;
    obj->fp.v[1] = r[1] - owner->piece.y + obj->y;
    obj->fp.v[3] = r[3] - owner->piece.y + obj->y;
    obj->fp.next = 0;
}

// FUNCTION: LEGOLAND 0x0041d5b0
struct PathSeg *FUN_0041d5b0(struct Ride *ride, struct PieceClass *cls, struct Int16Pair *pos, struct SprEnt *ent) {
    struct PathSeg *obj = MemAlloc(sizeof(struct PathSeg), 0, 0, 0);

    if (obj == NULL) {
        return 0;
    }
    InitPieceHead((struct PieceHead *)obj);
    obj->ride = ride;
    obj->owner = ent->owner;
    obj->pos = *pos;
    obj->cls = cls;
    AttachPathSegToCoaster(obj, ent);
    if (ent->id_a != -1 && ent->id_b != -1) {
        InterpolateCoasterHeights(ent->h_a, ent->h_b);
    }
    obj->cls->method_28(obj);
    return obj;
}

// FUNCTION: LEGOLAND 0x0041d630
struct PathSeg *FUN_0041d630(struct Ride *ride, struct PieceClass *cls, struct Int16Pair *pos, struct SprEnt *ent) {
    struct PathSeg *obj = MemAlloc(sizeof(struct PathSeg), 0, 0, 0);

    if (obj == NULL) {
        return 0;
    }
    InitPieceHead((struct PieceHead *)obj);
    obj->ride = ride;
    obj->owner = ent->owner;
    obj->pos = *pos;
    obj->cls = cls;
    AttachPathSegToCoaster(obj, ent);
    if (ent->id_a != -1) {
        InterpolateCoasterHeights(ent->h_a, obj);
    }
    if (ent->id_b != -1) {
        InterpolateCoasterHeights(obj, ent->h_b);
    }
    obj->cls->method_28(obj);
    return obj;
}

// FUNCTION: LEGOLAND 0x0041d6c0
void SetCoasterPieceCount(unsigned int value) {
    CoasterPieceCount = value;
}

// FUNCTION: LEGOLAND 0x0041d6d0
unsigned int AddCoasterPieceCount(unsigned int delta) {
    CoasterPieceCount += delta;
    return CoasterPieceCount;
}

// FUNCTION: LEGOLAND 0x0041d6f0
unsigned int GetCoasterPieceCount(void) {
    return CoasterPieceCount;
}

// FUNCTION: LEGOLAND 0x0041d700
struct PathSeg *AddCoasterPiece(struct Ride *ride, struct PieceClass *cls, struct Int16Pair *pos) {
    struct SprEnt *result = CheckCoasterPiecePlacement(cls, pos);
    if (result->field_0 != 0) {
        AddCoasterPieceCount(1);
        if ((cls->flags & 1) != 0) {
            return FUN_0041d630(ride, cls, pos, &CoasterPlacement);
        } else {
            return FUN_0041d5b0(ride, cls, pos, &CoasterPlacement);
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041d760
void RemovePathSegMapObject(struct PathSeg *obj) {
    TileId tile;
    struct Point pt;

    tile.pos.x = (unsigned char)obj->x;
    pt.x = obj->x;
    tile.pos.y = (unsigned char)obj->y;
    pt.y = obj->y;
    BasicObjectDCalcCursor(obj->ride->element, &pt);
    RemoveMapObjectIfEnabled(obj->ride->element, tile, &QueryCursor);
}

// FUNCTION: LEGOLAND 0x0041d7c0
unsigned int IsCoasterEndPiece(struct PathSeg *value) {
    if (CastleCoaster.flags != 2) {
        if (value != CastleCoaster.f_a8) {
            if (value != CastleCoaster.f_c0) {
                return 0;
            }
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0041d7f0
void RemoveCoasterPiece(void *arg) {
    struct PathSeg *obj = arg;
    struct CastleState *owner = obj->owner;

    if (obj != NULL) {
        RemovePathSegMapObject(obj);
        AddCoasterPieceCount(-1);
        if (owner->flags == 2) {
            ResetAnim((struct PieceEnd *)&obj->next->dir_a);
            ResetAnim((struct PieceEnd *)&obj->prev->dir_b);
            owner->f_a8 = obj->prev;
            FindPathSegDoorAPoints(obj->prev, &CastleCoaster.pts_a);
            owner->f_c0 = obj->next;
            FindPathSegDoorBPoints(obj->next, &CastleCoaster.pts_b);
            ResetCoasterHeightsBack((struct PathSeg *)owner->f_a8);
            ResetCoasterHeightsForward((struct PathSeg *)owner->f_c0);
            FUN_0041cfc0((struct PathSeg *)owner->f_a8);
            FUN_0041cfc0((struct PathSeg *)owner->f_c0);
            owner->flags = 1;
            FUN_00424e70(owner);
        } else if (obj == owner->f_a8) {
            ResetAnim((struct PieceEnd *)&obj->prev->dir_b);
            owner->f_a8 = obj->prev;
            FindPathSegDoorAPoints(obj->prev, &CastleCoaster.pts_a);
            ResetCoasterHeightsBack((struct PathSeg *)owner->f_a8);
            FUN_0041cfc0((struct PathSeg *)owner->f_a8);
        } else {
            ResetAnim((struct PieceEnd *)&obj->next->dir_a);
            owner->f_c0 = obj->next;
            FindPathSegDoorBPoints(obj->next, &CastleCoaster.pts_b);
            ResetCoasterHeightsForward((struct PathSeg *)owner->f_c0);
            FUN_0041cfc0((struct PathSeg *)owner->f_c0);
        }
        FUN_0041d1c0(&CastleCoaster.pts_a.mask);
        FUN_0041d1c0(&CastleCoaster.pts_b.mask);
        ((struct PieceClass *)obj->cls)->method_2c(obj);
        MemFree(obj);
    }
}

// FUNCTION: LEGOLAND 0x0041d950
void PositionTrainCarsAndBogies(void *obj, float val, void *b) {
    struct Train *host = obj;
    struct TrainCar *head;
    struct TrainCar *cur;
    struct TrainCar *next;
    struct CurveWalker elem;
    struct CurveWalker out;
    unsigned int tmp[3];
    float t;
    float f;

    next = host->head.next;
    head = &host->head;
    cur = head;
    host->motion.f4 = val;
    host->walker = *(struct CurveWalker *)b;
    PositionTrainCarAndBogies(head, (struct CurveWalker *)b, val);
    while (next != head) {
        t = cur->rear.w0.t;
        elem = cur->rear.w0.walker;
        GetTrainCarRearPos(cur, (float *)tmp);
        FindPathPosAtRangeBehind((float *)tmp, 30.0f, &elem, t, 4.8f, &out, &f);
        PositionTrainCarAndBogies(next, &out, f);
        cur = next;
        next = next->next;
    }
}

// FUNCTION: LEGOLAND 0x0041da10
void PositionTrainCars(void *obj, float val, void *b) {
    struct Train *host = obj;
    struct TrainCar *head;
    struct TrainCar *cur;
    struct TrainCar *next;
    struct CurveWalker elem;
    struct CurveWalker out;
    unsigned int tmp[3];
    float t;
    float f;

    next = host->head.next;
    head = &host->head;
    cur = head;
    host->motion.f4 = val;
    host->walker = *(struct CurveWalker *)b;
    PositionTrainCar(head, (struct CurveWalker *)b, val);
    while (next != head) {
        t = cur->rear.w0.t;
        elem = cur->rear.w0.walker;
        GetTrainCarRearPos(cur, (float *)tmp);
        FindPathPosAtRangeBehind((float *)tmp, 30.0f, &elem, t, 4.8f, &out, &f);
        PositionTrainCar(next, &out, f);
        cur = next;
        next = next->next;
    }
}

// FUNCTION: LEGOLAND 0x0041dad0
void FUN_0041dad0(struct Train *holder, float value) {
    holder->motion.f8 = value;
}

// FUNCTION: LEGOLAND 0x0041dae0
float SumTrainCarWeights(struct Train *obj) {
    float sum = 0.0f;
    struct TrainCar *head = &obj->head;
    struct TrainCar *cur = head;

    do {
        sum += GetTrainCarWeight(cur);
        cur = cur->next;
    } while (cur != head);
    return sum;
}

// FUNCTION: LEGOLAND 0x0041db20
void FUN_0041db20(float a, struct FloatArray *out) {
    struct TrainCar *node;
    float *p;
    unsigned int n;

    n = 1;
    node = &DAT_004d83b4->head;
    PositionTrainCars(DAT_004d83b4, a, &DAT_004d83a0);
    out->data[0] = 0.0f;
    p = &out->data[1];
    do {
        out->data[0] += GetTrainCarWeight(node);
        GetTrainCarCenter((unsigned char *)node, (unsigned int *)p);
        node = node->next;
        n += 3;
        p += 3;
    } while (node != &DAT_004d83b4->head);
    out->count = n;
}

// FUNCTION: LEGOLAND 0x0041db90
void FUN_0041db90(struct Train *obj, float *a, float *b) {
    struct TrainCar *node;
    struct CurveWalker saved;
    float savedval;
    struct FloatArray out;
    float *p;
    float sum;
    volatile float x; /* volatile: the original re-reads this from its stack slot */
    int k;

    saved = obj->walker;
    DAT_004d83b4 = obj;
    savedval = obj->motion.f4;
    DAT_004d83a0 = obj->walker;
    VecDerivative(FUN_0041db20, &DAT_004d8270, savedval, 0.1f, &out);
    *b = out.data[0];
    *a = 0.0f;
    p = &out.data[1];
    node = &obj->head;
    do {
        sum = FLOAT_004ab390;
        for (k = 0; k < 3; k++) {
            x = *p++;
            sum += x * x;
        }
        *a += GetTrainCarMass(node) * sum * 2.5201562e-06f;
        node = node->next;
    } while (node != &obj->head);
    obj->walker = saved;
    obj->motion.f4 = savedval;
    DAT_004d829c[DAT_004d83c0 & 0x3f] = *a;
    DAT_004d83c0++;
}

// FUNCTION: LEGOLAND 0x0041dca0
float FUN_0041dca0(struct Train *param) {
    float a;
    float b;
    float t;
    float r;

    t = SumTrainCarWeights(param);
    FUN_0041db90(param, &a, &b);
    r = (param->motion.f8 - t) * 2.0f / a;
    if (r > FLOAT_004ab390) {
        return (float)sqrt(r);
    }
    return FLOAT_004ab390;
}

// FUNCTION: LEGOLAND 0x0041dd00
float FUN_0041dd00(struct Train *param, float result) {
    struct FVec3 v;
    float t;

    FUN_00429c60(&param->walker, 1, *(unsigned int *)&param->motion.f4, 0.0f, &v);
    t = lego_invsqrtf(Vec3Dot(&v, &v));
    t = t * result;
    return t * FLOAT_004ab404;
}

// FUNCTION: LEGOLAND 0x0041dd50
float FUN_0041dd50(struct Train *param) {
    float result = FUN_0041dca0(param);
    return FUN_0041dd00(param, result);
}

// FUNCTION: LEGOLAND 0x0041dd70
float SumTrainCarMasses(struct Train *obj) {
    float sum = 0.0f;
    struct TrainCar *head = &obj->head;
    struct TrainCar *cur = head;

    do {
        sum += GetTrainCarMass(cur);
        cur = cur->next;
    } while (cur != head);
    return sum;
}

// FUNCTION: LEGOLAND 0x0041ddb0
float GetTrainKineticEnergy(struct Train *param_1, float param_2) {
    float result = SumTrainCarMasses(param_1);
    result = result * param_2;
    result = result * param_2;
    result = result * 0.5f;
    return result;
}

// FUNCTION: LEGOLAND 0x0041ddd0
void FUN_0041ddd0(struct Integrator *a, struct FloatArray *b) {
    struct Train *e = a->owner;

    e->motion.kind = 2;
    PositionTrainCars(e, b->data[0], &e->walker);
    FUN_0041dad0(e, b->data[1]);
}

// FUNCTION: LEGOLAND 0x0041de10
void FUN_0041de10(struct Integrator *obj, float unused, struct FloatArray *out) {
    struct Train *e = obj->owner;
    float a;
    float b;
    float t;

    out->count = 2;
    t = SumTrainCarWeights(e);
    FUN_0041db90(e, &a, &b);
    t = (e->motion.f8 - t) * 2.0f / a;
    if (t > FLOAT_004ab390) {
        t = lego_invsqrtf(t);
    } else {
        t = FLOAT_004ab390;
    }
    out->data[0] = t;
    out->data[1] = 0.0f;
    if (b > FLT_MIN) {
        if (FUN_0041dd00(e, t) < 0.05) {
            out->data[1] = b;
        }
    }
}

// FUNCTION: LEGOLAND 0x0041dec0
void InitTrainIntegrator(struct Train *self) {
    InitIntegrator(&self->integ, 2);
    self->integ.store = FUN_0041ddd0;
    self->integ.state = &self->motion;
    self->integ.owner = self;
    self->integ.deriv = FUN_0041de10;
    self->motion.kind = 2;
}

struct BisectPair {
    float p;
    unsigned int q;
};

struct BisectOut {
    float p;
    unsigned int q;
    unsigned int r;
    unsigned int w;
};

// FUNCTION: LEGOLAND 0x0041df00
float BisectTrainSegmentEnd(struct Train *obj, float t) {
    struct BisectPair s;
    struct AnimOut anim;
    float lo;
    float hi;

    /* bisection on t in [0, t]: t itself holds the midpoint being tried */
    anim = obj->motion;
    hi = t;
    lo = 0.0f;
    s.p = 0.0f;
    s.q = anim.kind;
    while (hi - lo > DAT_004ab418) {
        t = (hi + lo) * 0.5f;
        PositionTrainCars(obj, anim.f4, &obj->walker);
        FUN_0041dad0((struct Train *)obj, anim.f8);
        IntegratorStepRK4(&obj->integ, &s.p, t);
        if (obj->motion.f4 > obj->walker.node->end) {
            hi = t;
        } else {
            lo = t;
        }
    }
    PositionTrainCars(obj, anim.f4, &obj->walker);
    FUN_0041dad0((struct Train *)obj, anim.f8);
    t = (hi + lo) * 0.5f;
    IntegratorStepRK4(&obj->integ, &s.p, t);
    return t;
}

// FUNCTION: LEGOLAND 0x0041e000
float StepTrainToSegmentEnd(struct Train *obj, float total) {
    struct BisectOut s;
    struct AnimOut anim;
    int i = 0;
    int n = (int)ceil(total * 70.0f);
    float acc = 0.0f;
    float dt;

    s.p = 0.0f;
    dt = total / n;
    for (i = 0; i < n; i++) {
        anim = obj->motion;
        s.q = anim.kind;
        IntegratorStepRK4(&obj->integ, &s.p, dt);
        if (obj->motion.f4 > obj->walker.node->end) {
            PositionTrainCars(obj, anim.f4, &obj->walker);
            FUN_0041dad0((struct Train *)obj, anim.f8);
            return BisectTrainSegmentEnd(obj, dt) + acc;
        }
        acc += dt;
    }
    return total;
}

// FUNCTION: LEGOLAND 0x0041e0e0
float FUN_0041e0e0(struct Train *self, float time_left) {
    return self->track->method_110(self, time_left);
}

// FUNCTION: LEGOLAND 0x0041e100
float FUN_0041e100(struct Train *self, float time_left) {
    void (*saved)(struct Integrator *, float, struct FloatArray *) = self->integ.deriv;
    float used = self->track->method_114(self, time_left);

    self->integ.deriv = saved;
    return used;
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x0041e130
__declspec(naked) void FUN_0041e130(void) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x14
        push ebx
        push esi
        push edi
        call GetGameTimer
        mov esi, dword ptr [ebp + 8]
        mov dword ptr [ebp - 0xc], eax
        fild dword ptr [ebp - 0xc]
        mov eax, dword ptr [esi + 0x6c]
        mov dword ptr [ebp + 8], 0
        mov dword ptr [ebp - 0x14], eax
        fisub dword ptr [esi + 4]
        fmul dword ptr [FLOAT_004ab42c]
        fst dword ptr [ebp - 4]
        fcomp dword ptr [FLOAT_004ab428]
        fnstsw ax
        test ah, 0x41
        jne L41e174
        mov dword ptr [ebp - 4], 0x3f4ccccd
L41e174:
        push eax
        push edx
        rdtsc
        mov dword ptr [ebp - 8], eax
        pop edx
        pop eax
        fld dword ptr [ebp - 4]
        fsub dword ptr [FLOAT_004ab424]
        lea ebx, [esi + 0xc]
        fstp dword ptr [ebp - 0x10]
L41e18c:
        mov eax, dword ptr [esi]
        mov edi, dword ptr [ebx]
        test al, 8
        je L41e1a6
        fld dword ptr [ebp - 4]
        fsub dword ptr [ebp + 8]
        push ecx
        fstp dword ptr [esp]
        push esi
        call FUN_0041e0e0
        jmp L41e1c1
L41e1a6:
        fld dword ptr [ebp - 4]
        fsub dword ptr [ebp + 8]
        push ecx
        test al, 0x10
        fstp dword ptr [esp]
        push esi
        je L41e1bc
        call FUN_0041e100
        jmp L41e1c1
L41e1bc:
        call StepTrainToSegmentEnd
L41e1c1:
        fadd dword ptr [ebp + 8]
        add esp, 8
        fstp dword ptr [ebp + 8]
        fld dword ptr [ebp + 8]
        fcomp dword ptr [ebp - 0x10]
        fnstsw ax
        test ah, 1
        je L41e216
        push ebx
        call CurveWalkerNext
        mov ecx, dword ptr [esi + 0x10]
        add esp, 4
        mov edx, dword ptr [ecx + 0x44]
        mov ecx, dword ptr [ebx]
        cmp edi, ecx
        mov dword ptr [esi + 0x24], edx
        je L41e18c
        mov eax, dword ptr [esi]
        test al, 8
        je L41e1fd
        and al, 0xf7
        or al, 4
        mov dword ptr [esi], eax
        jmp L41e18c
L41e1fd:
        test al, 4
        je L41e18c
        mov edx, dword ptr [ebp - 0x14]
        add edx, 4
        cmp ecx, edx
        jne L41e18c
        and al, 0xfb
        or al, 0x10
        mov dword ptr [esi], eax
        jmp L41e18c
L41e216:
        mov eax, dword ptr [ebp - 0xc]
        mov dword ptr [esi + 4], eax
        push eax
        push edx
        rdtsc
        sub eax, dword ptr [ebp - 8]
        mov dword ptr [ebp - 8], eax
        pop edx
        pop eax
        mov ecx, dword ptr [ebp - 8]
        mov eax, dword ptr [DAT_004d83bc]
        add eax, ecx
        pop edi
        pop esi
        mov dword ptr [DAT_004d83bc], eax
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// FUNCTION: LEGOLAND 0x0041e240
void UpdateTrainStartDelay(struct Train *timed) {
    int now = GetGameTimer();
    if (now > timed->field_8) {
        timed->flags &= ~0x40;
    }
    timed->field_4 = now;
}

// FUNCTION: LEGOLAND 0x0041e260
unsigned int PlaceInEmptyTrainSeat(struct Train *host, void *arg) {
    struct TrainCar *node = &host->head;

    while (PlaceInEmptySeatOfCar(node, arg) == 0) {
        node = node->next;
        if (node == &host->head) {
            return 0;
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0041e2b0
struct TrainSeat *FindEmptyTrainSeat(struct Train *host) {
    struct TrainCar *node = &host->head;
    struct TrainSeat *r;

    while ((r = FindEmptySeatInCar(node)) == 0) {
        node = node->next;
        if (node == &host->head) {
            return 0;
        }
    }
    return r;
}

// FUNCTION: LEGOLAND 0x0041e2f0
struct CastleVisitor *FindTrainOccupant(struct Train *host) {
    struct TrainCar *node = &host->head;
    struct CastleVisitor *r;
    while ((r = FindOccupantInCar(node)) == 0) {
        node = node->next;
        if (node == &host->head) {
            return 0;
        }
    }
    return r;
}

// FUNCTION: LEGOLAND 0x0041e330
void ForEachRingNode(struct Train *host, void (*visit)(struct TrainCar *car)) {
    struct TrainCar *start = &host->head;
    struct TrainCar *node = start;
    do {
        visit(node);
        node = node->next;
    } while (node != start);
}

// FUNCTION: LEGOLAND 0x0041e360
void FUN_0041e360(struct Train *host) {
    ForEachRingNode(host, FUN_0041e950);
}

// FUNCTION: LEGOLAND 0x0041e380
void UnlinkAllTrainCars(struct Train *host) {
    ForEachRingNode(host, UnlinkTrainCar);
}

// FUNCTION: LEGOLAND 0x0041e3a0
void UpdateAllTrainCarBounds(struct Train *host) {
    ForEachRingNode(host, UpdateTrainCarBounds);
}

// FUNCTION: LEGOLAND 0x0041e3c0
unsigned int DrawTrainCarOnDrawPiece(struct TrainCar *param) {
    return DrawTrainCarOnPiece(param, TrainDrawPiece);
}

// FUNCTION: LEGOLAND 0x0041e3e0
void DrawTrainCarsOnPiece(struct Train *host, struct PathSeg *value) {
    TrainDrawPiece = value;
    ForEachRingNode(host, (void (*)(struct TrainCar *))DrawTrainCarOnDrawPiece);
}

// FUNCTION: LEGOLAND 0x0041e400
void ClearAllTrainCarsDrawn(struct Train *host) {
    ForEachRingNode(host, ClearTrainCarDrawn);
}

// FUNCTION: LEGOLAND 0x0041e420
void AddTrainCar(struct Train *self, unsigned int flag) {
    struct TrainCar *node = AllocTrainCar(flag);
    struct TrainCar *head;

    if (node != NULL) {
        self->head.next->prev = node;
        head = &((struct Train *)self)->head;
        node->next = head->next;
        head->next = node;
        node->prev = head;
    }
}

// FUNCTION: LEGOLAND 0x0041e460
void RemoveTrainCar(struct TrainCar *node, struct Train *host) {
    if (node != &host->head) {
        node->next->prev = node->prev;
        node->prev->next = node->next;
        FreeTrainCar(node);
    }
}

// FUNCTION: LEGOLAND 0x0041e4a0
unsigned int IsTrainRunning(struct Train *obj) {
    if (*(signed char *)&obj->flags & 2) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041e4b0
unsigned int IsTrainStartDelayed(struct Train *obj) {
    if (*(signed char *)&obj->flags & 0x40) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041e4c0
unsigned int StartTrain(struct Train *obj, unsigned int delay) {
    obj->flags |= 2;
    if (delay != 0) {
        obj->flags |= 0x40;
        obj->field_8 = GetGameTimer() + delay;
        return obj->field_8;
    }
    obj->flags &= ~0x40;
    return obj->flags;
}

// FUNCTION: LEGOLAND 0x0041e4f0
void StopTrain(struct Train *obj) {
    obj->flags &= 0xfffffffd;
    FUN_0041e500(obj);
}

// FUNCTION: LEGOLAND 0x0041e500
void FUN_0041e500(struct Train *obj) {
    struct CurveWalker t;
    float slot;

    obj->flags = 8;
    t.piece = (struct PathSeg *)&obj->track->piece;
    obj->track->method_10c(obj->track, &t.node, &slot, &t.pos);
    PositionTrainCarsAndBogies(obj, slot, &t);
    obj->field_4 = GetGameTimer();
    obj->motion.f8 = SumTrainCarWeights(obj);
    obj->motion.f8 = GetTrainKineticEnergy(obj, 0.1f) + obj->motion.f8;
}

// FUNCTION: LEGOLAND 0x0041e570
struct Train *CreateTrain(struct CastleState *arg) {
    struct Train *self = MemAlloc(sizeof(struct Train), 0, 0, 0);
    if (self == NULL) {
        return NULL;
    }
    self->track = arg;
    InitTrainCar(&self->head, 0);
    AddTrainCar(self, 2);
    AddTrainCar(self, 1);
    self->flags = 0;
    InitTrainIntegrator((struct Train *)self);
    FUN_0041e500(self);
    FUN_0041e360((struct Train *)self);
    return self;
}

// FUNCTION: LEGOLAND 0x0041e5d0
void DestroyTrain(struct Train **param_1) {
    struct Train *host = (struct Train *)*param_1;
    struct TrainCar *node = &host->head;
    struct TrainCar *next;

    UnlinkAllTrainCars(host);
    do {
        next = node->next;
        RemoveTrainCar(node, host);
        node = next;
    } while (next != &host->head);
    MemFree(*param_1);
    *param_1 = NULL;
}

// FUNCTION: LEGOLAND 0x0041e620
void FUN_0041e620(void) {
    InitVecMath(&DAT_004d8270, 10);
}

// FUNCTION: LEGOLAND 0x0041e630
void ClearTrainCarDrawn(struct TrainCar *obj) {
    obj->flags &= 0xfffffffe;
}

// FUNCTION: LEGOLAND 0x0041e640
void SetTrainCarDrawnFlag(unsigned int *flags, unsigned int set) {
    if (set == 1) {
        *flags |= 1;
    } else {
        *flags &= 0xfffffffe;
    }
}

// FUNCTION: LEGOLAND 0x0041e660
unsigned int IsTrainCarDrawn(struct TrainCar *a) {
    if (*(signed char *)&a->flags & 1) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041e670
unsigned int IsTrainCarOnPiece(struct TrainCar *obj, struct PathSeg *id) {
    if (obj->front.w0.walker.piece == id) {
        return 1;
    }
    return obj->rear.w0.walker.piece == id;
}

// FUNCTION: LEGOLAND 0x0041e6a0
void InitTrainCar(struct TrainCar *param, unsigned int value) {
    float v[3];

    v[1] = 0.0f;
    v[2] = -4.0f;
    param->kind = value;
    v[0] = DAT_004b559c[value];
    InitTrainSeat(&param->seats[0], (const struct FVec3 *)v);
    v[0] = v[0] - FLOAT_004b55a8;
    InitTrainSeat(&param->seats[1], (const struct FVec3 *)v);
    param->next = param;
    param->prev = param;
    param->flags = 0;
}

// FUNCTION: LEGOLAND 0x0041e720
unsigned int PlaceInEmptySeatOfCar(struct TrainCar *host, void *arg) {
    struct TrainSeat *slot = &host->seats[0];
    int i = 0;
    while (i <= 1) {
        if (slot->method_10(slot) == 0) {
            slot->method_14(slot, arg);
            return 1;
        }
        i++;
        slot++;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041e760
struct TrainSeat *FindEmptySeatInCar(struct TrainCar *host) {
    struct TrainSeat *slot = &host->seats[0];
    int i = 0;
    while (i <= 1) {
        if (slot->method_10(slot) == 0) {
            return slot;
        }
        i++;
        slot++;
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x0041e790
struct CastleVisitor *FindOccupantInCar(struct TrainCar *host) {
    struct TrainSeat *slot = &host->seats[0];
    int i = 0;

    while (i <= 1) {
        if (slot->method_10(slot) != 0) {
            return TakeTrainSeatOccupant(slot);
        }
        i++;
        slot++;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041e7c0
void FUN_0041e7c0(struct TrainCar *host) {
    struct TrainSeat *slot = &host->seats[0];
    int i = 2;
    do {
        slot->method_18(slot);
        slot++;
        i--;
    } while (i != 0);
}

// FUNCTION: LEGOLAND 0x0041e7e0
float GetTrainCarMass(struct TrainCar *obj) {
    return DAT_004ab430;
}

// FUNCTION: LEGOLAND 0x0041e7f0
void GetTrainCarCenter(unsigned char *obj, unsigned int *out) {
    obj += offsetof(struct TrainCar, center);
    out[0] = *(unsigned int *)obj;
    out[1] = *(unsigned int *)(obj + 4);
    out[2] = *(unsigned int *)(obj + 8);
}

// FUNCTION: LEGOLAND 0x0041e810
float GetTrainCarWeight(struct TrainCar *obj) {
    return obj->weight;
}

// FUNCTION: LEGOLAND 0x0041e820
void PositionTrainCar(struct TrainCar *obj, struct CurveWalker *e, float t) {
    float vA[3];
    float vB[3];
    struct CurveWalker k;
    float f;

    SetTrainWheelPos(&obj->front.w0, e, *(unsigned int *)&t);
    GetTrainWheelPos(&obj->front.w0, 2, vA);
    FindPathPosAtRangeBehind(vA, 30.0f, &obj->front.w0.walker, obj->front.w0.t, 4.8f, &k, &f);
    SetTrainWheelPos(&obj->rear.w0, &k, *(unsigned int *)&f);
    GetTrainWheelPos(&obj->rear.w0, 2, vB);
    obj->center.x = (vA[0] + vB[0]) * 0.5f;
    obj->center.y = (vA[1] + vB[1]) * 0.5f;
    obj->center.z = (vA[2] + vB[2]) * 0.5f;
    obj->weight = GetTrainCarMass(obj) * obj->center.z * -0.00134937500115484f;
}

// FUNCTION: LEGOLAND 0x0041e8f0
void PositionTrainCarAndBogies(struct TrainCar *obj, struct CurveWalker *e, float t) {
    PositionTrainCar(obj, e, t);
    InitTrainBogie(&obj->front, &obj->front.w0.walker, obj->front.w0.t);
    InitTrainBogie(&obj->rear, &obj->rear.w0.walker, obj->rear.w0.t);
}

// FUNCTION: LEGOLAND 0x0041e930
unsigned int GetTrainCarRearPos(struct TrainCar *obj, float *param) {
    return GetTrainWheelPos(&obj->rear.w0, 2, param);
}

// FUNCTION: LEGOLAND 0x0041e950
void FUN_0041e950(struct TrainCar *obj) {
    FUN_004266b0(&obj->link);
}

// FUNCTION: LEGOLAND 0x0041e970
void UnlinkTrainCar(struct TrainCar *param_1) {
    UnlinkListLink(&param_1->link);
}

struct RectI {
    int var_0;
    int var_4;
    int var_8;
    int var_c;
};

// FUNCTION: LEGOLAND 0x0041e990
void UpdateTrainCarBounds(struct TrainCar *obj) {
    struct {
        struct RectI r;
        unsigned int loc1[3];
        unsigned int b[9];
    } s;

    GetTrainCarCenterAndAxis(obj, (float *)s.loc1);
    FUN_00420fb0(CoasterTrainCarLms[0], s.loc1, s.b, &s.r);
    FUN_00426700((struct RectI *)&obj->link, &s.r);
}

// FUNCTION: LEGOLAND 0x0041e9e0
void GetTrainCarCenterAndAxis(struct TrainCar *obj, float *out) {
    float a[3];
    float b[3];

    GetTrainWheelPos(&obj->front.w0, 2, a);
    GetTrainWheelPos(&obj->rear.w0, 2, b);
    out[0] = (a[0] + b[0]) * 0.5f;
    out[1] = (a[1] + b[1]) * 0.5f;
    out[2] = (a[2] + b[2]) * 0.5f;
    out[3] = a[0] - b[0];
    out[4] = a[1] - b[1];
    out[5] = a[2] - b[2];
    BuildUprightBasis((struct FVec3 *)(out + 3), (struct FVec3 *)(out + 3));
}

// FUNCTION: LEGOLAND 0x0041ea70
void DrawTrainCar(struct TrainCar *a) {
    struct CarXform x;

    FUN_00425c40();
    DrawTrainBogie(&a->front);
    DrawTrainBogie(&a->rear);
    GetTrainCarCenterAndAxis(a, &x.pos.x);
    DrawLmsModel(CoasterTrainCarLms[a->kind], CoasterTrainCarLfm[a->kind], &x.pos, x.m, 0);
    a->seats[0].draw(&a->seats[0], &x);
    a->seats[1].draw(&a->seats[1], &x);
}

// FUNCTION: LEGOLAND 0x0041eaf0
unsigned int DrawTrainCarOnPiece(struct TrainCar *a, struct PathSeg *b) {
    if (IsTrainCarDrawn(a) == 0) {
        if (IsTrainCarOnPiece(a, b) != 0) {
            DrawTrainCar(a);
            SetTrainCarDrawnFlag(&a->flags, 1);
        }
    }
}

// FUNCTION: LEGOLAND 0x0041eb30
struct TrainCar *AllocTrainCar(unsigned int arg) {
    struct TrainCar *obj = MemAlloc(sizeof(struct TrainCar), 0, 0, 0);
    if (obj == NULL) {
        return NULL;
    }
    InitTrainCar(obj, arg);
    return obj;
}

// FUNCTION: LEGOLAND 0x0041eb60
void FreeTrainCar(void *param_1) {
    MemFree(param_1);
}

// FUNCTION: LEGOLAND 0x0041eb70
void LoadCoasterTrainCarModels(void) {
    // STRING: LEGOLAND 0x004b55d8
    CoasterTrainCarLms[0] = GetLmsByName("coastertrain.headcar");
    // STRING: LEGOLAND 0x004b55c4
    CoasterTrainCarLms[1] = GetLmsByName("coastertrain.midcar");
    // STRING: LEGOLAND 0x004b55ac
    CoasterTrainCarLms[2] = GetLmsByName("coastertrain.tailcar");
    CoasterTrainCarLfm[0] = GetLfmByName("coastertrain.headcar");
    CoasterTrainCarLfm[1] = GetLfmByName("coastertrain.midcar");
    CoasterTrainCarLfm[2] = GetLfmByName("coastertrain.tailcar");
}

// FUNCTION: LEGOLAND 0x0041ebd0
unsigned int FindCastleDispatchIndex(struct Ride *arg) {
    unsigned int index = 0;
    struct DispatchRow *row = CastleDispatchTable;
    while ((INT_PTR)row < (INT_PTR)&DAT_0082adb0) {
        struct Element *target = row->element;
        if (target != NULL && arg == target->ride) {
            return index;
        }
        index++;
        row++;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041ec00
struct Ride *GetCastleDispatchRide(unsigned int param) {
    struct Element *target = CastleDispatchTable[param].element;
    return target->ride;
}

// FUNCTION: LEGOLAND 0x0041ec20
unsigned int CastleDispatchIndex(struct Element *param) {
    unsigned int index = 0;
    struct DispatchRow *row = CastleDispatchTable;
    while ((INT_PTR)row < (INT_PTR)&DAT_0082adb0) {
        if (param == row->element) {
            return index;
        }
        row++;
        index++;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041ec40
struct Element *GetCastleDispatchElement(unsigned int param) {
    return CastleDispatchTable[param].element;
}

// FUNCTION: LEGOLAND 0x0041ec50
void CastleDispatchSetEditMode(struct Element *obj) {
    CastleDispatchTable[CastleDispatchIndex(obj)].fn_4(obj);
}

// FUNCTION: LEGOLAND 0x0041ec70
void CastleDispatchCalcCursor(struct Element *obj, struct Point *a, unsigned int b) {
    CastleDispatchTable[CastleDispatchIndex(obj)].fn_8(obj, a, b);
}

// FUNCTION: LEGOLAND 0x0041eca0
void FUN_0041eca0(unsigned int idx, short *p) {
    int v[2];

    v[0] = p[0];
    v[1] = p[1];
    CastleDispatchTable[idx].fn_c(CastleDispatchTable[idx].element, v);
}

// FUNCTION: LEGOLAND 0x0041ece0
void CastleDispatchAddObject(struct Element *obj, int *a) {
    ((void (*)(struct Element *, int *)) * (void (**)())((char *)CastleDispatchTable + CastleDispatchIndex(obj) * sizeof(struct DispatchRow) + offsetof(struct DispatchRow, fn_c)))(obj, a);
}

// FUNCTION: LEGOLAND 0x0041ed00
void CastleDispatchDCalcCursor(struct Element *obj, struct Point *a) {
    ((void (*)(struct Element *, struct Point *)) * (void (**)())((char *)CastleDispatchTable + CastleDispatchIndex(obj) * sizeof(struct DispatchRow) + offsetof(struct DispatchRow, fn_10)))(obj, a);
}

// FUNCTION: LEGOLAND 0x0041ed50
void CastleDispatchRemoveObject(struct Element *obj, TileId a, struct Cursor *b) {
    ((void (*)(struct Element *, TileId, struct Cursor *)) * (void (**)())((char *)CastleDispatchTable + CastleDispatchIndex(obj) * sizeof(struct DispatchRow) + offsetof(struct DispatchRow, fn_14)))(obj, a, b);
}

// FUNCTION: LEGOLAND 0x0041ed80
void SetMapObjectsEnabled(unsigned int param) {
    MapObjectsEnabled = param;
}

// FUNCTION: LEGOLAND 0x0041ed90
void AddMapObjectIfEnabled(struct Element *element, int *pos) {
    if (MapObjectsEnabled != 0) {
        AddBasicObject(element, pos);
    }
}

// FUNCTION: LEGOLAND 0x0041edb0
void RemoveMapObjectIfEnabled(struct Element *element, TileId tile, struct Cursor *cursor) {
    if (MapObjectsEnabled != 0) {
        StandardRemoveObject(element, tile, cursor);
    }
}

struct KindInfo {
    unsigned char pad_0[0x1c];
    unsigned int field_1c;
};

struct KindOwner {
    unsigned char pad_0[0xc];
    struct KindInfo *field_c;
};

struct KindObj {
    struct KindOwner *owner;
    unsigned char pad_4[0x8];
    unsigned short flags;
};

// FUNCTION: LEGOLAND 0x0041ede0
unsigned int IsMapElementBlocking(struct KindObj *obj) {
    struct KindOwner *owner;
    struct KindInfo *info;
    unsigned short flags;

    if (obj == NULL) {
        return 1;
    }
    flags = obj->flags;
    if ((flags & 0x8f8) == 0) {
        return 0;
    }
    if (flags & 0x40) {
        return 1;
    }
    owner = obj->owner;
    if (owner != NULL) {
        info = owner->field_c;
    } else {
        info = NULL;
    }
    if ((flags & 0x8) && owner != NULL && info == PathControlObject) {
        return 0;
    }
    if (flags & 0x800) {
        return 0;
    }
    if ((flags & 0x88) && info != NULL && (info->field_1c & 0x200000)) {
        return 0;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0041ee40
unsigned int IsAreaListClear(short *pos, struct AreaNode *b) {
    struct AreaNode *node;
    struct MapElement *elem;
    int x;
    int y;

    if (MapObjectsEnabled != 0) {
        node = b;
        do {
            for (y = node->y + pos[1]; y < node->h + pos[1]; y++) {
                for (x = node->x + pos[0]; x < node->w + pos[0]; x++) {
                    if (x >= 0 && x < lpConfig->width && y >= 0 && y < lpConfig->height) {
                        elem = &GameMap[y][x];
                    } else {
                        elem = NULL;
                    }
                    if (IsMapElementBlocking((struct KindObj *)elem) != 0) {
                        return 0;
                    }
                }
            }
            node = node->next;
        } while (node != NULL);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0041ef00
int FUN_0041ef00(void) {
    return 1;
}

struct PlaneSet {
    int count;
    float p[4][3];
};

// FUNCTION: LEGOLAND 0x0041ef20
void SetClipRect(int a, int b, int c, int d) {
    struct RectI r;

    r.var_0 = b;
    r.var_4 = a;
    r.var_c = c;
    r.var_8 = d;
    SetClipPlanesFromRect((int *)&r, ClipPlanes);
}

// FUNCTION: LEGOLAND 0x0041ef60
void *ClipTriangle(void *verts, int *out, unsigned int code, int n) {
    int sel = 0;

    if (code == 0xf) {
        *out = 3;
        return verts;
    }
    FUN_0041f030(n + 1);
    if ((char)(code & 3) < 3) {
        sel = 1;
    }
    if ((char)(code & 0xc) < 0xc) {
        sel |= 2;
    }
    switch (sel) {
    case 3:
        return ClipPolygonToPlanes(3, verts, out, 4, ClipPlanes->p[0]);
    case 2:
        return ClipPolygonToPlanes(3, verts, out, 2, ClipPlanes->p[2]);
    case 1:
        return ClipPolygonToPlanes(3, verts, out, 2, ClipPlanes->p[0]);
    }
    return out;
}

// FUNCTION: LEGOLAND 0x0041f030
void FUN_0041f030(unsigned int param) {
    DAT_004b5608 = 0x1c;
    DAT_004b560c = param;
}

union FloatBits {
    float f;
    unsigned int u;
};

struct FDist {
    union FloatBits mag;
    unsigned int sign;
};

// FUNCTION: LEGOLAND 0x0041f050
int FUN_0041f050(int n, int **src, int **dst, int **cursor, float *pl) {
    struct FDist m, pm;
    int count = 0;
    int *pool = *cursor;
    int i, k;
    int *prev, *cur;
    float t;
    int code;

    src[n] = src[0];
    cur = src[0];
    m.mag.f = pl[2] - (cur[2] * pl[0] + cur[1] * pl[1]);
    code = m.mag.u & 0x80000000;
    m.mag.u &= 0x7fffffff;
    for (i = 1; i <= n; i++) {
        pm.mag = m.mag;
        prev = cur;
        cur = src[i];
        code = (code >> 1) & 0x40000000;
        m.mag.f = pl[2] - (cur[2] * pl[0] + cur[1] * pl[1]);
        code |= m.mag.u & 0x80000000;
        m.mag.u &= 0x7fffffff;
        switch (code) {
        case 0x40000000:
            t = pm.mag.f / (pm.mag.f + m.mag.f);
            *dst++ = prev;
            for (k = 0; k <= DAT_004b560c; k++) {
                pool[k] = prev[k] + (int)((cur[k] - prev[k]) * t);
            }
            *dst++ = pool;
            pool = (int *)((char *)pool + DAT_004b5608);
            count += 2;
            break;
        case 0xc0000000:
            *dst++ = prev;
            count++;
            break;
        case 0x80000000:
            t = m.mag.f / (pm.mag.f + m.mag.f);
            for (k = 0; k <= DAT_004b560c; k++) {
                pool[k] = cur[k] + (int)((prev[k] - cur[k]) * t);
            }
            *dst++ = pool;
            pool = (int *)((char *)pool + DAT_004b5608);
            count++;
            break;
        }
    }
    *cursor = pool;
    return count;
}

// FUNCTION: LEGOLAND 0x0041f2b0
int **ClipPolygonToPlanes(int n, int **verts, int *outCount, int nPlanes, float *planes) {
    int i;

    for (i = 0; i < n; i++) {
        DAT_004d884c[i] = verts[i];
    }
    DAT_004d87c4 = DAT_004d83c4;
    for (i = 0; i < nPlanes; i++) {
        n = FUN_0041f050(n, DAT_004b5600[i & 1], DAT_004b5600[(i - 1) & 1], &DAT_004d87c4, planes);
        if (n == 0) {
            *outCount = 0;
            return NULL;
        }
        planes += 3;
    }
    *outCount = n;
    return DAT_004b5600[i & 1];
}

struct Pair {
    unsigned int field_0;
    unsigned int field_4;
};

// FUNCTION: LEGOLAND 0x0041f350
int **ClipPolygonToPlaneSet(int param1, int **param2, int *param3, struct PlaneSet *param4) {
    return ClipPolygonToPlanes(param1, param2, param3, param4->count, param4->p[0]);
}

// FUNCTION: LEGOLAND 0x0041f380
void SetClipPlanesFromRect(const int *r, struct PlaneSet *out) {
    out->count = 4;
    out->p[0][0] = 1.0f;
    out->p[0][1] = 0.0f;
    out->p[0][2] = (float)r[1];
    out->p[1][0] = -1.0f;
    out->p[1][1] = 0.0f;
    out->p[1][2] = -(float)r[3];
    out->p[2][0] = 0.0f;
    out->p[2][1] = 1.0f;
    out->p[2][2] = (float)r[0];
    out->p[3][0] = 0.0f;
    out->p[3][1] = -1.0f;
    out->p[3][2] = -(float)r[2];
}

// FUNCTION: LEGOLAND 0x0041f3e0
struct FloatArray ***BuildVecDifferenceTable(void (*fn)(float, struct FloatArray *), struct VecMath *ar, int n, float a, float b) {
    int cnt = n + 1;
    struct FloatArray **blk;
    int i;
    int j;
    int k;

    blk = ar->alloc((cnt + 1) * cnt >> 1);
    if (blk == NULL) {
        return NULL;
    }
    for (i = 0; i <= n; i++) {
        DAT_004d88cc[i] = blk;
        blk += cnt - i;
    }
    a -= (n >> 1) * b;
    for (i = 0; i <= n; i++) {
        fn(a, DAT_004d88cc[0][i]);
        a += b;
    }
    for (k = 1; k <= n; k++) {
        for (j = 0; j < n - k + 1; j++) {
            ar->sub(DAT_004d88cc[k - 1][j + 1], DAT_004d88cc[k - 1][j], DAT_004d88cc[k][j]);
        }
    }
    return DAT_004d88cc;
}

// FUNCTION: LEGOLAND 0x0041f4c0
unsigned int FUN_0041f4c0(struct FloatArray ***param1, struct VecMath *param2, int param3) {
    int n = param3 + 1;
    int tri = ((n + 1) * n) >> 1;
    return ((unsigned int (*)(struct FloatArray **, unsigned int))param2->release)(*param1, tri);
}

// FUNCTION: LEGOLAND 0x0041f4e0
int VecDerivative(void (*fn)(float, struct FloatArray *), struct VecMath *a, float b, float c, struct FloatArray *d) {
    struct VecMath *ar = a;
    struct FloatArray ***t;
    struct FloatArray **h;
    struct FloatArray *p;
    struct FloatArray *q;

    c *= 0.5f;
    t = BuildVecDifferenceTable(fn, ar, 4, b, c);
    h = ar->alloc(2);
    p = h[0];
    q = h[1];
    if (t == NULL) {
        return 0;
    }
    ar->add(t[1][1], t[1][2], p);
    ar->add(t[3][0], t[3][1], q);
    ar->scale(q, DAT_004b5610);
    ar->add(p, q, d);
    ar->scale(d, 0.5f / c);
    ar->release(h, 2);
    FUN_0041f4c0(t, ar, 4);
    return 1;
}

// FUNCTION: LEGOLAND 0x0041f5a0
int FUN_0041f5a0(void (*fn)(float, struct FloatArray *), struct VecMath *a, float b, float c, struct FloatArray *d) {
    struct VecMath *ar = a;
    struct FloatArray **h;
    struct FloatArray *q;
    struct FloatArray *r;
    int i;

    c *= 0.5f;
    h = ar->alloc(2);
    q = h[1];
    r = d;
    fn(DAT_004b5614 * c + b, r);
    ar->scale(r, DAT_004b5624);
    for (i = 0; i <= 2; i++) {
        fn(c * DAT_004b5618[i] + b, q);
        ar->scale(q, DAT_004b5628[i]);
        ar->add(q, r, r);
    }
    ar->scale(r, 1.0f / c);
    ar->release(h, 2);
    return 1;
}

// FUNCTION: LEGOLAND 0x0041f650
float **BuildDifferenceTable(double (*fn)(float), int n, float a, float b) {
    int cnt = n + 1;
    float **t;
    float *p;
    int i;
    int j;
    int k;

    /* the rows' pointers, then the triangle of cnt * (cnt + 1) / 2 floats */
    t = (float **)MemAlloc(((cnt + 1) * cnt >> 1) * sizeof(float) + cnt * sizeof(float *), 0, 0, 0);
    if (t == NULL) {
        return NULL;
    }
    p = (float *)(t + n + 1);
    for (i = 0; i <= n; i++) {
        t[i] = p;
        p += n + 1 - i;
    }
    a -= (n >> 1) * b;
    for (i = 0; i <= n; i++) {
        t[0][i] = (float)fn(a);
        a += b;
    }
    for (k = 1; k <= n; k++) {
        for (j = 0; j < n - k + 1; j++) {
            t[k][j] = t[k - 1][j + 1] - t[k - 1][j];
        }
    }
    return t;
}

// FUNCTION: LEGOLAND 0x0041f710
void FUN_0041f710(void *param) {
    MemFree(param);
}

// FUNCTION: LEGOLAND 0x0041f720
int ScalarDerivative(double (*fn)(float), float p2, float p3, float *out) {
    float **r;

    p3 *= 0.5f;
    r = BuildDifferenceTable(fn, 4, p2, p3);
    if (!r) {
        return 0;
    }
    p3 = ((r[1][2] + r[1][1]) - (r[3][1] + r[3][0]) * FLOAT_004b5634) * 0.5f / p3;
    FUN_0041f710(r);
    *out = p3;
    return 1;
}

// FUNCTION: LEGOLAND 0x0041f790
void FUN_0041f790(float x, struct FloatArray *out) {
    double e;
    e = exp(x);
    out->count = 3;
    out->data[0] = (float)e;
    out->data[1] = (float)(2.0 * e);
    out->data[2] = (float)(2.0 * exp(2.0 * x));
}

// FUNCTION: LEGOLAND 0x0041f7e0
double LogFloat(float param) {
    return log(param);
}

// FUNCTION: LEGOLAND 0x0041f7f0
void FUN_0041f7f0(void) {
    float buf;
    struct VecMath x;
    struct FloatArray y;

    ScalarDerivative(LogFloat, 3.0f, 1.0f, &buf);
    InitVecMathOps();
    InitVecMath(&x, 3);
    VecDerivative(FUN_0041f790, &x, 3.0f, 0.01f, &y);
}

// FUNCTION: LEGOLAND 0x0041f850
struct SegBlob *CurveWalkerNext(struct CurveWalker *cursor) {
    struct SegBlob *result = cursor->node->next;
    if (result == 0) {
        struct PathSeg *piece = cursor->piece->next;
        cursor->piece = piece;
        result = FUN_0041cff0(piece, &cursor->pos);
    }
    cursor->node = result;
    return result;
}

// FUNCTION: LEGOLAND 0x0041f880
void CurveWalkerPrev(struct CurveWalker *walker) {
    struct SegBlob *node = walker->node->prev;
    if (node != NULL) {
        walker->node = node;
        return;
    }
    walker->piece = walker->piece->prev;
    walker->node = FUN_0041cff0(walker->piece, &walker->pos);
    if (walker->node->next == NULL) {
        return;
    }
    do {
        walker->node = walker->node->next;
    } while (walker->node->next != NULL);
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x0041f8d0
__declspec(naked) void FUN_0041f8d0(void) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x60
        mov edx, dword ptr [ebp + 0x10]
        push ebx
        push esi
        mov esi, dword ptr [ebp + 0x14]
        mov ecx, dword ptr [ebp + 0x18]
        mov ebx, dword ptr [ebp + 8]
        mov eax, dword ptr [esi]
        push edi
        mov dword ptr [ebp + 0x14], eax
        mov eax, dword ptr [esi + edx*8 - 4]
        lea eax, [eax + eax*2]
        shl eax, 4
        inc word ptr [eax + ecx + 2]
        mov ebx, dword ptr [ebx*4 + DAT_00829c60]
        lea eax, [eax + ecx + 2]
        mov edi, dword ptr [CoasterSurfaceBits]
        mov eax, dword ptr [esi + edx*8 - 4]
        lea eax, [eax + eax*2]
        shl eax, 4
        movsx eax, word ptr [eax + ecx + 2]
        mov dword ptr [ebp - 4], eax
        mov eax, dword ptr [DAT_004b5b24]
        mov dword ptr [ebp - 0xc], eax
        mov eax, dword ptr [CoasterSurfacePitch]
        mov dword ptr [ebp - 8], eax
        mov eax, dword ptr [DAT_0060f900]
        inc eax
        mov dword ptr [DAT_0060f900], eax
        mov eax, dword ptr [ebp + 0xc]
        mov eax, dword ptr [eax]
        mov ax, word ptr [ebx + eax*2]
        mov ebx, dword ptr [ebp + 0x14]
        mov dword ptr [ebp + 0x10], eax
        mov eax, dword ptr [esi + edx*8 - 4]
        lea eax, [eax + eax*2]
        shl eax, 4
        movsx eax, word ptr [eax + ecx + 2]
        mov dword ptr [esi + edx*8], eax
        mov edx, dword ptr [ebp - 8]
        imul edx, ebx
        lea eax, [edi + edx*2]
        mov dword ptr [ebp - 0x10], eax
L41f964:
        mov eax, dword ptr [esi + 4]
        add esi, 8
        lea eax, [eax + eax*2]
        shl eax, 4
        add eax, ecx
        mov edx, dword ptr [eax + 4]
        test edx, edx
        mov edx, dword ptr [eax + 8]
        mov eax, dword ptr [eax + 0x1c]
        je L41f989
        sub edx, eax
        mov dword ptr [ebp - 0x24], eax
        mov dword ptr [ebp - 0x38], edx
        jmp L41f991
L41f989:
        sub edx, eax
        mov dword ptr [ebp - 0x4c], eax
        mov dword ptr [ebp - 0x60], edx
L41f991:
        cmp ebx, dword ptr [esi]
        jge L41f9ee
L41f995:
        inc dword ptr [ebp + 0x14]
        mov eax, dword ptr [ebp - 0x60]
        mov ebx, dword ptr [ebp - 0x38]
        add eax, dword ptr [ebp - 0x4c]
        add ebx, dword ptr [ebp - 0x24]
        mov dword ptr [ebp - 0x60], eax
        mov dword ptr [ebp - 0x38], ebx
        mov ecx, ebx
        sub ecx, eax
        cmp ecx, 0x8000
        jns L41f9b8
        jmp L41f9d4
L41f9b8:
        sar eax, 0x10
        sar ebx, 0x10
        mov edi, dword ptr [ebp - 0x10]
        xchg eax, ebx
        mov dx, word ptr [ebp + 0x10]
        sub ebx, eax
        lea edi, [edi + eax*2]
L41f9cb:
        mov word ptr [edi + ebx*2], dx
        add ebx, 1
        jle L41f9cb
L41f9d4:
        mov ecx, dword ptr [ebp - 8]
        mov edx, dword ptr [ebp + 0x14]
        lea eax, [ecx + ecx]
        mov ecx, dword ptr [ebp - 0x10]
        add ecx, eax
        mov eax, dword ptr [esi]
        cmp edx, eax
        mov dword ptr [ebp - 0x10], ecx
        jl L41f995
        mov ecx, dword ptr [ebp + 0x18]
L41f9ee:
        mov ebx, dword ptr [ebp + 0x14]
        mov eax, dword ptr [ebp - 4]
        cmp ebx, eax
        jl L41f964
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x0041fa10
__declspec(naked) void FUN_0041fa10(void) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x64
        mov ecx, dword ptr [ebp + 0x14]
        push ebx
        push esi
        mov esi, dword ptr [ebp + 0x10]
        mov eax, dword ptr [ecx]
        mov edx, dword ptr [ebp + 0x18]
        mov dword ptr [ebp - 4], eax
        mov eax, dword ptr [ecx + esi*8 - 4]
        push edi
        lea eax, [eax + eax*2]
        shl eax, 4
        inc word ptr [eax + edx + 2]
        mov edi, dword ptr [DAT_004b5b24]
        lea eax, [eax + edx + 2]
        mov eax, dword ptr [ecx + esi*8 - 4]
        mov esi, dword ptr [CoasterSurfaceBits]
        lea eax, [eax + eax*2]
        shl eax, 4
        movsx eax, word ptr [eax + edx + 2]
        mov dword ptr [ebp - 8], eax
        mov eax, dword ptr [CoasterSurfacePitch]
        mov dword ptr [ebp - 0xc], eax
        mov eax, dword ptr [ebp + 0xc]
        mov ebx, dword ptr [eax + 4]
        mov dword ptr [ebp + 0xc], ebx
        mov ebx, dword ptr [DAT_0060f900]
        inc ebx
        mov dword ptr [DAT_0060f900], ebx
        mov ebx, dword ptr [ebp + 8]
        mov eax, dword ptr [eax]
        mov ebx, dword ptr [ebx*4 + DAT_00829c60]
        mov ax, word ptr [ebx + eax*2]
        mov ebx, dword ptr [ebp + 0x10]
        mov dword ptr [ebp + 8], eax
        mov eax, dword ptr [ecx + ebx*8 - 4]
        lea eax, [eax + eax*2]
        shl eax, 4
        movsx eax, word ptr [eax + edx + 2]
        mov dword ptr [ecx + ebx*8], eax
        mov eax, dword ptr [ebp - 0xc]
        imul eax, dword ptr [ebp - 4]
        shl eax, 1
        add esi, eax
        add edi, eax
        mov dword ptr [ebp - 0x14], esi
        mov dword ptr [ebp - 0x10], edi
L41fab1:
        mov eax, dword ptr [ecx + 4]
        add ecx, 8
        mov dword ptr [ebp + 0x14], ecx
        lea eax, [eax + eax*2]
        shl eax, 4
        add eax, edx
        mov esi, dword ptr [eax + 4]
        test esi, esi
        mov esi, dword ptr [eax + 8]
        je L41fad9
        mov eax, dword ptr [eax + 0x1c]
        sub esi, eax
        mov dword ptr [ebp - 0x28], eax
        mov dword ptr [ebp - 0x3c], esi
        jmp L41faf2
L41fad9:
        mov edi, dword ptr [eax + 0x1c]
        mov ebx, dword ptr [eax + 0xc]
        mov eax, dword ptr [eax + 0x20]
        sub esi, edi
        sub ebx, eax
        mov dword ptr [ebp - 0x50], edi
        mov dword ptr [ebp - 0x48], eax
        mov dword ptr [ebp - 0x64], esi
        mov dword ptr [ebp - 0x5c], ebx
L41faf2:
        mov eax, dword ptr [ebp - 4]
        mov esi, dword ptr [ecx]
        cmp eax, esi
        jge L41fb84
        mov ecx, dword ptr [ebp - 0xc]
        lea edx, [ecx + ecx]
        mov dword ptr [ebp + 0x10], edx
L41fb08:
        inc dword ptr [ebp - 4]
        mov eax, dword ptr [ebp - 0x64]
        mov edx, dword ptr [ebp - 0x5c]
        mov ebx, dword ptr [ebp - 0x3c]
        add eax, dword ptr [ebp - 0x50]
        add edx, dword ptr [ebp - 0x48]
        add ebx, dword ptr [ebp - 0x28]
        mov dword ptr [ebp - 0x64], eax
        mov dword ptr [ebp - 0x5c], edx
        mov dword ptr [ebp - 0x3c], ebx
        mov ecx, ebx
        sub ecx, eax
        cmp ecx, 0x8000
        jns L41fb34
        jmp L41fb62
L41fb34:
        sar eax, 0x10
        sar ebx, 0x10
        mov edi, dword ptr [ebp - 0x14]
        mov esi, dword ptr [ebp - 0x10]
        xchg eax, ebx
        sub ebx, eax
        lea edi, [edi + eax*2]
        lea esi, [esi + eax*2]
        mov ax, word ptr [ebp + 8]
L41fb4d:
        mov ecx, edx
        mov word ptr [edi + ebx*2], ax
        add edx, dword ptr [ebp + 0xc]
        sar ecx, 0x10
        mov word ptr [esi + ebx*2], cx
        add ebx, 1
        jle L41fb4d
L41fb62:
        mov eax, dword ptr [ebp + 0x10]
        mov edi, dword ptr [ebp - 0x14]
        mov esi, dword ptr [ebp - 0x10]
        mov ecx, dword ptr [ebp + 0x14]
        add edi, eax
        add esi, eax
        mov eax, dword ptr [ebp - 4]
        mov edx, dword ptr [ecx]
        cmp eax, edx
        mov dword ptr [ebp - 0x14], edi
        mov dword ptr [ebp - 0x10], esi
        jl L41fb08
        mov edx, dword ptr [ebp + 0x18]
L41fb84:
        mov eax, dword ptr [ebp - 4]
        mov esi, dword ptr [ebp - 8]
        cmp eax, esi
        jl L41fab1
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x0041fba0
__declspec(naked) void FUN_0041fba0(void) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x64
        mov ecx, dword ptr [ebp + 0x14]
        push ebx
        push esi
        mov esi, dword ptr [ebp + 0x10]
        mov eax, dword ptr [ecx]
        mov edx, dword ptr [ebp + 0x18]
        mov dword ptr [ebp - 4], eax
        mov eax, dword ptr [ecx + esi*8 - 4]
        push edi
        lea eax, [eax + eax*2]
        shl eax, 4
        inc word ptr [eax + edx + 2]
        mov edi, dword ptr [DAT_004b5b24]
        lea eax, [eax + edx + 2]
        mov eax, dword ptr [ecx + esi*8 - 4]
        mov esi, dword ptr [CoasterSurfaceBits]
        lea eax, [eax + eax*2]
        shl eax, 4
        movsx eax, word ptr [eax + edx + 2]
        mov dword ptr [ebp - 8], eax
        mov eax, dword ptr [CoasterSurfacePitch]
        mov dword ptr [ebp - 0xc], eax
        mov eax, dword ptr [ebp + 0xc]
        mov ebx, dword ptr [eax + 4]
        mov dword ptr [ebp + 0xc], ebx
        mov ebx, dword ptr [DAT_0060f900]
        inc ebx
        mov dword ptr [DAT_0060f900], ebx
        mov ebx, dword ptr [ebp + 8]
        mov eax, dword ptr [eax]
        mov ebx, dword ptr [ebx*4 + DAT_00829c60]
        mov ax, word ptr [ebx + eax*2]
        mov ebx, dword ptr [ebp + 0x10]
        mov dword ptr [ebp + 8], eax
        mov eax, dword ptr [ecx + ebx*8 - 4]
        lea eax, [eax + eax*2]
        shl eax, 4
        movsx eax, word ptr [eax + edx + 2]
        mov dword ptr [ecx + ebx*8], eax
        mov eax, dword ptr [ebp - 0xc]
        imul eax, dword ptr [ebp - 4]
        shl eax, 1
        add esi, eax
        add edi, eax
        mov dword ptr [ebp - 0x14], esi
        mov dword ptr [ebp - 0x10], edi
L41fc41:
        mov eax, dword ptr [ecx + 4]
        add ecx, 8
        mov dword ptr [ebp + 0x14], ecx
        lea eax, [eax + eax*2]
        shl eax, 4
        add eax, edx
        mov esi, dword ptr [eax + 4]
        test esi, esi
        mov esi, dword ptr [eax + 8]
        je L41fc69
        mov eax, dword ptr [eax + 0x1c]
        sub esi, eax
        mov dword ptr [ebp - 0x28], eax
        mov dword ptr [ebp - 0x3c], esi
        jmp L41fc82
L41fc69:
        mov edi, dword ptr [eax + 0x1c]
        mov ebx, dword ptr [eax + 0xc]
        mov eax, dword ptr [eax + 0x20]
        sub esi, edi
        sub ebx, eax
        mov dword ptr [ebp - 0x50], edi
        mov dword ptr [ebp - 0x48], eax
        mov dword ptr [ebp - 0x64], esi
        mov dword ptr [ebp - 0x5c], ebx
L41fc82:
        mov eax, dword ptr [ebp - 4]
        mov esi, dword ptr [ecx]
        cmp eax, esi
        jge L41fd1a
        mov ecx, dword ptr [ebp - 0xc]
        lea edx, [ecx + ecx]
        mov dword ptr [ebp + 0x10], edx
L41fc98:
        inc dword ptr [ebp - 4]
        mov eax, dword ptr [ebp - 0x64]
        mov edx, dword ptr [ebp - 0x5c]
        mov ebx, dword ptr [ebp - 0x3c]
        add eax, dword ptr [ebp - 0x50]
        add edx, dword ptr [ebp - 0x48]
        add ebx, dword ptr [ebp - 0x28]
        mov dword ptr [ebp - 0x64], eax
        mov dword ptr [ebp - 0x5c], edx
        mov dword ptr [ebp - 0x3c], ebx
        mov ecx, ebx
        sub ecx, eax
        cmp ecx, 0x8000
        jns L41fcc4
        jmp L41fcf8
L41fcc4:
        sar eax, 0x10
        sar ebx, 0x10
        mov edi, dword ptr [ebp - 0x14]
        mov esi, dword ptr [ebp - 0x10]
        xchg eax, ebx
        sub ebx, eax
        lea edi, [edi + eax*2]
        lea esi, [esi + eax*2]
        mov ax, word ptr [ebp + 8]
L41fcdd:
        mov ecx, edx
        sar ecx, 0x10
        cmp cx, word ptr [esi + ebx*2]
        jb L41fcf0
        mov word ptr [edi + ebx*2], ax
        mov word ptr [esi + ebx*2], cx
L41fcf0:
        add edx, dword ptr [ebp + 0xc]
        add ebx, 1
        jle L41fcdd
L41fcf8:
        mov eax, dword ptr [ebp + 0x10]
        mov edi, dword ptr [ebp - 0x14]
        mov esi, dword ptr [ebp - 0x10]
        mov ecx, dword ptr [ebp + 0x14]
        add edi, eax
        add esi, eax
        mov eax, dword ptr [ebp - 4]
        mov edx, dword ptr [ecx]
        cmp eax, edx
        mov dword ptr [ebp - 0x14], edi
        mov dword ptr [ebp - 0x10], esi
        jl L41fc98
        mov edx, dword ptr [ebp + 0x18]
L41fd1a:
        mov eax, dword ptr [ebp - 4]
        mov esi, dword ptr [ebp - 8]
        cmp eax, esi
        jl L41fc41
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// FUNCTION: LEGOLAND 0x0041fd30
void FUN_0041fd30(void) {
    int i;
    for (i = 0; i < 16; i++) {
        ((unsigned int *)DAT_004d88f4)[i] = 0;
    }
    DAT_004d89c4 = &DAT_004d88f4[0x40];
    for (i = 0x40; i <= 0x7f; i++) {
        DAT_004d88f4[i] = (unsigned char)(i - 0x40);
    }
    for (i = 0; i < 16; i++) {
        ((unsigned int *)DAT_004d8974)[i] = 0x3f3f3f3f;
    }
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x0041fd80
__declspec(naked) void FUN_0041fd80(void) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x64
        mov ecx, dword ptr [ebp + 0x14]
        push ebx
        push esi
        mov esi, dword ptr [ebp + 0x10]
        mov edx, dword ptr [ebp + 0x18]
        mov ebx, dword ptr [ecx]
        mov eax, dword ptr [ecx + esi*8 - 4]
        push edi
        mov dword ptr [ebp - 4], ebx
        mov dword ptr [ebp + 0x10], 0
        lea eax, [eax + eax*2]
        shl eax, 4
        inc word ptr [eax + edx + 2]
        mov edi, dword ptr [CoasterSurfaceBits]
        lea eax, [eax + edx + 2]
        mov eax, dword ptr [ecx + esi*8 - 4]
        lea eax, [eax + eax*2]
        shl eax, 4
        movsx eax, word ptr [eax + edx + 2]
        mov dword ptr [ebp - 8], eax
        mov eax, dword ptr [DAT_004b5b24]
        mov dword ptr [ebp - 0x10], eax
        mov eax, dword ptr [CoasterSurfacePitch]
        mov dword ptr [ebp - 0xc], eax
        mov eax, dword ptr [ebp + 8]
        mov eax, dword ptr [eax*4 + DAT_00829c60]
        mov dword ptr [DAT_004d89b4 + 0xc], eax
        mov eax, dword ptr [DAT_0060f900]
        inc eax
        mov dword ptr [DAT_0060f900], eax
        mov eax, dword ptr [ecx + esi*8 - 4]
        lea eax, [eax + eax*2]
        shl eax, 4
        movsx eax, word ptr [eax + edx + 2]
        mov dword ptr [ecx + esi*8], eax
        mov esi, dword ptr [ebp + 0xc]
        mov eax, dword ptr [esi + 4]
        test eax, eax
        jge L41fe1b
        neg eax
        mov dword ptr [esi + 4], eax
        mov dword ptr [ebp + 0x10], 1
L41fe1b:
        mov eax, dword ptr [esi + 4]
        sar eax, 0x10
        mov dword ptr [DAT_004d89b4], eax
        mov eax, dword ptr [esi + 4]
        shl eax, 0x10
        mov dword ptr [DAT_004d89b4 + 0x8], eax
        mov eax, dword ptr [ebp - 0xc]
        imul eax, ebx
        lea eax, [edi + eax*2]
        mov dword ptr [ebp - 0x14], eax
L41fe3d:
        mov eax, dword ptr [ecx + 4]
        add ecx, 8
        mov dword ptr [ebp + 0x14], ecx
        lea eax, [eax + eax*2]
        shl eax, 4
        add eax, edx
        mov esi, dword ptr [eax + 4]
        mov edi, dword ptr [eax + 0x1c]
        test esi, esi
        mov esi, dword ptr [eax + 8]
        je L41fe73
        mov ebx, dword ptr [eax + 0xc]
        mov eax, dword ptr [eax + 0x20]
        sub esi, edi
        sub ebx, eax
        mov dword ptr [ebp - 0x28], edi
        mov dword ptr [ebp - 0x24], eax
        mov dword ptr [ebp - 0x3c], esi
        mov dword ptr [ebp - 0x38], ebx
        jmp L41fe89
L41fe73:
        mov ebx, dword ptr [eax + 0xc]
        mov eax, dword ptr [eax + 0x20]
        sub esi, edi
        sub ebx, eax
        mov dword ptr [ebp - 0x50], edi
        mov dword ptr [ebp - 0x4c], eax
        mov dword ptr [ebp - 0x64], esi
        mov dword ptr [ebp - 0x60], ebx
L41fe89:
        mov eax, dword ptr [ebp - 4]
        mov esi, dword ptr [ecx]
        cmp eax, esi
        jge L41ff64
        mov ecx, dword ptr [ebp - 0xc]
        lea edx, [ecx + ecx]
        mov dword ptr [ebp + 8], edx
L41fe9f:
        inc dword ptr [ebp - 4]
        mov eax, dword ptr [ebp - 0x64]
        mov ebx, dword ptr [ebp - 0x3c]
        add eax, dword ptr [ebp - 0x50]
        add ebx, dword ptr [ebp - 0x28]
        mov dword ptr [ebp - 0x64], eax
        mov dword ptr [ebp - 0x3c], ebx
        mov ecx, ebx
        sub ecx, eax
        cmp ecx, 0x8000
        jns L41fecb
        mov ecx, dword ptr [ebp - 0x60]
        add ecx, dword ptr [ebp - 0x4c]
        mov dword ptr [ebp - 0x60], ecx
        jmp L41ff46
L41fecb:
        sar eax, 0x10
        sar ebx, 0x10
        mov edi, dword ptr [ebp - 0x14]
        xchg eax, ebx
        sub ebx, eax
        lea edi, [edi + eax*2]
        mov ecx, dword ptr [ebp - 0x60]
        xor edx, edx
        add ecx, dword ptr [ebp - 0x4c]
        mov esi, dword ptr [DAT_004d89b4 + 0xc]
        mov dword ptr [ebp - 0x60], ecx
        ror ecx, 0x10
        mov eax, ecx
        and ecx, 0xffff
        and eax, 0xffff0000
        add ecx, dword ptr [DAT_004d89c4]
        cmp edx, dword ptr [ebp + 0x10]
        jne L41ff27
        xor edx, edx
L41ff08:
        mov dl, byte ptr [ecx]
        mov dx, word ptr [esi + edx*2]
        add eax, dword ptr [DAT_004d89b4 + 0x8]
        mov word ptr [edi + ebx*2], dx
        adc ecx, dword ptr [DAT_004d89b4]
        xor edx, edx
        add ebx, 1
        jle L41ff08
        jmp L41ff46
L41ff27:
        xor edx, edx
L41ff29:
        mov dl, byte ptr [ecx]
        mov dx, word ptr [esi + edx*2]
        add eax, dword ptr [DAT_004d89b4 + 0x8]
        mov word ptr [edi + ebx*2], dx
        sbb ecx, dword ptr [DAT_004d89b4]
        xor edx, edx
        add ebx, 1
        jle L41ff29
L41ff46:
        mov eax, dword ptr [ebp + 8]
        mov ecx, dword ptr [ebp - 0x14]
        mov edx, dword ptr [ebp + 0x14]
        add ecx, eax
        mov dword ptr [ebp - 0x14], ecx
        mov ecx, dword ptr [ebp - 4]
        cmp ecx, dword ptr [edx]
        jl L41fe9f
        mov ecx, edx
        mov edx, dword ptr [ebp + 0x18]
L41ff64:
        mov eax, dword ptr [ebp - 4]
        mov esi, dword ptr [ebp - 8]
        cmp eax, esi
        jl L41fe3d
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x0041ff80
__declspec(naked) void FUN_0041ff80(void) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x64
        mov ecx, dword ptr [ebp + 0x14]
        push ebx
        push esi
        mov esi, dword ptr [ebp + 0x10]
        mov eax, dword ptr [ecx]
        mov edx, dword ptr [ebp + 0x18]
        mov dword ptr [ebp - 4], eax
        mov eax, dword ptr [ecx + esi*8 - 4]
        push edi
        mov dword ptr [ebp + 0x10], 0
        lea eax, [eax + eax*2]
        shl eax, 4
        inc word ptr [eax + edx + 2]
        mov edi, dword ptr [CoasterSurfaceBits]
        lea eax, [eax + edx + 2]
        mov ebx, dword ptr [DAT_004b5b24]
        mov eax, dword ptr [ecx + esi*8 - 4]
        lea eax, [eax + eax*2]
        shl eax, 4
        movsx eax, word ptr [eax + edx + 2]
        mov dword ptr [ebp - 8], eax
        mov eax, dword ptr [CoasterSurfacePitch]
        mov dword ptr [ebp - 0xc], eax
        mov eax, dword ptr [ebp + 8]
        mov eax, dword ptr [eax*4 + DAT_00829c60]
        mov dword ptr [DAT_004d89b4 + 0xc], eax
        mov eax, dword ptr [DAT_0060f900]
        inc eax
        mov dword ptr [DAT_0060f900], eax
        mov eax, dword ptr [ecx + esi*8 - 4]
        lea eax, [eax + eax*2]
        shl eax, 4
        movsx eax, word ptr [eax + edx + 2]
        mov dword ptr [ecx + esi*8], eax
        mov eax, dword ptr [ebp + 0xc]
        mov esi, dword ptr [eax + 4]
        test esi, esi
        jge L420019
        neg esi
        mov dword ptr [eax + 4], esi
        mov dword ptr [ebp + 0x10], 1
L420019:
        mov esi, dword ptr [eax + 4]
        sar esi, 0x10
        mov dword ptr [DAT_004d89b4], esi
        mov esi, dword ptr [eax + 4]
        and esi, 0xffffff00
        shl esi, 0x10
        mov dword ptr [DAT_004d89b4 + 0x8], esi
        mov eax, dword ptr [eax + 8]
        sar eax, 8
        and eax, 0xffffff
        or esi, eax
        mov eax, dword ptr [ebp - 0xc]
        imul eax, dword ptr [ebp - 4]
        shl eax, 1
        add edi, eax
        add ebx, eax
        mov dword ptr [DAT_004d89b4 + 0x8], esi
        mov dword ptr [ebp - 0x14], edi
        mov dword ptr [ebp - 0x10], ebx
L42005d:
        mov eax, dword ptr [ecx + 4]
        add ecx, 8
        mov dword ptr [ebp + 0x14], ecx
        lea eax, [eax + eax*2]
        shl eax, 4
        add eax, edx
        mov esi, dword ptr [eax + 4]
        mov edi, dword ptr [eax + 0x1c]
        test esi, esi
        mov esi, dword ptr [eax + 8]
        je L420093
        mov ebx, dword ptr [eax + 0xc]
        mov eax, dword ptr [eax + 0x20]
        sub esi, edi
        sub ebx, eax
        mov dword ptr [ebp - 0x28], edi
        mov dword ptr [ebp - 0x24], eax
        mov dword ptr [ebp - 0x3c], esi
        mov dword ptr [ebp - 0x38], ebx
        jmp L4200c0
L420093:
        mov ebx, dword ptr [eax + 0xc]
        sub esi, edi
        mov dword ptr [ebp - 0x60], ebx
        mov ebx, dword ptr [eax + 0x20]
        mov dword ptr [ebp - 0x50], edi
        mov edi, dword ptr [ebp - 0x60]
        mov dword ptr [ebp - 0x4c], ebx
        mov ebx, dword ptr [eax + 0x10]
        mov eax, dword ptr [eax + 0x24]
        mov dword ptr [ebp - 0x64], esi
        mov esi, dword ptr [ebp - 0x4c]
        mov dword ptr [ebp - 0x48], eax
        sub edi, esi
        sub ebx, eax
        mov dword ptr [ebp - 0x60], edi
        mov dword ptr [ebp - 0x5c], ebx
L4200c0:
        mov eax, dword ptr [ebp - 4]
        mov esi, dword ptr [ecx]
        cmp eax, esi
        jge L4201e4
        mov ecx, dword ptr [ebp - 0xc]
        lea edx, [ecx + ecx]
        mov dword ptr [ebp + 8], edx
L4200d6:
        inc dword ptr [ebp - 4]
        mov eax, dword ptr [ebp - 0x64]
        mov edx, dword ptr [ebp - 0x5c]
        mov ebx, dword ptr [ebp - 0x3c]
        add eax, dword ptr [ebp - 0x50]
        add edx, dword ptr [ebp - 0x48]
        add ebx, dword ptr [ebp - 0x28]
        mov dword ptr [ebp - 0x64], eax
        mov dword ptr [ebp - 0x5c], edx
        mov dword ptr [ebp - 0x3c], ebx
        mov ecx, ebx
        sub ecx, eax
        cmp ecx, 0x8000
        jns L42010e
        mov ecx, dword ptr [ebp - 0x60]
        add ecx, dword ptr [ebp - 0x4c]
        mov dword ptr [ebp - 0x60], ecx
        jmp L4201be
L42010e:
        sar eax, 0x10
        sar ebx, 0x10
        mov edi, dword ptr [ebp - 0x14]
        mov esi, dword ptr [ebp - 0x10]
        xchg eax, ebx
        sub ebx, eax
        lea edi, [edi + eax*2]
        lea esi, [esi + eax*2]
        mov ecx, dword ptr [ebp - 0x60]
        mov eax, edx
        add ecx, dword ptr [ebp - 0x4c]
        sar eax, 8
        mov dword ptr [ebp - 0x60], ecx
        and eax, 0xffffff
        xor edx, edx
        sar ecx, 0x10
        add ecx, dword ptr [DAT_004d89c4]
        cmp edx, dword ptr [ebp + 0x10]
        jne L420184
        push ebp
        mov ebp, dword ptr [DAT_004d89b4 + 0xc]
        xor edx, edx
L42014f:
        mov edx, eax
        sar edx, 8
        cmp dx, word ptr [esi + ebx*2]
        jb L42016b
        mov word ptr [esi + ebx*2], dx
        xor edx, edx
        mov dl, byte ptr [ecx]
        mov dx, word ptr [ebp + edx*2]
        mov word ptr [edi + ebx*2], dx
L42016b:
        add eax, dword ptr [DAT_004d89b4 + 0x8]
        adc ecx, dword ptr [DAT_004d89b4]
        and eax, 0xfeffffff
        add ebx, 1
        jle L42014f
        pop ebp
        jmp L4201be
L420184:
        push ebp
        mov ebp, dword ptr [DAT_004d89b4 + 0xc]
L42018b:
        mov edx, eax
        sar edx, 8
        cmp dx, word ptr [esi + ebx*2]
        jb L4201a7
        mov word ptr [esi + ebx*2], dx
        xor edx, edx
        mov dl, byte ptr [ecx]
        mov dx, word ptr [ebp + edx*2]
        mov word ptr [edi + ebx*2], dx
L4201a7:
        add eax, dword ptr [DAT_004d89b4 + 0x8]
        sbb ecx, dword ptr [DAT_004d89b4]
        and eax, 0xfeffffff
        add ebx, 1
        jle L42018b
        pop ebp
L4201be:
        mov eax, dword ptr [ebp + 8]
        mov edi, dword ptr [ebp - 0x14]
        mov esi, dword ptr [ebp - 0x10]
        mov ecx, dword ptr [ebp + 0x14]
        add edi, eax
        add esi, eax
        mov eax, dword ptr [ebp - 4]
        mov edx, dword ptr [ecx]
        cmp eax, edx
        mov dword ptr [ebp - 0x14], edi
        mov dword ptr [ebp - 0x10], esi
        jl L4200d6
        mov edx, dword ptr [ebp + 0x18]
L4201e4:
        mov eax, dword ptr [ebp - 4]
        mov esi, dword ptr [ebp - 8]
        cmp eax, esi
        jl L42005d
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

#pragma optimize("", off)
// FUNCTION: LEGOLAND 0x00420200
float IntegrateSimpson(float (*fn)(unsigned int), unsigned int a, unsigned int b, float tol) {
    int it2;
    int iib;
    float hh_;
    float s_;
    float add0;
    float pr0;
    float area0;
    float strideb;
    float cxxb;

    it2 = 1;
    hh_ = *(float *)&b - *(float *)&a;
    s_ = fn(a) + fn(b);
    add0 = 0;
    area0 = s_ * hh_ * 0.5f * FLOAT_004ab43c;
    do {
        pr0 = area0;
        s_ = s_ - 2.0f * add0;
        add0 = 0;
        strideb = hh_;
        hh_ = 0.5f * hh_;
        cxxb = hh_ + *(float *)&a;
        for (iib = 1; iib <= it2; iib++) {
            add0 = fn(*(unsigned int *)&cxxb) + add0;
            cxxb = cxxb + strideb;
        }
        s_ = 4.0f * add0 + s_;
        area0 = s_ * hh_;
        it2 = it2 + it2;
    } while (fabs(area0 - pr0) > tol * fabs(pr0));
    return FLOAT_004ab438 * area0;
}
#pragma optimize("", on)

// FUNCTION: LEGOLAND 0x00420310
void IntegratorStepRK4(struct Integrator *ar, float *p, float c) {
    struct FloatArray **h;
    struct FloatArray *v0;
    struct FloatArray *v1;
    struct FloatArray *v2;
    struct FloatArray *v3;
    int i;

    h = ar->math.alloc(4);
    v0 = h[0];
    v1 = h[1];
    v2 = h[2];
    v3 = h[3];
    ar->math.zero(v2, ar->math.dim);
    ar->load(ar, v0);
    ar->load(ar, v1);
    for (i = 0; i <= 3; i++) {
        ar->math.blend(v0, 1.0f, v2, DAT_004b5660[i][0], v3);
        ar->store(ar, v3);
        ar->deriv(ar, *p + c * DAT_004b5660[i][0], v2);
        ar->math.scale(v2, c);
        ar->math.axpy(v2, DAT_004b5660[i][1], v1);
    }
    ar->store(ar, v1);
    *p += c;
    ar->math.release(h, 4);
}

// FUNCTION: LEGOLAND 0x004203d0
void IntegratorStoreState(struct Integrator *self, struct FloatArray *param) {
    self->math.copy(param, self->state);
}

// FUNCTION: LEGOLAND 0x004203f0
unsigned int IntegratorLoadState(struct Integrator *self, struct FloatArray *param) {
    return ((unsigned int (*)(void *, struct FloatArray *))self->math.copy)(self->state, param);
}

// FUNCTION: LEGOLAND 0x00420410
void InitIntegrator(struct Integrator *param, unsigned int count) {
    struct Integrator *host = param;
    InitVecMath(&host->math, 2);
    host->store = IntegratorStoreState;
    host->load = IntegratorLoadState;
}

// FUNCTION: LEGOLAND 0x00420440
void LoadRollercoasterFiles(void) {
    char buf[256];
    int i;
    int n;

    RollercoasterLpt = LoadRollercoasterLpt();
    // STRING: LEGOLAND 0x004b56a0
    SetCurrentDirectoryIfNotNull("RollerCoaster\\RollerCoaster\\CreatedData");
    // STRING: LEGOLAND 0x004b5690
    LoadObjAndTxtFiles("ROLLERCOASTER");
    // STRING: LEGOLAND 0x004b5684
    SetCurrentDirectoryIfNotNull("..\\..\\..");
    n = FUN_004225d0();
    for (i = 0; i < n; i++) {
        GetCoasterObjLine(i, buf);
        LmsFileTable[i] = LoadLmsFile(buf);
    }
    n = FUN_004225d0();
    for (i = 0; i < n; i++) {
        GetCoasterObjLine(i, buf);
        LfmFileTable[i] = LoadLfmFile(buf, &LfmFileSizes[i]);
    }
    n = FUN_00422640();
    for (i = 0; i < n; i++) {
        FormatRollercoasterFileName(i, buf);
        LtxFileTable[i] = LoadLtxFile(buf);
    }
}

// FUNCTION: LEGOLAND 0x00420530
unsigned int SetCurrentDirectoryIfNotNull(const char *param) {
    if (param == NULL) {
        return 0;
    }
    return SetCurrentDirectoryA(param);
}

// FUNCTION: LEGOLAND 0x00420550
void *LoadCreatedDataFile(const char *fileName, unsigned int *sizeOut) {
    HANDLE hFile;
    unsigned int fileSize;
    void *buffer;
    DWORD bytesRead;

    if (fileName == 0) {
        return 0;
    }

    // STRING: LEGOLAND 0x004b56a0
    SetCurrentDirectoryIfNotNull("RollerCoaster\\RollerCoaster\\CreatedData");
    hFile = CreateFileA(fileName, 0x80000000, 0x1, 0, 0x3, 0x8000000, 0);
    if (hFile == (HANDLE)-1) {
        // STRING: LEGOLAND 0x004b5684
        SetCurrentDirectoryIfNotNull("..\\..\\..");
        return 0;
    }

    fileSize = GetFileSize(hFile, 0);
    buffer = MemAlloc(fileSize, 0, (unsigned int)(UINT_PTR)DAT_004d8bb0, 0);
    if (buffer == 0) {
        CloseHandle(hFile);
        SetCurrentDirectoryIfNotNull("..\\..\\..");
        return 0;
    }

    ReadFile(hFile, buffer, fileSize, &bytesRead, 0);
    if (bytesRead != fileSize) {
        MemFree(buffer);
        CloseHandle(hFile);
        SetCurrentDirectoryIfNotNull("..\\..\\..");
        return 0;
    }

    CloseHandle(hFile);
    SetCurrentDirectoryIfNotNull("..\\..\\..");
    if (sizeOut != 0) {
        *sizeOut = fileSize;
    }
    return buffer;
}

// FUNCTION: LEGOLAND 0x00420640
struct LmsFile *LoadLmsFile(const char *name) {
    char buffer[256];
    struct LmsFile *f;
    // STRING: LEGOLAND 0x004b56c8
    wsprintfA(buffer, "%s.lms", name);
    f = LoadCreatedDataFile(buffer, 0);
    if (f != 0) {
        *(UINT_PTR *)&f->field_c += (UINT_PTR)f;
        *(UINT_PTR *)&f->field_14 += (UINT_PTR)f;
        *(UINT_PTR *)&f->field_10 += (UINT_PTR)f;
        *(UINT_PTR *)&f->field_18 += (UINT_PTR)f;
        *(UINT_PTR *)&f->field_20 += (UINT_PTR)f;
        *(UINT_PTR *)&f->field_28 += (UINT_PTR)f;
        return f;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x004206b0
struct LmsFile *GetLmsByName(const char *s) {
    unsigned int index = FindCoasterObjLine(s);
    if (index != 0xffffffff) {
        return LmsFileTable[index];
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x004206d0
struct LmsRec *LoadLfmFile(const char *name, unsigned int *param2) {
    char buffer[256];
    // STRING: LEGOLAND 0x004b56d0
    wsprintfA(buffer, "%s.lfm", name);
    return LoadCreatedDataFile(buffer, param2);
}

// FUNCTION: LEGOLAND 0x00420710
struct LmsRec *GetLfmByName(const char *s) {
    unsigned int index = FindCoasterObjLine(s);
    if (index != 0xffffffff) {
        return LfmFileTable[index];
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00420730
unsigned int GetLfmSizeByName(const char *s) {
    unsigned int index = FindCoasterObjLine(s);
    if (index != 0xffffffff) {
        return LfmFileSizes[index];
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00420750
void *LoadLtxFile(const char *name) {
    char buffer[256];
    // STRING: LEGOLAND 0x004b56d8
    wsprintfA(buffer, "%s.ltx", name);
    return LoadCreatedDataFile(buffer, 0);
}

// FUNCTION: LEGOLAND 0x00420780
void *GetLtxFileTableEntry(unsigned int param) {
    return LtxFileTable[param];
}

// FUNCTION: LEGOLAND 0x00420790
unsigned int FUN_00420790(const char *param) {
    return FindCoasterTxtLine(param);
}

// FUNCTION: LEGOLAND 0x004207a0
int *LoadRollercoasterLpt(void) {
    // STRING: LEGOLAND 0x004b56e0
    return LoadCreatedDataFile("Rollercoaster.lpt", 0);
}

// FUNCTION: LEGOLAND 0x004207c0
int *GetRollercoasterLpt(void) {
    return RollercoasterLpt;
}

// FUNCTION: LEGOLAND 0x004207d0
int FindRollercoasterLptEntry(unsigned int key) {
    int *table = RollercoasterLpt;
    int count = table[0];
    int i;
    int *p = table + 1;
    for (i = 0; i < count; i++) {
        if (((*p++ ^ key) & 0xffffff) == 0) {
            return i;
        }
    }
    return -1;
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x00420810
__declspec(naked) void FUN_00420810(void) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0xcc
        push ebx
        push esi
        push edi
        push eax
        push edx
        rdtsc
        mov dword ptr [ebp - 8], eax
        pop edx
        pop eax
        mov edx, dword ptr [ebp + 0x10]
        mov edi, dword ptr [ebp + 8]
        xor ecx, ecx
        xor eax, eax
        cmp edx, ecx
        lea edx, [ebp - 0xcc]
        setne al
        mov dword ptr [ebp - 0x4c], eax
        lea eax, [ebp - 0xb0]
        mov dword ptr [ebp - 0x20], eax
        mov eax, dword ptr [edi + 0x1c]
        mov dword ptr [ebp - 0x24], edx
        lea edx, [ebp - 0x94]
        cmp eax, ecx
        mov dword ptr [ebp - 0x1c], edx
        mov dword ptr [ebp - 0x14], offset DAT_004b5648
        mov dword ptr [ebp + 0x10], ecx
        jle L4209f1
        mov dword ptr [ebp - 0xc], ecx
L42086a:
        add ecx, dword ptr [edi + 0x18]
        movsx edx, word ptr [ecx + 6]
        movsx eax, word ptr [ecx + 4]
        mov dword ptr [ebp - 0x58], edx
        mov dword ptr [ebp - 0x5c], eax
        mov dword ptr [ebp - 0x50], eax
        shl edx, 4
        movsx esi, word ptr [ecx + 8]
        shl eax, 4
        mov edi, edx
        mov edx, eax
        mov dword ptr [ebp - 0x54], esi
        mov eax, dword ptr [edx + DAT_004d8bb0 + 0x8]
        mov ebx, dword ptr [edi + DAT_004d8bb0 + 0x8]
        sub ebx, eax
        mov eax, dword ptr [edx + DAT_004d8bb0 + 0xc]
        mov dword ptr [ebp - 4], ebx
        mov ebx, dword ptr [edi + DAT_004d8bb0 + 0xc]
        fild dword ptr [ebp - 4]
        sub ebx, eax
        shl esi, 4
        fstp dword ptr [ebp - 0x38]
        mov dword ptr [ebp - 4], ebx
        mov ebx, dword ptr [esi + DAT_004d8bb0 + 0x8]
        fild dword ptr [ebp - 4]
        sub ebx, dword ptr [edx + DAT_004d8bb0 + 0x8]
        fstp dword ptr [ebp - 0x30]
        mov dword ptr [ebp - 4], ebx
        mov ebx, dword ptr [esi + DAT_004d8bb0 + 0xc]
        fild dword ptr [ebp - 4]
        sub ebx, eax
        mov dword ptr [ebp - 4], ebx
        fstp dword ptr [ebp - 0x34]
        fild dword ptr [ebp - 4]
        fst dword ptr [ebp - 0x2c]
        fmul dword ptr [ebp - 0x38]
        fld dword ptr [ebp - 0x34]
        fmul dword ptr [ebp - 0x30]
        fsubp st(1), st
        fst dword ptr [ebp - 0x28]
        fcomp dword ptr [FLOAT_004ab390]
        fnstsw ax
        test ah, 0x41
        jne L4209d3
        mov eax, dword ptr [edx + DAT_004d8bb0 + 0x14]
        mov edi, dword ptr [edi + DAT_004d8bb0 + 0x14]
        mov esi, dword ptr [esi + DAT_004d8bb0 + 0x14]
        mov edx, eax
        or eax, edi
        and edx, 0xff
        or eax, esi
        and edx, edi
        mov dword ptr [ebp - 0x40], eax
        and eax, 0xf
        and edx, esi
        cmp al, 0xf
        mov dword ptr [ebp - 0x44], edx
        jne L4209d3
        movsx eax, word ptr [ecx + 2]
        fld dword ptr [DAT_004dcbb8 + 0x8]
        mov edi, dword ptr [ebp + 8]
        lea eax, [eax + eax*2]
        mov edx, dword ptr [edi + 0x10]
        fmul dword ptr [edx + eax*4 + 8]
        fld dword ptr [DAT_004dcbb8 + 0x4]
        fmul dword ptr [edx + eax*4 + 4]
        lea eax, [edx + eax*4]
        faddp st(1), st
        fld dword ptr [DAT_004dcbb8]
        fmul dword ptr [eax]
        faddp st(1), st
        fadd dword ptr [DAT_00829a60]
        fstp dword ptr [ebp - 4]
        fld dword ptr [ebp - 4]
        fistp dword ptr [ebp - 0x10]
        mov eax, dword ptr [ebp - 0x10]
        mov edx, dword ptr [ebp + 0xc]
        mov dword ptr [ebp - 0x3c], eax
        mov esi, 3
        movsx eax, word ptr [ecx]
        lea ecx, [eax + eax*2]
        mov eax, dword ptr [edx + ecx*4]
        lea edx, [ebp - 0x5c]
        mov dword ptr [ebp - 0x48], eax
        lea ecx, [ebp - 0xc4]
L42099a:
        mov eax, dword ptr [edx]
        add edx, 4
        shl eax, 4
        add ecx, 0x1c
        dec esi
        mov ebx, dword ptr [eax + DAT_004d8bb0 + 0xc]
        mov dword ptr [ecx - 0x20], ebx
        mov ebx, dword ptr [eax + DAT_004d8bb0 + 0x8]
        mov eax, dword ptr [eax + DAT_004d8bb0 + 0x10]
        mov dword ptr [ecx - 0x1c], ebx
        mov dword ptr [ecx - 0x18], eax
        jne L42099a
        lea ecx, [ebp - 0x4c]
        push ecx
        push 2
        call FUN_0042a2f0
        add esp, 8
        jmp L4209d6
L4209d3:
        mov edi, dword ptr [ebp + 8]
L4209d6:
        mov eax, dword ptr [ebp + 0x10]
        mov ecx, dword ptr [ebp - 0xc]
        mov edx, dword ptr [edi + 0x1c]
        inc eax
        add ecx, 0x10
        cmp eax, edx
        mov dword ptr [ebp + 0x10], eax
        mov dword ptr [ebp - 0xc], ecx
        jl L42086a
L4209f1:
        push eax
        push edx
        rdtsc
        sub eax, dword ptr [ebp - 8]
        mov dword ptr [ebp - 8], eax
        pop edx
        pop eax
        mov edx, dword ptr [ebp - 8]
        mov eax, dword ptr [DAT_004dcbc8]
        add eax, edx
        pop edi
        pop esi
        mov dword ptr [DAT_004dcbc8], eax
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x00420a20
__declspec(naked) void FUN_00420a20(void) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0xd0
        push ebx
        push esi
        push edi
        push eax
        push edx
        rdtsc
        mov dword ptr [ebp - 4], eax
        pop edx
        pop eax
        mov esi, dword ptr [ebp + 0x10]
        xor edx, edx
        xor eax, eax
        cmp esi, edx
        setne al
        mov dword ptr [ebp - 0x50], eax
        lea ecx, [ebp - 0xd0]
        lea eax, [ebp - 0xb4]
        mov dword ptr [ebp - 0x28], ecx
        mov dword ptr [ebp - 0x24], eax
        mov eax, dword ptr [ebp + 8]
        lea ecx, [ebp - 0x98]
        mov dword ptr [ebp - 0x18], offset DAT_004b5658
        mov dword ptr [ebp - 0x20], ecx
        mov ecx, dword ptr [eax + 0x24]
        cmp ecx, edx
        mov dword ptr [ebp - 8], edx
        jle L420c10
        mov dword ptr [ebp - 0xc], edx
L420a7a:
        mov ebx, dword ptr [eax + 0x20]
        movsx eax, word ptr [edx + ebx + 4]
        movsx ecx, word ptr [edx + ebx + 6]
        add edx, ebx
        mov dword ptr [ebp - 0x5c], ecx
        mov dword ptr [ebp - 0x60], eax
        mov dword ptr [ebp - 0x54], eax
        shl ecx, 4
        movsx esi, word ptr [edx + 8]
        shl eax, 4
        mov edi, ecx
        mov ecx, eax
        mov dword ptr [ebp - 0x58], esi
        mov eax, dword ptr [ecx + DAT_004d8bb0 + 0x8]
        mov ebx, dword ptr [edi + DAT_004d8bb0 + 0x8]
        sub ebx, eax
        mov eax, dword ptr [ecx + DAT_004d8bb0 + 0xc]
        mov dword ptr [ebp + 0x10], ebx
        mov ebx, dword ptr [edi + DAT_004d8bb0 + 0xc]
        fild dword ptr [ebp + 0x10]
        sub ebx, eax
        shl esi, 4
        fstp dword ptr [ebp - 0x3c]
        mov dword ptr [ebp + 0x10], ebx
        mov ebx, dword ptr [esi + DAT_004d8bb0 + 0x8]
        fild dword ptr [ebp + 0x10]
        sub ebx, dword ptr [ecx + DAT_004d8bb0 + 0x8]
        fstp dword ptr [ebp - 0x34]
        mov dword ptr [ebp + 0x10], ebx
        mov ebx, dword ptr [esi + DAT_004d8bb0 + 0xc]
        fild dword ptr [ebp + 0x10]
        sub ebx, eax
        mov dword ptr [ebp + 0x10], ebx
        fstp dword ptr [ebp - 0x38]
        fild dword ptr [ebp + 0x10]
        fst dword ptr [ebp - 0x30]
        fmul dword ptr [ebp - 0x3c]
        fld dword ptr [ebp - 0x38]
        fmul dword ptr [ebp - 0x34]
        fsubp st(1), st
        fst dword ptr [ebp - 0x2c]
        fcomp dword ptr [FLOAT_004ab390]
        fnstsw ax
        test ah, 0x41
        jne L420bf2
        mov eax, dword ptr [ecx + DAT_004d8bb0 + 0x14]
        mov edi, dword ptr [edi + DAT_004d8bb0 + 0x14]
        mov esi, dword ptr [esi + DAT_004d8bb0 + 0x14]
        mov ecx, eax
        or eax, edi
        and ecx, 0xff
        or eax, esi
        and ecx, edi
        mov dword ptr [ebp - 0x44], eax
        and eax, 0xf
        and ecx, esi
        cmp al, 0xf
        mov dword ptr [ebp - 0x48], ecx
        jne L420bf2
        lea edi, [ebp - 0x60]
        lea ecx, [ebp - 0xc8]
        lea esi, [edx + 0xa]
        mov dword ptr [ebp + 0x10], 3
L420b5f:
        movsx eax, word ptr [esi]
        fld dword ptr [DAT_004dcbb8 + 0x8]
        mov ebx, dword ptr [ebp + 8]
        lea eax, [eax + eax*2]
        mov ebx, dword ptr [ebx + 0x14]
        fmul dword ptr [ebx + eax*4 + 8]
        fld dword ptr [DAT_004dcbb8 + 0x4]
        fmul dword ptr [ebx + eax*4 + 4]
        lea eax, [ebx + eax*4]
        faddp st(1), st
        fld dword ptr [DAT_004dcbb8]
        fmul dword ptr [eax]
        faddp st(1), st
        fadd dword ptr [DAT_00829a60]
        fstp dword ptr [ebp - 0x10]
        fld dword ptr [ebp - 0x10]
        fistp dword ptr [ebp - 0x14]
        mov eax, dword ptr [edi]
        add esi, 2
        shl eax, 4
        add edi, 4
        add ecx, 0x1c
        mov ebx, dword ptr [eax + DAT_004d8bb0 + 0xc]
        mov dword ptr [ecx - 0x20], ebx
        mov ebx, dword ptr [eax + DAT_004d8bb0 + 0x8]
        mov eax, dword ptr [eax + DAT_004d8bb0 + 0x10]
        mov dword ptr [ecx - 0x1c], ebx
        mov ebx, dword ptr [ebp - 0x14]
        mov dword ptr [ecx - 0x18], ebx
        mov dword ptr [ecx - 0x14], eax
        mov eax, dword ptr [ebp + 0x10]
        dec eax
        mov dword ptr [ebp + 0x10], eax
        jne L420b5f
        movsx eax, word ptr [edx]
        mov edx, dword ptr [ebp + 0xc]
        lea ecx, [eax + eax*2]
        mov eax, dword ptr [edx + ecx*4]
        lea ecx, [ebp - 0x50]
        push ecx
        push 3
        mov dword ptr [ebp - 0x4c], eax
        call FUN_0042a2f0
        add esp, 8
L420bf2:
        mov eax, dword ptr [ebp + 8]
        mov ecx, dword ptr [ebp - 8]
        mov edx, dword ptr [ebp - 0xc]
        inc ecx
        mov esi, dword ptr [eax + 0x24]
        add edx, 0x10
        cmp ecx, esi
        mov dword ptr [ebp - 8], ecx
        mov dword ptr [ebp - 0xc], edx
        jl L420a7a
L420c10:
        push eax
        push edx
        rdtsc
        sub eax, dword ptr [ebp - 4]
        mov dword ptr [ebp - 4], eax
        pop edx
        pop eax
        mov edx, dword ptr [ebp - 4]
        mov eax, dword ptr [DAT_004dcbc8]
        add eax, edx
        pop edi
        pop esi
        mov dword ptr [DAT_004dcbc8], eax
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x00420c40
__declspec(naked) void FUN_00420c40(void) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0xd0
        push ebx
        push esi
        push edi
        push eax
        push edx
        rdtsc
        mov dword ptr [ebp - 8], eax
        pop edx
        pop eax
        mov edx, dword ptr [ebp + 0x10]
        xor esi, esi
        xor eax, eax
        cmp edx, esi
        setne al
        mov dword ptr [ebp - 0x50], eax
        lea eax, [ebp - 0x98]
        mov dword ptr [ebp - 0x20], eax
        mov eax, dword ptr [ebp + 8]
        lea ecx, [ebp - 0xd0]
        lea edx, [ebp - 0xb4]
        mov dword ptr [ebp - 0x28], ecx
        mov ecx, dword ptr [eax + 0x2c]
        cmp ecx, esi
        mov dword ptr [ebp - 0x24], edx
        mov dword ptr [ebp - 0x18], offset DAT_004b5f50
        mov dword ptr [ebp - 0x10], esi
        jle L420e5f
        mov dword ptr [ebp - 0xc], esi
L420c9a:
        mov ebx, dword ptr [eax + 0x28]
        movsx eax, word ptr [esi + ebx + 4]
        movsx ecx, word ptr [esi + ebx + 6]
        add esi, ebx
        mov dword ptr [ebp - 0x5c], ecx
        mov dword ptr [ebp - 0x60], eax
        mov dword ptr [ebp - 0x54], eax
        shl ecx, 4
        movsx edx, word ptr [esi + 8]
        shl eax, 4
        mov edi, ecx
        mov ecx, eax
        mov dword ptr [ebp - 0x58], edx
        mov eax, dword ptr [ecx + DAT_004d8bb0 + 0x8]
        mov ebx, dword ptr [edi + DAT_004d8bb0 + 0x8]
        sub ebx, eax
        mov eax, dword ptr [ecx + DAT_004d8bb0 + 0xc]
        mov dword ptr [ebp + 0x10], ebx
        mov ebx, dword ptr [edi + DAT_004d8bb0 + 0xc]
        fild dword ptr [ebp + 0x10]
        sub ebx, eax
        shl edx, 4
        fstp dword ptr [ebp - 0x3c]
        mov dword ptr [ebp + 0x10], ebx
        mov ebx, dword ptr [edx + DAT_004d8bb0 + 0x8]
        fild dword ptr [ebp + 0x10]
        sub ebx, dword ptr [ecx + DAT_004d8bb0 + 0x8]
        fstp dword ptr [ebp - 0x34]
        mov dword ptr [ebp + 0x10], ebx
        mov ebx, dword ptr [edx + DAT_004d8bb0 + 0xc]
        fild dword ptr [ebp + 0x10]
        sub ebx, eax
        mov dword ptr [ebp + 0x10], ebx
        fstp dword ptr [ebp - 0x38]
        fild dword ptr [ebp + 0x10]
        fst dword ptr [ebp - 0x30]
        fmul dword ptr [ebp - 0x3c]
        fld dword ptr [ebp - 0x38]
        fmul dword ptr [ebp - 0x34]
        fsubp st(1), st
        fst dword ptr [ebp - 0x2c]
        fcomp dword ptr [FLOAT_004ab390]
        fnstsw ax
        test ah, 0x41
        jne L420e41
        mov eax, dword ptr [ecx + DAT_004d8bb0 + 0x14]
        mov edi, dword ptr [edi + DAT_004d8bb0 + 0x14]
        mov edx, dword ptr [edx + DAT_004d8bb0 + 0x14]
        mov ecx, eax
        or eax, edi
        and ecx, 0xff
        or eax, edx
        and ecx, edi
        mov dword ptr [ebp - 0x44], eax
        and eax, 0xf
        and ecx, edx
        cmp al, 0xf
        mov dword ptr [ebp - 0x48], ecx
        jne L420e41
        movsx eax, word ptr [esi + 2]
        fld dword ptr [DAT_004dcbb8 + 0x8]
        mov edx, dword ptr [ebp + 8]
        lea ecx, [eax + eax*2]
        mov eax, dword ptr [edx + 0x10]
        fmul dword ptr [eax + ecx*4 + 8]
        fld dword ptr [DAT_004dcbb8 + 0x4]
        fmul dword ptr [eax + ecx*4 + 4]
        lea eax, [eax + ecx*4]
        faddp st(1), st
        fld dword ptr [DAT_004dcbb8]
        fmul dword ptr [eax]
        faddp st(1), st
        fadd dword ptr [DAT_00829a60]
        fstp dword ptr [ebp + 0x10]
        fld dword ptr [ebp + 0x10]
        fistp dword ptr [ebp - 0x14]
        mov ecx, dword ptr [ebp - 0x14]
        mov ebx, dword ptr [ebp + 0xc]
        mov dword ptr [ebp - 0x40], ecx
        xor edi, edi
        movsx eax, word ptr [esi]
        lea ecx, [ebp - 0x60]
        mov dword ptr [ebp + 0x10], edi
        lea edx, [eax + eax*2]
        mov dword ptr [ebp - 4], ecx
        mov eax, dword ptr [ebx + edx*4]
        mov dword ptr [ebp - 0x4c], eax
        lea eax, [ebp - 0xc8]
L420dd1:
        mov edx, dword ptr [ebp - 4]
        add eax, 0x1c
        mov ecx, dword ptr [edx]
        shl ecx, 4
        mov edx, dword ptr [ecx + DAT_004d8bb0 + 0xc]
        mov dword ptr [eax - 0x20], edx
        mov edx, dword ptr [ecx + DAT_004d8bb0 + 0x8]
        mov dword ptr [eax - 0x1c], edx
        mov ecx, dword ptr [ecx + DAT_004d8bb0 + 0x10]
        movsx edx, word ptr [esi]
        lea edx, [edx + edx*2]
        lea edi, [edi + edx*2]
        xor edx, edx
        mov dl, byte ptr [ebx + edi*2 + 4]
        mov edi, dword ptr [ebp + 0x10]
        mov dword ptr [eax - 0x18], edx
        movsx edx, word ptr [esi]
        lea edx, [edx + edx*2]
        lea edi, [edi + edx*2]
        xor edx, edx
        mov dl, byte ptr [ebx + edi*2 + 5]
        mov edi, dword ptr [ebp + 0x10]
        mov dword ptr [eax - 0x14], edx
        mov dword ptr [eax - 0x10], ecx
        mov ecx, dword ptr [ebp - 4]
        inc edi
        add ecx, 4
        cmp edi, 2
        mov dword ptr [ebp + 0x10], edi
        mov dword ptr [ebp - 4], ecx
        jle L420dd1
        lea edx, [ebp - 0x50]
        push edx
        push 4
        call FUN_0042a2f0
        add esp, 8
L420e41:
        mov eax, dword ptr [ebp + 8]
        mov ecx, dword ptr [ebp - 0x10]
        mov esi, dword ptr [ebp - 0xc]
        inc ecx
        mov edx, dword ptr [eax + 0x2c]
        add esi, 0x10
        cmp ecx, edx
        mov dword ptr [ebp - 0x10], ecx
        mov dword ptr [ebp - 0xc], esi
        jl L420c9a
L420e5f:
        push eax
        push edx
        rdtsc
        sub eax, dword ptr [ebp - 8]
        mov dword ptr [ebp - 8], eax
        pop edx
        pop eax
        mov eax, dword ptr [ebp - 8]
        mov ecx, dword ptr [DAT_004dcbc8]
        add ecx, eax
        pop edi
        pop esi
        mov dword ptr [DAT_004dcbc8], ecx
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x00420e90
__declspec(naked) unsigned int DrawLmsModel(struct LmsFile *a, struct LmsRec *b, void *c, void *d, unsigned int e) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x9c
        push ebx
        push esi
        push edi
        push eax
        push edx
        rdtsc
        mov dword ptr [ebp - 4], eax
        pop edx
        pop eax
        mov esi, dword ptr [ebp + 8]
        test esi, esi
        je L420fa7
        mov edi, dword ptr [ebp + 0xc]
        test edi, edi
        je L420fa7
        mov ebx, dword ptr [ebp + 0x14]
        lea eax, [ebp - 0x5c]
        push eax
        push ebx
        call FUN_00426510
        lea ecx, [ebp - 0x5c]
        push 1
        push ecx
        push offset DAT_004dcbb8
        push offset DAT_004b5ca0
        call TransformPoints3x4
        fld dword ptr [DAT_004dcbb8]
        fmul dword ptr [DAT_0082999c]
        mov eax, dword ptr [ebp + 0x10]
        lea edx, [ebp - 0x5c]
        push edx
        push ebx
        push eax
        fstp dword ptr [DAT_004dcbb8]
        fld dword ptr [DAT_004dcbb8 + 0x4]
        fmul dword ptr [DAT_0082999c]
        fstp dword ptr [DAT_004dcbb8 + 0x4]
        fld dword ptr [DAT_004dcbb8 + 0x8]
        fmul dword ptr [DAT_0082999c]
        fstp dword ptr [DAT_004dcbb8 + 0x8]
        call MakeMat4FromRotTrans
        lea ecx, [ebp - 0x9c]
        lea edx, [ebp - 0x5c]
        push ecx
        push edx
        push offset DAT_008299fc
        call Mat4Multiply
        mov eax, dword ptr [esi]
        mov edx, dword ptr [esi + 0xc]
        push eax
        lea ecx, [ebp - 0x9c]
        push 0x10
        push ecx
        push offset DAT_004d8bb0 + 0x8
        push edx
        call FUN_00426250
        add esp, 0x44
        push eax
        push edx
        rdtsc
        sub eax, dword ptr [ebp - 4]
        mov dword ptr [ebp - 4], eax
        pop edx
        pop eax
        mov eax, dword ptr [ebp - 4]
        mov ecx, dword ptr [DAT_004dcbc8]
        add ecx, eax
        mov dword ptr [DAT_004dcbc8], ecx
        lea ecx, [ebp - 0x1c]
        push ecx
        call GetCoasterSurface
        add esp, 4
        test eax, eax
        je L420fa7
        mov ebx, dword ptr [ebp + 0x18]
        push ebx
        push edi
        push esi
        call FUN_00420810
        push ebx
        push edi
        push esi
        call FUN_00420a20
        push ebx
        push edi
        push esi
        call FUN_00420c40
        lea edx, [ebp - 0x1c]
        push edx
        call FUN_00423790
        add esp, 0x28
L420fa7:
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// FUNCTION: LEGOLAND 0x00420fb0
unsigned int FUN_00420fb0(struct LmsFile *param_1, unsigned int *param_2, unsigned int *param_3, struct RectI *param_4) {
    return FUN_00426750(param_1->box, param_2, param_3, param_4);
}

// GLOBAL: LEGOLAND 0x004b5648
void *DAT_004b5648[2] = {FUN_0041f8d0, FUN_0041fba0};

// GLOBAL: LEGOLAND 0x004b5658
void *DAT_004b5658[2] = {FUN_0041fd80, FUN_0041ff80};

// GLOBAL: LEGOLAND 0x004b5700
float DAT_004b5700[6][3] = {
    {0.0f, 0.0f, -1.0f},
    {0.0f, 0.0f, 1.0f},
    {1.0f, 0.0f, 0.0f},
    {-1.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, -1.0f, 0.0f},
};

// GLOBAL: LEGOLAND 0x004b5748
struct BoxFace DAT_004b5748[12] = {
    {0, 0, {0, 1, 2}, {0, 0, 0}},
    {0, 0, {0, 2, 3}, {0, 0, 0}},
    {0, 1, {7, 6, 5}, {0, 0, 0}},
    {0, 1, {7, 5, 4}, {0, 0, 0}},
    {0, 2, {0, 5, 1}, {0, 0, 0}},
    {0, 2, {0, 4, 5}, {0, 0, 0}},
    {0, 3, {2, 7, 3}, {0, 0, 0}},
    {0, 3, {2, 6, 7}, {0, 0, 0}},
    {0, 4, {1, 6, 2}, {0, 0, 0}},
    {0, 3, {1, 5, 6}, {0, 0, 0}},
    {0, 5, {3, 4, 0}, {0, 0, 0}},
    {0, 3, {3, 7, 4}, {0, 0, 0}},
};

// GLOBAL: LEGOLAND 0x004b5808
BoxFace DAT_004b5808[12] = {
    {0, 0, {0, 1, 2}, {0, 0, 0}},
    {0, 0, {0, 2, 3}, {0, 0, 0}},
    {0, 1, {7, 6, 5}, {0, 0, 0}},
    {0, 1, {7, 5, 4}, {0, 0, 0}},
    {0, 2, {0, 5, 1}, {0, 0, 0}},
    {0, 2, {0, 4, 5}, {0, 0, 0}},
    {0, 3, {2, 7, 3}, {0, 0, 0}},
    {0, 3, {2, 6, 7}, {0, 0, 0}},
    {0, 4, {1, 6, 2}, {0, 0, 0}},
    {0, 3, {1, 5, 6}, {0, 0, 0}},
    {0, 5, {3, 4, 0}, {0, 0, 0}},
    {0, 3, {3, 7, 4}, {0, 0, 0}},
};

// GLOBAL: LEGOLAND 0x004b58c8
struct BoxSolid DAT_004b58c8 = {8, 0, 12, NULL, DAT_004b5700, NULL, DAT_004b5748, 12};

// GLOBAL: LEGOLAND 0x004b5f50
void *DAT_004b5f50[2] = {FUN_00428860, FUN_00428860};

// FUNCTION: LEGOLAND 0x00420fd0
void FUN_00420fd0(struct BoxSolid *box, struct FVec3 *verts, struct FVec3 *xverts, int unused, float w, float h, float d) {
    int i;

    *box = DAT_004b58c8;
    box->verts = verts;
    box->xverts = xverts;
    box->verts[0].x = w;
    box->verts[0].y = -h;
    box->verts[0].z = -d;
    box->verts[1].x = w;
    box->verts[1].y = h;
    box->verts[1].z = -d;
    box->verts[2].x = -w;
    box->verts[2].y = h;
    box->verts[2].z = -d;
    box->verts[3].x = -w;
    box->verts[3].y = -h;
    box->verts[3].z = -d;
    for (i = 0; i <= 3; i++) {
        box->verts[i + 4] = box->verts[i];
        box->verts[i + 4].z = 0.0f;
    }
    if (xverts != NULL) {
        for (i = 0; i <= 7; i++) {
            struct FVec3 tmp = box->verts[i];
            FUN_00425d50(&tmp.x);
            box->xverts[i] = tmp;
        }
        for (i = 0; i < 12; i++) {
            DAT_004b5808[i].plane = -1;
            DAT_004b5808[i].u[0] = DAT_004b5808[i].t[0];
            DAT_004b5808[i].u[1] = DAT_004b5808[i].t[1];
            DAT_004b5808[i].u[2] = DAT_004b5808[i].t[2];
        }
        box->nFaceA = 0;
        box->nFaceB = 12;
        box->faceB = DAT_004b5808;
    }
}

// FUNCTION: LEGOLAND 0x00421130
void FUN_00421130(struct BoxSolid *box, struct FVec3 *verts, struct FVec3 *xverts, int unused, float w, float h, float d) {
    int i;

    *box = DAT_004b58c8;
    box->verts = verts;
    box->xverts = xverts;
    box->verts[0].x = w;
    box->verts[0].y = -h;
    box->verts[0].z = -d;
    box->verts[1].x = w;
    box->verts[1].y = h;
    box->verts[1].z = -d;
    box->verts[2].x = -w;
    box->verts[2].y = h;
    box->verts[2].z = -d;
    box->verts[3].x = -w;
    box->verts[3].y = -h;
    box->verts[3].z = -d;
    for (i = 0; i <= 3; i++) {
        box->verts[i + 4] = box->verts[i];
        box->verts[i + 4].z = d;
    }
    if (xverts != NULL) {
        for (i = 0; i <= 7; i++) {
            struct FVec3 tmp = box->verts[i];
            FUN_00425d50(&tmp.x);
            box->xverts[i] = tmp;
        }
        for (i = 0; i < 12; i++) {
            DAT_004b5808[i].plane = -1;
            DAT_004b5808[i].u[0] = DAT_004b5808[i].t[0];
            DAT_004b5808[i].u[1] = DAT_004b5808[i].t[1];
            DAT_004b5808[i].u[2] = DAT_004b5808[i].t[2];
        }
        box->nFaceA = 0;
        box->nFaceB = 12;
        box->faceB = DAT_004b5808;
    }
}

// FUNCTION: LEGOLAND 0x004212a0
void FloatArrayAdd(struct FloatArray *a, struct FloatArray *b, struct FloatArray *out) {
    int i;
    out->count = a->count;
    for (i = 0; i < a->count; i++) {
        float t = b->data[i];
        out->data[i] = t + a->data[i];
    }
}

// FUNCTION: LEGOLAND 0x004212e0
void FloatArraySub(struct FloatArray *a, struct FloatArray *b, struct FloatArray *out) {
    int i;
    out->count = a->count;
    for (i = 0; i < a->count; i++) {
        out->data[i] = a->data[i] - b->data[i];
    }
}

// FUNCTION: LEGOLAND 0x00421320
void CopyFloatArray(struct FloatArray *src, struct FloatArray *dst) {
    memcpy(dst, src, sizeof(struct FloatArray));
}

// FUNCTION: LEGOLAND 0x00421340
void FloatArrayScale(struct FloatArray *arr, float f) {
    int i;
    for (i = 0; i < arr->count; i++) {
        arr->data[i] = f * arr->data[i];
    }
}

// FUNCTION: LEGOLAND 0x00421360
float FloatArrayMaxAbs(struct FloatArray *arr) {
    float max = FLOAT_004ab390;
    int i;
    for (i = 0; i < arr->count; i++) {
        float v = (float)fabs(arr->data[i]);
        if (v > max) {
            max = v;
        }
    }
    return max;
}

// FUNCTION: LEGOLAND 0x004213a0
void BlendFloatArrays(struct FloatArray *a, float s, struct FloatArray *b, float t, struct FloatArray *out) {
    int i;

    out->count = a->count;
    for (i = 0; i < a->count; i++) {
        out->data[i] = a->data[i] * s + b->data[i] * t;
    }
    out->count = a->count;
}

// FUNCTION: LEGOLAND 0x00421400
void ZeroIntBuffer(struct FloatArray *buf, int size) {
    int i;
    if (size > 0) {
        for (i = 0; i < size; i++) {
            *(int *)&buf->data[i] = 0;
        }
    }
    buf->count = size;
}

// FUNCTION: LEGOLAND 0x00421430
void FloatArrayAddScaled(struct FloatArray *a, float f, struct FloatArray *out) {
    int i;
    out->count = a->count;
    for (i = 0; i < a->count; i++) {
        out->data[i] += f * a->data[i];
    }
}

// FUNCTION: LEGOLAND 0x00421470
void InitVecMathOps(void) {
    struct FloatArray **table;
    struct FloatArray *cur;

    VecMathOps.add = FloatArrayAdd;
    VecMathOps.sub = FloatArraySub;
    VecMathOps.copy = CopyFloatArray;
    VecMathOps.scale = FloatArrayScale;
    VecMathOps.maxabs = FloatArrayMaxAbs;
    VecMathOps.blend = BlendFloatArrays;
    VecMathOps.zero = ZeroIntBuffer;
    VecMathOps.axpy = FloatArrayAddScaled;
    VecMathOps.alloc = AllocVecScratch;
    VecMathOps.release = FreeVecScratch;

    table = VecScratchPool;
    cur = DAT_004dcc00;
    while ((INT_PTR)cur < (INT_PTR)&VecScratchTop) {
        *table = cur;
        cur++;
        table += 1;
    }
}

// FUNCTION: LEGOLAND 0x004214f0
struct FloatArray **AllocVecScratch(unsigned int param_1) {
    unsigned int index = VecScratchTop;
    VecScratchTop = VecScratchTop + param_1;
    return &VecScratchPool[index];
}

// FUNCTION: LEGOLAND 0x00421510
void FreeVecScratch(struct FloatArray **param_1, unsigned int param_2) {
    VecScratchTop = VecScratchTop - param_2;
}

// FUNCTION: LEGOLAND 0x00421540
void InitVecMath(struct VecMath *dest, unsigned int value) {
    VecMathOps.dim = value;
    memcpy(dest, &VecMathOps, sizeof(struct VecMath));
}

// FUNCTION: LEGOLAND 0x00421560
unsigned int DrawCastleVisitor(struct CastleVisitor *param_2, struct CarXform *param_1) {
    FUN_00425c40();
    return DrawLmsModel(param_2->lms, param_2->field_8, param_1, &param_1->m, 0);
}

// FUNCTION: LEGOLAND 0x00421590
void SeatCastleVisitor(struct CastleVisitor *arg1, struct TrainSeat *arg2) {
    SetTrainSeatOccupant(arg2, arg1);
    arg1->seat = arg2;
}

// FUNCTION: LEGOLAND 0x004215b0
void UnseatCastleVisitor(struct CastleVisitor *param_1) {
    struct TrainSeat *temp = param_1->seat;
    TakeTrainSeatOccupant(temp);
    param_1->seat = 0;
}

// FUNCTION: LEGOLAND 0x004215d0
struct CastleVisitor *CreateCastleVisitor(struct BlokeInfo *a1) {
    struct CastleVisitor *self = MemAlloc(sizeof(struct CastleVisitor), 0, 0, 0);
    if (self == NULL) {
        return NULL;
    }
    self->flags = 0;
    if (a1->sex == 0) {
        self->lms = GetLmsByName(SitLoManSitName);
        self->field_8 = GetLfmByName(SitLoManSitName);
    } else {
        self->lms = GetLmsByName(SitLoGirlSitName);
        self->field_8 = GetLfmByName(SitLoGirlSitName);
    }
    self->field_8 = FUN_00421660(a1);
    if (self->field_8 == 0) {
        self->flags = 1;
    }
    self->draw = DrawCastleVisitor;
    self->unlink = FreeCastleVisitor;
    self->bloke = (struct Bloke *)a1->bloke;
    return self;
}

// FUNCTION: LEGOLAND 0x00421660
struct LmsRec *FUN_00421660(struct BlokeInfo *p) {
    struct LmsFile *lms;
    struct LmsRec *src;
    struct LmsRec *dst;
    unsigned int *colours;
    unsigned int size;
    const char **names;
    struct KeyPair colPairs[2];
    struct KeyPair namePairs[2];
    char *nm;
    unsigned int *cp;
    int i;
    int k;
    struct LmsRef *r;

    if (p->sex == 0) {
        lms = GetLmsByName(SitLoManSitName);
        src = GetLfmByName(SitLoManSitName);
        size = GetLfmSizeByName(SitLoManSitName);
        names = DAT_004b596c;
        colours = DAT_004b5964;
    } else {
        lms = GetLmsByName(SitLoGirlSitName);
        src = GetLfmByName(SitLoGirlSitName);
        size = GetLfmSizeByName(SitLoGirlSitName);
        names = DAT_004b597c;
        colours = DAT_004b5974;
    }
    dst = MemAlloc(size, 0, 0, 0);
    if (dst == NULL) {
        return 0;
    }
    for (i = 0, nm = p->chest; i < 2; i++) {
        namePairs[i].key = FUN_00420790(*names++);
        namePairs[i].val = FUN_00420790(nm);
        nm += 0x14;
    }
    cp = &p->leg;
    for (i = 0; i < 2; i++) {
        colPairs[i].key = FindRollercoasterLptEntry(*colours++);
        colPairs[i].val = FindRollercoasterLptEntry(*cp);
        cp++;
    }
    memcpy(dst, src, size);
    for (i = 0; i < lms->field_2c; i++) {
        unsigned int key;
        r = lms->field_28 + i;
        key = src[r->idx].key;
        for (k = 0; k <= 1; k++) {
            if (namePairs[k].key == key) {
                key = namePairs[k].val;
                break;
            }
        }
        dst[r->idx].key = key;
    }
    for (i = 0; i < lms->field_1c; i++) {
        unsigned int key;
        r = lms->field_18 + i;
        key = src[r->idx].key;
        for (k = 0; k <= 1; k++) {
            if (colPairs[k].key == key) {
                key = colPairs[k].val;
                break;
            }
        }
        dst[r->idx].key = key;
    }
    for (i = 0; i < lms->field_24; i++) {
        unsigned int key;
        r = lms->field_20 + i;
        key = src[r->idx].key;
        for (k = 0; k <= 1; k++) {
            if (colPairs[k].key == key) {
                key = colPairs[k].val;
                break;
            }
        }
        dst[r->idx].key = key;
    }
    return dst;
}

// FUNCTION: LEGOLAND 0x00421890
struct BlokeInfo *GetBlokeAppearance(struct BlokeSex0 *bloke) {
    DAT_004b5988.bloke = bloke;
    DAT_004b5988.sex = GetSexOfBloke(bloke) != 0;
    DAT_004b5988.leg = GetLegColourOfBloke(bloke) & 0xffffff;
    DAT_004b5988.arm = GetArmColourOfBloke(bloke) & 0xffffff;
    strcpy(DAT_004b5988.chest, GetChestTextureNameOfBloke(bloke));
    strcpy(DAT_004b5988.face, GetFaceTextureNameOfBloke(bloke));
    return &DAT_004b5988;
}

// FUNCTION: LEGOLAND 0x00421930
struct CastleVisitor *AddCastleVisitor(struct Bloke *param, struct CastleState *timer) {
    struct CastleVisitor *node;
    struct CastleVisitor *old_head;

    node = CreateCastleVisitor(GetBlokeAppearance((struct BlokeSex0 *)param));

    old_head = timer->visitors.prev;
    old_head->next = node;
    old_head = timer->visitors.prev;
    node->prev = old_head;
    node->next = (struct CastleVisitor *)&timer->visitors;
    timer->visitors.prev = node;

    node->time = GetGameTimer();
    node->flags = 1;

    return node;
}

// FUNCTION: LEGOLAND 0x00421980
void FreeCastleVisitor(struct CastleVisitor *p) {
    p->prev->next = p->next;
    p->next->prev = p->prev;

    if ((p->flags & 1U) == 0U) {
        MemFree(p->field_8);
    }

    MemFree(p);
}

struct Vec5 {
    float field_0;
    float field_4;
    unsigned int field_8;
    float field_c;
    float field_10;
};

struct Vec3 {
    float field_0;
    float field_4;
    unsigned int field_8;
};

// FUNCTION: LEGOLAND 0x004219c0
void FUN_004219c0(struct Vec5 *arg1, float float_arg, struct Vec3 *arg3) {
    arg3->field_0 = arg1->field_0 + (float_arg * arg1->field_c);
    arg3->field_4 = arg1->field_4 + (float_arg * arg1->field_10);
    arg3->field_8 = arg1->field_8;
}

struct Words3 {
    unsigned int field_0;
    unsigned int field_4;
    unsigned int field_8;
};

// FUNCTION: LEGOLAND 0x004219f0
void FUN_004219f0(unsigned char *param_1, unsigned int param_2, struct Words3 *param_3) {
    param_1 = param_1 + 12;
    param_3->field_0 = *(unsigned int *)param_1;
    param_3->field_4 = *(unsigned int *)(param_1 + 4);
    param_3->field_8 = *(unsigned int *)(param_1 + 8);
}

struct Vec7 {
    float field_0;
    float field_4;
    unsigned int field_8;
    float field_c;
    float field_10;
    unsigned char pad_14[4];
    float field_18;
    float field_1c;
};

// FUNCTION: LEGOLAND 0x00421a10
void FUN_00421a10(struct Vec7 *s1, float arg2, struct Vec3 *s2) {
    s2->field_0 = arg2 * s1->field_c + s1->field_18 + s1->field_0;
    s2->field_4 = arg2 * s1->field_10 + s1->field_1c + s1->field_4;
    s2->field_8 = s1->field_8;
}

// FUNCTION: LEGOLAND 0x00421a40
void FUN_00421a40(struct Vec7 *a, float c, struct Vec3 *b) {
    b->field_0 = c * a->field_c + a->field_0 - a->field_18;
    b->field_4 = c * a->field_10 + a->field_4 - a->field_1c;
    b->field_8 = a->field_8;
}

struct Struct1a70A {
    unsigned char pad_0[0x44];
    unsigned int field_44;
    unsigned int field_48;
};

// FUNCTION: LEGOLAND 0x00421a70
int FUN_00421a70(struct Struct1a70A *param_1, struct Pair *param_2) {
    param_2->field_0 = param_1->field_44;
    param_2->field_4 = param_1->field_48;
    return 2;
}

struct Struct1a90 {
    unsigned int field_0;
    unsigned int field_4;
    float field_8;
};

// FUNCTION: LEGOLAND 0x00421a90
void FUN_00421a90(unsigned int param_1, unsigned int param_2, struct Struct1a90 *param_3) {
    param_3->field_0 = 0;
    param_3->field_4 = 0;
    param_3->field_8 = 1.0f;
}

struct Obj421ab0 {
    struct Words3 field_0;
    float field_c[3];
    struct Words3 field_18;
    float field_24[4];
    unsigned char pad_34[0xc];
    float field_40;
    unsigned int field_44;
    float field_48;
    void **field_4c;
    struct SegBlob *field_50; /* next */
    struct SegBlob *field_54; /* prev */
};

// FUNCTION: LEGOLAND 0x00421ab0
void InitLineCurve(struct Obj421ab0 *obj, float *a, float *b, struct Words3 *c) {
    float sum = FLOAT_004ab390;
    float d;
    int i;

    for (i = 0; i < 3; i++) {
        d = b[i] - a[i];
        obj->field_c[i] = d;
        sum += d * d;
    }
    obj->field_40 = (float)sqrt(sum);
    obj->field_0 = *(struct Words3 *)a;
    obj->field_18 = *c;
    obj->field_4c = &CurveFunctionTable[8];
    obj->field_44 = 0;
    obj->field_48 = 1.0f;
    obj->field_50 = NULL;
    obj->field_54 = NULL;
}

// FUNCTION: LEGOLAND 0x00421b40
void FUN_00421b40(float m[3][3], float angle, float out[3]) {
    int j;
    float s = (float)sin(angle);
    float c = (float)cos(angle);
    for (j = 0; j < 2; j++) {
        out[j] = c * m[0][j] + s * m[1][j] + m[2][j];
    }
    out[2] = m[2][2];
}

// FUNCTION: LEGOLAND 0x00421b90
void FUN_00421b90(float m[3][3], float angle, float out[3]) {
    int j;
    float s = (float)sin(angle);
    angle = (float)cos(angle);
    s = -s;
    for (j = 0; j < 2; j++) {
        out[j] = angle * m[1][j] + s * m[0][j];
    }
    out[2] = 0.0f;
}

// FUNCTION: LEGOLAND 0x00421be0
void FUN_00421be0(float m[12], float angle, float out[3]) {
    int j;
    float s = (float)sin(angle) * m[9];
    float c = (float)cos(angle) * m[9];
    for (j = 0; j < 2; j++) {
        out[j] = c * m[j] + s * m[j + 3] + m[j + 6];
    }
    out[2] = m[8];
}

// FUNCTION: LEGOLAND 0x00421c30
void FUN_00421c30(float m[12], float angle, float out[3]) {
    int j;
    float s = (float)sin(angle) * m[10];
    float c = (float)cos(angle) * m[10];
    for (j = 0; j < 2; j++) {
        out[j] = c * m[j] + s * m[j + 3] + m[j + 6];
    }
    out[2] = m[8];
}

struct Floats6 {
    float field_0;
    float field_4;
    float field_8;
    float field_c;
    float field_10;
    float field_14;
};

// FUNCTION: LEGOLAND 0x00421c80
unsigned int FUN_00421c80(unsigned int param_1, struct Floats6 *ptr_0) {
    ptr_0->field_0 = 0.0f;
    ptr_0->field_4 = 0.31415900588035583f;
    ptr_0->field_8 = 0.6283180117607117f;
    ptr_0->field_c = 0.9424769878387451f;
    ptr_0->field_10 = 1.2566360235214233f;
    ptr_0->field_14 = 1.5707950592041016f;
    return 6;
}

// FUNCTION: LEGOLAND 0x00421cc0
void FUN_00421cc0(int param_1, int param_2, struct Struct1a90 *param_3) {
    param_3->field_0 = 0;
    param_3->field_4 = 0;
    param_3->field_8 = 1.0f;
}

struct Obj421ce0 {
    struct Words3 field_0;
    struct Words3 field_c;
    struct Words3 field_18;
    unsigned int field_24;
    unsigned int field_28;
    unsigned char pad_2c[0x18];
    unsigned int field_44;
    float field_48;
    void **field_4c;
    struct SegBlob *field_50; /* next */
    struct SegBlob *field_54; /* prev */
};

// FUNCTION: LEGOLAND 0x00421ce0
void FUN_00421ce0(struct Words3 *a, struct Words3 *b, struct Words3 *c, struct Obj421ce0 *obj, unsigned int d, unsigned int e) {
    obj->field_0 = *a;
    obj->field_c = *b;
    obj->field_18 = *c;
    obj->field_4c = &CurveFunctionTable[16];
    obj->field_24 = d;
    obj->field_28 = e;
    obj->field_44 = 0;
    obj->field_48 = 1.5707950592041016f;
    obj->field_50 = NULL;
    obj->field_54 = NULL;
}

struct Struct1d60 {
    float field_0;
    float field_4;
    unsigned char pad_8[4];
    float field_c;
    float field_10;
    unsigned char pad_14[16];
    float field_24;
    float field_28;
    float field_2c;
    float field_30;
};

struct Floats3 {
    float field_0;
    float field_4;
    float field_8;
};

// FUNCTION: LEGOLAND 0x00421d60
void FUN_00421d60(struct Struct1d60 *a, float b, struct Floats3 *out) {
    out->field_0 = a->field_0 + b * a->field_c;
    out->field_4 = a->field_4 + b * a->field_10;
    out->field_8 = a->field_30 + b * (a->field_2c + b * (a->field_28 + b * a->field_24));
}

// FUNCTION: LEGOLAND 0x00421da0
void FUN_00421da0(unsigned char *param_1, float t, struct Words3 *out) {
    float *f = (float *)param_1;
    float v;
    *out = *(struct Words3 *)(param_1 + 12);
    v = t * f[9];
    v = v * FLOAT_004ab43c;
    v = v + (f[10] + f[10]);
    *(float *)&out->field_8 = v * t + f[11];
}

// FUNCTION: LEGOLAND 0x00421df0
void FUN_00421df0(float *a, float x, float *out) {
    out[0] = x * a[3] + a[6] + a[0];
    out[1] = x * a[4] + a[7] + a[1];
    out[2] = ((x * a[9] + a[10]) * x + a[11]) * x + a[12];
}

struct Struct1e40 {
    float field_0;
    float field_4;
    float pad_8;
    float field_c;
    float field_10;
    float pad_14;
    float field_18;
    float field_1c;
    float pad_20;
    float coef_t3;
    float coef_t2;
    float coef_t1;
    float coef_t0;
    unsigned char pad_34[0x44 - 0x34];
    float t_start;
    float t_end;
};

// FUNCTION: LEGOLAND 0x00421e40
void FUN_00421e40(struct Struct1e40 *ptr1, float multiplier, struct Floats3 *ptr2) {
    float temp;

    ptr2->field_0 = multiplier * ptr1->field_c + ptr1->field_0 - ptr1->field_18;
    ptr2->field_4 = multiplier * ptr1->field_10 + ptr1->field_4 - ptr1->field_1c;

    temp = multiplier * ptr1->coef_t3 + ptr1->coef_t2;
    temp = temp * multiplier + ptr1->coef_t1;
    temp = temp * multiplier + ptr1->coef_t0;
    ptr2->field_8 = temp;
}

// FUNCTION: LEGOLAND 0x00421e90
void SubdivideCurveAtMaxError(float x0, float y0, float x1, float y1) {
    float roots[2];
    float b;
    float by;
    float bx;
    float m;
    float best = 1.17549435e-38f;
    struct Struct1e40 *o = DAT_004dd648;
    float *r = roots;
    int i = 2;

    m = (y1 - y0) / (x1 - x0);
    b = y0 - m * x0;
    {
        float a;
        volatile float b2;
        volatile float sq;
        float c;
        a = o->coef_t3;
        a = a * FLOAT_004ab43c;
        b2 = o->coef_t2 + o->coef_t2;
        c = o->coef_t1 - m;
        sq = (float)sqrt(b2 * b2 - c * a * 4.0f);
        roots[0] = (sq - b2) / (a + a);
        roots[1] = (-b2 - sq) / (a + a);
    }
    for (; i > 0; i--, r++) {
        float y;
        float d;
        if (*r >= x0 && *r <= x1) {
            y = o->coef_t3;
            y = y * *r + o->coef_t2;
            y = y * *r + o->coef_t1;
            y = y * *r + o->coef_t0;
            d = (float)fabs(y - m * *r - b);
            if (d > best) {
                bx = *r;
                by = y;
                best = d;
            }
        }
    }
    if (fabs(best) > 1.0) {
        *DAT_004dd64c++ = bx;
        DAT_004dd650++;
        SubdivideCurveAtMaxError(x0, y0, bx, by);
        SubdivideCurveAtMaxError(bx, by, x1, y1);
    }
}

// FUNCTION: LEGOLAND 0x00422000
int BuildCurveSplitPoints(struct Struct1e40 *o, float *out) {
    int i;
    int j;
    float t;

    DAT_004dd644 = o;
    DAT_004dd648 = o;
    DAT_004dd650 = 2;
    DAT_004dd64c = out;
    *DAT_004dd64c = o->t_start;
    DAT_004dd64c++;
    *DAT_004dd64c = o->t_end;
    DAT_004dd64c++;
    SubdivideCurveAtMaxError(o->t_start, ((o->t_start * o->coef_t3 + o->coef_t2) * o->t_start + o->coef_t1) * o->t_start + o->coef_t0, o->t_end, ((o->t_end * o->coef_t3 + o->coef_t2) * o->t_end + o->coef_t1) * o->t_end + o->coef_t0);
    for (i = DAT_004dd650 - 1; i >= 0; i--) {
        for (j = 0; j < i; j++) {
            if (out[j] > out[j + 1]) {
                t = out[j];
                out[j] = out[j + 1];
                out[j + 1] = t;
            }
        }
    }
    return DAT_004dd650;
}

struct V3 {
    float x, y, z;
};

// FUNCTION: LEGOLAND 0x004220e0
void FUN_004220e0(float *obj, float t, float *out) {
    float c, v;

    *(struct V3 *)out = *(struct V3 *)(obj + 3);
    v = t * obj[9];
    out[2] = (v * FLOAT_004ab43c + obj[10] * 2) * t + obj[11];
    FUN_00425d50(out);
    if (fabs(out[1]) < DAT_004ab418) {
        c = out[2];
        if (*(unsigned int *)&out[0] & 0x80000000) {
            out[2] = -out[0];
            out[0] = c;
        } else {
            out[2] = out[0];
            out[0] = -c;
        }
    } else {
        c = out[2];
        if (*(unsigned int *)&out[1] & 0x80000000) {
            out[2] = -out[1];
            out[1] = c;
        } else {
            out[2] = out[1];
            out[1] = -c;
        }
    }
}

// FUNCTION: LEGOLAND 0x00422180
void FUN_00422180(float *a1, float *a2, struct Words3 *a3, struct Obj421ab0 *a4) {
    float q[4];
    float sum;
    float *p;
    float *out;
    int j;

    q[0] = a2[2];
    q[1] = a1[2];
    q[2] = 0.0f;
    q[3] = 0.0f;
    a2[2] = 0.0f;
    a1[2] = 0.0f;
    InitLineCurve(a4, a1, a2, a3);
    p = &DAT_004b5abc[0][0];
    out = a4->field_24;
    do {
        sum = FLOAT_004ab390;
        for (j = 0; j < 4; j++) {
            sum += q[j] * *p++;
        }
        *out++ = sum;
    } while ((INT_PTR)p <= (INT_PTR)&DAT_004b5abc[3][0]);
    a4->field_4c = &CurveFunctionTable[0];
}

// FUNCTION: LEGOLAND 0x00422210
void InitCurveFunctionTable(void) {
    CurveFunctionTable[0] = FUN_00421df0;
    CurveFunctionTable[1] = FUN_00421da0;
    CurveFunctionTable[3] = FUN_00421da0;
    CurveFunctionTable[5] = FUN_00421da0;
    CurveFunctionTable[9] = FUN_004219f0;
    CurveFunctionTable[11] = FUN_004219f0;
    CurveFunctionTable[13] = FUN_004219f0;
    CurveFunctionTable[2] = FUN_00421d60;
    CurveFunctionTable[4] = FUN_00421e40;
    CurveFunctionTable[6] = BuildCurveSplitPoints;
    CurveFunctionTable[7] = FUN_004220e0;
    CurveFunctionTable[8] = FUN_00421a10;
    CurveFunctionTable[10] = FUN_004219c0;
    CurveFunctionTable[12] = FUN_00421a40;
    CurveFunctionTable[14] = FUN_00421a70;
    CurveFunctionTable[15] = FUN_00421a90;
    CurveFunctionTable[16] = FUN_00421be0;
    CurveFunctionTable[17] = FUN_00421b90;
    CurveFunctionTable[18] = FUN_00421b40;
    CurveFunctionTable[19] = FUN_00421b90;
    CurveFunctionTable[20] = FUN_00421c30;
    CurveFunctionTable[21] = FUN_00421b90;
    CurveFunctionTable[22] = FUN_00421c80;
    CurveFunctionTable[23] = FUN_00421cc0;
}

// FUNCTION: LEGOLAND 0x004222f0
int IsCrLf(unsigned short *param_1) {
    return param_1[0] == 0xa0d;
}

// FUNCTION: LEGOLAND 0x00422300
unsigned char *CopyTextLine(unsigned char *src, unsigned char *dst) {
    if (IsCrLf((unsigned short *)src) == 0) {
        do {
            *dst = *src;
            dst++;
            src++;
        } while (IsCrLf((unsigned short *)src) == 0);
        return dst;
    }
    return dst;
}

// FUNCTION: LEGOLAND 0x00422340
unsigned char *FindLineStart(struct CoasterTextFile *range, int count) {
    int n = 0;
    unsigned char *p = range->data;
    unsigned char *end = p + range->size;

    if (count == 0) {
        return p;
    }
    while (p < end - 1) {
        if (IsCrLf((unsigned short *)p) != 0) {
            n++;
            p += 2;
            if (n == count) {
                return p;
            }
        } else {
            p++;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00422390
unsigned int GetTextLine(struct CoasterTextFile *a1, char *a2, unsigned int a3) {
    unsigned char *result1 = FindLineStart(a1, a3);
    if (result1 == 0)
        return 0;
    {
        unsigned char *p = CopyTextLine(result1, (unsigned char *)a2);
        p[0] = 0;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x004223c0
int CountTextLines(struct CoasterTextFile *range) {
    unsigned char *p = range->data;
    unsigned char *end = p + range->size;
    int n = 0;

    while (p < end - 1) {
        if (IsCrLf((unsigned short *)p) != 0) {
            n++;
            p += 2;
        } else {
            p++;
        }
    }
    return n;
}

// FUNCTION: LEGOLAND 0x00422400
int FindTextLine(struct CoasterTextFile *param_1, const char *param_2) {
    char buf[0x100];
    int i = 0;

    while (GetTextLine(param_1, buf, i) != 0) {
        if (_strcmpi(param_2, buf) == 0) {
            return i;
        }
        i++;
    }
    return -1;
}

// FUNCTION: LEGOLAND 0x00422470
void *ReadFileToBuffer(const char *fileName, unsigned int *bytesReadPtr) {
    HANDLE hFile;
    unsigned int fileSize;
    void *buffer;
    DWORD bytesRead;

    if (fileName == 0) {
        return 0;
    }

    hFile = CreateFileA(fileName, 0x80000000, 0x1, 0, 0x3, 0x8000000, 0);
    if (hFile == (HANDLE)-1) {
        return 0;
    }

    fileSize = GetFileSize(hFile, 0);
    buffer = MemAlloc(fileSize, 0, (unsigned int)(UINT_PTR)DAT_004d8bb0, 0);
    if (buffer == 0) {
        CloseHandle(hFile);
        return 0;
    }

    ReadFile(hFile, buffer, fileSize, &bytesRead, 0);
    if (bytesRead != fileSize) {
        MemFree(buffer);
        CloseHandle(hFile);
        return 0;
    }

    CloseHandle(hFile);
    *bytesReadPtr = fileSize;
    return buffer;
}

// FUNCTION: LEGOLAND 0x00422590
unsigned int FindCoasterObjLine(const char *s) {
    return FindTextLine(&CoasterObjFile, s);
}

// FUNCTION: LEGOLAND 0x004225b0
void GetCoasterObjLine(unsigned int param_1, char *param_2) {
    GetTextLine(&CoasterObjFile, param_2, param_1);
}

// FUNCTION: LEGOLAND 0x004225d0
unsigned int FUN_004225d0(void) {
    return DAT_004dd868;
}

// FUNCTION: LEGOLAND 0x00422600
unsigned int FindCoasterTxtLine(const char *param) {
    return FindTextLine(&CoasterTxtFile, param);
}

// FUNCTION: LEGOLAND 0x00422620
void FormatRollercoasterFileName(int value, char *buffer) {
    // STRING: LEGOLAND 0x004b5afc
    wsprintfA(buffer, "%s%04d", DAT_004dd760, value);
}

// FUNCTION: LEGOLAND 0x00422640
unsigned int FUN_00422640(void) {
    return DAT_004dd86c;
}

// FUNCTION: LEGOLAND 0x004226c0
int LoadObjAndTxtFiles(const char *name) {
    char buffer[256];

    // STRING: LEGOLAND 0x004b5b04
    wsprintfA(buffer, "%s.obj", name);
    CoasterObjFile.data = ReadFileToBuffer(buffer, &CoasterObjFile.size);
    if (CoasterObjFile.data == 0) {
        return 0;
    }
    // STRING: LEGOLAND 0x004b5b0c
    wsprintfA(buffer, "%s.txt", name);
    CoasterTxtFile.data = ReadFileToBuffer(buffer, &CoasterTxtFile.size);
    if (CoasterTxtFile.data == 0) {
        MemFree(CoasterObjFile.data);
        return 0;
    }
    DAT_004dd868 = CountTextLines(&CoasterObjFile);
    DAT_004dd86c = CountTextLines(&CoasterTxtFile);
    strcpy(DAT_004dd760, name);
    return 1;
}

// FUNCTION: LEGOLAND 0x004227a0
void FreeObjAndTxtFiles(void) {
    MemFree(CoasterTxtFile.data);
    MemFree(CoasterObjFile.data);
}

struct MVert {
    int x;
    int y;
    int rest[3];
};
struct MEdge {
    int v0;
    int v1;
};
struct MTri {
    int e[3];
};
struct EdgeMesh {
    int f0;
    int nVerts;
    int nEdges;
    int nTris;
    struct MVert *verts;
    struct MEdge *edges;
    struct MTri *tris;
};

// FUNCTION: LEGOLAND 0x004227c0
struct EdgeMesh *EdgeMeshRemoveBackfaces(struct EdgeMesh *src) {
    int *triFlag;
    volatile int ptmp45;
    int nV = 0;
    int *edgeFlag;
    int j;
    int i;
    int removedTris = 0;
    int *vertFlag;
    int k;
    struct EdgeMesh *out = NULL;
    int nEdges = src->nEdges;
    int removedEdges = 0;
    int removedVerts = 0;

    for (i = 0; i < src->nTris; i++) {
        for (k = 0; k < 3; k++) {
            struct MEdge *ed = &src->edges[src->tris[i].e[k] & 0x7fffffff];
            if (ed->v0 > nV) {
                nV = ed->v0;
            }
            if (ed->v1 > nV) {
                nV = ed->v1;
            }
        }
    }
    nV++;
    triFlag = (int *)malloc(src->nTris * 4);
    edgeFlag = (int *)malloc(nEdges * 4);
    vertFlag = (int *)malloc(nV * 4);
    if (triFlag != NULL) {
        if (edgeFlag != NULL && vertFlag != NULL) {
            memset(triFlag, 0, src->nTris * 4);
            memset(edgeFlag, 0, nEdges * 4);
            memset(vertFlag, 0, nV * 4);
            for (i = 0; i < src->nTris; i++) {
                int v[3];
                float z;
                struct MVert *p0;
                struct MVert *p1;
                struct MVert *p2;
                double dx1;
                double dy1;
                double dx2;
                double dy2;
                unsigned int bits;
                for (k = 0; k < 3; k++) {
                    int e = src->tris[i].e[k];
                    if (e & 0x80000000) {
                        v[k] = src->edges[e & 0x7fffffff].v1;
                    } else {
                        ptmp45 = src->edges[e & 0x7fffffff].v0;
                        v[k] = ptmp45;
                    }
                }
                p0 = &src->verts[v[0]];
                p1 = &src->verts[v[1]];
                dx1 = p1->x - p0->x;
                dy1 = p1->y - p0->y;
                p2 = &src->verts[v[2]];
                dx2 = p2->x - p0->x;
                dy2 = p2->y - p0->y;
                z = dy2 * dx1 - dx2 * dy1;
                bits = *(unsigned int *)&z;
                if (bits & 0x80000000) {
                    triFlag[i] = 1;
                    removedTris++;
                }
            }
            for (i = 0; i < src->nTris; i++) {
                if (triFlag[i] != 0) {
                    for (k = 0; k < 3; k++) {
                        int found = 0;
                        for (j = 0; j < src->nTris; j++) {
                            if (triFlag[j] == 0) {
                                if (((src->tris[j].e[0] ^ ((int *)src->tris)[i * 3 + k]) & 0x7fffffff) == 0 || ((src->tris[j].e[1] ^ ((int *)src->tris)[i * 3 + k]) & 0x7fffffff) == 0 || ((src->tris[j].e[2] ^ ((int *)src->tris)[i * 3 + k]) & 0x7fffffff) == 0) {
                                    found = 1;
                                    break;
                                }
                            }
                        }
                        if (!found) {
                            edgeFlag[((int *)src->tris)[i * 3 + k] & 0x7fffffff] = 1;
                        }
                    }
                }
            }
            i = 0;
            while (i < nEdges) {
                if (edgeFlag[i] != 0) {
                    int *pv = &src->edges[i].v0;
                    for (k = 0; k < 2; k++) {
                        int found = 0;
                        for (j = 0; j < nEdges; j++) {
                            if (edgeFlag[j] == 0) {
                                if (*pv == src->edges[j].v0 || *pv == src->edges[j].v1) {
                                    found = 1;
                                    break;
                                }
                            }
                        }
                        if (!found) {
                            vertFlag[*pv] = 1;
                        }
                        pv++;
                    }
                }
                i++;
            }
            for (i = 0; i < nEdges; i++) {
                if (edgeFlag[i] != 0) {
                    removedEdges++;
                }
            }
            for (i = 0; i < nV; i++) {
                if (vertFlag[i] != 0) {
                    removedVerts++;
                }
            }
            out = (struct EdgeMesh *)malloc((nEdges - removedEdges) * 8 + (src->nTris - removedTris) * 12 + (nV - removedVerts) * 20 + 0x1c);
            if (out != NULL) {
                out->nEdges = nEdges - removedEdges - 1;
                out->nTris = src->nTris - removedTris;
                out->nVerts = nV - removedVerts - 1;
                out->edges = (struct MEdge *)(out + 1);
                out->tris = (struct MTri *)(out->edges + (nEdges - removedEdges));
                out->verts = (struct MVert *)(out->tris + (src->nTris - removedTris));
                j = 0;
                for (i = 0; i < nV; i++) {
                    if (vertFlag[i] == 0) {
                        out->verts[j] = src->verts[i];
                        vertFlag[i] = i - j;
                        j++;
                    }
                }
                for (i = 0, j = 0; nEdges > i; i++) {
                    if (edgeFlag[i] == 0) {
                        out->edges[j].v0 = src->edges[i].v0 - vertFlag[src->edges[i].v0];
                        out->edges[j].v1 = src->edges[i].v1 - vertFlag[src->edges[i].v1];
                        edgeFlag[i] = i - j;
                        j++;
                    }
                }
                j = 0;
                for (i = 0; i < src->nTris; i++) {
                    if (triFlag[i] == 0) {
                        int e = src->tris[i].e[0];
                        if (0x80000000 & e) {
                            out->tris[j].e[0] = (e - edgeFlag[e & 0x7fffffff]) | 0x80000000;
                        } else {
                            out->tris[j].e[0] = e - edgeFlag[e];
                        }
                        e = src->tris[i].e[1];
                        if (e & 0x80000000) {
                            out->tris[j].e[1] = (e - edgeFlag[e & 0x7fffffff]) | 0x80000000;
                        } else {
                            out->tris[j].e[1] = e - edgeFlag[e];
                        }
                        e = src->tris[i].e[2];
                        if (e & 0x80000000) {
                            out->tris[j].e[2] = (e - edgeFlag[e & 0x7fffffff]) | 0x80000000;
                        } else {
                            out->tris[j].e[2] = e - edgeFlag[e];
                        }
                        j++;
                    }
                }
            }
        }
        free(triFlag);
    }
    if (edgeFlag != NULL) {
        free(edgeFlag);
    }
    if (vertFlag != NULL) {
        free(vertFlag);
    }
    return out;
}

// FUNCTION: LEGOLAND 0x00422e10
void FUN_00422e10(int param1, int index) {
    char *p = DAT_00829c54 + (index << 7);
    DAT_00829c60[index] = p;
    FUN_00422e40(param1, p);
}

// FUNCTION: LEGOLAND 0x00422e40
void FUN_00422e40(int param1, char *entry) {
    unsigned short *out;
    unsigned int gbits;
    unsigned int rshift;
    float rf, gf, bf;
    float dr, dg, db;
    double ar;
    double ag;
    double ab;
    int i;

    if (DisplayPixelFormat == 2) {
        gbits = 6;
        rshift = 11;
    } else {
        gbits = 5;
        rshift = 10;
    }
    ar = FLOAT_004ab390;
    ag = FLOAT_004ab390;
    ab = FLOAT_004ab390;
    rf = (float)(((unsigned int)param1 >> 19) & 0x1f);
    dr = rf * 0.03125f;
    gf = (float)(((unsigned int)param1 & 0xff00) >> (16 - gbits));
    dg = gf * 0.03125f;
    bf = (float)(((unsigned int)param1 >> 3) & 0x1f);
    db = bf * 0.03125f;
    out = (unsigned short *)entry;
    for (i = 0; i < 33; i++) {
        *out++ = (unsigned short)(((short)(int)ar << rshift) | ((int)ag << 5) | (int)ab);
        ar += dr;
        ag += dg;
        ab += db;
    }
    ar = rf;
    ag = gf;
    ab = bf;
    dr = (float)(FLOAT_004ab444 - ar) * 0.032258064f;
    dg = (float)((float)((1 << gbits) - 1) - ag) * 0.032258064f;
    db = (float)(FLOAT_004ab444 - ab) * 0.032258064f;
    out = (unsigned short *)entry + 32;
    for (i = 0; i < 32; i++) {
        *out++ = (unsigned short)(((short)(int)ar << rshift) | ((int)ag << 5) | (int)ab);
        ar += dr;
        ag += dg;
        ab += db;
    }
}

// FUNCTION: LEGOLAND 0x00422fe0
void FUN_00422fe0(void) {
    int *p = GetRollercoasterLpt();
    int n = *p++;
    int i;

    DAT_0060f904 = n;
    DAT_00829c54 = MemAlloc(n << 7, 0, 0, 0);
    if (DAT_00829c54 == NULL) {
        FUN_00422e40(0xffffff, DAT_00579878);
        for (i = 0; i < 1024; i++) {
            DAT_00829c60[i] = DAT_00579878;
        }
        return;
    }
    for (i = 0; i < n; i++) {
        FUN_00422e10(*p++, i);
    }
}

#pragma pack(push, 2)
struct RecEnt {
    short w;
    int d;
    short s0n;
};

struct RecBuf {
    int n;
    int x;
    short s0_first;
    struct RecEnt ent[1];
};
#pragma pack(pop)

struct RecIdx {
    int v;
    int k;
};

struct RecSrc {
    int f0;
    int flag;
    int fx;
    int f0c;
    int f10;
    int f14;
    int f18;
    int d;
    int f20;
    int f24;
    int f28;
    int f2c;
};

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x00423140
__declspec(naked) void FUN_00423140(int param_1) {
    __asm {
        push ebp
        mov ebp, esp
        push ecx
        push esi
        push edi
        mov edi, offset DAT_004dd870
        push eax
        push edx
        rdtsc
        mov dword ptr [ebp - 4], eax
        pop edx
        pop eax
        mov eax, dword ptr [DAT_0060f90c]
        test eax, eax
        je L423177
        mov ecx, 0x25800
        xor eax, eax
        mov edi, offset DAT_004e3870
        mov dword ptr [DAT_0060f914], 0xa
        rep stosd
        jmp L4231c6
L423177:
        mov esi, dword ptr [DAT_00829a3c + 0x18]
        cmp esi, offset DAT_00829a3c
        je L4231a0
L423185:
        mov eax, dword ptr [esi + 0x10]
        test eax, eax
        je L423195
        push esi
        call FUN_00423480
        add esp, 4
L423195:
        mov esi, dword ptr [esi + 0x18]
        cmp esi, offset DAT_00829a3c
        jne L423185
L4231a0:
        cmp dword ptr [DAT_004b5b3c], offset DAT_004dd870
        je L4231c6
L4231ac:
        mov ecx, dword ptr [edi]
        mov eax, edi
        push eax
        lea edi, [edi + ecx*8 + 8]
        call FUN_004232b0
        mov eax, dword ptr [DAT_004b5b3c]
        add esp, 4
        cmp edi, eax
        jne L4231ac
L4231c6:
        mov dword ptr [DAT_004b5b3c], offset DAT_004dd870
        mov dword ptr [DAT_0060f90c], 0
        mov dword ptr [DAT_0060f908], 0
        push eax
        push edx
        rdtsc
        sub eax, dword ptr [ebp - 4]
        mov dword ptr [ebp - 4], eax
        pop edx
        pop eax
        mov edx, dword ptr [ebp - 4]
        pop edi
        mov dword ptr [DAT_0060f910], edx
        pop esi
        mov esp, ebp
        pop ebp
        ret
    }
}

// FUNCTION: LEGOLAND 0x00423200
void FUN_00423200(int n, int x, struct RecIdx *idx, struct RecSrc *src) {
    short *p = (short *)DAT_004b5b3c;
    struct RecBuf *saved = (struct RecBuf *)p;
    int i;

    DAT_0060f908++;
    if ((char *)p + 0x10 > DAT_004e3870) {
        DAT_0060f90c = 1;
        return;
    }
    if (n) {
        *(int *)p = n;
        saved->x = x;
        i = 0;
        if (n > 0) {
            p = (short *)saved + 5;
            for (; i < n; i++) {
                struct RecSrc *s = &src[idx->k];
                p[-1] = s->fx >> 16;
                *(int *)(p + 1) = s->d;
                p[0] = (short)idx->v;
                idx++;
                if (s->flag == 1) {
                    p[0] = -p[0];
                }
                p += 4;
            }
        }
        DAT_004b5b3c = (struct RecBuf *)((char *)saved + i * 8 + 8);
    }
}

// FUNCTION: LEGOLAND 0x004232b0
void FUN_004232b0(struct RecBuf *rb) {
    /* The edge tables are one block: the fill reads the last line from the high half of src[n - 1].f0, and
     * writes idx[n].v (src[0] when n == 8). */
    struct {
        struct RecIdx idx[8];
        struct RecSrc src[8];
    } poly;
    int n = rb->n;
    int i;

    for (i = 0; i < n; i++) {
        short w = rb->ent[i].w;
        short w2 = rb->ent[i].w;

        if (w < 0) {
            poly.src[i].flag = 1;
            poly.idx[i].v = -w;
        } else {
            poly.src[i].flag = 0;
            poly.idx[i].v = w2;
        }
        poly.idx[i].k = i;
        poly.src[i].fx = rb->ent[i - 1].s0n << 16;
        poly.src[i].d = rb->ent[i].d;
    }
    ((short *)&poly.src[i - 1].f0)[1] = (short)(rb->x - 1);
    FUN_00423350(n, poly.idx, poly.src);
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x00423350
__declspec(naked) void FUN_00423350(int n, struct RecIdx *idx, struct RecSrc *src) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x60
        mov edx, dword ptr [ebp + 8]
        push ebx
        push esi
        mov esi, dword ptr [ebp + 0xc]
        mov ecx, dword ptr [ebp + 0x10]
        push edi
        mov eax, dword ptr [esi]
        mov dword ptr [ebp - 4], 0
        mov dword ptr [ebp + 0xc], eax
        mov eax, dword ptr [esi + edx*8 - 4]
        lea eax, [eax + eax*2]
        shl eax, 4
        inc word ptr [eax + ecx + 2]
        mov edi, dword ptr [DAT_004b5b24]
        lea eax, [eax + ecx + 2]
        mov ebx, dword ptr [CoasterSurfacePitch]
        mov eax, dword ptr [esi + edx*8 - 4]
        mov dword ptr [ebp - 8], ebx
        lea eax, [eax + eax*2]
        shl eax, 4
        movsx eax, word ptr [eax + ecx + 2]
        mov dword ptr [ebp + 8], eax
        mov eax, dword ptr [CoasterSurfaceBits]
        mov dword ptr [ebp - 0x10], eax
        mov eax, dword ptr [DAT_0060f900]
        inc eax
        mov dword ptr [DAT_0060f900], eax
        mov eax, dword ptr [esi + edx*8 - 4]
        lea eax, [eax + eax*2]
        shl eax, 4
        movsx eax, word ptr [eax + ecx + 2]
        mov dword ptr [esi + edx*8], eax
        mov edx, ebx
        mov ebx, dword ptr [ebp + 0xc]
        imul edx, ebx
        lea eax, [edi + edx*2]
        mov dword ptr [ebp - 0xc], eax
L4233d5:
        mov eax, dword ptr [esi + 4]
        add esi, 8
        lea eax, [eax + eax*2]
        shl eax, 4
        add eax, ecx
        mov edx, dword ptr [eax + 4]
        test edx, edx
        mov edx, dword ptr [eax + 8]
        mov eax, dword ptr [eax + 0x1c]
        je L4233fa
        sub edx, eax
        mov dword ptr [ebp - 0x24], eax
        mov dword ptr [ebp - 0x38], edx
        jmp L423402
L4233fa:
        sub edx, eax
        mov dword ptr [ebp - 0x4c], eax
        mov dword ptr [ebp - 0x60], edx
L423402:
        cmp ebx, dword ptr [esi]
        jge L42345f
L423406:
        inc dword ptr [ebp + 0xc]
        mov eax, dword ptr [ebp - 0x60]
        mov ebx, dword ptr [ebp - 0x38]
        add eax, dword ptr [ebp - 0x4c]
        add ebx, dword ptr [ebp - 0x24]
        mov dword ptr [ebp - 0x60], eax
        mov dword ptr [ebp - 0x38], ebx
        mov ecx, ebx
        sub ecx, eax
        cmp ecx, 0x8000
        jns L423429
        jmp L423445
L423429:
        sar eax, 0x10
        sar ebx, 0x10
        mov edi, dword ptr [ebp - 0xc]
        xchg eax, ebx
        mov dx, word ptr [ebp - 4]
        sub ebx, eax
        lea edi, [edi + eax*2]
L42343c:
        mov word ptr [edi + ebx*2], dx
        add ebx, 1
        jle L42343c
L423445:
        mov ecx, dword ptr [ebp - 8]
        mov edx, dword ptr [ebp + 0xc]
        lea eax, [ecx + ecx]
        mov ecx, dword ptr [ebp - 0xc]
        add ecx, eax
        mov eax, dword ptr [esi]
        cmp edx, eax
        mov dword ptr [ebp - 0xc], ecx
        jl L423406
        mov ecx, dword ptr [ebp + 0x10]
L42345f:
        mov ebx, dword ptr [ebp + 0xc]
        mov eax, dword ptr [ebp + 8]
        cmp ebx, eax
        jl L4233d5
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// FUNCTION: LEGOLAND 0x00423480
void FUN_00423480(struct ClearRect *r) {
    unsigned short *row = DAT_004b5b24 + r->top * CoasterSurfacePitch;
    int y;
    int x;
    unsigned short *p;

    for (y = r->top; y <= r->bottom; y++) {
        p = row + r->left;
        for (x = r->left; x <= r->right; x++) {
            *p++ = 0;
        }
        row += CoasterSurfacePitch;
    }
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x004234e0
__declspec(naked) void FUN_004234e0(void *param1) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0xe4
        push ebx
        push esi
        lea eax, [ebp - 0x74]
        push edi
        push eax
        call GetCoasterSurface
        add esp, 4
        push eax
        push edx
        rdtsc
        mov dword ptr [ebp - 8], eax
        pop edx
        pop eax
        lea edx, [ebp - 0xc8]
        lea ecx, [ebp - 0xe4]
        mov dword ptr [ebp - 0x20], edx
        mov edx, dword ptr [ebp + 8]
        lea eax, [ebp - 0xac]
        mov dword ptr [ebp - 0x24], ecx
        mov ecx, dword ptr [edx + 0xc]
        mov dword ptr [ebp - 0x1c], eax
        xor eax, eax
        mov dword ptr [ebp - 0x4c], 1
        cmp ecx, eax
        mov dword ptr [ebp - 0x14], offset DAT_004b5658
        mov dword ptr [ebp - 0x10], eax
        jle L4236ad
        mov dword ptr [ebp - 0xc], eax
L423540:
        mov ecx, dword ptr [ebp - 0xc]
        mov esi, dword ptr [edx + 0x18]
        add ecx, esi
        lea esi, [ebp - 0x5c]
        sub esi, ecx
        mov edi, 3
L423552:
        mov eax, dword ptr [ecx]
        test eax, 0x80000000
        je L423569
        mov ebx, dword ptr [edx + 0x14]
        and eax, 0x7fffffff
        mov eax, dword ptr [ebx + eax*8 + 4]
        jmp L423574
L423569:
        mov ebx, dword ptr [edx + 0x14]
        and eax, 0x7fffffff
        mov eax, dword ptr [ebx + eax*8]
L423574:
        mov dword ptr [esi + ecx], eax
        add ecx, 4
        dec edi
        jne L423552
        mov ecx, dword ptr [ebp - 0x58]
        mov eax, dword ptr [ebp - 0x5c]
        mov esi, dword ptr [edx + 0x10]
        mov dword ptr [ebp - 0x50], eax
        lea ecx, [ecx + ecx*4]
        lea eax, [eax + eax*4]
        mov ebx, dword ptr [esi + eax*4]
        lea edi, [esi + ecx*4]
        lea ecx, [esi + eax*4]
        mov eax, dword ptr [edi]
        sub eax, ebx
        mov dword ptr [ebp - 4], eax
        fild dword ptr [ebp - 4]
        fstp dword ptr [ebp - 0x38]
        mov eax, dword ptr [edi + 4]
        mov ebx, dword ptr [ecx + 4]
        sub eax, ebx
        mov dword ptr [ebp - 4], eax
        mov eax, dword ptr [ebp - 0x54]
        fild dword ptr [ebp - 4]
        lea eax, [eax + eax*4]
        fstp dword ptr [ebp - 0x30]
        lea ebx, [esi + eax*4]
        mov eax, dword ptr [esi + eax*4]
        sub eax, dword ptr [ecx]
        mov dword ptr [ebp - 4], eax
        fild dword ptr [ebp - 4]
        fstp dword ptr [ebp - 0x34]
        mov eax, dword ptr [ebx + 4]
        sub eax, dword ptr [ecx + 4]
        mov dword ptr [ebp - 4], eax
        fild dword ptr [ebp - 4]
        fst dword ptr [ebp - 0x2c]
        fmul dword ptr [ebp - 0x38]
        fld dword ptr [ebp - 0x34]
        fmul dword ptr [ebp - 0x30]
        fsubp st(1), st
        fst dword ptr [ebp - 0x28]
        fcomp dword ptr [FLOAT_004ab390]
        fnstsw ax
        test ah, 0x41
        jne L423692
        mov dword ptr [ebp - 0x44], 0xff
        mov dword ptr [ebp - 0x40], 0
        mov edx, dword ptr [ecx + 0xc]
        mov dword ptr [ebp - 0x40], edx
        mov eax, dword ptr [ecx + 0xc]
        and eax, 0xff
        mov dword ptr [ebp - 0x44], eax
        mov ecx, dword ptr [edi + 0xc]
        or edx, ecx
        mov dword ptr [ebp - 0x40], edx
        mov ecx, dword ptr [edi + 0xc]
        and eax, ecx
        mov dword ptr [ebp - 0x44], eax
        mov ecx, dword ptr [ebx + 0xc]
        or edx, ecx
        mov dword ptr [ebp - 0x40], edx
        mov edi, dword ptr [ebx + 0xc]
        and edx, 0xf
        and eax, edi
        cmp dl, 0xf
        mov dword ptr [ebp - 0x44], eax
        jne L42368f
        lea edx, [ebp - 0x5c]
        lea eax, [ebp - 0xdc]
        mov edi, 3
L423650:
        mov ecx, dword ptr [edx]
        add edx, 4
        add eax, 0x1c
        dec edi
        lea ecx, [ecx + ecx*4]
        mov ebx, dword ptr [esi + ecx*4 + 4]
        lea ecx, [esi + ecx*4]
        mov dword ptr [eax - 0x20], ebx
        mov ebx, dword ptr [ecx]
        mov dword ptr [eax - 0x1c], ebx
        mov ebx, dword ptr [ecx + 0x10]
        mov dword ptr [eax - 0x18], ebx
        mov ecx, dword ptr [ecx + 8]
        mov dword ptr [eax - 0x14], ecx
        jne L423650
        mov edx, dword ptr [ebp + 8]
        lea ecx, [ebp - 0x4c]
        push ecx
        push 3
        mov eax, dword ptr [edx]
        mov dword ptr [ebp - 0x48], eax
        call FUN_0042a2f0
        add esp, 8
L42368f:
        mov edx, dword ptr [ebp + 8]
L423692:
        mov eax, dword ptr [ebp - 0x10]
        mov esi, dword ptr [ebp - 0xc]
        mov ecx, dword ptr [edx + 0xc]
        inc eax
        add esi, 0xc
        cmp eax, ecx
        mov dword ptr [ebp - 0x10], eax
        mov dword ptr [ebp - 0xc], esi
        jl L423540
L4236ad:
        push eax
        push edx
        rdtsc
        sub eax, dword ptr [ebp - 8]
        mov dword ptr [ebp - 8], eax
        pop edx
        pop eax
        mov edx, dword ptr [ebp - 8]
        mov esi, dword ptr [DAT_0060f8fc]
        lea eax, [ebp - 0x74]
        add esi, edx
        push eax
        mov dword ptr [DAT_0060f8fc], esi
        call FUN_00423790
        add esp, 4
        mov eax, 1
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// FUNCTION: LEGOLAND 0x004236f0
unsigned short FPUSetSinglePrecision(void) {
    unsigned int control_word;
    unsigned short replacement;
    unsigned char exception_masks;

    __asm {
        fstcw word ptr control_word
    }
    exception_masks = control_word;
    if ((control_word & 0x300) != 0 || (exception_masks & 0x3f) != 0x3f || (control_word & 0xc00) != 0) {
        __asm {
            mov ax, word ptr control_word
            and ax, 0xfcff
            or ax, 0x3f
            and ax, 0xf3ff
            mov replacement, ax
            fldcw replacement
        }
    }
    return control_word;
}

// FUNCTION: LEGOLAND 0x00423730
void FPURestoreControlWord(unsigned short control_word) {
    __asm {
        fldcw control_word
    }
}

// FUNCTION: LEGOLAND 0x00423740
void FUN_00423740(void) {
    FUN_00422fe0();
    FUN_0041fd30();
}

// FUNCTION: LEGOLAND 0x00423760
int GetCoasterSurface(struct VideoArg *arg) {
    int result = GetVideoSurface(arg);
    if (result == 0) {
        return 0;
    }
    CoasterSurfaceBits = arg->bits;
    CoasterSurfacePitch = arg->pitch >> 1;
    return 1;
}

// FUNCTION: LEGOLAND 0x00423790
void FUN_00423790(void) {
}

// FUNCTION: LEGOLAND 0x004237a0
void DrawEdgeSet(struct EdgeSet *set) {
    int i;

    for (i = 0; i < set->count; i++) {
        DrawDottedLine(&set->base[set->pairs[i].a], &set->base[set->pairs[i].b], -1);
    }
}

// FUNCTION: LEGOLAND 0x004237f0
void DrawDottedLine(void *a, void *b, short c) {
    int *pa = (int *)a;
    int *pb = (int *)b;
    int pts[30][4];
    float x, y, dx, dy;
    int i;

    x = (float)pa[0];
    y = (float)pa[1];
    dx = (float)(pb[0] - pa[0]) * FLOAT_004ab44c;
    dy = (float)(pb[1] - pa[1]) * FLOAT_004ab44c;
    for (i = 0; i < 30; i++) {
        pts[i][0] = (int)x;
        pts[i][1] = (int)y;
        x += dx;
        y += dy;
    }
    PlotClippedPoints(&pts[0][0], 30, c);
}

// FUNCTION: LEGOLAND 0x004238a0
void PlotClippedPoints(int *points, int count, short c) {
    struct VideoArg surf;
    int x;
    int y;

    if (GetSprite(&surf, NULL)) {
        while (count-- > 0) {
            x = points[0];
            if (x > DAT_008299ac[0] && x < DAT_008299ac[2]) {
                y = points[1];
                if (y > DAT_008299ac[1] && y < DAT_008299ac[3]) {
                    ((unsigned short *)surf.bits)[x + (surf.pitch >> 1) * y] = c;
                }
            }
            points += 4;
        }
        ReleaseSprite(&surf);
    }
}

// FUNCTION: LEGOLAND 0x00423930
unsigned int IsCastleCoasterPlaced(void) {
    return CastleCoasterPlaced;
}

// FUNCTION: LEGOLAND 0x00423940
void FUN_00423940(struct PathSeg *arg1) {
    arg1->dir_a = 2;
    arg1->f18 = (float)DAT_004b5b48.f8;
    arg1->dir_b = 8;
    arg1->f24 = (float)DAT_004b5b48.f4;
}

// FUNCTION: LEGOLAND 0x00423970
void FUN_00423970(struct PathSeg *param_1) {
    param_1->f18 = (float)DAT_004b5b48.f8;
    param_1->f24 = (float)DAT_004b5b48.f4;
}

// FUNCTION: LEGOLAND 0x00423990
struct SegBlob *FUN_00423990(void) {
    return DAT_006102f8;
}

// FUNCTION: LEGOLAND 0x004239a0
void FUN_004239a0(void) {
}

// FUNCTION: LEGOLAND 0x004239b0
void GetCastleCoasterDoorA(struct Int16Pair *a, struct Int16Pair *b) {
    unsigned char *p = (unsigned char *)&CastleCoasterRide->footprint;
    b->x = *(unsigned short *)(p + 8) + a->x;
    b->y = *(unsigned short *)(p + 4) + a->y - 2;
}

// FUNCTION: LEGOLAND 0x004239e0
void GetCastleCoasterDoorB(struct Int16Pair *a, struct Int16Pair *b) {
    unsigned char *global_ptr = (unsigned char *)&CastleCoasterRide->footprint;
    b->x = *(unsigned short *)global_ptr + a->x - 2;
    b->y = *(unsigned short *)(global_ptr + 0xc) + a->y;
}

// FUNCTION: LEGOLAND 0x00423a10
void FUN_00423a10(void) {
    struct Int16Pair origin;
    struct Int16Pair q;
    struct Int16Pair p;
    struct Int16Pair r;
    struct Int16Pair pt;
    struct Int16Pair pt1;
    struct FVec3 v1;
    struct FVec3 v2;
    struct FVec3 v3;
    struct FVec3 c;
    float t;
    float step;
    int n;
    int i;
    int cnt = 0;

    origin.x = 0;
    origin.y = 0;
    GetCastleCoasterDoorB(&origin, &q);
    GetCastleCoasterDoorA(&origin, &p);
    r.x = q.x;
    r.y = p.y;
    pt.x = r.x + 1;
    pt.y = q.y + 2;
    pt1.x = r.x + 1;
    pt1.y = r.y + 2;
    MapPosToWorld(&pt, 0.0f, &v1);
    MapPosToWorld(&pt1, 0.0f, &v2);
    c.x = -10.0f;
    c.y = 0.0f;
    c.z = 0.0f;
    InitLineCurve((struct Obj421ab0 *)&DAT_006102f8[0], &v1.x, &v2.x, (struct Words3 *)&c);
    n = (q.y - r.y) >> 1;
    t = DAT_006102f8[0].start;
    pt = q;
    step = (DAT_006102f8[0].end - DAT_006102f8[0].start) / n;
    for (i = 0; i < n; i++) {
        DAT_0060f914[cnt].xy = pt;
        pt.y -= 2;
        DAT_0060f914[cnt].k = 0;
        DAT_0060f914[cnt].a = t;
        t += step;
        DAT_0060f914[cnt].b = t;
        cnt++;
    }
    pt.x = -1;
    pt.y = 0;
    MapPosToWorld(&pt, 0.0f, &v1);
    pt.x = 0;
    pt.y = -1;
    MapPosToWorld(&pt, 0.0f, &v2);
    pt.x = r.x + 2;
    pt.y = r.y + 2;
    MapPosToWorld(&pt, 0.0f, &v3);
    FUN_00421ce0((struct Words3 *)&v1, (struct Words3 *)&v2, (struct Words3 *)&v3, (struct Obj421ce0 *)&DAT_006102f8[1], 0x3fc00000, 0x3f000000);
    DAT_0060f914[cnt].xy = r;
    DAT_0060f914[cnt].k = 1;
    DAT_0060f914[cnt].a = DAT_006102f8[1].start;
    DAT_0060f914[cnt].b = DAT_006102f8[1].end;
    cnt++;
    pt.x = r.x + 2;
    pt.y = r.y + 1;
    pt1.x = p.x + 2;
    pt1.y = p.y + 1;
    MapPosToWorld(&pt, 0.0f, &v1);
    MapPosToWorld(&pt1, 0.0f, &v2);
    c.x = 0.0f;
    c.y = -10.0f;
    c.z = 0.0f;
    InitLineCurve((struct Obj421ab0 *)&DAT_006102f8[2], &v1.x, &v2.x, (struct Words3 *)&c);
    DAT_006102f8[0].prev = 0;
    DAT_006102f8[0].next = &DAT_006102f8[1];
    DAT_006102f8[1].prev = &DAT_006102f8[0];
    DAT_006102f8[1].next = &DAT_006102f8[2];
    DAT_006102f8[2].prev = &DAT_006102f8[1];
    DAT_006102f8[2].next = 0;
    n = (p.x - r.x) >> 1;
    t = DAT_006102f8[2].start;
    step = (DAT_006102f8[2].end - DAT_006102f8[2].start) / n;
    pt = r;
    pt.x = r.x + 2;
    for (i = 0; i < n; i++) {
        DAT_0060f914[cnt].xy = pt;
        pt.x += 2;
        DAT_0060f914[cnt].k = 2;
        DAT_0060f914[cnt].a = t;
        t += step;
        DAT_0060f914[cnt].b = t;
        cnt++;
    }
    for (i = 0; i < cnt; i++) {
        DAT_0060f914[i + 1].fp.v[0] = DAT_0060f914[i].xy.x;
        DAT_0060f914[i + 1].fp.v[1] = DAT_0060f914[i].xy.y;
        DAT_0060f914[i + 1].fp.v[2] = DAT_0060f914[i].xy.x + 1;
        DAT_0060f914[i + 1].fp.v[3] = DAT_0060f914[i].xy.y + 1;
        DAT_0060f914[i + 1].fp.next = &DAT_0060f914[i + 2].fp;
    }
    DAT_00610a08 = cnt;
    DAT_0060f914[i].fp.next = 0;
}

// FUNCTION: LEGOLAND 0x00423d40
void InitCastleCoasterDoors(void) {
    struct Int16Pair a;
    struct Int16Pair b;
    struct Int16Pair c;

    a.x = 0;
    a.y = 0;
    GetCastleCoasterDoorA(&a, &b);
    GetCastleCoasterDoorB(&a, &c);
    DAT_004b5b48.door_a.x = b.x;
    DAT_004b5b48.door_a.y = b.y;
    DAT_004b5b48.door_b.x = c.x;
    DAT_004b5b48.door_b.y = c.y;
    FUN_00423a10();
}

// FUNCTION: LEGOLAND 0x00423db0
void InitCastleCoaster(void) {
    CastleCoaster.method_10c = FUN_00424850;
    CastleCoaster.method_110 = FUN_00424890;
    CastleCoaster.method_114 = FUN_00424990;
    InitCastleCoasterDoors();
}

// FUNCTION: LEGOLAND 0x00423de0
void FUN_00423de0(void) {
    struct Ride *v0 = CastleCoasterRide;
    memcpy(&DAT_00829a80, &v0->footprint, sizeof(struct Footprint));
    DAT_00829a80.next = &DAT_0060f914[1].fp;
    DAT_0060f914[DAT_00610a08].fp.next = 0;
}

// FUNCTION: LEGOLAND 0x00423e20
void FUN_00423e20(void) {
    struct PathSeg *n;
    struct Footprint *tail;

    DAT_00829a80 = CastleCoasterRide->footprint;
    DAT_00829a80.next = &DAT_0060f914[1].fp;
    DAT_0060f914[DAT_00610a08].fp.next = 0;
    tail = &DAT_0060f914[DAT_00610a08].fp;
    if (CastleCoaster.flags == 2) {
        for (n = (struct PathSeg *)CastleCoaster.piece.next; n != (struct PathSeg *)&CastleCoaster.piece.flags; n = n->next) {
            tail->next = &n->fp;
            tail = &n->fp;
        }
        tail->next = 0;
        return;
    }
    for (n = (struct PathSeg *)CastleCoaster.piece.next; n != 0; n = n->next) {
        tail->next = &n->fp;
        tail = &n->fp;
    }
    for (n = (struct PathSeg *)CastleCoaster.piece.prev; n != 0; n = n->prev) {
        tail->next = &n->fp;
        tail = &n->fp;
    }
    tail->next = 0;
}

// FUNCTION: LEGOLAND 0x00423ec0
void RemoveAllCoasterPieces(struct CastleState *castle) {
    struct PathSeg *n;
    struct PathSeg *next;
    struct PathSeg *end;

    if (CastleCoaster.flags == 2) {
        n = (struct PathSeg *)CastleCoaster.piece.next;
        while (n != (struct PathSeg *)&CastleCoaster.piece.flags) {
            next = n->next;
            RemoveCoasterPiece(n);
            n = next;
        }
        return;
    }
    n = (struct PathSeg *)CastleCoaster.f_c0;
    end = (struct PathSeg *)&castle->piece;
    while (n != end) {
        next = n->next;
        RemoveCoasterPiece(n);
        n = next;
    }
    n = (struct PathSeg *)CastleCoaster.f_a8;
    while (n != end) {
        next = n->prev;
        RemoveCoasterPiece(n);
        n = next;
    }
}

// FUNCTION: LEGOLAND 0x00423f40
int FUN_00423f40(struct Point *pos, float *a2, struct Point *a3, float *a4, struct PathSeg *node, int *a6) {
    struct FVec3 a;
    struct FVec3 b;
    struct SegBlob *c;
    struct PathSeg *n2;

    *a6 = node->prev == (struct PathSeg *)&CastleCoaster.piece.flags;
    c = FUN_0041cff0(node, &a);
    c->vt->method_8(c, (c->end + c->start) * 0.5f, &b.x);
    b.z += a.z;
    *a2 = b.z * -2.0f;
    n2 = node->next;
    if (n2 != 0 && n2 != (struct PathSeg *)&CastleCoaster.piece.flags) {
        c = FUN_0041cff0(n2, &a);
        c->vt->method_8(c, (c->end + c->start) * 0.5f, &b.x);
        b.z += a.z;
        *a4 = b.z * -2.0f;
        a3->x = n2->x;
        a3->y = n2->y;
        return 1;
    }
    *a3 = *pos;
    if (node->next == (struct PathSeg *)&CastleCoaster.piece.flags) {
        a3->y -= 16;
    }
    *a4 = 0.0f;
    return 1;
}

// FUNCTION: LEGOLAND 0x00424050
int FUN_00424050(struct Point *pos, void *a2, void *a3, void *a4, void *a5) {
    struct PathSeg *n;
    struct PathSeg *next;

    if (CastleCoaster.flags == 2) {
        n = (struct PathSeg *)CastleCoaster.piece.next;
        while (n != (struct PathSeg *)&CastleCoaster.piece.flags) {
            next = n->next;
            if (n->x == pos->x && n->y == pos->y) {
                return FUN_00423f40(pos, a2, a3, a4, n, a5);
            }
            n = next;
        }
        return 0;
    }
    n = (struct PathSeg *)CastleCoaster.f_c0;
    while (n != (struct PathSeg *)&CastleCoaster.piece.flags) {
        next = n->next;
        if (n->x == pos->x && n->y == pos->y) {
            return FUN_00423f40(pos, a2, a3, a4, n, a5);
        }
        n = next;
    }
    n = (struct PathSeg *)CastleCoaster.f_a8;
    while (n != (struct PathSeg *)&CastleCoaster.piece.flags) {
        next = n->prev;
        if (n->x == pos->x && n->y == pos->y) {
            return FUN_00423f40(pos, a2, a3, a4, n, a5);
        }
        n = next;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00424140
void *GetPlacedCastleCoaster(void) {
    return CastleCoasterPlaced ? &CastleCoaster : 0;
}

// FUNCTION: LEGOLAND 0x00424150
void CastleObjLoad(struct Element *param_1) {
    CastleCoasterPlaced = 0;
    FUN_00477400();
    DAT_00829abc = param_1;
    CastleCoasterRide = param_1->ride;
    if (CastleCoasterRide->layer != NULL) {
        CastleCoasterRide->layer->flags |= 0x2000;
    }
    CastleCoasterRide->flags |= 0x20;
    CastleMatteSprite = LoadSprite("Castle Matte.lls", 1);
    FUN_00425a50();
    InitVecMathOps();
    LoadRollercoasterFiles();
    FUN_00423740();
    InitCurveFunctionTable();
    FUN_00428b70();
    FUN_0041ef00();
    FUN_0042a2e0();
    InitCastleCoaster();
    FUN_0041e620();
    LoadCoasterTrainCarModels();
    LoadCoasterTrainWheelModel();
}

// FUNCTION: LEGOLAND 0x004241e0
void CastleObjUnload(void) {
    KillSprite(CastleMatteSprite);
    CastleCoaster.pts_a.mask = 0;
    CastleCoaster.pts_b.mask = 0;
    ClearDirPoints(&CastleCoaster.pts_a);
    ClearDirPoints(&CastleCoaster.pts_b);
    CastleCoaster.f_a8 = 0;
    CastleCoaster.f_c0 = 0;
    ResetAnim((struct PieceEnd *)&CastleCoaster.piece.end_a);
    ResetAnim((struct PieceEnd *)&CastleCoaster.piece.end_b);
    CastleCoaster.flags = 0;
}

// FUNCTION: LEGOLAND 0x00424240
unsigned int FUN_00424240(void) {
    if (CastleCoasterPlaced != 0) {
        return CastleCoasterPlaced;
    }
    EditMode.unk0 = 1;
    EditMode.unk8 = CastleCoasterRide;
    DefaultCursor(&EditCursor);
    FUN_00423de0();
    return ((unsigned int (*)(const char *))SetEditCursorFootPrint)((const char *)&DAT_00829a80); /* TODO: fold — SetEditCursorFootPrint leaves memcpy's eax, used here as uint return */
}

// FUNCTION: LEGOLAND 0x00424280
void FUN_00424280(struct Element *param_1, int *param_2, unsigned int param_3) {
    struct Cursor *cursor;

    FUN_00423de0();
    SetEditCursorFootPrint(&DAT_00829a80);
    ScreenToMapRef((int *)param_2, &EditCursor.tile_x, param_3);
    EditCursor.field_1830 = 0;
    cursor = GetOrAddPathCursor(&EditCursor);
    if (cursor != NULL) {
        cursor->field_1414[1]++;
        cursor->field_1414[0]++;
    }
    ResetCursorBuildable(&EditCursor);
    ValidateCursor(&EditCursor, param_1->data);
    if (CastleCoasterPlaced != 0) {
        SetCursorBlocked(&EditCursor, 0xf);
        PropagateCursorBlocked(&EditCursor);
    }
}

// FUNCTION: LEGOLAND 0x00424320
void PlaceCastleCoaster(struct Element *obj, short *pt) {
    short v[2];

    SetCoasterPieceCount(0);
    AddMapObjectIfEnabled(obj, (int *)pt);
    v[0] = pt[0];
    v[1] = pt[2];
    FUN_004245b0(v);
    InitPieceHead(&CastleCoaster.piece);
    CastleCoaster.flags = 1;
    CastleCoaster.piece.x = pt[0];
    CastleCoaster.piece.y = pt[2];
    CastleCoaster.piece.cls = &DAT_004b5b48;
    CastleCoaster.piece.owner = &CastleCoaster;
    CastleCoaster.piece.ride = CastleCoasterRide;
    CastleCoaster.f_a8 = (struct PathSeg *)&CastleCoaster.piece;
    CastleCoaster.f_c0 = (struct PathSeg *)&CastleCoaster.piece;
    CastleCoaster.pts_a.mask = 0;
    CastleCoaster.pts_b.mask = 0;
    FindPathSegDoorAPoints((struct PathSeg *)&CastleCoaster.piece, &CastleCoaster.pts_a);
    FindPathSegDoorBPoints((struct PathSeg *)&CastleCoaster.piece, &CastleCoaster.pts_b);
    FUN_0041d1c0(&CastleCoaster.pts_a.mask);
    FUN_0041d1c0(&CastleCoaster.pts_b.mask);
    FUN_0041cfc0((struct PathSeg *)&CastleCoaster.piece.flags);
    FUN_0041cfd0((struct PathSeg *)&CastleCoaster.piece, 0);
    CreateCastleTrain((struct CastleState *)&CastleCoaster);
    InitCastleVisitorList((struct CastleState *)&CastleCoaster);
    CastleCoaster.piece.flags |= 6;
    CastleCoasterPlaced = 1;
}

// FUNCTION: LEGOLAND 0x00424440
void FUN_00424440(struct Element *param_1, struct Point *param_2) {
    int x;
    int y;

    FUN_00423e20();
    x = CastleCoaster.piece.x;
    memcpy(QueryCursor.field_1414, &DAT_00829a80, sizeof(struct Footprint));
    y = CastleCoaster.piece.y;
    QueryCursor.tile_x = x;
    QueryCursor.tile_y = y;
    // STRING: LEGOLAND 0x004b5b7c
    if ((void *)QueryClass != ElemID("CASTLE_DUMMY")->data) {
        ResetCursorBuildable(&QueryCursor);
        return;
    }
    QueryCursor.field_140c = 0;
}

// FUNCTION: LEGOLAND 0x004244b0
void RemoveCastleCoaster(struct Element *param_1, TileId param_2, struct Cursor *param_3) {
    struct Cursor cursor;
    TileId tile;
    struct Element *handle;

    if (CastleCoasterPlaced != 0) {
        tile.pos.x = CastleCoaster.piece.x;
        tile.pos.y = CastleCoaster.piece.y;
        cursor.tile_x = tile.pos.x;
        cursor.tile_y = tile.pos.y;
        cursor.footprint = CastleCoasterRide->footprint;
        cursor.field_1414[4] = 0;
        StopCastleTrain((struct CastleState *)&CastleCoaster);
        RemoveAllCoasterPieces(&CastleCoaster);
        ClearDirPoints(&CastleCoaster.pts_a);
        ClearDirPoints(&CastleCoaster.pts_b);
        CastleCoaster.f_a8 = 0;
        CastleCoaster.f_c0 = 0;
        DestroyCastleTrain((struct CastleState *)&CastleCoaster);
        FreeAllCastleVisitors((struct CastleState *)&CastleCoaster);
        CastleRemoveAllRiders();
        handle = GetCastleDispatchElement(0);
        RemoveMapObjectIfEnabled(handle, tile, &cursor);
        FUN_00424620(&CastleCoaster.piece.pos);
        CastleCoasterPlaced = 0;
        FUN_004775f0();
        FreeMoviePool();
        SetCoasterPieceCount(0);
    }
}

// FUNCTION: LEGOLAND 0x004245b0
void FUN_004245b0(short *param_1) {
    int pt[2];
    int i;

    for (i = 0; i < (int)DAT_00610a08; i++) {
        short *q = &DAT_0060f914[i].xy.y;
        pt[0] = q[-1] + param_1[0];
        pt[1] = param_1[1] + q[0];
        AddMapObjectIfEnabled(DAT_00829c00, pt);
    }
    AddCoasterPieceCount(DAT_00610a08);
}

// FUNCTION: LEGOLAND 0x00424620
void FUN_00424620(struct Int16Pair *pos) {
    struct Cursor cursor;
    TileId tile;
    int i;

    for (i = 0; i < (int)DAT_00610a08; i++) {
        tile.pos.x = DAT_0060f914[i].xy.x + pos->x;
        tile.pos.y = DAT_0060f914[i].xy.y + pos->y;
        cursor.tile_x = tile.pos.x;
        cursor.tile_y = tile.pos.y;
        cursor.footprint = DAT_00829c34->footprint;
        cursor.field_1414[4] = 0;
        RemoveMapObjectIfEnabled(DAT_00829c00, tile, &cursor);
    }
    AddCoasterPieceCount(-(int)DAT_00610a08);
}

// FUNCTION: LEGOLAND 0x004246e0
unsigned int FUN_004246e0(struct Element *param_1, unsigned int param_2) {
    if (param_2 != 0 && CastleCoaster.flags != 2) {
        return 0;
    }
    return GetCoasterPieceCount();
}

// FUNCTION: LEGOLAND 0x00424700
void RenderCastleDummy(struct Element *a1, unsigned int a2, unsigned int a3, unsigned char *p) {
    struct FVec3 buf;
    struct SegBlob obj;
    short key[2];
    int want;
    int i;

    MapPosToWorld(&CastleCoaster.piece.pos, (float)DAT_004b5b48.f8, &buf);
    key[0] = p[0];
    key[1] = p[1];
    want = *(int *)key;
    for (i = 0; i < (int)DAT_00610a08; i++) {
        key[0] = DAT_0060f914[i].xy.x + CastleCoaster.piece.x;
        key[1] = DAT_0060f914[i].xy.y + CastleCoaster.piece.y;
        if (want == *(int *)key) {
            obj = ((struct SegBlob *)&DAT_006102f8)[DAT_0060f914[i].k];
            obj.start = DAT_0060f914[i].a;
            obj.end = DAT_0060f914[i].b;
            FUN_004294f0(&obj, (struct FVec3 *)&buf, 1, 0);
            break;
        }
    }
    DrawCastleTrainOnPiece((struct PieceHead *)&CastleCoaster.piece.flags);
}

// FUNCTION: LEGOLAND 0x00424800
void FUN_00424800(void) {
    TileId tile;

    tile.pos.x = 0;
    tile.pos.y = 0;
    RemoveCastleCoaster(0, tile, 0);
}

// FUNCTION: LEGOLAND 0x00424820
void FUN_00424820(void) {
    FUN_00424440(0, 0);
}

// FUNCTION: LEGOLAND 0x00424830
void CastleDummyLoad(struct Element *param_1) {
    struct Ride *inner;

    inner = param_1->ride;
    DAT_00829c00 = param_1;
    DAT_00829c34 = inner;
    inner->layer->flags |= 0x2000;
}

// FUNCTION: LEGOLAND 0x00424850
void FUN_00424850(struct CastleState *p1, struct SegBlob **p2, float *p3, struct FVec3 *p4) {
    *p2 = &DAT_006102f8[2];
    *(unsigned int *)p3 = 0x3dcccccd;
    MapPosToWorld(&p1->piece.pos, (float)DAT_004b5b48.f8, p4);
}

// FUNCTION: LEGOLAND 0x00424890
float FUN_00424890(struct Train *param_1, float param_2) {
    return StepTrainToSegmentEnd(param_1, param_2);
}

// FUNCTION: LEGOLAND 0x004248b0
void FUN_004248b0(struct Integrator *obj, float unused, struct FloatArray *out) {
    struct Train *e = obj->owner;
    struct CastleState *sub = e->track;
    struct FVec3 v;
    void *s[2]; /* the piece and stretch of a CurveWalker (the rest is not read) */
    float t;
    float r;

    FUN_0041de10(obj, unused, out);
    t = FUN_0041dd50(e);
    MapPosToWorld(&sub->piece.pos, (float)DAT_004b5b48.f8, &v);
    r = t * t;
    s[0] = &sub->piece;
    s[1] = &DAT_006102f8[2];
    r = -(r / (CurveArcLength((struct CurveWalker *)s, 0x3dcccccd, &e->walker, *(unsigned int *)&e->motion.f4, 1, 0) * FLOAT_004ab404 * 2));

    out->data[1] = SumTrainCarMasses(e) * r * t;
}

// FUNCTION: LEGOLAND 0x00424960
int FUN_00424960(struct Train *param_1) {
    if (param_1->walker.node != &DAT_006102f8[2]) {
        return 0;
    }
    if (param_1->motion.f4 > FLOAT_004ab454) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00424990
float FUN_00424990(struct Train *self, float value) {
    if (FUN_00424960(self) != 0) {
        StopCastleTrain(self->track);
        self->track->field_e0 = GetGameTimer();
        return value;
    }
    self->integ.deriv = FUN_004248b0;
    return StepTrainToSegmentEnd(self, value);
}

// FUNCTION: LEGOLAND 0x004249e0
void CreateCastleTrain(struct CastleState *param_1) {
    param_1->field_d8 = CreateTrain(param_1);
    param_1->field_e0 = 0;
}

// FUNCTION: LEGOLAND 0x00424a00
void DestroyCastleTrain(struct CastleState *param_1) {
    DestroyTrain(&param_1->field_d8);
}

// FUNCTION: LEGOLAND 0x00424a20
void DrawCastleTrainOnPiece(struct PieceHead *param_1) {
    struct Train *obj = param_1->owner->field_d8;
    if (FUN_00426650() != 0) {
        DrawTrainCarsOnPiece(obj, (struct PathSeg *)param_1);
    }
}

// FUNCTION: LEGOLAND 0x00424a50
void UpdateCastleTrain(struct CastleState *param_1) {
    struct Train *host = (struct Train *)param_1->field_d8;

    ClearAllTrainCarsDrawn(host);
    if (IsTrainRunning(param_1->field_d8) != 0) {
        if (IsTrainStartDelayed(param_1->field_d8) != 0) {
            UpdateTrainStartDelay((struct Train *)host);
        }
        FUN_0041e130(host);
    }
    UpdateAllTrainCarBounds(host);
}

// FUNCTION: LEGOLAND 0x00424ab0
void StopCastleTrain(struct CastleState *this) {
    if (IsTrainRunning(this->field_d8) != 0) {
        StopTrain(this->field_d8);
        UnseatAllCastleVisitors((struct CastleState *)this);
    }
}

// FUNCTION: LEGOLAND 0x00424ae0
void StartCastleTrain(struct CastleState *param_1, unsigned int param_2) {
    FUN_0041e500(param_1->field_d8);
    StartTrain((struct Train *)param_1->field_d8, param_2);
}

// FUNCTION: LEGOLAND 0x00424b10
void InitCastleVisitorList(struct CastleState *param_1) {
    struct CastleVisitor *addr = &param_1->visitors;
    param_1->visitors.next = addr;
    param_1->visitors.prev = addr;
}

// FUNCTION: LEGOLAND 0x00424b30
struct CastleVisitor *FindWaitingCastleVisitor(struct CastleState *list) {
    struct CastleVisitor *node = list->visitors.next;
    struct CastleVisitor *list_end = (struct CastleVisitor *)&list->visitors;
    while (node != list_end && node->flags != 1) {
        node = node->next;
    }
    return (node == list_end) ? 0 : node;
}

// FUNCTION: LEGOLAND 0x00424b60
struct CastleVisitor *FindSeatedCastleVisitor(struct CastleState *list) {
    struct CastleVisitor *node = list->visitors.next;
    struct CastleVisitor *list_end = (struct CastleVisitor *)&list->visitors;
    while (node != list_end && node->flags != 2) {
        node = node->next;
    }
    return (node == list_end) ? 0 : node;
}

// FUNCTION: LEGOLAND 0x00424b90
void *FindFinishedCastleVisitor(struct CastleState *list) {
    struct CastleVisitor *cur = list->visitors.next;
    struct CastleVisitor *end = (struct CastleVisitor *)&list->visitors;

    while (cur != end && cur->flags != 4) {
        cur = cur->next;
    }
    return (cur == end) ? 0 : cur;
}

// FUNCTION: LEGOLAND 0x00424bc0
unsigned int GetLongestCastleVisitorWait(struct CastleState *ctx) {
    unsigned int result = 0;
    unsigned int t = GetGameTimer();
    struct CastleVisitor *cur = ctx->visitors.next;
    struct CastleVisitor *end = (struct CastleVisitor *)&ctx->visitors;

    if (cur != end) {
        do {
            if (cur->flags == 1) {
                unsigned int v = t - cur->time;
                if ((int)v > (int)result) {
                    result = v;
                }
            }
            cur = cur->next;
        } while (cur != end);
    }
    return result;
}

// FUNCTION: LEGOLAND 0x00424c10
unsigned int CountWaitingCastleVisitors(struct CastleState *ctx) {
    unsigned int result = 0;
    struct CastleVisitor *cur = ctx->visitors.next;
    struct CastleVisitor *end = (struct CastleVisitor *)&ctx->visitors;
    while (cur != end) {
        if (cur->flags == 1) {
            ++result;
        }
        cur = cur->next;
    }
    return result;
}

// FUNCTION: LEGOLAND 0x00424c40
int ShouldCastleTrainDepart(struct CastleState *arg) {
    int temp1 = GetLongestCastleVisitorWait(arg);
    int temp2 = CountWaitingCastleVisitors(arg);
    if (temp1 > 0x1388 || temp2 > 3) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00424c70
void BoardCastleVisitors(struct CastleState *param_1) {
    struct CastleState *host = (struct CastleState *)param_1;
    struct CastleVisitor *node;
    struct CastleVisitor *cur;
    struct TrainSeat *ring;
    int found;
    int now = GetGameTimer();

    if (CastleCoaster.flags == 2) {
        if (IsTrainRunning(param_1->field_d8) != 0) {
            if (IsTrainStartDelayed(param_1->field_d8) == 0) {
                return;
            }
        }
        if (ShouldCastleTrainDepart(host) == 0) {
            return;
        }
        if (FindWaitingCastleVisitor(host) == 0) {
            return;
        }
        found = 0;
        while ((node = FindWaitingCastleVisitor(host)) != 0) {
            ring = FindEmptyTrainSeat(param_1->field_d8);
            if (ring == 0) {
                break;
            }
            SeatCastleVisitor((struct CastleVisitor *)node, ring);
            node->flags = 2;
            found = 1;
        }
        if (found != 0) {
            StartCastleTrain(param_1, 0xbb8);
        }
        return;
    }
    for (cur = (struct CastleVisitor *)host->visitors.next; cur != (struct CastleVisitor *)&host->visitors; cur = cur->next) {
        if (now - (int)cur->time > 0x1388) {
            cur->flags = 4;
        }
    }
}

// FUNCTION: LEGOLAND 0x00424d80
void UnseatAllCastleVisitors(struct CastleState *list) {
    struct CastleVisitor *current = FindSeatedCastleVisitor(list);
    while (current != NULL) {
        UnseatCastleVisitor((struct CastleVisitor *)current);
        current->flags = 4;
        current = FindSeatedCastleVisitor(list);
    }
}

// FUNCTION: LEGOLAND 0x00424dc0
void ReleaseFinishedCastleVisitors(struct CastleState *arg) {
    struct CastleVisitor *current = FindFinishedCastleVisitor(arg);
    if (current != NULL) {
        while (current != NULL) {
            struct Bloke *next = current->bloke;
            next->param_action = 33;
            FreeCastleVisitor((struct CastleVisitor *)current);
            current = FindFinishedCastleVisitor(arg);
        }
    }
}

// FUNCTION: LEGOLAND 0x00424df0
void FreeAllCastleVisitors(struct CastleState *esi) {
    struct CastleVisitor *current = esi->visitors.next;
    struct CastleVisitor *target = (struct CastleVisitor *)&esi->visitors;
    while (current != target) {
        FreeCastleVisitor((struct CastleVisitor *)current);
        current = esi->visitors.next;
    }
}

// FUNCTION: LEGOLAND 0x00424e20
void CastleRemoveAllRiders(void) {
    struct Ride *ride = GetCastleDispatchRide(0);
    struct RideNode *node = ride->riders;
    if (node != 0) {
        do {
            struct RideNode *next_node = node->next;
            node->rider->flags &= 0xfff7;
            RemoveBlokeFromRide(ride, node);
            node = next_node;
        } while (node != 0);
    }
}

// FUNCTION: LEGOLAND 0x00424e60
void FUN_00424e60(struct CastleState *owner) {
}

// FUNCTION: LEGOLAND 0x00424e70
void FUN_00424e70(struct CastleState *owner) {
    StopCastleTrain((struct CastleState *)&CastleCoaster);
}

#pragma optimize("", off)
// FUNCTION: LEGOLAND 0x00424e80
void FUN_00424e80(void) {
    DAT_00610804[DAT_00610a10 & 0x3f] = DAT_0060f908;
    DAT_00610500[DAT_00610a10 & 0x3f] = DAT_0060f900;
    DAT_00610400[DAT_00610a10 & 0x3f] = DAT_0060f8fc;
    DAT_00610704[DAT_00610a10 & 0x3f] = DAT_00611644;
    DAT_00610604[DAT_00610a10 & 0x3f] = DAT_00615f68;
    DAT_006100f8[DAT_00610a10 & 0x3f] = DAT_004dcbc8;
    DAT_0060fdf8[DAT_00610a10 & 0x3f] = DAT_004d83bc;
    DAT_0060fef8[DAT_00610a10 & 0x3f] = DAT_0060f910;
    DAT_0060fbf8[DAT_00610a10 & 0x3f] = DAT_00615fc4;
    DAT_006101f8[DAT_00610a10 & 0x3f] = DAT_00615fc8;
    DAT_0060fcf8[DAT_00610a10 & 0x3f] = DAT_00615fcc;
    DAT_00610904[DAT_00610a10 & 0x3f] = DAT_00610704[DAT_00610a10 & 0x3f] + DAT_00610400[DAT_00610a10 & 0x3f] + DAT_0060fdf8[DAT_00610a10 & 0x3f] + DAT_00610604[DAT_00610a10 & 0x3f] + DAT_006100f8[DAT_00610a10 & 0x3f] + DAT_0060fef8[DAT_00610a10 & 0x3f];
    DAT_0060f908 = 0;
    DAT_0060f900 = 0;
    DAT_0060f8fc = 0;
    DAT_00611644 = 0;
    DAT_00615f68 = 0;
    DAT_004dcbc8 = 0;
    DAT_004d83bc = 0;
    DAT_0060f910 = 0;
    DAT_00615fc4 = 0;
    DAT_00615fc8 = 0;
    DAT_00615fcc = 0;
}
#pragma optimize("", on)

#pragma optimize("", off)
// FUNCTION: LEGOLAND 0x00425050
void RenderCastleObj(Element *obj, unsigned int param_2, unsigned int param_3, TileId *tile, void *param_5, unsigned int clip) {
    struct Ride *r;
    struct RideNode *walk;
    struct Bloke *rider;
    struct Point off;
    struct Point pos;

    r = obj->ride;
    RenderItems_New();
    DAT_00610a18 = NULL;
    walk = r->riders;
    while (walk != NULL) {
        if (tile->id == walk->tile.id) {
            rider = walk->rider;
            if (rider->param_action < 0x10) {
                AddBlokeToRenderList(&DAT_00610a18, walk, walk->person->field_20);
            }
            if (rider->param_action >= 0x21) {
                AddBlokeToRenderList(&DAT_00610a18, walk, walk->person->field_20);
            }
        }
        walk = walk->next;
    }
    RenderBlokeList((struct RenderItemNode **)&DAT_00610a18);
    if (CastleMatteSprite != NULL) {
        pos = GetScreenCoordsForObject(tile, r);
        off = GetRenderOffsetForLayer(r->layer, 2);
        AdjustOffsetForViewMode(&off);
        PrintSprite(CastleMatteSprite, pos.x + off.x, pos.y + off.y, clip, 0);
    }
}
#pragma optimize("", on)

#pragma optimize("", off)
// FUNCTION: LEGOLAND 0x00425170
void CastleCoasterUpdate(Element *obj) {
    FUN_00424e80();
    FUN_00425e20();
    FUN_00423140(0);
    if (CastleCoasterPlaced != 0) {
        BoardCastleVisitors((struct CastleState *)&CastleCoaster);
        UpdateCastleTrain((struct CastleState *)&CastleCoaster);
        ReleaseFinishedCastleVisitors((struct CastleState *)&CastleCoaster);
    }
}
#pragma optimize("", on)

#pragma optimize("", off)
// FUNCTION: LEGOLAND 0x004251c0
void CastleObjUpdate(Element *obj) {
    /* /Od build: stack-slot order follows the (hashed) local names; these single-letter
     * names reproduce the original layout (a at ebp-4 ... i at ebp-0x24). */
    RideNode *a; /* current rider node */
    TileId *b; /* &a->tile */
    struct CastleState *c; /* CastleCoaster */
    Ride *d; /* the castle ride */
    unsigned int e; /* tile y */
    unsigned int f; /* tile x */
    Bloke *g; /* rider */
    RideNode *h; /* next rider node */
    struct CastleVisitor *i; /* timer node (unused) */

    d = obj->ride;
    a = d->riders;
    c = &CastleCoaster;
    if (c->f_a8 != NULL) {
        FindPathSegDoorAPoints(c->f_a8, &c->pts_a);
    }
    if (c->f_c0 != NULL) {
        FindPathSegDoorBPoints(c->f_c0, &c->pts_b);
    }
    while (a != NULL) {
        h = a->next;
        b = &a->tile;
        f = b->pos.x + d->x;
        e = b->pos.y + d->y;
        g = a->rider;
        switch (g->param_action) {
        case 0:
            g->flags |= 8;
            if (g->low_level_action == 0) {
                g->dest.x = (f - 8) << 8;
                g->dest.y = (e - 1) << 8;
                g->target_dir = ((CalcMoveLineIntFn)CalcMoveLine)(g->pos, g->dest, &g->nav) + 0x10;
                g->low_level_action = 7;
                NewDirForAction(g, (g->target_dir >> 5) + 3);
                g->param_action++;
            }
            break;
        case 1:
            if (g->low_level_action == 0) {
                g->param_action = 0x10;
            }
            break;
        case 0x10:
            i = AddCastleVisitor(g, &CastleCoaster);
            g->param_action = 0x20;
            break;
        case 0x21:
            if (g->low_level_action == 0) {
                g->dest.x = (f << 8) + 0x80;
                g->dest.y = (e << 8) + 0x80;
                g->target_dir = ((CalcMoveLineIntFn)CalcMoveLine)(g->pos, g->dest, &g->nav) + 0x10;
                g->low_level_action = 7;
                NewDirForAction(g, (g->target_dir >> 5) + 3);
                g->param_action++;
            }
            break;
        case 0x22:
            if (g->low_level_action == 0) {
                g->param_action = 0x40;
            }
            break;
        case 0x40:
            g->flags &= 0xfff7;
            RemoveBlokeFromRide(d, a);
            break;
        }
        a = h;
    }
    CastleCoasterUpdate(obj);
}
#pragma optimize("", on)

#pragma optimize("", off)
// FUNCTION: LEGOLAND 0x004254d0
void CastleGetInterfaces(struct Element *head, struct Ride *obj) {
    // STRING: LEGOLAND 0x004b5c0c
    if (_stricmp("CASTLE OBJ", head->name) == 0) {
        obj->cb_8c = CastleDispatchSetEditMode;
        obj->cb_90 = CastleDispatchCalcCursor;
        obj->cb_94 = CastleDispatchDCalcCursor;
        obj->cb_98 = CastleDispatchAddObject;
        obj->cb_9c = CastleDispatchRemoveObject;
        obj->cb_a4 = CastleObjLoad;
        obj->cb_ac = CastleObjUnload;
        obj->cb_pre_render = CastleObjUpdate;
        obj->cb_b0 = RenderCastleObj;
        obj->save_hook = CastleObj_Save;
        obj->load_hook = CastleObj_Load;
        obj->cb_c0 = (int (*)())FUN_004246e0;
        CastleDispatchTable[0].element = head;
        CastleDispatchTable[0].fn_4 = (void (*)())FUN_00424240;
        CastleDispatchTable[0].fn_8 = FUN_00424280;
        CastleDispatchTable[0].fn_10 = FUN_00424440;
        CastleDispatchTable[0].fn_c = PlaceCastleCoaster;
        CastleDispatchTable[0].fn_14 = RemoveCastleCoaster;
        DAT_0082adb0[2] = head;
    }
    // STRING: LEGOLAND 0x004b5b7c
    else if (_stricmp("CASTLE_DUMMY", head->name) == 0) {
        obj->cb_8c = CastleDispatchSetEditMode;
        obj->cb_90 = CastleDispatchCalcCursor;
        obj->cb_94 = CastleDispatchDCalcCursor;
        obj->cb_98 = CastleDispatchAddObject;
        obj->cb_9c = CastleDispatchRemoveObject;
        obj->cb_a4 = CastleDummyLoad;
        obj->cb_b0 = RenderCastleDummy;
        CastleDispatchTable[1].fn_10 = FUN_00424820;
        CastleDispatchTable[1].fn_14 = FUN_00424800;
        DAT_0082adb0[3] = head;
    }
    // STRING: LEGOLAND 0x004b5bfc
    else if (_stricmp("SQUARE_TRACK", head->name) == 0) {
        obj->cb_8c = CastleDispatchSetEditMode;
        obj->cb_90 = CastleDispatchCalcCursor;
        obj->cb_94 = CastleDispatchDCalcCursor;
        obj->cb_98 = CastleDispatchAddObject;
        obj->cb_9c = CastleDispatchRemoveObject;
        obj->cb_a4 = CastleLoadBasicTiles;
        obj->cb_ac = CastleUnloadBasicTiles;
        obj->cb_b0 = RenderSquareTrack;
        obj->cb_90 = SquareTrackCalcCursor;
        CastleDispatchTable[2].element = head;
        CastleDispatchTable[2].fn_4 = FUN_00427940;
        CastleDispatchTable[2].fn_8 = FUN_00427b20;
        CastleDispatchTable[2].fn_10 = FUN_00427970;
        CastleDispatchTable[2].fn_c = FUN_00427bc0;
        CastleDispatchTable[2].fn_14 = RemoveQueriedCoasterPiece;
        DAT_0082adb0[0] = head;
    }
    // STRING: LEGOLAND 0x004b5be8
    else if (_stricmp("SQUARE_TRACK_HEIGHT", head->name) == 0) {
        obj->cb_8c = CastleDispatchSetEditMode;
        obj->cb_90 = CastleDispatchCalcCursor;
        obj->cb_94 = CastleDispatchDCalcCursor;
        obj->cb_98 = CastleDispatchAddObject;
        obj->cb_9c = CastleDispatchRemoveObject;
        obj->cb_a4 = SquareTrackHeightLoad;
        obj->cb_b0 = RenderSquareTrack;
        CastleDispatchTable[3].element = head;
        CastleDispatchTable[3].fn_4 = FUN_00427940;
        CastleDispatchTable[3].fn_8 = FUN_00427c90;
        CastleDispatchTable[3].fn_10 = FUN_00427970;
        CastleDispatchTable[3].fn_c = FUN_00427ea0;
        CastleDispatchTable[3].fn_14 = RemoveQueriedCoasterPiece;
        DAT_0082adb0[5] = head;
    }
    // STRING: LEGOLAND 0x004b5bd0
    else if (_stricmp("SQUARE_TRACK_HEIGHT_0", head->name) == 0) {
        obj->cb_8c = CastleDispatchSetEditMode;
        obj->cb_90 = CastleDispatchCalcCursor;
        obj->cb_94 = CastleDispatchDCalcCursor;
        obj->cb_98 = CastleDispatchAddObject;
        obj->cb_9c = CastleDispatchRemoveObject;
        obj->cb_a4 = SquareTrackHeight0Load;
        obj->cb_b0 = RenderSquareTrack;
        CastleDispatchTable[4].element = head;
        CastleDispatchTable[4].fn_4 = FUN_00427940;
        CastleDispatchTable[4].fn_8 = FUN_00427c90;
        CastleDispatchTable[4].fn_10 = FUN_00427970;
        CastleDispatchTable[4].fn_c = FUN_00427ea0;
        CastleDispatchTable[4].fn_14 = RemoveQueriedCoasterPiece;
        DAT_0082adb0[4] = head;
    }
    // STRING: LEGOLAND 0x004b5bb4
    else if (_stricmp("SQUARE_TRACK_HEIGHT_PATH", head->name) == 0) {
        obj->cb_8c = CastleDispatchSetEditMode;
        obj->cb_90 = CastleDispatchCalcCursor;
        obj->cb_94 = CastleDispatchDCalcCursor;
        obj->cb_98 = CastleDispatchAddObject;
        obj->cb_9c = CastleDispatchRemoveObject;
        obj->cb_a4 = SquareTrackHeightPathLoad;
        obj->cb_b0 = RenderSquareTrack;
        CastleDispatchTable[5].element = head;
        CastleDispatchTable[5].fn_4 = FUN_00427940;
        CastleDispatchTable[5].fn_8 = FUN_004280b0;
        CastleDispatchTable[5].fn_10 = FUN_00427970;
        CastleDispatchTable[5].fn_c = FUN_00428300;
        CastleDispatchTable[5].fn_14 = RemoveQueriedCoasterPiece;
        DAT_0082adb0[1] = head;
    }
    // STRING: LEGOLAND 0x004b5ba0
    else if (_stricmp("ROLLER_COASTER_LOAD", head->name) == 0) {
        obj->cb_8c = CastleDispatchSetEditMode;
        obj->cb_90 = CastleDispatchCalcCursor;
        obj->cb_94 = CastleDispatchDCalcCursor;
        obj->cb_98 = CastleDispatchAddObject;
        obj->cb_9c = CastleDispatchRemoveObject;
        obj->cb_8c = (void (*)())CastleObj_Load;
    }
    // STRING: LEGOLAND 0x004b5b8c
    else if (_stricmp("ROLLER_COASTER_SAVE", head->name) == 0) {
        obj->cb_8c = CastleDispatchSetEditMode;
        obj->cb_90 = CastleDispatchCalcCursor;
        obj->cb_94 = CastleDispatchDCalcCursor;
        obj->cb_98 = CastleDispatchAddObject;
        obj->cb_9c = CastleDispatchRemoveObject;
        obj->cb_8c = (void (*)())CastleObj_Save;
    }
}
#pragma optimize("", on)

// FUNCTION: LEGOLAND 0x00425a50
void FUN_00425a50(void) {
    DAT_004b5c1c[0].m[0][0] = 1.60334f;
    DAT_004b5c1c[0].m[0][1] = -1.60334f;
    DAT_004b5c1c[0].m[0][2] = 0.0f;
    DAT_004b5c1c[0].m[0][3] = 0.0f;
    DAT_004b5c1c[0].m[1][0] = 0.801688f;
    DAT_004b5c1c[0].m[1][1] = 0.801688f;
    DAT_004b5c1c[0].m[1][2] = 1.96416f;
    DAT_004b5c1c[0].m[1][3] = 0.0f;
    DAT_004b5c1c[0].m[2][0] = 3.19995f;
    DAT_004b5c1c[0].m[2][1] = 3.19995f;
    DAT_004b5c1c[0].m[2][2] = 0.0f;
    DAT_004b5c1c[0].m[2][3] = 32767.5f;
    DAT_004b5c1c[0].m[3][0] = 0.0f;
    DAT_004b5c1c[0].m[3][1] = 0.0f;
    DAT_004b5c1c[0].m[3][2] = 0.0f;
    DAT_004b5c1c[0].m[3][3] = 1.0f;
    DAT_004b5c1c[1] = DAT_004b5c1c[0];
    *(volatile float *)&DAT_004b5c1c[1].m[0][0] *= 0.5f;
    DAT_004b5c1c[1].m[0][1] *= 0.5f;
    DAT_004b5c1c[1].m[1][0] *= 0.5f;
    DAT_004b5c1c[1].m[1][1] *= 0.5f;
    DAT_004b5c1c[1].m[1][2] *= 0.5f;
    lego_invsqrtf_init();
    lego_sqrtf_init();
    DAT_00829990[0] = 1.0f;
    DAT_00829990[1] = 1.0f;
    DAT_00829990[2] = DAT_004b5c1c[0].m[1][0] * -2.0f / DAT_004b5c1c[0].m[1][2];
    FUN_00425d50(DAT_00829990);
    FUN_00425d50((float *)&DAT_004b5cb0);
    FUN_00425d50((float *)&DAT_004b5cc0);
    FUN_00425bd0();
    FUN_00426740();
}

// FUNCTION: LEGOLAND 0x00425bd0
void FUN_00425bd0(void) {
    float f;
    unsigned int i1;
    unsigned int i2;
    unsigned int i3;

    f = DAT_004b5cac;
    f = f + DAT_0061164c;
    f = f * 0.5f;

    i1 = DAT_004b5cb0;
    i2 = DAT_004b5cb4;
    i3 = DAT_004b5cb8;
    DAT_004b5ca0 = i1;

    memcpy(&i1, &DAT_0061164c, sizeof(i1));
    DAT_004b5ca4 = i2;
    memcpy(&i2, &DAT_004b5cac, sizeof(i2));
    DAT_004b5ca8 = i3;

    DAT_00611648 = i1;
    DAT_004b5c9c = i2;
    DAT_00829a60 = f;

    f = DAT_004b5cac;
    f = f - DAT_0061164c;
    f = f * 0.5f;
    DAT_0082999c = f;
}

// FUNCTION: LEGOLAND 0x00425c40
void FUN_00425c40(void) {
    float f;
    unsigned int i1;
    unsigned int i2;
    unsigned int i3;

    f = DAT_004b5cbc;
    f = f + DAT_00611650;
    f = f * 0.5f;

    i1 = DAT_004b5cc0;
    i2 = DAT_004b5cc4;
    i3 = DAT_004b5cc8;
    DAT_004b5ca0 = i1;

    memcpy(&i1, &DAT_00611650, sizeof(i1));
    DAT_004b5ca4 = i2;
    memcpy(&i2, &DAT_004b5cbc, sizeof(i2));
    DAT_004b5ca8 = i3;

    if (f == f) {
        DAT_00611648 = i1;
        DAT_004b5c9c = i2;
        DAT_00829a60 = f;
    }

    f = DAT_004b5cbc;
    f = f - DAT_00611650;
    f = f * 0.5f;
    DAT_0082999c = f;
}

// FUNCTION: LEGOLAND 0x00425cb0
void MapPosToWorld(const struct Int16Pair *in, float f, struct FVec3 *out) {
    short temp;
    float temp_f;
    temp = in->x;
    out->x = (float)temp * FLOAT_004ab45c;
    temp = in->y;
    out->y = (float)temp * FLOAT_004ab45c;
    temp_f = f;
    out->z = temp_f * FLOAT_004ab458;
}

// FUNCTION: LEGOLAND 0x00425cf0
void Vec3Cross(const struct FVec3 *a, const struct FVec3 *b, struct FVec3 *result) {
    result->x = b->z * a->y - a->z * b->y;
    result->y = a->z * b->x - a->x * b->z;
    result->z = a->x * b->y - a->y * b->x;
}

// FUNCTION: LEGOLAND 0x00425d30
float Vec3Dot(struct FVec3 *param_1, struct FVec3 *param_2) {
    return param_1->z * param_2->z + param_1->y * param_2->y + param_1->x * param_2->x;
}

// FUNCTION: LEGOLAND 0x00425d50
void FUN_00425d50(float *v) {
    float sum = FLOAT_004ab390;
    float s;
    int i;

    for (i = 0; i < 3; i++) {
        sum += v[i] * v[i];
    }
    s = lego_sqrtf(sum);
    for (i = 0; i < 3; i++) {
        v[i] *= s;
    }
}

// FUNCTION: LEGOLAND 0x00425da0
unsigned int Vec3Equal(const float *a, const float *b) {
    if (a[0] != b[0]) {
        return 0;
    }
    if (a[1] != b[1]) {
        return 0;
    }
    if (a[2] != b[2]) {
        return 0;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00425de0
void Mat2Invert(float *a) {
    float inv = 1.0f / (a[0] * a[3] - a[1] * a[2]);
    float t = a[0];

    a[0] = inv * a[3];
    a[3] = t * inv;
    a[1] = -(inv * a[1]);
    a[2] = -(inv * a[2]);
}

// FUNCTION: LEGOLAND 0x00425e20
void FUN_00425e20(void) {
    struct FMat4 m;
    float a[4];
    struct Point pt;
    struct Point ref;
    int bounds[4];

    int mid;
    float dx;
    float fx, fy, fm, fb;
    float dy;

    DAT_008299bc = DAT_004b5c1c[1];
    a[0] = DAT_008299bc.m[0][0];
    a[1] = DAT_008299bc.m[0][1];
    a[2] = DAT_008299bc.m[1][0];
    a[3] = DAT_008299bc.m[1][1];
    Mat2Invert(a);
    pt.x = (lpConfig->view_width >> 1) + lpConfig->view_x;
    pt.y = (lpConfig->view_height >> 1) + lpConfig->view_y;
    DAT_008299bc.m[0][3] = (float)pt.x;
    DAT_008299bc.m[1][3] = (float)pt.y;
    ScreenToMapRef(&pt.x, &ref.x, 0);
    GetTileBounds(&ref, bounds);
    DAT_008299a0[2] = 0.0f;
    mid = (bounds[2] + bounds[0]) >> 1;
    fx = (float)pt.x;
    fm = (float)mid;
    dx = fx - fm;
    fy = (float)pt.y;
    fb = (float)bounds[1];
    dy = fy - fb;
    DAT_008299a0[0] = dy * a[1] + dx * a[0];
    DAT_008299a0[1] = dy * a[3] + dx * a[2];
    DAT_008299a0[0] += (float)ref.x * FLOAT_004ab45c;
    DAT_008299a0[1] += (float)ref.y * FLOAT_004ab45c;
    DAT_008299ac[0] = lpConfig->view_x;
    DAT_008299ac[1] = lpConfig->view_y;
    DAT_008299ac[2] = lpConfig->view_width + lpConfig->view_x;
    DAT_008299ac[3] = lpConfig->view_height + lpConfig->view_y;
    Mat4Identity(m.m);
    m.m[0][3] = -DAT_008299a0[0];
    m.m[1][3] = -DAT_008299a0[1];
    m.m[2][3] = -DAT_008299a0[2];
    Mat4Multiply(DAT_008299bc.m, m.m, DAT_008299fc.m);
    SetClipRect(DAT_008299ac[0], DAT_008299ac[1], DAT_008299ac[2], DAT_008299ac[3]);
}

// FUNCTION: LEGOLAND 0x00426000
void FUN_00426000(int flag) {
    struct FMat4 m;

    if (flag) {
        DAT_008299bc = DAT_004b5c1c[1];
    } else {
        DAT_008299bc = DAT_004b5c1c[0];
    }
    DAT_008299bc.m[0][3] = 0.0f;
    DAT_008299bc.m[1][3] = 0.0f;
    DAT_008299ac[0] = lpConfig->view_x;
    DAT_008299ac[1] = lpConfig->view_y;
    DAT_008299ac[2] = lpConfig->view_width + lpConfig->view_x;
    DAT_008299ac[3] = lpConfig->view_height + lpConfig->view_y;
    DAT_008299a0[0] = 0.0f;
    DAT_008299a0[1] = 0.0f;
    DAT_008299a0[2] = 0.0f;
    Mat4Identity(m.m);
    m.m[0][3] = 0.0f;
    m.m[1][3] = 0.0f;
    m.m[2][3] = 0.0f;
    Mat4Multiply(DAT_008299bc.m, m.m, DAT_008299fc.m);
}

// FUNCTION: LEGOLAND 0x004260e0
float FUN_004260e0(void) {
    return DAT_008299bc.m[1][2];
}

// FUNCTION: LEGOLAND 0x004260f0
void Mat4Identity(float m[4][4]) {
    int i;
    int j;

    for (i = 0; i <= 3; i++) {
        for (j = 0; j <= 3; j++) {
            m[i][j] = 0.0f;
            if (i == j) {
                m[i][j] = 1.0f;
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x00426120
void Mat4Multiply(float a[4][4], float b[4][4], float out[4][4]) {
    int i;
    int j;
    int k;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            float sum = FLOAT_004ab390;
            for (k = 0; k < 4; k++) {
                sum += a[i][k] * b[k][j];
            }
            out[i][j] = sum;
        }
    }
}

struct Mat4x4 {
    unsigned int m[4][4];
};

// FUNCTION: LEGOLAND 0x00426190
void Mat4Transpose(struct Mat4x4 *param_2, struct Mat4x4 *param_1) {
    int i;
    int j;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            param_1->m[i][j] = param_2->m[j][i];
        }
    }
}

// FUNCTION: LEGOLAND 0x004261c0
void TransformPoints3x4(float *in, float *out, float m[4][4], int n) {
    int j;
    int k;

    while (n-- > 0) {
        for (j = 0; j < 3; j++) {
            float sum = FLOAT_004ab390;
            for (k = 0; k < 3; k++) {
                sum += in[k] * m[j][k];
            }
            out[j] = sum + m[j][3];
        }
        out += 3;
        in += 3;
    }
}

// FUNCTION: LEGOLAND 0x00426230
unsigned int FUN_00426230(float in[][3], int out[][4], int n) {
    return FUN_00426250(in, out, &DAT_008299fc.m[0][0], 0x10, n);
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x00426250
__declspec(naked) unsigned int FUN_00426250(float in[][3], int out[][4], float *m, unsigned int stride, int n) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 8
        mov eax, dword ptr [ebp + 0x18]
        mov ecx, eax
        dec eax
        test ecx, ecx
        jle L42639c
        push ebx
        mov ebx, dword ptr [ebp + 0xc]
        inc eax
        push esi
        push edi
        mov dword ptr [ebp - 8], eax
L42626e:
        mov edx, dword ptr [DAT_00829a3c + 0x18]
        mov edi, ebx
        mov dword ptr [ebp + 0xc], edx
        mov edx, dword ptr [ebp + 0x10]
        mov dword ptr [ebp - 4], 3
L426283:
        fld dword ptr [FLOAT_004ab390]
        mov ecx, dword ptr [ebp + 8]
        mov eax, edx
        mov esi, 3
L426293:
        fld dword ptr [ecx]
        fmul dword ptr [eax]
        add eax, 4
        add ecx, 4
        dec esi
        faddp st(1), st
        jne L426293
        fadd dword ptr [edx + 0xc]
        fstp dword ptr [ebp + 0x18]
        fld dword ptr [ebp + 0x18]
        fistp dword ptr [ebp + 0x18]
        mov eax, dword ptr [ebp + 0x18]
        add edx, 0x10
        mov dword ptr [edi], eax
        mov eax, dword ptr [ebp - 4]
        add edi, 4
        dec eax
        mov dword ptr [ebp - 4], eax
        jne L426283
        mov ecx, dword ptr [ebx]
        mov dword ptr [ebx + 0xc], 0
        cmp ecx, dword ptr [DAT_008299ac]
        jge L4262da
        mov eax, dword ptr [ebx + 0xc]
        or al, 2
        jmp L4262ee
L4262da:
        mov edx, dword ptr [ebx]
        mov eax, dword ptr [DAT_008299ac + 0x8]
        cmp edx, eax
        mov eax, dword ptr [ebx + 0xc]
        jg L4262ec
        or al, 3
        jmp L4262ee
L4262ec:
        or al, 1
L4262ee:
        mov dword ptr [ebx + 0xc], eax
        mov eax, dword ptr [ebx + 4]
        cmp eax, dword ptr [DAT_008299ac + 0x4]
        jge L426303
        mov eax, dword ptr [ebx + 0xc]
        or al, 8
        jmp L426316
L426303:
        mov ecx, dword ptr [DAT_008299ac + 0xc]
        cmp eax, ecx
        mov eax, dword ptr [ebx + 0xc]
        jg L426314
        or al, 0xc
        jmp L426316
L426314:
        or al, 4
L426316:
        mov ecx, dword ptr [ebp + 0xc]
        mov dword ptr [ebx + 0xc], eax
        cmp ecx, offset DAT_00829a3c
        je L42637e
        mov esi, dword ptr [ebx]
        mov edi, dword ptr [ebx + 4]
        jmp L42632e
L42632b:
        mov ecx, dword ptr [ebp + 0xc]
L42632e:
        cmp esi, dword ptr [ecx]
        jge L426339
        mov eax, 2
        jmp L426348
L426339:
        mov edx, dword ptr [ecx + 8]
        xor eax, eax
        cmp esi, edx
        setg al
        dec eax
        and eax, 2
        inc eax
L426348:
        cmp edi, dword ptr [ecx + 4]
        jge L426351
        or al, 8
        jmp L42635c
L426351:
        cmp edi, dword ptr [ecx + 0xc]
        jg L42635a
        or al, 0xc
        jmp L42635c
L42635a:
        or al, 4
L42635c:
        mov edx, dword ptr [ecx + 0x10]
        and edx, eax
        cmp edx, 0xf
        je L426376
        mov ecx, dword ptr [ecx + 0x18]
        cmp ecx, offset DAT_00829a3c
        mov dword ptr [ebp + 0xc], ecx
        jne L42632b
        jmp L42637e
L426376:
        mov eax, dword ptr [ebx + 0xc]
        or al, 0xf0
        mov dword ptr [ebx + 0xc], eax
L42637e:
        mov edx, dword ptr [ebp + 8]
        mov ecx, dword ptr [ebp + 0x14]
        mov eax, dword ptr [ebp - 8]
        add edx, 0xc
        add ebx, ecx
        dec eax
        mov dword ptr [ebp + 8], edx
        mov dword ptr [ebp - 8], eax
        jne L42626e
        pop edi
        pop esi
        pop ebx
L42639c:
        mov esp, ebp
        pop ebp
        ret
    }
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x004263a0
__declspec(naked) void FUN_004263a0(void *pts, unsigned int m[4][4], int n, struct RectI *out) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x14
        mov ecx, dword ptr [ebp + 0x14]
        mov eax, 0x80000000
        mov dword ptr [ecx + 8], eax
        mov dword ptr [ecx + 0xc], eax
        mov eax, dword ptr [ebp + 0x10]
        mov dword ptr [ecx], 0x7fffffff
        mov dword ptr [ecx + 4], 0x7fffffff
        mov ecx, eax
        dec eax
        test ecx, ecx
        jle L426458
        push ebx
        inc eax
        push esi
        push edi
        mov dword ptr [ebp - 4], eax
L4263d6:
        mov edx, dword ptr [ebp + 0xc]
        lea edi, [ebp - 0x14]
        mov ebx, 2
L4263e1:
        fld dword ptr [FLOAT_004ab390]
        mov ecx, dword ptr [ebp + 8]
        mov eax, edx
        mov esi, 3
L4263f1:
        fld dword ptr [ecx]
        fmul dword ptr [eax]
        add eax, 4
        add ecx, 4
        dec esi
        faddp st(1), st
        jne L4263f1
        fadd dword ptr [edx + 0xc]
        fstp dword ptr [ebp + 0x10]
        fld dword ptr [ebp + 0x10]
        fistp dword ptr [ebp + 0x10]
        mov eax, dword ptr [ebp + 0x10]
        add edx, 0x10
        mov dword ptr [edi], eax
        add edi, 4
        dec ebx
        jne L4263e1
        mov ecx, dword ptr [ebp + 0x14]
        mov eax, dword ptr [ebp - 0x14]
        cmp eax, dword ptr [ecx + 8]
        jle L426428
        mov dword ptr [ecx + 8], eax
L426428:
        cmp eax, dword ptr [ecx]
        jge L42642e
        mov dword ptr [ecx], eax
L42642e:
        mov eax, dword ptr [ebp - 0x10]
        mov edx, dword ptr [ecx + 0xc]
        cmp eax, edx
        jle L42643b
        mov dword ptr [ecx + 0xc], eax
L42643b:
        cmp eax, dword ptr [ecx + 4]
        jge L426443
        mov dword ptr [ecx + 4], eax
L426443:
        mov ecx, dword ptr [ebp + 8]
        mov eax, dword ptr [ebp - 4]
        add ecx, 0xc
        dec eax
        mov dword ptr [ebp + 8], ecx
        mov dword ptr [ebp - 4], eax
        jne L4263d6
        pop edi
        pop esi
        pop ebx
L426458:
        mov esp, ebp
        pop ebp
        ret
    }
}

// FUNCTION: LEGOLAND 0x00426460
void FUN_00426460(unsigned int *dst, void *src) {
    unsigned int i;
    unsigned int j;
    for (i = 0; i < 3; ++i) {
        for (j = 0; j < 3; ++j) {
            dst[j] = *(unsigned int *)((unsigned char *)src + i * 4 + j * 16);
        }
        dst += 3;
    }
}

// FUNCTION: LEGOLAND 0x00426490
void Mat3ToMat4Transposed(unsigned int *src, unsigned int dst[4][4]) {
    int i;
    int j;

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            dst[j][i] = src[i * 3 + j];
        }
        dst[3][i] = 0;
    }
    dst[2][3] = 0;
    dst[1][3] = 0;
    dst[0][3] = 0;
    dst[3][3] = 0x3f800000;
}

// FUNCTION: LEGOLAND 0x004264e0
void MakeMat4FromRotTrans(unsigned int *t, unsigned int *m3, unsigned int dst[4][4]) {
    Mat3ToMat4Transposed(m3, dst);
    dst[0][3] = t[0];
    dst[1][3] = t[1];
    dst[2][3] = t[2];
}

// FUNCTION: LEGOLAND 0x00426510
void FUN_00426510(unsigned int *m3, struct Mat4x4 *out) {
    float t[3];
    unsigned int dst[4][4];
    t[0] = 0.0f;
    t[1] = 0.0f;
    t[2] = 0.0f;
    MakeMat4FromRotTrans((unsigned int *)t, m3, dst);
    Mat4Transpose((struct Mat4x4 *)dst, out);
}

// FUNCTION: LEGOLAND 0x00426560
void BuildBasisFromDir(struct FVec3 *dir, struct FVec3 *basis) {
    int i;

    basis[2].x = 0.0f;
    basis[2].y = 0.0f;
    basis[2].z = 1.0f;
    Vec3Cross(&basis[2], dir, &basis[1]);
    Vec3Cross(dir, &basis[1], &basis[2]);
    basis[0] = *dir;
    for (i = 0; i < 3; i++) {
        FUN_00425d50(&basis[i].x);
    }
}

// FUNCTION: LEGOLAND 0x004265d0
int ClipRectIToRect(struct RectI *a, struct RectI *b) {
    if (a->var_0 > b->var_8) {
        return 0;
    }
    if (a->var_8 < b->var_0) {
        return 0;
    }
    if (a->var_4 > b->var_c) {
        return 0;
    }
    if (a->var_c < b->var_4) {
        return 0;
    }
    if (a->var_0 < b->var_0) {
        a->var_0 = b->var_0;
    }
    if (a->var_8 > b->var_8) {
        a->var_8 = b->var_8;
    }
    if (a->var_4 < b->var_4) {
        a->var_4 = b->var_4;
    }
    if (a->var_c > b->var_c) {
        a->var_c = b->var_c;
    }
    return 0xF;
}

// FUNCTION: LEGOLAND 0x00426650
unsigned int FUN_00426650(void) {
    struct ListLink *n = (struct ListLink *)DAT_00829a3c.var_18;

    while (n != (struct ListLink *)&DAT_00829a3c) {
        if (n->kind == 0xf) {
            return 1;
        }
        n = n->var_18;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x004266b0
void FUN_004266b0(struct ListLink *param_1) {
    DAT_00829a3c.var_18->var_14 = param_1;
    param_1->var_18 = DAT_00829a3c.var_18;
    DAT_00829a3c.var_18 = param_1;
    param_1->var_14 = &DAT_00829a3c;
}

// FUNCTION: LEGOLAND 0x004266e0
void UnlinkListLink(struct ListLink *param_1) {
    struct ListLink *temp_ptr_1;
    struct ListLink *temp_ptr_2;

    temp_ptr_1 = param_1->var_18;
    temp_ptr_2 = param_1->var_14;
    temp_ptr_1->var_14 = temp_ptr_2;

    temp_ptr_1 = param_1->var_14;
    temp_ptr_2 = param_1->var_18;
    temp_ptr_1->var_18 = temp_ptr_2;
}

// FUNCTION: LEGOLAND 0x00426700
void FUN_00426700(struct RectI *dst, struct RectI *src) {
    *dst = *src;
    ((int *)dst)[4] = ClipRectIToRect(dst, (struct RectI *)&DAT_008299ac);
}

// FUNCTION: LEGOLAND 0x00426740
void FUN_00426740(void) {
    DAT_00829a3c.var_18 = &DAT_00829a3c;
    DAT_00829a3c.var_14 = &DAT_00829a3c;
}

// FUNCTION: LEGOLAND 0x00426750
unsigned int FUN_00426750(void *ptr, unsigned int *a, unsigned int *b, struct RectI *c) {
    unsigned int m[4][4];
    unsigned int m2[4][4];

    MakeMat4FromRotTrans(a, b, m);
    Mat4Multiply(DAT_008299fc.m, (float (*)[4])m, (float (*)[4])m2);
    FUN_004263a0(ptr, m2, 8, c);
}

// FUNCTION: LEGOLAND 0x004267b0
void FUN_004267b0(unsigned int *t, unsigned int *m3, struct EdgeObj *obj) {
    unsigned int m[4][4];
    int xf[8][4];
    float pts[8][3];
    int i;

    MakeMat4FromRotTrans(t, m3, m);
    TransformPoints3x4(&obj->pts[0][0], &pts[0][0], (float (*)[4])m, obj->npts);
    FUN_00426230(pts, xf, obj->npts);
    for (i = 0; i < obj->nedges; i++) {
        DrawDottedLine(xf[obj->edges[i].a], xf[obj->edges[i].b], obj->color);
    }
}

// FUNCTION: LEGOLAND 0x00426850
void FUN_00426850(float x, float y, float z, struct EdgeObj *obj) {
    int i;
    obj->npts = 8;
    obj->nedges = 12;
    obj->pts[0][0] = x;
    obj->pts[0][1] = -y;
    obj->pts[0][2] = 0.0f;
    obj->pts[1][0] = x;
    obj->pts[1][1] = y;
    obj->pts[1][2] = 0.0f;
    obj->pts[2][0] = -x;
    obj->pts[2][1] = y;
    obj->pts[3][0] = -x;
    obj->pts[3][1] = -y;
    obj->pts[3][2] = 0.0f;
    for (i = 0; i < 4; i++) {
        *(struct FVec3 *)obj->pts[i + 4] = *(struct FVec3 *)obj->pts[i];
        obj->pts[i + 4][2] = -z;
    }
    obj->edges[0].a = 0;
    obj->edges[0].b = 1;
    obj->edges[1].a = 1;
    obj->edges[1].b = 2;
    obj->edges[2].a = 2;
    obj->edges[2].b = 3;
    obj->edges[3].a = 3;
    obj->edges[3].b = 0;
    for (i = 0; i < 4; i++) {
        obj->edges[i + 4].a = obj->edges[i].a + 4;
        obj->edges[i + 4].b = obj->edges[i].b + 4;
    }
    obj->edges[8].a = 0;
    obj->edges[8].b = 4;
    obj->edges[9].a = 1;
    obj->edges[9].b = 5;
    obj->edges[10].a = 1;
    obj->edges[10].b = 5;
    obj->edges[11].a = 3;
    obj->edges[11].b = 7;
}

// FUNCTION: LEGOLAND 0x00426960
float lego_sqrtf(float x) {
    __asm {
        fld x
        call dword ptr [DAT_00829a5c]
        fstp x
    }
    return x;
}

// FUNCTION: LEGOLAND 0x00426980
__declspec(naked) void HASM_lego_sqrtf(void) {
    __asm {
        fstp dword ptr [esp - 10h]
        mov dword ptr [esp - 8], ebx
        mov dword ptr [esp - 0Ch], edx
        mov edx, dword ptr [esp - 10h]
        mov ebx, 007FFFFFh
        mov dword ptr [esp - 4], eax
        mov eax, edx
        shr eax, 11h
        and ebx, edx
        and eax, 3Fh
        or  ebx, 3F800000h
        mov dword ptr [esp - 10h], ebx
        mov ebx, dword ptr [esp - 8]
        fld dword ptr [esp - 10h]
        fmul dword ptr [eax*8 + sqrtf_table + 4]
        shr edx, 17h
        fadd dword ptr [eax*8 + sqrtf_table]
        mov eax, dword ptr [esp - 4]
        fmul dword ptr [edx*4 + sqrtf_exp_table]
        mov edx, dword ptr [esp - 0Ch]
        ret
    }
}

// FUNCTION: LEGOLAND 0x004269e0
void lego_sqrtf_init(void) {
    int i;
    float x;
    struct {
        float a;
        float b;
    } *t = (void *)&DAT_00610a18;

    for (i = 0; i < 64; i++) {
        if (i == 29) {
            i = 29;
        }
        x = (float)sqrt((float)i * 0.015625f + 1.0f);
        t[i + 1].a = FLOAT_004ab43c / (x + x);
        t[i + 1].b = FLOAT_004ab468 / (x * x * x * 2.0f);
    }
    sqrtf_exp_table[0] = 1.0f;
    for (i = 1; i <= 255; i++) {
        sqrtf_exp_table[i] = 1.0f / (float)sqrt(pow(2.0, i - 127));
    }
    __asm {
        push eax
        lea eax, HASM_lego_sqrtf
        mov DAT_00829a5c, eax
        pop eax
    }
}

// FUNCTION: LEGOLAND 0x00426a90
float lego_invsqrtf(float x) {
    __asm {
        fld x
        call dword ptr [DAT_00829a58]
        fstp x
    }
    return x;
}

// FUNCTION: LEGOLAND 0x00426ab0
__declspec(naked) void HASM_lego_invsqrtf(void) {
    __asm {
        fstp dword ptr [esp - 10h]
        mov dword ptr [esp - 8], ebx
        mov dword ptr [esp - 0Ch], edx
        mov edx, dword ptr [esp - 10h]
        mov ebx, 007FFFFFh
        mov dword ptr [esp - 4], eax
        mov eax, edx
        shr eax, 11h
        and ebx, edx
        and eax, 3Fh
        or  ebx, 3F800000h
        mov dword ptr [esp - 10h], ebx
        mov ebx, dword ptr [esp - 8]
        fld dword ptr [esp - 10h]
        fmul dword ptr [eax*8 + invsqrtf_table + 4]
        shr edx, 17h
        fadd dword ptr [eax*8 + invsqrtf_table]
        mov eax, dword ptr [esp - 4]
        fmul dword ptr [edx*4 + invsqrtf_exp_table]
        mov edx, dword ptr [esp - 0Ch]
        ret
    }
}

/* Lookup tables of the inverse square root (lego_invsqrtf): 64 (x/2, 1/(2x)) mantissa pairs, then one
 * factor per float exponent. The exponent table starts 0x204 bytes after the mantissa table (one unused
 * float between them): lego_invsqrtf_init's loop end pointer is exactly invsqrtf_exp_table. Defined here,
 * initialised, so the two stay in this order and adjacent as in the original. */
// GLOBAL: LEGOLAND 0x00610c40
float invsqrtf_table[129] = {0};

// GLOBAL: LEGOLAND 0x00610e44
float invsqrtf_exp_table[256] = {0};

// FUNCTION: LEGOLAND 0x00426b10
void lego_invsqrtf_init(void) {
    int i;
    int j;
    float x;

    for (i = 0; i < 64; i++) {
        x = (float)sqrt((float)i * 0.015625f + 1.0f);
        invsqrtf_table[i * 2] = x * 0.5f;
        invsqrtf_table[i * 2 + 1] = 0.5f / x;
    }
    for (j = 0; j <= 255; j++) {
        invsqrtf_exp_table[j] = (float)sqrt(pow(2.0, j - 127));
    }
    __asm {
        push eax
        lea eax, HASM_lego_invsqrtf
        mov DAT_00829a58, eax
        pop eax
    }
}

// FUNCTION: LEGOLAND 0x00426ba0
void RelocatePointer(UINT_PTR *param_1, UINT_PTR param_2) {
    if (*param_1 != 0) {
        *param_1 = *param_1 + param_2;
    }
}

// FUNCTION: LEGOLAND 0x00426bc0
void UnrelocatePointer(UINT_PTR *param_1, UINT_PTR param_2) {
    UINT_PTR var_0 = *param_1;
    if (var_0 != 0) {
        *param_1 = var_0 - param_2;
    }
}

struct TrackEntry {
    unsigned int var_0;
    short var_4;
    short var_6;
};

struct TrackList {
    unsigned int var_0;
    unsigned int var_4;
    unsigned int var_8;
    struct TrackEntry *var_c;
};

// FUNCTION: LEGOLAND 0x00426be0
void PrintTrackList(struct TrackList *edi) {
    int i;
    for (i = 0; i < (int)edi->var_8; i += 1) {
        struct TrackEntry *ptr = edi->var_c + i;
        // STRING: LEGOLAND 0x004b5cd0
        DBPrintf("Track %2x, Type %2x at (%2x, %2x)\n", i, ptr->var_0, ptr->var_4, ptr->var_6);
    }
}

struct Struct426d80X {
    struct CastleState *field_0;
    unsigned int field_4;
    unsigned int field_8;
    unsigned int field_c;
    unsigned int field_10;
};

struct Struct426e80Dst {
    unsigned int field_0;
    unsigned int field_4;
};

struct Struct426f40Dst {
    unsigned int field_0;
    float field_4;
    float field_8;
    unsigned int field_c;
    unsigned int field_10;
    unsigned int field_14;
    unsigned int field_18;
    unsigned int field_1c;
};

struct SaveRec {
    unsigned int size;
    unsigned int field_4;
    int pairCount;
    struct Struct426e80Dst *pairs;
    unsigned int timerCount;
    struct Struct426f40Dst *timer;
    unsigned int recCount;
    struct Struct427070Src *recs;
};

// FUNCTION: LEGOLAND 0x00426c20
int CastleObj_Load(struct Element *element) {
    struct SaveRec *rec = ReadSaveRec();
    void *obj;
    int i;

    if (rec == 0) {
        return 0;
    }
    if (rec == (struct SaveRec *)-1) {
        return 1;
    }
    RelocatePointer((UINT_PTR *)&rec->pairs, (UINT_PTR)rec);
    RelocatePointer((UINT_PTR *)&rec->timer, (UINT_PTR)rec);
    RelocatePointer((UINT_PTR *)&rec->recs, (UINT_PTR)rec);
    SetMapObjectsEnabled(0);
    for (i = 0; i < rec->pairCount; i++) {
        FUN_0041eca0(rec->pairs[i].field_0, (short *)&rec->pairs[i].field_4);
    }
    obj = GetPlacedCastleCoaster();
    if (rec->timer != 0) {
        LoadCastleTrain(rec->timer, obj);
    }
    if (rec->recs != 0) {
        LoadCastleVisitors(rec->recs, rec->recCount, obj);
    }
    MemFree(rec);
    SetMapObjectsEnabled(1);
    return 1;
}

// FUNCTION: LEGOLAND 0x00426ce0
int CastleObj_Save(struct Element *element) {
    struct Struct426d80X info;
    unsigned int zero;
    struct SaveRec *rec;
    struct CastleState *obj = GetPlacedCastleCoaster();

    if (obj != 0) {
        MeasureCastleSave(obj, &info);
        rec = MemAlloc(info.field_4, 0, 0, 0);
        if (rec == 0) {
            return 0;
        }
        WriteCastleSave(&info, rec);
        UnrelocatePointer((UINT_PTR *)&rec->pairs, (UINT_PTR)rec);
        UnrelocatePointer((UINT_PTR *)&rec->timer, (UINT_PTR)rec);
        UnrelocatePointer((UINT_PTR *)&rec->recs, (UINT_PTR)rec);
        WriteSaveRec((unsigned int *)rec);
        MemFree(rec);
        return 1;
    }
    zero = 0;
    SaveGameWrite(&zero, 4);
    return 1;
}

// FUNCTION: LEGOLAND 0x00426d80
void MeasureCastleSave(struct CastleState *param1, struct Struct426d80X *param2) {
    param2->field_0 = param1;
    param2->field_8 = CountCoasterPieces(param1);
    if (IsTrainRunning(param1->field_d8) != 0) {
        param2->field_c = 1;
    } else {
        param2->field_c = 0;
    }
    param2->field_10 = CountCastleVisitors((struct CastleState *)param1);
    param2->field_4 = (param2->field_8 + param2->field_c * 4 + 4 + param2->field_10) * 8;
}

// FUNCTION: LEGOLAND 0x00426de0
void WriteCastleSave(struct Struct426d80X *src, struct SaveRec *dst) {
    unsigned char *end;

    dst->size = src->field_4;
    dst->pairCount = src->field_8;
    dst->pairs = (struct Struct426e80Dst *)&dst[1];
    SaveCoasterPieces(src->field_0, dst->pairs);
    end = (unsigned char *)(dst->pairs + dst->pairCount);
    if (IsTrainRunning(src->field_0->field_d8) != 0) {
        struct Train *t = (struct Train *)src->field_0->field_d8;
        dst->timerCount = 1;
        dst->timer = (struct Struct426f40Dst *)end;
        SaveCastleTrain(t, (struct Struct426f40Dst *)end);
        end = (unsigned char *)(dst->timer + dst->timerCount);
    } else {
        dst->timer = 0;
    }
    dst->recCount = src->field_10;
    if (dst->recCount != 0) {
        dst->recs = (struct Struct427070Src *)end;
        SaveCastleVisitors((struct CastleState *)src->field_0, (struct Struct427050Dst *)end);
    } else {
        dst->recs = 0;
    }
}

// FUNCTION: LEGOLAND 0x00426e80
void SavePathSegRef(struct PathSeg *src, struct Struct426e80Dst *dst) {
    dst->field_0 = FindCastleDispatchIndex(src->ride);
    dst->field_4 = src->xy;
}

// FUNCTION: LEGOLAND 0x00426ea0
struct PathSeg *LoadPathSegRef(unsigned char *param_1, struct CastleState *param_2) {
    return FindPathSegByXY(param_2, (unsigned int *)(param_1 + 4));
}

// FUNCTION: LEGOLAND 0x00426ec0
void SaveCurveWalker(struct CurveWalker *arg0, unsigned int *arg1) {
    struct FVec3 local;
    struct SegBlob *cur;
    struct SegBlob *end;
    unsigned int count = 0;

    SavePathSegRef(arg0->piece, (struct Struct426e80Dst *)arg1);
    cur = FUN_0041cff0(arg0->piece, &local);
    end = arg0->node;
    while (cur != end) {
        cur = cur->next;
        if (cur == NULL)
            break;
        count++;
    }
    arg1[2] = count;
}

// FUNCTION: LEGOLAND 0x00426f10
void LoadCurveWalker(unsigned char *a, struct CurveWalker *b, struct CastleState *c) {
    b->piece = LoadPathSegRef(a, c);
    b->node = FUN_0041cff0(b->piece, &b->pos);
}

// FUNCTION: LEGOLAND 0x00426f40
void SaveCastleTrain(struct Train *src, struct Struct426f40Dst *dst) {
    SaveCurveWalker(&src->walker, &dst->field_14);
    dst->field_0 = src->flags;
    dst->field_4 = src->motion.f4;
    dst->field_8 = src->motion.f8;
    dst->field_c = GetGameTimer() - src->field_4;
    dst->field_10 = src->field_8 - GetGameTimer();
}

// FUNCTION: LEGOLAND 0x00426f90
void LoadCastleTrain(struct Struct426f40Dst *src, struct CastleState *obj) {
    struct Train *dst = obj->field_d8;

    dst->flags = src->field_0;
    LoadCurveWalker((unsigned char *)&src->field_14, (struct CurveWalker *)&dst->walker, (struct CastleState *)obj);
    PositionTrainCars(dst, src->field_4, &dst->walker);
    FUN_0041dad0(dst, src->field_8);
    dst->field_4 = GetGameTimer() - src->field_c;
    dst->field_8 = GetGameTimer() + src->field_10;
}

// FUNCTION: LEGOLAND 0x00426ff0
int FindCastleRiderIndex(struct Bloke *param) {
    int i = 0;
    struct RideNode *n = GetCastleDispatchRide(0)->riders;

    while (n != 0) {
        if (param == n->rider) {
            return i;
        }
        n = n->next;
        i++;
    }
    return -1;
}

// FUNCTION: LEGOLAND 0x00427020
struct Bloke *GetCastleRiderByIndex(unsigned int index) {
    unsigned int i = 0;
    struct RideNode *n = GetCastleDispatchRide(0)->riders;

    while (n != 0) {
        if (index == i) {
            return n->rider;
        }
        n = n->next;
        i++;
    }
    return 0;
}

struct Struct427050Dst {
    unsigned int field_0;
    unsigned int field_4;
};

// FUNCTION: LEGOLAND 0x00427050
void SaveCastleVisitor(struct CastleVisitor *param1, struct Struct427050Dst *param2) {
    param2->field_0 = param1->flags;
    param2->field_4 = FindCastleRiderIndex(param1->bloke);
}

struct Struct427070Src {
    unsigned int field_0;
    unsigned int field_4;
};

// FUNCTION: LEGOLAND 0x00427070
void LoadCastleVisitor(struct Struct427070Src *param_1, struct CastleState *param_2) {
    struct CastleVisitor *node;
    unsigned int kind;
    struct Bloke *index = GetCastleRiderByIndex(param_1->field_4);
    node = AddCastleVisitor(index, param_2);
    kind = param_1->field_0;
    node->flags = kind;
    if (kind == 2) {
        SeatCastleVisitor((struct CastleVisitor *)node, FindEmptyTrainSeat((struct Train *)param_2->field_d8));
    }
}

// FUNCTION: LEGOLAND 0x004270c0
void SaveCastleVisitors(struct CastleState *a1, struct Struct427050Dst *a2) {
    struct CastleVisitor *node = a1->visitors.next;
    struct CastleVisitor *end = &a1->visitors;
    if (node != end) {
        struct Struct427050Dst *out = a2;
        do {
            struct Struct427050Dst *cur = out;
            out = out + 1;
            SaveCastleVisitor(node, cur);
            node = node->next;
        } while (node != end);
    }
}

// FUNCTION: LEGOLAND 0x00427100
void LoadCastleVisitors(struct Struct427070Src *arr, int count, struct CastleState *obj) {
    for (; count > 0; count--) {
        LoadCastleVisitor(arr++, obj);
    }
}

// FUNCTION: LEGOLAND 0x00427130
unsigned int CountCastleVisitors(struct CastleState *main) {
    unsigned int counter = 0;
    struct CastleVisitor *node = main->visitors.next;
    struct CastleVisitor *end = &main->visitors;
    while (node != end) {
        node = node->next;
        counter++;
    }
    return counter;
}

// FUNCTION: LEGOLAND 0x00427150
unsigned int CountCoasterPieces(struct CastleState *arg) {
    unsigned int count = 1;

    if (arg->flags == 2) {
        struct PathSeg *next = arg->piece.next;
        struct PathSeg *end = (struct PathSeg *)&arg->piece;
        if (next != end) {
            do {
                next = next->next;
                count++;
            } while (next != end);
        }
        return count;
    }

    if (arg->piece.next != NULL) {
        struct PathSeg *curr = arg->piece.next;
        do {
            curr = curr->next;
            count++;
        } while (curr != NULL);
    }

    if (arg->piece.prev != NULL) {
        struct PathSeg *curr = arg->piece.prev;
        do {
            curr = curr->prev;
            count++;
        } while (curr != NULL);
    }

    return count;
}

// FUNCTION: LEGOLAND 0x00427190
void SaveCoasterPieces(struct CastleState *obj, struct Struct426e80Dst *out) {
    struct PathSeg *cur;

    out->field_0 = FindCastleDispatchIndex(obj->piece.ride);
    out->field_4 = obj->piece.xy;
    cur = obj->piece.next;
    out++;
    if (obj->flags == 2) {
        struct PathSeg *end = (struct PathSeg *)&obj->piece;
        if (cur != end) {
            do {
                SavePathSegRef(cur, out++);
                cur = cur->next;
            } while (cur != end);
        }
        return;
    }
    while (cur != 0) {
        SavePathSegRef(cur, out++);
        cur = cur->next;
    }
    cur = obj->piece.prev;
    while (cur != 0) {
        SavePathSegRef(cur, out++);
        cur = cur->prev;
    }
}

// FUNCTION: LEGOLAND 0x00427220
void WriteSaveRec(unsigned int *param_1) {
    SaveGameWrite(param_1, *param_1);
}

// FUNCTION: LEGOLAND 0x00427240
struct SaveRec *ReadSaveRec(void) {
    unsigned int size;
    struct SaveRec *rec;

    SaveGameRead(&size, 4);
    if (size == 0) {
        return (struct SaveRec *)-1;
    }
    rec = MemAlloc(size, 0, 0, 0);
    if (rec == 0) {
        return rec;
    }
    rec->size = size;
    SaveGameRead(&rec->field_4, size - 4);
    return rec;
}

// FUNCTION: LEGOLAND 0x004272a0
int SaveRollerCoasterFile(unsigned int *data) {
    unsigned int size = *data;
    DWORD written;
    HANDLE hFile;

    if (RollerCoasterSavePath && data) {
        hFile = CreateFileA(RollerCoasterSavePath, 0x40000000, 0, 0, 2, 0x8000000, 0);
        if (hFile != (HANDLE)-1) {
            WriteFile(hFile, data, size, &written, 0);
            if (written != size) {
                CloseHandle(hFile);
            } else {
                CloseHandle(hFile);
                return 1;
            }
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00427310
void *LoadRollerCoasterFile(void) {
    DWORD bytesRead;
    HANDLE hFile;
    unsigned int fileSize;
    void *buffer;

    if (!RollerCoasterSavePath) {
        return 0;
    }
    hFile = CreateFileA(RollerCoasterSavePath, 0x80000000, 1, 0, 3, 0x8000000, 0);
    if (hFile == (HANDLE)-1) {
        return 0;
    }
    fileSize = GetFileSize(hFile, 0);
    buffer = MemAlloc(fileSize, 0, (unsigned int)(UINT_PTR)DAT_004d8bb0, 0);
    if (buffer == 0) {
        CloseHandle(hFile);
        return 0;
    }
    ReadFile(hFile, buffer, fileSize, &bytesRead, 0);
    if (bytesRead != fileSize) {
        MemFree(buffer);
        CloseHandle(hFile);
        return 0;
    }
    CloseHandle(hFile);
    return buffer;
}

// FUNCTION: LEGOLAND 0x004273c0
unsigned char IsTrainSeatOccupied(struct TrainSeat *param_1) {
    return (unsigned char)(param_1->occupant != 0);
}

// FUNCTION: LEGOLAND 0x004273d0
void SetTrainSeatOccupant(struct TrainSeat *param_1, void *param_2) {
    param_1->occupant = param_2;
}

// FUNCTION: LEGOLAND 0x004273e0
struct CastleVisitor *TakeTrainSeatOccupant(struct TrainSeat *self) {
    struct CastleVisitor *value = self->occupant;
    self->occupant = 0;
    return value;
}

// FUNCTION: LEGOLAND 0x004273f0
void FUN_004273f0(struct TrainSeat *param_1) {
    struct CastleVisitor *obj = param_1->occupant;
    if (obj != NULL) {
        obj->unlink(obj);
    }
}

// FUNCTION: LEGOLAND 0x00427410
void DrawTrainSeat(struct TrainSeat *obj, struct CarXform *xf) {
    struct FVec3 old;

    if (obj->occupant != 0) {
        old = xf->pos;
        xf->pos.x = xf->m[6] * obj->pos.z + xf->m[0] * obj->pos.x + xf->m[3] * obj->pos.y + old.x;
        xf->pos.y = xf->m[7] * obj->pos.z + xf->m[1] * obj->pos.x + xf->m[4] * obj->pos.y + old.y;
        xf->pos.z = xf->m[8] * obj->pos.z + xf->m[2] * obj->pos.x + xf->m[5] * obj->pos.y + old.z;
        obj->occupant->draw(obj->occupant, xf);
        xf->pos = old;
    }
}

// FUNCTION: LEGOLAND 0x004274b0
void InitTrainSeat(struct TrainSeat *obj, const struct FVec3 *src) {
    obj->pos = *src;
    obj->occupant = 0;
    /* the original stores this byte-returning function in an unsigned int slot (callers test eax) */
    obj->method_10 = (unsigned int (*)(struct TrainSeat *))IsTrainSeatOccupied;
    obj->method_14 = SetTrainSeatOccupant;
    obj->method_18 = FUN_004273f0;
    obj->draw = DrawTrainSeat;
}

struct MapHeader {
    unsigned short field_0;
};

struct Struct4274f0Coord {
    short field_0;
    short field_2;
};

// FUNCTION: LEGOLAND 0x004274f0
void FUN_004274f0(struct Struct4274f0Coord *coord) {
    if (MapObjectsEnabled != 0) {
        SetMapTile(coord->field_0, coord->field_2, ((struct MapHeader *)BasicTilesData)->field_0 + 1);
        SetMapTile(coord->field_0 + 1, coord->field_2, ((struct MapHeader *)BasicTilesData)->field_0 + 1);
        SetMapTile(coord->field_0, coord->field_2 + 1, ((struct MapHeader *)BasicTilesData)->field_0 + 1);
        SetMapTile(coord->field_0 + 1, coord->field_2 + 1, ((struct MapHeader *)BasicTilesData)->field_0 + 1);
    }
}

// FUNCTION: LEGOLAND 0x004275b0
void FUN_004275b0(void) {}

// FUNCTION: LEGOLAND 0x004275c0
void FUN_004275c0(void) {}

// FUNCTION: LEGOLAND 0x004275d0
void SquareTrackCalcCursor(Element *obj, int *x, unsigned int y) {
    struct SprEnt *result;
    struct PieceClass *key;
    unsigned int bit;
    unsigned int *mask;
    struct Cursor *c;
    struct Ride *ride = obj->ride;
    struct Footprint *fp = &ride->footprint;
    int n = 0;
    int i;
    int found = 0;

    EditCursor.footprint = *fp;
    EditCursor.field_1830 = 0;
    ScreenToMapRef((int *)x, &EditCursor.tile_x, y);
    if (CastleCoaster.flags != 2) {
        bit = 1;
        for (i = 0; i <= 3; i++) {
            if ((bit & CastleCoaster.pts_a.mask) && EditCursor.tile_x - CastleCoaster.pts_a.pt[i][0] >= -1 && EditCursor.tile_x - CastleCoaster.pts_a.pt[i][0] <= 1 && (int)EditCursor.tile_y - CastleCoaster.pts_a.pt[i][1] >= -1 && (int)EditCursor.tile_y - CastleCoaster.pts_a.pt[i][1] <= 1) {
                EditCursor.tile_x = CastleCoaster.pts_a.pt[i][0];
                EditCursor.tile_y = CastleCoaster.pts_a.pt[i][1];
                found = 1;
                break;
            }
            bit <<= 1;
        }
        if (!found) {
            bit = 1;
            for (i = 0; i <= 3; i++) {
                if ((bit & CastleCoaster.pts_b.mask) && EditCursor.tile_x - CastleCoaster.pts_b.pt[i][0] >= -1 && EditCursor.tile_x - CastleCoaster.pts_b.pt[i][0] <= 1 && (int)EditCursor.tile_y - CastleCoaster.pts_b.pt[i][1] >= -1 && (int)EditCursor.tile_y - CastleCoaster.pts_b.pt[i][1] <= 1) {
                    EditCursor.tile_x = CastleCoaster.pts_b.pt[i][0];
                    EditCursor.tile_y = CastleCoaster.pts_b.pt[i][1];
                    found = 1;
                    break;
                }
                bit <<= 1;
            }
        }
        ValidateCursor(&EditCursor, ride);
        bit = 1;
        for (i = 0; i <= 3; i++) {
            if (bit & CastleCoaster.pts_a.mask) {
                DefaultCursor(&DAT_0081ce00[n]);
                DAT_0081ce00[n].next = EditCursor.next;
                EditCursor.next = &DAT_0081ce00[n];
                DAT_0081ce00[n].tile_x = CastleCoaster.pts_a.pt[i][0];
                DAT_0081ce00[n].tile_y = CastleCoaster.pts_a.pt[i][1];
                DAT_0081ce00[n].footprint = *fp;
                DAT_0081ce00[n].field_1828 = 0x2032;
                ResetCursorBuildable(&DAT_0081ce00[n]);
                n++;
            }
            bit <<= 1;
        }
        bit = 1;
        for (i = 0; i <= 3; i++) {
            if (bit & CastleCoaster.pts_b.mask) {
                DefaultCursor(&DAT_0081ce00[n]);
                DAT_0081ce00[n].next = EditCursor.next;
                EditCursor.next = &DAT_0081ce00[n];
                DAT_0081ce00[n].tile_x = CastleCoaster.pts_b.pt[i][0];
                DAT_0081ce00[n].tile_y = CastleCoaster.pts_b.pt[i][1];
                DAT_0081ce00[n].footprint = *fp;
                DAT_0081ce00[n].field_1828 = 0x2032;
                ResetCursorBuildable(&DAT_0081ce00[n]);
                n++;
            }
            bit <<= 1;
        }
        {
            struct Int16Pair pair;
            pair.x = (unsigned short)EditCursor.tile_x;
            pair.y = (unsigned short)EditCursor.tile_y;
            key = FindPieceClassForElement(obj);
            if (key == 0) {
                key = &DAT_004b5d20;
            }
            result = (struct SprEnt *)CheckCoasterPiecePlacement(key, &pair);
            if (result->field_0 == 0) {
                SetCursorBlocked(&EditCursor, 0xe);
            }
        }
        if (IsCursorBuildable(&EditCursor)) {
            SetCursorBlocked(&EditCursor, 0xe);
            bit = 1;
            mask = &CastleCoaster.pts_a.mask;
            c = DAT_0081ce00;
            for (i = 0; i < 8; i++) {
                if (i == 4) {
                    bit = 1;
                    mask = &CastleCoaster.pts_b.mask;
                }
                if (bit & *mask) {
                    if (EditCursor.tile_x == c->tile_x && EditCursor.tile_y == c->tile_y) {
                        ResetCursorBuildable(&EditCursor);
                        return;
                    }
                    c++;
                }
                bit <<= 1;
            }
        }
    } else {
        SetCursorBlocked(&EditCursor, 0xe);
    }
}

// FUNCTION: LEGOLAND 0x00427940
void FUN_00427940(struct Element *param_1) {
    struct Ride *value = param_1->ride;
    EditMode.unk0 = 1;
    EditMode.unk8 = value;
    SetEditCursorFootPrint(&value->footprint);
    EditCursor.field_1830 = 0;
}

struct Struct427970Src {
    short field_0;
    short pad_2;
    short field_4;
};

// FUNCTION: LEGOLAND 0x00427970
void FUN_00427970(Element *obj, const struct Struct427970Src *src) {
    struct Ride *key = obj->ride;
    struct Int16Pair pair;
    struct PathSeg *node;

    BasicObjectDCalcCursor(obj, (struct Point *)src);
    pair.x = src->field_0;
    pair.y = src->field_4;
    node = FindPathSegAt((struct CastleState *)&CastleCoaster, key, (short *)&pair);
    if (IsCoasterEndPiece(node) == 0) {
        SetCursorBlocked(&QueryCursor, 1);
        CoasterPieceToRemove = 0;
    } else {
        ResetCursorBuildable(&QueryCursor);
        CoasterPieceToRemove = node;
    }
}

// FUNCTION: LEGOLAND 0x004279f0
void RemoveQueriedCoasterPiece(void) {
    RemoveCoasterPiece(CoasterPieceToRemove);
}

// FUNCTION: LEGOLAND 0x00427a00
void RenderSquareTrack(struct Element *unused0, unsigned int unused1, unsigned int unused2, const unsigned char *src) {
    struct PieceHead *node;
    struct Int16Pair tile;
    tile.x = src[0];
    tile.y = src[1];
    node = (struct PieceHead *)FindPathSegByXY((struct CastleState *)&CastleCoaster, (unsigned int *)&tile);
    FUN_00428700(node);
    DrawCastleTrainOnPiece(node);
}

struct Struct427a40 {
    unsigned char pad_0[0x14];
    unsigned int field_14;
    unsigned int field_18;
    unsigned int field_1c;
    unsigned int field_20;
    unsigned int field_24;
};

// FUNCTION: LEGOLAND 0x00427a40
void FUN_00427a40(struct Struct427a40 *param_1) {
    if (param_1->field_20 == 0xffffffff) {
        param_1->field_20 = GetOppositeDirection(param_1->field_14);
        param_1->field_24 = param_1->field_18;
    } else if (param_1->field_14 == 0xffffffff) {
        param_1->field_14 = GetOppositeDirection(param_1->field_20);
        param_1->field_18 = param_1->field_24;
    }
}

struct Struct427a80 {
    unsigned char pad_0[0x18];
    unsigned int field_18;
    unsigned char pad_1c[0x24 - 0x1c];
    float field_24;
};

// FUNCTION: LEGOLAND 0x00427a80
void FUN_00427a80(struct Struct427a80 *param_1, float param_2) {
    param_1->field_24 = param_2;
    param_1->field_18 = *(unsigned int *)&param_2;
}

// FUNCTION: LEGOLAND 0x00427aa0
void CastleLoadBasicTiles(Element *obj) {
    struct Element *handle;

    DAT_00829c08 = obj->ride;
    DAT_00829c08->layer->flags |= 0x2000;
    // STRING: LEGOLAND 0x004b5f34
    if (LLIDB_FindElement("BASIC TILES 1", &handle, 0) == 0) {
        BasicTilesData = LLIDB_LoadData((void *)handle);
    }
    FUN_004284d0();
    FUN_00428750();
}

// FUNCTION: LEGOLAND 0x00427af0
void CastleUnloadBasicTiles(void) {
    struct Element *handle;
    unsigned int packed;
    // STRING: LEGOLAND 0x004b5f34
    if (LLIDB_FindElement("BASIC TILES 1", &handle, 0) == 0) {
        LLIDB_UnLoadData(handle);
    }
}

// FUNCTION: LEGOLAND 0x00427b20
void FUN_00427b20(Element *obj, int *x, unsigned int y) {
    struct Int16Pair tile;
    struct SprEnt *result;

    SetEditCursorFootPrint(&obj->ride->footprint);
    ScreenToMapRef((int *)x, &EditCursor.tile_x, y);
    ValidateCursor(&EditCursor, obj->data);
    EditCursor.field_1828 |= 8;
    tile.x = (unsigned short)EditCursor.tile_x;
    tile.y = (unsigned short)EditCursor.tile_y;
    result = CheckCoasterPiecePlacement(&DAT_004b5d20, &tile);
    if (result->field_0 != 0) {
        ResetCursorBuildable(&EditCursor);
    } else {
        SetCursorBlocked(&EditCursor, 0xe);
    }
}

struct Struct427bc0Src {
    short field_0;
    short pad_2;
    short field_4;
};

// FUNCTION: LEGOLAND 0x00427bc0
void FUN_00427bc0(struct Element *unused, const struct Struct427bc0Src *src) {
    struct Int16Pair pair;
    struct PathSeg *entry;
    pair.x = src->field_0;
    pair.y = src->field_4;
    entry = AddCoasterPiece(DAT_00829c08, &DAT_004b5d20, &pair);
    if (entry != NULL) {
        entry->flags |= 6;
    }
}

// FUNCTION: LEGOLAND 0x00427c00
struct PieceClass *FindPieceClassForElement(struct Element *key) {
    int i;
    for (i = 0; i < (int)DAT_00611958; i++) {
        if (DAT_00611688[i].key == key)
            return DAT_00611688[i].value;
    }
    return 0;
}

struct Struct427c30 {
    unsigned char pad_0[0x14];
    unsigned int field_14;
    unsigned int field_18;
    unsigned char pad_1c[0x20 - 0x1c];
    unsigned int field_20;
    unsigned int field_24;
};

// FUNCTION: LEGOLAND 0x00427c30
void FUN_00427c30(struct Struct427c30 *param_1) {
    if (param_1->field_20 == 0xffffffff) {
        param_1->field_20 = GetOppositeDirection(param_1->field_14);
        param_1->field_24 = param_1->field_18;
        return;
    }
    if (param_1->field_14 == 0xffffffff) {
        param_1->field_14 = GetOppositeDirection(param_1->field_20);
        param_1->field_18 = param_1->field_24;
    }
}

struct Struct427c70Inner {
    unsigned char pad_0[4];
    int field_4;
    int field_8;
};

struct Struct427c70 {
    unsigned char pad_0[0xc];
    struct Struct427c70Inner *field_c;
    unsigned char pad_10[0x18 - 0x10];
    float field_18;
    unsigned char pad_1c[0x24 - 0x1c];
    float field_24;
};

// FUNCTION: LEGOLAND 0x00427c70
void FUN_00427c70(struct Struct427c70 *param_1) {
    struct Struct427c70Inner *inner = param_1->field_c;
    param_1->field_24 = (float)inner->field_4;
    param_1->field_18 = (float)inner->field_8;
}

// FUNCTION: LEGOLAND 0x00427c90
void FUN_00427c90(Element *obj, int *x, unsigned int y) {
    struct Int16Pair pair;
    struct SprEnt *result;
    struct PieceClass *key;
    unsigned int bit;
    int n;
    struct Ride *ride = obj->ride;
    int i;

    SetEditCursorFootPrint(&ride->footprint);
    EditCursor.field_1830 = 0;
    ScreenToMapRef((int *)x, &EditCursor.tile_x, y);
    n = 0;
    bit = 1;
    for (i = 0; i <= 3; i++) {
        if (bit & CastleCoaster.pts_a.mask) {
            DefaultCursor(&DAT_0081ce00[n]);
            DAT_0081ce00[n].next = EditCursor.next;
            EditCursor.next = &DAT_0081ce00[n];
            DAT_0081ce00[n].tile_x = CastleCoaster.pts_a.pt[i][0];
            DAT_0081ce00[n].tile_y = CastleCoaster.pts_a.pt[i][1];
            DAT_0081ce00[n].footprint = ride->footprint;
            DAT_0081ce00[n].field_1828 = 0x2032;
            ResetCursorBuildable(&DAT_0081ce00[n]);
            n++;
        }
        bit <<= 1;
    }
    bit = 1;
    for (i = 0; i <= 3; i++) {
        if (bit & CastleCoaster.pts_b.mask) {
            DefaultCursor(&DAT_0081ce00[n]);
            DAT_0081ce00[n].next = EditCursor.next;
            EditCursor.next = &DAT_0081ce00[n];
            DAT_0081ce00[n].tile_x = CastleCoaster.pts_b.pt[i][0];
            DAT_0081ce00[n].tile_y = CastleCoaster.pts_b.pt[i][1];
            DAT_0081ce00[n].footprint = ride->footprint;
            DAT_0081ce00[n].field_1828 = 0x2032;
            ResetCursorBuildable(&DAT_0081ce00[n]);
            n++;
        }
        bit <<= 1;
    }
    ValidateCursor(&EditCursor, obj->data);
    pair.x = (unsigned short)EditCursor.tile_x;
    pair.y = (unsigned short)EditCursor.tile_y;
    EditCursor.field_1828 |= 8;
    key = FindPieceClassForElement(obj);
    if (key != 0) {
        result = CheckCoasterPiecePlacement(key, &pair);
        if (result->field_0 != 0) {
            ResetCursorBuildable(&EditCursor);
        } else {
            SetCursorBlocked(&EditCursor, 0xe);
        }
    }
}

// FUNCTION: LEGOLAND 0x00427ea0
void FUN_00427ea0(Element *obj, const struct Struct427bc0Src *src) {
    struct Int16Pair pair;
    struct Ride *ride = obj->ride;
    struct PieceClass *key;

    pair.x = src->field_0;
    pair.y = src->field_4;
    key = FindPieceClassForElement(obj);
    if (key != 0) {
        AddCoasterPiece(ride, key, &pair)->flags |= 6;
    }
}

// FUNCTION: LEGOLAND 0x00427ef0
void SquareTrackHeightLoad(Element *param_1) {
    struct Ride *a = param_1->ride;
    DAT_00829bfc = a;
    a->layer->flags |= 0x2000;
    DAT_00611688[DAT_00611958].key = param_1;
    DAT_00611688[DAT_00611958].value = &DAT_004b5d58;
    DAT_00611958++;
}

// FUNCTION: LEGOLAND 0x00427f30
void SquareTrackHeight0Load(Element *param_1) {
    struct Ride *a = param_1->ride;
    DAT_00829a64 = a;
    a->layer->flags |= 0x2000;
    DAT_00611688[DAT_00611958].key = param_1;
    DAT_00611688[DAT_00611958].value = &DAT_004b5d90;
    DAT_00611958++;
}

struct Struct427f70Arg {
    unsigned char pad_0[4];
    short field_4;
    short field_6;
};

// FUNCTION: LEGOLAND 0x00427f70
void RemoveRollerCoasterPath2x2(const struct Struct427f70Arg *arg) {
    int coords[2];

    coords[0] = arg->field_4;
    coords[1] = arg->field_6;
    RemoveRollerCoasterPath(coords);

    coords[0] = arg->field_4 + 1;
    coords[1] = arg->field_6;
    RemoveRollerCoasterPath(coords);

    coords[0] = arg->field_4 + 1;
    coords[1] = arg->field_6 + 1;
    RemoveRollerCoasterPath(coords);

    coords[0] = arg->field_4;
    coords[1] = arg->field_6 + 1;
    RemoveRollerCoasterPath(coords);
}

struct Struct427ff0Point {
    unsigned short pad_0;
    unsigned short pad_2;
    short x;
    short y;
};

// FUNCTION: LEGOLAND 0x00427ff0
void AddRollerCoasterPath2x2(struct Struct427ff0Point *point) {
    int coords[2];

    coords[0] = point->x;
    coords[1] = point->y;
    AddRollerCoasterPath(coords);

    coords[0] = point->x + 1;
    coords[1] = point->y;
    AddRollerCoasterPath(coords);

    coords[0] = point->x + 1;
    coords[1] = point->y + 1;
    AddRollerCoasterPath(coords);

    coords[0] = point->x;
    coords[1] = point->y + 1;
    AddRollerCoasterPath(coords);
}

// FUNCTION: LEGOLAND 0x00428070
void SquareTrackHeightPathLoad(Element *param_1) {
    struct Ride *a = param_1->ride;
    a->layer->flags |= 0x2000;
    DAT_00611688[DAT_00611958].key = param_1;
    DAT_00611688[DAT_00611958].value = &DAT_004b5dc8;
    DAT_00611958++;
}

/* Placement-cursor callback for SQUARE_TRACK_HEIGHT_PATH. Add cursors for the enabled
 * points in both point sets, then validate the footprint and the selected track piece. */
// FUNCTION: LEGOLAND 0x004280b0
void FUN_004280b0(Element *obj, int *x, unsigned int y) {
    int total;
    struct Int16Pair pair;
    int a;
    int b;
    struct SprEnt *result;
    struct PieceClass *key;
    unsigned int bit;
    int n;
    struct Ride *ride;
    struct Footprint *fp;
    int i;

    /* Capture the ride before clearing the cursor chain, and calculate the footprint
     * address afterward. This order reproduces MSVC6's original setup instructions. */
    ride = obj->ride;
    total = 0;
    EditCursor.field_1830 = 0;
    fp = &ride->footprint;
    SetEditCursorFootPrint(fp);
    ScreenToMapRef((int *)x, &EditCursor.tile_x, y);
    n = 0;
    bit = 1;
    for (i = 0; i <= 3; i++) {
        if (bit & CastleCoaster.pts_a.mask) {
            DefaultCursor(&DAT_0081ce00[n]);
            DAT_0081ce00[n].next = EditCursor.next;
            EditCursor.next = &DAT_0081ce00[n];
            DAT_0081ce00[n].tile_x = CastleCoaster.pts_a.pt[i][0];
            DAT_0081ce00[n].tile_y = CastleCoaster.pts_a.pt[i][1];
            DAT_0081ce00[n].footprint = *fp;
            DAT_0081ce00[n].field_1828 = 0x2032;
            ResetCursorBuildable(&DAT_0081ce00[n]);
            n++;
        }
        bit <<= 1;
    }
    bit = 1;
    for (i = 0; i <= 3; i++) {
        if (bit & CastleCoaster.pts_b.mask) {
            DefaultCursor(&DAT_0081ce00[n]);
            DAT_0081ce00[n].next = EditCursor.next;
            EditCursor.next = &DAT_0081ce00[n];
            DAT_0081ce00[n].tile_x = CastleCoaster.pts_b.pt[i][0];
            DAT_0081ce00[n].tile_y = CastleCoaster.pts_b.pt[i][1];
            DAT_0081ce00[n].footprint = *fp;
            DAT_0081ce00[n].field_1828 = 0x2032;
            ResetCursorBuildable(&DAT_0081ce00[n]);
            n++;
        }
        bit <<= 1;
    }
    ValidateCursor(&EditCursor, obj->data);
    pair.x = (unsigned short)EditCursor.tile_x;
    pair.y = (unsigned short)EditCursor.tile_y;
    EditCursor.field_1828 |= 8;
    key = FindPieceClassForElement(obj);
    if (key != 0) {
        result = CheckCoasterPiecePlacement(key, &pair);
        if (result->field_0 == 0) {
            SetCursorBlocked(&EditCursor, 0xe);
        } else {
            FUN_0041cf70(result->owner, &a, &b);
            if (a >= 0) {
                total = a;
            }
            if (b >= 0) {
                total += b;
            }
            if (total >= 2) {
                SetCursorBlocked(&EditCursor, 0xb);
            } else {
                ResetCursorBuildable(&EditCursor);
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x00428300
void FUN_00428300(Element *obj, const struct Struct427bc0Src *src) {
    struct Int16Pair pair;
    struct Ride *ride = obj->ride;
    struct PieceClass *key;

    pair.x = src->field_0;
    pair.y = src->field_4;
    key = FindPieceClassForElement(obj);
    if (key != 0) {
        AddCoasterPiece(ride, key, &pair)->flags &= ~6;
    }
}

// FUNCTION: LEGOLAND 0x00428350
void FUN_00428350(int a, int b, int c, struct Obj421ab0 *obj, int d) {
    Vector3 pt;

    pt.x = DAT_006117c0[b].x;
    pt.y = DAT_006117c0[b].y;
    pt.z = DAT_006117c0[b].z - (float)(c * 6);
    InitLineCurve(obj, &DAT_006117c0[a].x, &pt.x, (struct Words3 *)&DAT_00611658[d]);
}

// FUNCTION: LEGOLAND 0x004283c0
unsigned int FUN_004283c0(struct PieceHead *pair) {
    unsigned int kind = FUN_0041ce60(pair);
    int v = (((int)pair->end_b.height - (int)pair->end_a.height) >> 1) + 2;

    if (pair->end_a.dir == GetOppositeDirection(pair->end_b.dir)) {
        if (kind == 8) {
            v += kind;
        } else if (kind == 2) {
            v += 0xd;
        } else if (kind == 0xd) {
            v += 0x12;
        } else if (kind == 7) {
            v += 0x17;
        }
        return v;
    }
    switch (kind) {
    case 12:
        return 0;
    case 3:
        return 1;
    case 4:
        return 2;
    case 1:
        return 3;
    case 6:
        return 4;
    case 9:
        return 5;
    case 14:
        return 6;
    case 11:
        return 7;
    }
    return 0xffffffff;
}

// FUNCTION: LEGOLAND 0x004284d0
void FUN_004284d0(void) {
    struct Obj421ce0 *base;
    register struct Obj421ce0 *cur;
    struct Words3 *p;
    struct Words3 *q;
    int x;
    unsigned int y;
    int j;
    register int qi;
    register unsigned int i;
    unsigned int k;
    int n;
    int ptmp22;
    n = 0;

    for (i = 0; i <= 3; i++) {
        DAT_006117c0[i].x = DAT_004b5e00[i][0] * 20.0f;
        DAT_006117c0[i].y = DAT_004b5e00[i][1] * 20.0f;
        DAT_006117c0[i].z = DAT_004b5e00[i][2] * 20.0f;
    }
    for (i = 0; i <= 3; i++) {
        DAT_00611658[i].x = DAT_004b5e30[i][0] * 20.0f;
        DAT_00611658[i].y = DAT_004b5e30[i][1] * 20.0f;
        DAT_00611658[i].z = DAT_004b5e30[i][2] * 20.0f;
    }
    i = 0;
    while (3 >= i) {
        DAT_00611750[i].x = DAT_004b5e60[i][0] * 20.0f;
        DAT_00611750[i].y = DAT_004b5e60[i][1] * 20.0f;
        DAT_00611750[i].z = DAT_004b5e60[i][2] * 20.0f;
        i++;
    }
    i = 0;
    while (i <= 3) {
        DAT_006116e0[i].x = DAT_004b5e90[i][0] * 20.0f;
        DAT_006116e0[i].y = DAT_004b5e90[i][1] * 20.0f;
        { DAT_006116e0[i].z = DAT_004b5e90[i][2] * 20.0f; }
        ++i;
    }
    i = 0;
    while (3 >= i) {
        ptmp22 = DAT_004b5ef4[i][1];
        qi = ptmp22;
        q = (struct Words3 *)&DAT_00611750[qi];
        y = DAT_004b5f14[i][1];
        p = (struct Words3 *)&DAT_00611750[DAT_004b5ef4[i][0]];
        x = DAT_004b5f14[i][0];
        FUN_00421ce0(p, q, (struct Words3 *)&DAT_006116e0[i], &((struct Obj421ce0 *)DAT_00828fe0)[n++], x, y);
        if (5 == n) {
            n = 5;
        }
        FUN_00421ce0(q, p, (struct Words3 *)&DAT_006116e0[i], &((struct Obj421ce0 *)DAT_00828fe0)[n++], y, x);
        i++;
    }
    j = 0;
    base = &((struct Obj421ce0 *)DAT_00828fe0)[n];
    while (j <= 3) {
        cur = base;
        base = 5 + base;
        for (k = 0; k < 5; k++) {
            FUN_00428350(DAT_004b5ec0[j], (DAT_004b5ec0[j] - 2) & 3, DAT_004b5ee0[k], (struct Obj421ab0 *)cur++, j);
        }
        j++;
    }
}

struct Struct4286e0Element {
    unsigned int field_0;
    unsigned int field_4;
};

// FUNCTION: LEGOLAND 0x004286e0
struct Struct4286e0Element *FUN_004286e0(void *param1) {
    unsigned int result = FUN_004283c0(param1);
    unsigned int index = result + (result + result * 4) * 2;
    return &((struct Struct4286e0Element *)DAT_00828fe0)[index];
}

// FUNCTION: LEGOLAND 0x00428700
void FUN_00428700(void *node) {
    struct FVec3 buf;
    unsigned int idx = FUN_0041ce60((struct PieceHead *)node);
    struct SegBlob *r = FUN_0041cff0(node, &buf);
    if (*(unsigned char *)node & 6) {
        FUN_004294f0(r, &buf, DAT_00611710[idx], 0);
    } else {
        FUN_004294f0(r, &buf, DAT_00611710[idx], 1);
    }
}

struct DirRule {
    unsigned int from;
    unsigned int to;
    unsigned int kind;
};

struct AnimPairBuf {
    struct PieceHead pair;
    unsigned char pad_2c[0xa4 - 0x2c];
};

// FUNCTION: LEGOLAND 0x00428750
void FUN_00428750(void) {
    int n;
    struct DirRule table[6];
    struct AnimPairBuf buf;

    table[0].from = 1;
    table[0].to = 4;
    table[0].kind = 2;
    table[1].from = 2;
    table[1].to = 8;
    table[1].kind = 2;
    table[2].from = 1;
    table[2].to = 2;
    table[2].kind = 1;
    table[3].from = 2;
    table[3].to = 4;
    table[3].kind = 2;
    table[4].from = 4;
    table[4].to = 8;
    table[4].kind = 1;
    table[5].from = 8;
    table[5].to = 1;
    table[5].kind = 1;
    for (n = 0; n < 6; n++) {
        unsigned int idx;

        buf.pair.end_a.dir = table[n].from;
        buf.pair.end_b.dir = table[n].to;
        DAT_00611710[FUN_0041ce60(&buf.pair)] = table[n].kind;
        buf.pair.end_a.dir = table[n].to;
        buf.pair.end_b.dir = table[n].from;
        idx = FUN_0041ce60(&buf.pair);
        if (table[n].kind == 2) {
            DAT_00611710[idx] = 1;
        } else {
            DAT_00611710[idx] = 2;
        }
    }
}

// FUNCTION: LEGOLAND 0x00428840
unsigned int LowestSetBitIndex(unsigned int param_1) {
    unsigned int mask = 1;
    unsigned int result = 0;
    if ((param_1 & 1) == 0) {
        while ((param_1 & mask) == 0) {
            mask = mask * 2;
            result = result + 1;
        }
    }
    return result;
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x00428860
__declspec(naked) void FUN_00428860(void) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x70
        mov eax, dword ptr [ebp + 0x14]
        mov edx, dword ptr [ebp + 0x10]
        push ebx
        push esi
        mov ecx, dword ptr [eax]
        push edi
        mov dword ptr [ebp - 4], ecx
        lea ecx, [eax + edx*8 - 4]
        mov edx, dword ptr [ebp + 0x18]
        mov dword ptr [ebp - 8], ecx
        mov eax, dword ptr [ecx]
        lea eax, [eax + eax*2]
        shl eax, 4
        inc word ptr [eax + edx + 2]
        lea eax, [eax + edx + 2]
        mov eax, dword ptr [ecx]
        lea ecx, [eax + eax*2]
        mov eax, dword ptr [CoasterSurfaceBits]
        shl ecx, 4
        mov dword ptr [ebp - 0x20], eax
        mov eax, dword ptr [ebp + 8]
        movsx edx, word ptr [ecx + edx + 2]
        mov ecx, dword ptr [DAT_004b5b24]
        mov dword ptr [ebp - 0x14], edx
        mov edx, dword ptr [CoasterSurfacePitch]
        push eax
        mov dword ptr [ebp - 0x1c], ecx
        mov dword ptr [ebp - 0x18], edx
        call GetLtxFileTableEntry
        mov esi, eax
        add esp, 4
        test esi, esi
        je L428b5c
        mov ecx, dword ptr [esi]
        push ecx
        call LowestSetBitIndex
        mov edx, dword ptr [esi + 4]
        mov edi, eax
        push edx
        call LowestSetBitIndex
        add esi, 8
        mov ecx, 8
        mov dword ptr [ebp + 8], esi
        sub ecx, edi
        mov esi, 0xff000000
        mov edx, ecx
        sar esi, cl
        sub edx, eax
        mov dword ptr [ebp - 0xc], ecx
        add edx, 8
        add esp, 8
        mov dword ptr [ebp - 0x10], edx
        mov dword ptr [DAT_0061195c], esi
        mov esi, dword ptr [ebp + 0xc]
        mov ebx, dword ptr [esi + 4]
        sar ebx, cl
        mov ecx, edx
        mov edx, 1
        mov dword ptr [DAT_00612160], ebx
        mov ebx, dword ptr [esi + 8]
        shl ebx, 8
        sar ebx, cl
        mov dword ptr [DAT_00612160 + 0x4], ebx
        mov ecx, dword ptr [esi + 0xc]
        mov dword ptr [DAT_00612160 + 0xc], ecx
        lea ecx, [eax + edi]
        mov eax, dword ptr [DAT_0060f900]
        shl edx, cl
        dec edx
        inc eax
        mov dword ptr [DAT_0060f900], eax
        mov eax, dword ptr [ebp - 8]
        mov dword ptr [DAT_00612160 + 0x14], edx
        mov edx, dword ptr [ebp + 0x18]
        mov eax, dword ptr [eax]
        lea ecx, [eax + eax*2]
        shl ecx, 4
        movsx eax, word ptr [ecx + edx + 2]
        mov ecx, dword ptr [ebp + 0x10]
        mov edx, dword ptr [ebp + 0x14]
        mov dword ptr [edx + ecx*8], eax
        mov eax, dword ptr [ebp - 0x18]
        imul eax, dword ptr [ebp - 4]
        mov edx, dword ptr [ebp - 0x20]
        mov ecx, dword ptr [ebp - 0x1c]
        shl eax, 1
        add edx, eax
        add ecx, eax
        mov dword ptr [ebp - 0x20], edx
        mov edx, dword ptr [DAT_0060f904]
        test edx, edx
        mov dword ptr [ebp - 0x1c], ecx
        jle L4289ad
        mov ecx, offset DAT_00611960
        mov eax, offset DAT_00829c60
L428998:
        mov edi, dword ptr [esi]
        mov ebx, dword ptr [eax]
        add eax, 4
        add ecx, 2
        mov di, word ptr [ebx + edi*2]
        dec edx
        mov word ptr [ecx - 2], di
        jne L428998
L4289ad:
        mov dword ptr [ebp + 0x10], offset DAT_00611960
L4289b4:
        mov ecx, dword ptr [ebp + 0x14]
        mov esi, dword ptr [ebp + 0x18]
        mov eax, dword ptr [ecx + 4]
        add ecx, 8
        mov dword ptr [ebp + 0x14], ecx
        lea eax, [eax + eax*2]
        shl eax, 4
        add eax, esi
        mov ecx, dword ptr [eax + 4]
        test ecx, ecx
        mov ecx, dword ptr [eax + 8]
        je L4289e2
        mov eax, dword ptr [eax + 0x1c]
        sub ecx, eax
        mov dword ptr [ebp - 0x34], eax
        mov dword ptr [ebp - 0x48], ecx
        jmp L428a20
L4289e2:
        mov ebx, dword ptr [eax + 0x10]
        mov edx, dword ptr [eax + 0x1c]
        mov esi, dword ptr [eax + 0xc]
        mov edi, dword ptr [eax + 0x20]
        mov dword ptr [ebp - 0x60], ebx
        mov ebx, dword ptr [eax + 0x24]
        sub ecx, edx
        mov dword ptr [ebp - 0x5c], edx
        mov edx, dword ptr [ebp - 0x60]
        mov dword ptr [ebp - 0x4c], ebx
        mov ebx, dword ptr [eax + 0x14]
        mov eax, dword ptr [eax + 0x28]
        mov dword ptr [ebp - 0x70], ecx
        mov ecx, dword ptr [ebp - 0x4c]
        sub esi, edi
        sub edx, ecx
        sub ebx, eax
        mov dword ptr [ebp - 0x50], edi
        mov dword ptr [ebp - 0x54], eax
        mov dword ptr [ebp - 0x64], esi
        mov dword ptr [ebp - 0x60], edx
        mov dword ptr [ebp - 0x68], ebx
L428a20:
        mov eax, dword ptr [ebp + 0x14]
        mov edx, dword ptr [ebp - 4]
        cmp edx, dword ptr [eax]
        jge L428b4e
        mov ecx, dword ptr [ebp - 0x18]
        lea edx, [ecx + ecx]
        mov dword ptr [ebp + 0xc], edx
L428a37:
        inc dword ptr [ebp - 4]
        mov eax, dword ptr [ebp - 0x70]
        mov ebx, dword ptr [ebp - 0x48]
        add eax, dword ptr [ebp - 0x5c]
        add ebx, dword ptr [ebp - 0x34]
        mov dword ptr [ebp - 0x70], eax
        mov dword ptr [ebp - 0x48], ebx
        mov ecx, ebx
        sub ecx, eax
        cmp ecx, 0x8000
        jns L428a78
        mov eax, dword ptr [ebp - 0x64]
        mov ebx, dword ptr [ebp - 0x60]
        mov ecx, dword ptr [ebp - 0x68]
        add eax, dword ptr [ebp - 0x50]
        add ebx, dword ptr [ebp - 0x4c]
        add ecx, dword ptr [ebp - 0x54]
        mov dword ptr [ebp - 0x64], eax
        mov dword ptr [ebp - 0x60], ebx
        mov dword ptr [ebp - 0x68], ecx
        jmp L428b2b
L428a78:
        sar eax, 0x10
        sar ebx, 0x10
        mov edi, dword ptr [ebp - 0x20]
        mov esi, dword ptr [ebp - 0x1c]
        xchg eax, ebx
        sub ebx, eax
        lea edi, [edi + eax*2]
        lea esi, [esi + eax*2]
        push ebx
        mov eax, dword ptr [ebp - 0x64]
        mov edx, dword ptr [ebp - 0x60]
        mov ebx, dword ptr [ebp - 0x68]
        add eax, dword ptr [ebp - 0x50]
        add edx, dword ptr [ebp - 0x4c]
        add ebx, dword ptr [ebp - 0x54]
        mov dword ptr [ebp - 0x64], eax
        mov dword ptr [ebp - 0x60], edx
        mov dword ptr [ebp - 0x68], ebx
        mov ecx, dword ptr [ebp - 0x10]
        shl edx, 8
        shr edx, cl
        mov ecx, dword ptr [ebp - 0xc]
        sar eax, cl
        push ebp
        sub esp, 8
        mov ecx, esi
        sub ecx, dword ptr [ebp + 8]
        mov dword ptr [esp + 4], ecx
        mov ecx, esi
        sub ecx, dword ptr [ebp + 0x10]
        sar ecx, 1
        mov dword ptr [esp], ecx
        mov ebp, ebx
        mov ebx, dword ptr [esp + 0xc]
L428ad3:
        mov ecx, ebp
        sar ecx, 0x10
        cmp cx, word ptr [esi + ebx*2]
        jb L428b0d
        mov word ptr [esi + ebx*2], cx
        mov ecx, dword ptr [DAT_0061195c]
        and ecx, edx
        or ecx, eax
        shr ecx, 0x10
        and ecx, dword ptr [DAT_00612160 + 0x14]
        sub ecx, dword ptr [esp + 4]
        mov cl, byte ptr [esi + ecx]
        and ecx, 0xff
        sub ecx, dword ptr [esp]
        mov cx, word ptr [esi + ecx*2]
        mov word ptr [edi + ebx*2], cx
L428b0d:
        add eax, dword ptr [DAT_00612160]
        add edx, dword ptr [DAT_00612160 + 0x4]
        add ebp, dword ptr [DAT_00612160 + 0xc]
        add ebx, 1
        jle L428ad3
        add esp, 8
        pop ebp
        add esp, 4
L428b2b:
        mov eax, dword ptr [ebp + 0xc]
        mov edi, dword ptr [ebp - 0x20]
        mov esi, dword ptr [ebp - 0x1c]
        mov ecx, dword ptr [ebp + 0x14]
        add edi, eax
        add esi, eax
        mov eax, dword ptr [ebp - 4]
        mov edx, dword ptr [ecx]
        cmp eax, edx
        mov dword ptr [ebp - 0x20], edi
        mov dword ptr [ebp - 0x1c], esi
        jl L428a37
L428b4e:
        mov edx, dword ptr [ebp - 4]
        mov eax, dword ptr [ebp - 0x14]
        cmp edx, eax
        jl L4289b4
L428b5c:
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// FUNCTION: LEGOLAND 0x00428b70
void FUN_00428b70(void) {
    FUN_00428f00();
    FUN_00429270();
    FUN_004294b0();
}

// FUNCTION: LEGOLAND 0x00428b80
void FUN_00428b80(struct SegBlob *curve, float *off) {
    float step = (curve->end - curve->start) * FLOAT_004ab44c;
    float t = curve->start;
    int i;
    int k = 0;

    for (i = 0; i < 30; i++) {
        curve->vt->method_8(curve, t, DAT_006122a0[k]);
        DAT_006122a0[k][0] += off[0];
        DAT_006122a0[k][1] += off[1];
        DAT_006122a0[k][2] += off[2];
        k++;
        curve->vt->method_0(curve, t, DAT_006122a0[k]);
        DAT_006122a0[k][0] += off[0];
        DAT_006122a0[k][1] += off[1];
        DAT_006122a0[k][2] += off[2];
        k++;
        curve->vt->method_10(curve, t, DAT_006122a0[k]);
        DAT_006122a0[k][0] += off[0];
        DAT_006122a0[k][1] += off[1];
        DAT_006122a0[k][2] += off[2];
        k++;
        t = t + step;
    }
    FUN_00426230(DAT_006122a0, DAT_006159c8, 0x5a);
    PlotClippedPoints(&DAT_006159c8[0][0], 0x5a, -1);
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x00428cb0
__declspec(naked) void FUN_00428cb0(void *p, unsigned int a, unsigned int b, unsigned int c, const char *name) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0xc8
        mov eax, dword ptr [ebp + 0x14]
        push ebx
        push esi
        push edi
        lea ecx, [eax - 1]
        mov dword ptr [ebp - 0x18], ecx
        push eax
        push edx
        rdtsc
        mov dword ptr [ebp - 4], eax
        pop edx
        pop eax
        test ecx, ecx
        jl L428e2a
        mov edx, dword ptr [ebp + 0x18]
        mov edi, dword ptr [ebp + 8]
        mov ebx, offset DAT_006139c8
        inc ecx
        mov dword ptr [ebp + 8], edx
        mov dword ptr [ebp + 0x14], ebx
        mov dword ptr [ebp + 0x18], ecx
        jmp L428cf0
L428ced:
        mov ebx, dword ptr [ebp + 0x14]
L428cf0:
        mov eax, dword ptr [ebp + 8]
        mov edx, dword ptr [edi + 0x4c]
        mov ecx, dword ptr [eax]
        lea eax, [ebp - 0x48]
        mov esi, ecx
        mov dword ptr [ebp - 0x14], ecx
        mov ecx, dword ptr [ebp + 0x10]
        push eax
        push esi
        push edi
        call dword ptr [edx + ecx*8 + 4]
        lea edx, [ebp - 0x3c]
        lea eax, [ebp - 0x48]
        push edx
        push eax
        call BuildBasisFromDir
        mov ecx, dword ptr [edi + 0x4c]
        mov eax, dword ptr [ebp + 0x10]
        lea edx, [ebp - 0x10]
        push edx
        push esi
        push edi
        call dword ptr [ecx + eax*8]
        mov eax, dword ptr [ebp + 0xc]
        lea ecx, [ebp - 0x88]
        fld dword ptr [ebp - 0x10]
        fadd dword ptr [eax]
        lea edx, [ebp - 0x3c]
        push ecx
        push edx
        fstp dword ptr [ebp - 0x10]
        fld dword ptr [ebp - 0xc]
        fadd dword ptr [eax + 4]
        fstp dword ptr [ebp - 0xc]
        fld dword ptr [ebp - 8]
        fadd dword ptr [eax + 8]
        lea eax, [ebp - 0x10]
        push eax
        fstp dword ptr [ebp - 8]
        call MakeMat4FromRotTrans
        lea ecx, [ebp - 0xc8]
        lea edx, [ebp - 0x88]
        push ecx
        push edx
        push offset DAT_008299fc
        call Mat4Multiply
        push 6
        lea eax, [ebp - 0xc8]
        push 0x14
        push eax
        push ebx
        push offset DAT_006121c8
        call FUN_00426250
        fld dword ptr [DAT_004b5ca8]
        fmul dword ptr [ebp - 0x28]
        fld dword ptr [DAT_004b5ca4]
        fmul dword ptr [ebp - 0x2c]
        add esp, 0x4c
        mov esi, offset DAT_006126d8
        add ebx, 0x10
        faddp st(1), st
        fld dword ptr [DAT_004b5ca0]
        fmul dword ptr [ebp - 0x30]
        faddp st(1), st
        fmul dword ptr [DAT_0082999c]
        fld dword ptr [DAT_004b5ca8]
        fmul dword ptr [ebp - 0x1c]
        fld dword ptr [DAT_004b5ca4]
        fmul dword ptr [ebp - 0x20]
        faddp st(1), st
        fld dword ptr [DAT_004b5ca0]
        fmul dword ptr [ebp - 0x24]
        faddp st(1), st
        fmul dword ptr [DAT_0082999c]
        fld dword ptr [DAT_00829a60]
L428de0:
        fld st(1)
        fmul dword ptr [esi + 4]
        fld st(3)
        fmul dword ptr [esi]
        faddp st(1), st
        fadd st, st(1)
        call _ftol
        mov dword ptr [ebx], eax
        add esi, 8
        add ebx, 0x14
        cmp esi, offset DAT_00612708
        jl L428de0
        mov edx, dword ptr [ebp + 0x14]
        mov ecx, dword ptr [ebp + 8]
        mov eax, dword ptr [ebp + 0x18]
        add edx, 0x78
        fstp st(0)
        add ecx, 4
        dec eax
        fstp st(0)
        fstp st(0)
        mov dword ptr [ebp + 0x14], edx
        mov dword ptr [ebp + 8], ecx
        mov dword ptr [ebp + 0x18], eax
        jne L428ced
        mov ecx, dword ptr [ebp - 0x18]
L428e2a:
        lea edx, [ecx + ecx*8]
        lea ecx, [ecx + ecx*2]
        shl ecx, 2
        lea eax, [edx + edx + 6]
        mov dword ptr [DAT_004b5f60 + 0xc], ecx
        mov dword ptr [DAT_004b5f60 + 0x8], eax
        push eax
        push edx
        rdtsc
        sub eax, dword ptr [ebp - 4]
        mov dword ptr [ebp - 4], eax
        pop edx
        pop eax
        mov edx, dword ptr [ebp - 4]
        mov eax, dword ptr [DAT_00615f68]
        add eax, edx
        pop edi
        mov dword ptr [DAT_00615f68], eax
        pop esi
        mov eax, offset DAT_004b5f60
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// FUNCTION: LEGOLAND 0x00428e70
void FUN_00428e70(struct SegBlob *p, const struct FVec3 *a, unsigned int b) {
    unsigned short handle = FPUSetSinglePrecision();
    unsigned int result = ((unsigned int (**)(void *, const char *))p->vt)[6](p, (const char *)DAT_00612178);
    DAT_00615f6c = result;
    FUN_00428cb0(p, a, b, result, DAT_00612178);
    FUN_004234e0(DAT_004b5f60);
    FPURestoreControlWord(handle);
}

// FUNCTION: LEGOLAND 0x00428ec0
void FUN_00428ec0(struct SegBlob *p, const struct FVec3 *a, unsigned int b) {
    unsigned short handle = FPUSetSinglePrecision();
    FUN_00428cb0(p, a, b, DAT_00615f6c, DAT_00612178);
    FUN_004234e0(DAT_004b5f60);
    FPURestoreControlWord(handle);
}

// FUNCTION: LEGOLAND 0x00428f00
void FUN_00428f00(void) {
    unsigned int *volatile ptmp7;
    unsigned int *ptmp8;
    int i, j, k, m, n, t, e, v;
    register float ang;
    register int cnt;
    float s;
    unsigned int *g, *out;
    int (*d)[2];
    int (*sp)[2];
    int k6, k18;

    for (i = 0; 4 >= i; i++) {
        DAT_00613908[i][0] = i;
        DAT_00613908[i][1] = i + 1;
    }
    DAT_00613908[5][0] = i;
    DAT_00613908[5][1] = 0;
    n = 6;
    for (j = 0; j <= 4; j++) {
        DAT_00613908[n][0] = j;
        DAT_00613908[n][1] = j + 6;
        n++;
        DAT_00613908[n][0] = j;
        DAT_00613908[n][1] = 7 + j;
        n++;
    }
    DAT_00613908[n][0] = j;
    DAT_00613908[n][1] = j + 6;
    n++;
    DAT_00613908[n][0] = j;
    DAT_00613908[n][1] = 6;
    n++;
    for (j = 0; j <= 4; j++) {
        DAT_00613908[n][0] = j + 6;
        DAT_00613908[n][1] = 7 + j;
        n++;
    }
    DAT_00613908[n][0] = j + 6;
    DAT_00613908[n][1] = 6;

    t = 0;
    v = 6;
    for (e = 18; 23 > e; e++) {
        DAT_00613878[t * 3] = e - 18 + 0x80000000;
        DAT_00613878[t * 3 + 1] = v + 1;
        DAT_00613878[t * 3 + 2] = v + 2 + 0x80000000;
        t++;
        DAT_00613878[t * 3] = v;
        DAT_00613878[t * 3 + 1] = e;
        DAT_00613878[t * 3 + 2] = v + 1 + 0x80000000;
        t++;
        v += 2;
    }
    DAT_00613878[t * 3] = 0x80000005;
    DAT_00613878[t * 3 + 1] = v + 1;
    DAT_00613878[t * 3 + 2] = 0x80000006;
    DAT_00613878[t * 3 + 3] = v;
    DAT_00613878[t * 3 + 4] = e;
    DAT_00613878[t * 3 + 5] = v + 1 + 0x80000000;

    n = 0;
    ptmp8 = DAT_00612708[0];
    ptmp7 = ptmp8;
    g = ptmp7;
    k = 0;
    while (k < 30) {
        k6 = k * 6;
        k18 = k * 18;
        cnt = 18;
        if (k == 29) {
            cnt = 24;
        }
        d = &DAT_006148b8[n];
        sp = DAT_00613908;
        n += cnt;
        do {
            (*d)[0] = (*sp)[0] + k6;
            (*d)[1] = (*sp)[1] + k6;
            d++;
            sp++;
        } while (--cnt);
        out = g;
        g = g + 36;
        for (i = 0; i < 12; i++) {
            for (j = 0; 3 > j; j++) {
                unsigned int x = DAT_00613878[i * 3 + j];
                *out++ = ((x & 0x7fffffff) + k18) | (x & 0x80000000);
            }
        }
        k++;
    }

    ang = FLOAT_004ab390;
    for (i = 0; i < (unsigned)6; i++) {
        DAT_006121c8[i][0] = 0.0f;
        DAT_006121c8[i][1] = (float)cos(ang) * 1.8f;
        s = (float)sin(ang);
        DAT_006121c8[i][2] = s * 1.8f;
        DAT_006126d8[i][0] = (float)cos(ang);
        DAT_006126d8[i][1] = s;
        ang += 1.0471976f;
    }
}

// FUNCTION: LEGOLAND 0x00429150
void FUN_00429150(struct SegBlob *curve, float *off, int flag) {
    struct FVec3 pos;
    struct FVec3 dir;
    struct FVec3 basis[3];
    float t = (curve->end + curve->start) * 0.5f;

    curve->vt->method_c(curve, t, &dir.x);
    BuildBasisFromDir(&dir, basis);
    curve->vt->method_8(curve, t, &pos.x);
    pos.x += off[0];
    pos.y += off[1];
    pos.z += off[2];
    if (pos.z > DOUBLE_004ab478) {
        FUN_00429490(&pos, basis);
        return;
    }
    if (Vec3Dot((struct FVec3 *)DAT_00829990, &basis[2]) < FLOAT_004ab390) {
        if (flag != 1) {
            FUN_004292f0(&pos, basis);
        }
        FUN_00429490(&pos, basis);
    } else {
        FUN_00429490(&pos, basis);
        if (flag != 1) {
            FUN_004292f0(&pos, basis);
        }
    }
}

// FUNCTION: LEGOLAND 0x00429270
void FUN_00429270(void) {
    int i;

    for (i = 0; i <= 3; i++) {
        DAT_006121c8[6 + i][1] = DAT_006121c8[6 + i][0] = DAT_004b5f80[0][i].a * 5.0f;
    }
    for (i = 0; i <= 3; i++) {
        DAT_00612210[1][i].a = DAT_004b5f80[1][i].a * 1.5f;
        DAT_00612210[1][i].b = DAT_004b5f80[1][i].b * 1.5f;
    }
    for (i = 0; i <= 3; i++) {
        DAT_00612210[2][i].a = DAT_004b5f80[2][i].a * 1.5f;
        DAT_00612210[2][i].b = DAT_004b5f80[2][i].b * 1.5f;
    }
}

// FUNCTION: LEGOLAND 0x004292f0
void FUN_004292f0(struct FVec3 *pos, struct FVec3 *basis) {
    float ident[9];
    float zero[3];
    struct FVec3 *n;
    struct FVec3 *src;
    struct FVec3 *dst;
    float fx;
    float fy;
    float d;
    int t;
    int i;

    n = basis + 2;
    __asm {
        push eax
        push edx
        rdtsc
        mov t, eax
        pop edx
        pop eax
    }
    src = (struct FVec3 *)DAT_00612210;
    dst = (struct FVec3 *)DAT_00612708[30];
    fx = (float)((int)pos->x * DOUBLE_004ab488) * 5.0f;
    fy = (float)((int)pos->y * DOUBLE_004ab488) * 5.0f;
    for (i = 0; i <= 3; i++) {
        dst[i].x = fx + src[i].x;
        dst[i].y = fy + src[i].y;
        *(int *)&dst[i].z = *(int *)&src[i].z;
    }
    for (i = 4; i <= 7; i++) {
        dst[i].x = src[i].x + pos->x;
        dst[i].y = src[i].y + pos->y;
        dst[i].z = 0.0f;
    }
    d = Vec3Dot(pos, n);
    for (i = 8; i <= 11; i++) {
        dst[i].x = src[i].x + pos->x;
        dst[i].y = src[i].y + pos->y;
        dst[i].z = (d - n->x * dst[i].x - dst[i].y * n->y) / n->z;
    }
    __asm {
        push eax
        push edx
        rdtsc
        sub eax, t
        mov t, eax
        pop edx
        pop eax
    }
    DAT_00615f68 += t;
    zero[0] = 0.0f;
    zero[1] = 0.0f;
    zero[2] = 0.0f;
    ident[0] = 1.0f;
    ident[1] = 0.0f;
    ident[2] = 0.0f;
    ident[3] = 0.0f;
    ident[4] = 1.0f;
    ident[5] = 0.0f;
    ident[6] = 0.0f;
    ident[7] = 0.0f;
    ident[8] = 1.0f;
    DrawLmsModel(&DAT_004b6150, (struct LmsRec *)DAT_00615f70, zero, ident, 1);
}

// FUNCTION: LEGOLAND 0x00429490
void FUN_00429490(struct FVec3 *param_1, struct FVec3 *param_2) {
    DrawLmsModel(&DAT_004b6300, (struct LmsRec *)DAT_004b62f0, param_1, param_2, 1);
}

// FUNCTION: LEGOLAND 0x004294b0
void FUN_004294b0(void) {
    int i;

    for (i = 0; i < 8; i++) {
        DAT_00614858[i].a = DAT_004b61e0[i].a * 2.5f;
        DAT_00614858[i].b = DAT_004b61e0[i].b * 8.2f;
        DAT_00614858[i].c = DAT_004b61e0[i].c;
    }
}

// FUNCTION: LEGOLAND 0x004294f0
void FUN_004294f0(struct SegBlob *a, struct FVec3 *b, unsigned int c, unsigned int d) {
    FUN_00425bd0();
    if (c == 1) {
        FUN_00428e70(a, b, 0);
        FUN_00429150(a, &b->x, d);
        FUN_00428ec0(a, b, 2);
    } else {
        FUN_00428e70(a, b, 2);
        FUN_00429150(a, &b->x, d);
        FUN_00428ec0(a, b, 0);
    }
}

// FUNCTION: LEGOLAND 0x00429560
void FUN_00429560(struct PathSeg *seg, struct PathSeg *end, int count, float h, float dh) {
    float t;
    float step;
    struct FVec3 b;
    struct FVec3 a;
    struct SegBlob blob;
    unsigned int di = DirMaskToIndex(seg->dir_a);
    unsigned int dout = DirMaskToIndex(end->dir_b);
    int i;

    step = 1.0f / (float)count;
    t = 0.0f;
    MapPosToWorld(&seg->pos, h, &a);
    a.x += DAT_004b6398[di][0];
    a.y += DAT_004b6398[di][1];
    MapPosToWorld(&end->pos, h + dh, &b);
    b.x += DAT_004b6398[dout][0];
    b.y += DAT_004b6398[dout][1];
    FUN_00422180(&a.x, &b.x, (struct Words3 *)DAT_004b63c8[di], (struct Obj421ab0 *)&blob);
    for (i = 0; i < count; i++) {
        seg->flags |= 1;
        seg->blob = blob;
        seg->blob.start = t;
        t += step;
        seg->blob.end = t;
        seg->field_40 = 0;
        seg->field_44 = 0;
        seg->field_48 = 0;
        seg = seg->next;
    }
}

// FUNCTION: LEGOLAND 0x00429690
struct PathSeg *FUN_00429690(struct PathSeg *n, float v, struct PathSeg *end) {
    while (!IsStraightSegment(n->cls, n->dir_a, n->dir_b) && n != end) {
        n->f24 = n->f18 = v;
        n = n->next;
    }
    return n;
}

// FUNCTION: LEGOLAND 0x004296f0
struct PathSeg *FindStraightRunEnd(struct PathSeg *param_1, int *param_2) {
    struct PathSeg *node = param_1->next;
    *param_2 = 1;

    if (!IsStraightSegment(node->cls, node->dir_a, node->dir_b)) {
        return node->prev;
    }

    do {
        ++(*param_2);
        node = node->next;
    } while (IsStraightSegment(node->cls, node->dir_a, node->dir_b));

    return node->prev;
}

// FUNCTION: LEGOLAND 0x00429750
void InterpolateCoasterHeights(struct PathSeg *seg, struct PathSeg *end) {
    struct PathSeg *n;
    struct PathSeg *node;
    unsigned int count;
    float base;
    float step;

    count = CountStraightSegsForward(seg->next, &n);
    base = (float)(int)seg->cls->f4;
    if (count) {
        step = (float)(int)(end->cls->f8 - seg->cls->f4) / (int)count;
        n = seg->next;
        while (n != end) {
            if (IsStraightSegment(n->cls, n->dir_a, n->dir_b)) {
                int k;
                float h;
                node = FindStraightRunEnd(n, &k);
                h = (float)k * step;
                FUN_00429560(n, node, k, base, h);
                base = h + base;
                n = node->next;
            } else {
                n = FUN_00429690(n, base, end);
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x00429840
struct PathSeg *FUN_00429840(struct PathSeg *n, int x) {
    struct PathSeg *r;
    unsigned int ok;

    if (n->next == 0) {
        ok = CountStraightSegsBack(n, &n);
        r = n;
        if (ok || (float)x == r->f24) {
            return r;
        }
        return 0;
    } else {
        ok = CountStraightSegsForward(n, &n);
        r = n;
        if (ok || (float)x == r->f18) {
            return r;
        }
        return 0;
    }
}

// FUNCTION: LEGOLAND 0x004298a0
int FUN_004298a0(struct PieceClass *info, struct CastleState *owner, struct PathSeg **a, struct PathSeg **b) {
    int n = CountStraightSegsBack(owner->f_a8, a) + CountStraightSegsForward(owner->f_c0, b);
    unsigned int da = GetOppositeDirection(owner->f_a8->dir_b);
    unsigned int db = GetOppositeDirection(owner->f_c0->dir_a);

    if (IsStraightSegment(info, da, db)) {
        n++;
    }
    return n > 0;
}

// FUNCTION: LEGOLAND 0x00429910
unsigned int IsStraightSegment(struct PieceClass *s, unsigned int v, unsigned int c) {
    unsigned int flag = (s->flags == 0);
    flag &= 1;
    if (flag) {
        if (c == GetOppositeDirection(v)) {
            return 1;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00429940
unsigned int CountStraightSegsBack(struct PathSeg *n, struct PathSeg **out) {
    unsigned int count = 0;

    while (!(n->cls->flags & 1)) {
        if (IsStraightSegment(n->cls, n->dir_a, n->dir_b)) {
            count++;
        }
        n = n->prev;
    }
    *out = n;
    return count;
}

// FUNCTION: LEGOLAND 0x00429990
unsigned int CountStraightSegsForward(struct PathSeg *n, struct PathSeg **out) {
    unsigned int count = 0;

    while (!(n->cls->flags & 1)) {
        if (IsStraightSegment(n->cls, n->dir_a, n->dir_b)) {
            count++;
        }
        n = n->next;
    }
    *out = n;
    return count;
}

// FUNCTION: LEGOLAND 0x004299e0
void ResetCoasterHeightsBack(struct PathSeg *n) {
    struct PathSeg *p = n;
    float v;

    while (!(p->cls->flags & 1)) {
        p = p->prev;
    }
    v = (float)(int)p->cls->f4;
    p = p->next;
    while (p != n->next) {
        unsigned int fl = p->flags & ~1u;
        p->f18 = v;
        p->f24 = v;
        p->flags = fl;
        p = p->next;
    }
}

// FUNCTION: LEGOLAND 0x00429a30
void ResetCoasterHeightsForward(struct PathSeg *n) {
    struct PathSeg *p = n;
    float v;

    while (!(p->cls->flags & 1)) {
        p = p->next;
    }
    v = (float)(int)p->cls->f8;
    p = p->prev;
    while (p != n->prev) {
        unsigned int fl = p->flags & ~1u;
        p->f18 = v;
        p->f24 = v;
        p->flags = fl;
        p = p->prev;
    }
}

// FUNCTION: LEGOLAND 0x00429a80
void FUN_00429a80(struct CurveWalker *a, int param0, int param2, struct FVec3 *b) {
    struct SegBlob *base = a->node;
    base->vt->pair[param0].func(base, param2, b);
    b->x += a->pos.x;
    b->y += a->pos.y;
    b->z += a->pos.z;
}

// FUNCTION: LEGOLAND 0x00429ac0
void FUN_00429ac0(struct CurveWalker *a, int b, int c, void *d) {
    struct SegBlob *obj = a->node;
    obj->vt->pair[b].fn(obj, c, d);
}

// FUNCTION: LEGOLAND 0x00429af0
void BuildUprightBasis(struct FVec3 *a, struct FVec3 *b) {
    struct FVec3 *m = b;
    struct FVec3 *v = a;
    int i;

    m[2].x = 0.0f;
    m[2].y = 0.0f;
    m[2].z = 1.0f;
    Vec3Cross(&m[2], v, &m[1]);
    Vec3Cross(v, &m[1], &m[2]);
    m[0] = *v;
    for (i = 0; i < 3; i++) {
        FUN_00425d50(&m[i].x);
    }
}

// FUNCTION: LEGOLAND 0x00429b60
void FUN_00429b60(struct CurveWalker *a, int b, void *out) {
    int local[3];
    FUN_00429ac0(a, 1, b, local);
    BuildUprightBasis((struct FVec3 *)&local[0], out);
}

// FUNCTION: LEGOLAND 0x00429b90
unsigned int FUN_00429b90(struct CurveWalker *param_1, unsigned int param_2, unsigned int param_3, float *param_4) {
    struct SegBlob *inner = param_1->node;
    return inner->vt->method_1c(inner, param_3, param_4);
}

// FUNCTION: LEGOLAND 0x00429bb0
void FUN_00429bb0(struct CurveWalker *a, int b, int c, float scale, float *d) {
    float v[3];

    FUN_00429a80(a, b, c, (struct FVec3 *)d);
    FUN_00429b90(a, b, c, v);
    d[0] = d[0] - v[0] * scale;
    d[1] = d[1] - v[1] * scale;
    d[2] = d[2] - v[2] * scale;
}

struct Out429c60 {
    int type;
    struct FVec3 v;
    unsigned char pad[0x54 - 0x10];
};

// FUNCTION: LEGOLAND 0x00429c10
void FUN_00429c10(float t, struct FloatArray *out) {
    struct FVec3 v;

    /* the integrator's time comes in as a float; its bits are the walker's position (FUN_00429c60 passed an int) */
    FUN_00429bb0(DAT_00615f84, DAT_00615f90, *(int *)&t, DAT_00615fd4, (float *)&v);
    out->data[0] = v.x;
    out->data[1] = v.y;
    out->data[2] = v.z;
    out->count = 3;
}

// FUNCTION: LEGOLAND 0x00429c60
void FUN_00429c60(struct CurveWalker *obj, int a, unsigned int b, float c, struct FVec3 *out) {
    struct Out429c60 local;

    if (c != FLOAT_004ab390) {
        DAT_00615f84 = obj;
        DAT_00615f90 = a;
        DAT_00615fd4 = c;
        VecDerivative(FUN_00429c10, &DAT_00615f98, *(float *)&b, 0.01f, (struct FloatArray *)&local);
        out->x = local.v.x;
        out->y = local.v.y;
        out->z = local.v.z;
    } else {
        FUN_00429ac0(obj, a, b, out);
    }
}

// FUNCTION: LEGOLAND 0x00429cf0
float FUN_00429cf0(unsigned int x) {
    float v[3];
    float w[3];
    float *c;
    float d;

    DAT_00615fcc++;
    FUN_00429a80(DAT_00615f84, 1, x, (struct FVec3 *)v);
    c = DAT_00615f8c;
    v[0] = v[0] - c[0];
    v[1] = v[1] - c[1];
    v[2] = v[2] - c[2];
    if ((d = v[2] * v[2] + v[1] * v[1] + v[0] * v[0]) > DAT_00615fdc) {
        DAT_00615fe4++;
        return 1.0f;
    }
    if (DAT_00615fd0 > DAT_00615fd4 && d < DAT_00615fe0) {
        return FLOAT_004ab468;
    }
    FUN_00429b90(DAT_00615f84, 1, x, w);
    DAT_00615fe8++;
    v[0] = v[0] - DAT_00615fd4 * w[0];
    v[1] = v[1] - DAT_00615fd4 * w[1];
    v[2] = v[2] - DAT_00615fd4 * w[2];
    return v[2] * v[2] + v[1] * v[1] + v[0] * v[0] - DAT_00615fd8;
}

// FUNCTION: LEGOLAND 0x00429e20
int BisectRoot(float (*fn)(unsigned int), float lo, float hi, float *out) {
    float fa;
    int n;

    DAT_00615fc4++;
    DAT_00615fc8++;
    n = 1;
    fa = fn(*(unsigned int *)&lo);
    {
        float fb = fn(*(unsigned int *)&hi);
        if (!((*(unsigned int *)&fa ^ *(unsigned int *)&fb) & 0x80000000)) {
            return 0;
        }
    }
    while (hi - lo > 0.005f) {
        float mid = (lo + hi) * 0.5f;
        float fb = fn(*(unsigned int *)&mid);
        if ((*(unsigned int *)&fb ^ *(unsigned int *)&fa) & 0x80000000) {
            hi = mid;
        } else {
            lo = mid;
            fa = fb;
        }
        DAT_00615fc8++;
        n++;
    }
    *out = (lo + hi) * 0.5f;
    if (n > DAT_00615fec) {
        DAT_00615fec = n;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00429f30
void FindPathPosAtRangeBehind(float *center, float r, struct CurveWalker *src, float hi, float x, struct CurveWalker *dst, float *out) {
    struct CurveWalker cur;
    float lo;
    int found;

    cur = *src;
    lo = *(float *)&cur.node->start_bits;
    DAT_00615fd8 = r * r;
    DAT_00615f8c = center;
    DAT_00615fd0 = r;
    DAT_00615fd4 = x;
    DAT_00615f84 = &cur;
    DAT_00615fdc = (r + x) * (r + x);
    DAT_00615fe0 = (r - x) * (r - x);
    found = DAT_004b63fc(FUN_00429cf0, lo, hi, out);
    while (!found) {
        float hi2;
        CurveWalkerPrev(&cur);
        lo = *(float *)&cur.node->start_bits;
        hi2 = *(float *)&cur.node->end_bits;
        found = DAT_004b63fc(FUN_00429cf0, lo, hi2, out);
    }
    *dst = cur;
}

// FUNCTION: LEGOLAND 0x0042a020
void FindPathPosAtRangeAhead(float *center, float r, struct CurveWalker *src, float start, float x, struct CurveWalker *dst, float *out) {
    struct CurveWalker cur;
    float end;
    int found;

    cur = *src;
    end = *(float *)&cur.node->end_bits;
    DAT_00615fd8 = r * r;
    DAT_00615f8c = center;
    DAT_00615fd0 = r;
    DAT_00615fd4 = x;
    DAT_00615f84 = &cur;
    DAT_00615fdc = (r + x) * (r + x);
    DAT_00615fe0 = (r - x) * (r - x);
    found = DAT_004b63fc(FUN_00429cf0, start, end, out);
    while (!found) {
        CurveWalkerNext(&cur);
        start = *(float *)&cur.node->start_bits;
        end = *(float *)&cur.node->end_bits;
        found = DAT_004b63fc(FUN_00429cf0, start, end, out);
    }
    *dst = cur;
}

// FUNCTION: LEGOLAND 0x0042a110
int CurveWalkersEqual(struct CurveWalker *p1, struct CurveWalker *p2) {
    if (p1->piece != p2->piece) {
        return 0;
    }
    if (p1->node != p2->node) {
        return 0;
    }
    return Vec3Equal(&p1->pos.x, &p2->pos.x) != 0;
}

// FUNCTION: LEGOLAND 0x0042a150
float CurveTangentLength(unsigned int b) {
    float sum = 0.0f;
    float v[3];
    int i;

    FUN_00429c60(DAT_00615f80, DAT_00615ff0, b, DAT_00615ff4, (struct FVec3 *)v);
    for (i = 0; i < 3; i++) {
        sum += v[i] * v[i];
    }
    return (float)sqrt(sum);
}

// FUNCTION: LEGOLAND 0x0042a1b0
float CurveArcLength(struct CurveWalker *a, unsigned int hi, struct CurveWalker *b, unsigned int lo, int idx, float c) {
    struct CurveWalker cur;
    float acc;

    DAT_00615ff0 = idx;
    DAT_00615ff4 = c;
    if (CurveWalkersEqual(a, b)) {
        DAT_00615f80 = a;
        return IntegrateSimpson(CurveTangentLength, lo, hi, 0.01f);
    }
    cur = *b;
    DAT_00615f80 = &cur;
    acc = IntegrateSimpson(CurveTangentLength, lo, cur.node->end_bits, 0.01f);
    CurveWalkerNext(&cur);
    while (!CurveWalkersEqual(&cur, a)) {
        DAT_00615f80 = &cur;
        acc = IntegrateSimpson(CurveTangentLength, cur.node->start_bits, cur.node->end_bits, 0.01f) + acc;
        CurveWalkerNext(&cur);
    }
    DAT_00615f80 = &cur;
    return IntegrateSimpson(CurveTangentLength, cur.node->start_bits, hi, 0.01f) + acc;
}

// FUNCTION: LEGOLAND 0x0042a2e0
void FUN_0042a2e0(void) {
    InitVecMath(&DAT_00615f98, 3);
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x0042a2f0
__declspec(naked) void FUN_0042a2f0(void) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x200
        push ebx
        push esi
        push edi
        mov edi, dword ptr [ebp + 0xc]
        mov ecx, 3
        mov dword ptr [ebp - 0x14], 0
        mov edx, dword ptr [edi + 0xc]
        lea esi, [edi + 0x28]
        and edx, 0xf0
        mov eax, esi
        cmp dl, 0xf0
        mov dword ptr [ebp - 4], ecx
        mov dword ptr [ebp - 0x40], 0x47800000
        lea ebx, [ebp - 0x80]
        mov dword ptr [ebp - 0x18], eax
        je L42a33e
        mov edx, dword ptr [ebp + 8]
        mov dword ptr [ebp - 0x20], 0
        dec edx
        mov dword ptr [ebp + 8], edx
        jmp L42a345
L42a33e:
        mov dword ptr [ebp - 0x20], 1
L42a345:
        mov edx, dword ptr [edi + 8]
        and edx, 0xf
        cmp edx, 0xf
        je L42a377
        mov ecx, dword ptr [ebp + 8]
        mov dword ptr [ebp - 4], 0
        push ecx
        push edx
        lea edx, [ebp - 4]
        push edx
        push eax
        call ClipTriangle
        mov ecx, dword ptr [ebp - 4]
        add esp, 0x10
        test ecx, ecx
        mov dword ptr [ebp - 0x18], eax
        je L42a5d0
L42a377:
        mov edx, dword ptr [eax]
        mov dword ptr [eax + ecx*4], edx
        mov edx, dword ptr [ebp + 8]
        mov ecx, dword ptr [edi + 0x10]
        cmp edx, 1
        mov dword ptr [ebp - 0x34], ecx
        jle L42a409
        fld dword ptr [FLOAT_004ab49c]
        fdiv dword ptr [edi + 0x24]
        mov ecx, 0xc
        lea eax, [ebp - 0x30]
        sub eax, ecx
        dec edx
        mov dword ptr [ebp - 0xc], eax
        mov dword ptr [ebp - 8], edx
        fstp dword ptr [ebp - 0x1c]
L42a3a7:
        mov edx, dword ptr [esi]
        mov eax, dword ptr [esi + 4]
        mov edx, dword ptr [edx + ecx]
        mov eax, dword ptr [ecx + eax]
        sub eax, edx
        mov dword ptr [ebp - 0x3c], eax
        mov eax, dword ptr [esi + 8]
        fild dword ptr [ebp - 0x3c]
        mov eax, dword ptr [ecx + eax]
        sub eax, edx
        fmul dword ptr [edi + 0x20]
        mov dword ptr [ebp - 0x38], eax
        fild dword ptr [ebp - 0x38]
        fmul dword ptr [edi + 0x1c]
        fsubp st(1), st
        fmul dword ptr [ebp - 0x1c]
        fstp dword ptr [ebp - 0x24]
        fld dword ptr [ebp - 0x24]
        fistp dword ptr [ebp - 0x10]
        mov eax, dword ptr [ebp - 0x10]
        mov edx, dword ptr [ebp - 0xc]
        cmp eax, 0xffe00000
        mov dword ptr [edx + ecx], eax
        jl L42a3f3
        cmp eax, 0x200000
        jle L42a3fa
L42a3f3:
        mov dword ptr [ebp - 0x10], 0
L42a3fa:
        mov eax, dword ptr [ebp - 8]
        add ecx, 4
        dec eax
        mov dword ptr [ebp - 8], eax
        jne L42a3a7
        mov eax, dword ptr [ebp - 0x18]
L42a409:
        mov ecx, dword ptr [ebp - 4]
        mov dword ptr [ebp - 0x1c], 0
        test ecx, ecx
        jle L42a5d0
        lea edx, [ebp - 0x200]
        mov dword ptr [ebp - 0x10], eax
        jmp L42a429
L42a426:
        mov eax, dword ptr [ebp - 0x10]
L42a429:
        mov esi, dword ptr [eax]
        mov ecx, dword ptr [eax + 4]
        mov edi, dword ptr [ecx + 4]
        mov eax, dword ptr [esi + 4]
        sub edi, eax
        mov dword ptr [ebp - 0xc], edi
        je L42a518
        fild dword ptr [ebp - 0xc]
        mov eax, edx
        fstp dword ptr [ebp - 0xc]
        fld dword ptr [ebp - 0x40]
        fdiv dword ptr [ebp - 0xc]
        test edi, edi
        mov edi, dword ptr [ebp - 0x14]
        mov dword ptr [ebx + 4], edi
        jge L42a4cf
        inc edi
        add edx, 0x30
        mov dword ptr [ebp - 0x14], edi
        mov edi, dword ptr [ecx + 4]
        mov dword ptr [ebx], edi
        mov dword ptr [eax + 4], 0
        mov di, word ptr [ecx + 4]
        mov dword ptr [ebp - 0xc], edx
        mov word ptr [eax], di
        mov di, word ptr [esi + 4]
        add ebx, 8
        mov word ptr [eax + 2], di
        fstp dword ptr [ebp - 0x24]
        mov edi, dword ptr [ebp + 8]
        test edi, edi
        jle L42a518
        lea edi, [eax + 0x1c]
        sub eax, ecx
        lea edx, [ecx + 8]
        sub esi, ecx
        mov ecx, dword ptr [ebp + 8]
        mov dword ptr [ebp - 0x18], ecx
L42a49d:
        mov ecx, dword ptr [edx]
        shl ecx, 0x10
        mov dword ptr [eax + edx], ecx
        mov ecx, dword ptr [edx]
        sub ecx, dword ptr [esi + edx]
        mov dword ptr [ebp - 8], ecx
        fild dword ptr [ebp - 8]
        fmul dword ptr [ebp - 0x24]
        fistp dword ptr [ebp - 8]
        mov ecx, dword ptr [ebp - 8]
        add edx, 4
        mov dword ptr [edi], ecx
        mov ecx, dword ptr [ebp - 0x18]
        add edi, 4
        dec ecx
        mov dword ptr [ebp - 0x18], ecx
        jne L42a49d
        mov edx, dword ptr [ebp - 0xc]
        jmp L42a518
L42a4cf:
        inc edi
        add edx, 0x30
        mov dword ptr [ebp - 0x14], edi
        mov edi, dword ptr [esi + 4]
        mov dword ptr [ebx], edi
        mov dword ptr [eax + 4], 1
        mov di, word ptr [esi + 4]
        add ebx, 8
        mov word ptr [eax], di
        mov di, word ptr [ecx + 4]
        mov word ptr [eax + 2], di
        fstp dword ptr [ebp - 0x38]
        mov edi, dword ptr [esi + 8]
        shl edi, 0x10
        mov dword ptr [eax + 8], edi
        mov ecx, dword ptr [ecx + 8]
        sub ecx, dword ptr [esi + 8]
        mov dword ptr [ebp - 8], ecx
        fild dword ptr [ebp - 8]
        fmul dword ptr [ebp - 0x38]
        fistp dword ptr [ebp - 8]
        mov ecx, dword ptr [ebp - 8]
        mov dword ptr [eax + 0x1c], ecx
L42a518:
        mov eax, dword ptr [ebp - 0x1c]
        mov esi, dword ptr [ebp - 0x10]
        mov ecx, dword ptr [ebp - 4]
        inc eax
        add esi, 4
        cmp eax, ecx
        mov dword ptr [ebp - 0x1c], eax
        mov dword ptr [ebp - 0x10], esi
        jl L42a426
        mov ebx, dword ptr [ebp - 0x14]
        test ebx, ebx
        je L42a5d0
        lea edi, [ebx - 1]
        test edi, edi
        jl L42a578
L42a545:
        test edi, edi
        jle L42a575
        lea eax, [ebp - 0x78]
        mov esi, edi
L42a54e:
        mov edx, dword ptr [eax - 8]
        mov ecx, dword ptr [eax]
        cmp edx, ecx
        jle L42a56c
        mov ebx, dword ptr [eax]
        mov ecx, edx
        mov edx, dword ptr [eax - 4]
        mov dword ptr [eax - 8], ebx
        mov ebx, dword ptr [eax + 4]
        mov dword ptr [eax - 4], ebx
        mov dword ptr [eax], ecx
        mov dword ptr [eax + 4], edx
L42a56c:
        add eax, 8
        dec esi
        jne L42a54e
        mov ebx, dword ptr [ebp - 0x14]
L42a575:
        dec edi
        jns L42a545
L42a578:
        mov esi, dword ptr [ebp + 0xc]
        test byte ptr [esi], 1
        je L42a5b0
        cmp dword ptr [ebp - 0x20], 1
        jne L42a5b0
        lea eax, [ebp - 0x200]
        lea ecx, [ebp - 0x80]
        push eax
        mov eax, dword ptr [ebp + ebx*8 - 0x84]
        push ecx
        lea edx, [eax + eax*2]
        shl edx, 4
        movsx eax, word ptr [ebp + edx - 0x1fe]
        push eax
        push ebx
        call FUN_00423200
        add esp, 0x10
L42a5b0:
        mov ecx, dword ptr [esi + 0x38]
        lea edx, [ebp - 0x200]
        lea eax, [ebp - 0x80]
        push edx
        push eax
        mov eax, dword ptr [esi + 4]
        lea edx, [ebp - 0x34]
        push ebx
        push edx
        mov edx, dword ptr [ebp - 0x20]
        push eax
        call dword ptr [ecx + edx*4]
        add esp, 0x14
L42a5d0:
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

static __inline void Init42a5e0(struct TrainWheel *id, struct CurveWalker *src, float v) {
    id->walker = *src;
    id->t = v;
}

// FUNCTION: LEGOLAND 0x0042a5e0
void InitTrainBogie(struct TrainBogie *o, struct CurveWalker *src, float param_5) {
    Init42a5e0(&o->w0, src, param_5);
    Init42a5e0(&o->w1, src, param_5);
}

// FUNCTION: LEGOLAND 0x0042a620
void SetTrainWheelPos(struct TrainWheel *param_1, struct CurveWalker *param_2, unsigned int param_5) {
    memcpy(&param_1->walker, param_2, sizeof(struct CurveWalker));
    *(unsigned int *)&param_1->t = param_5;
}

// FUNCTION: LEGOLAND 0x0042a640
unsigned int GetTrainWheelPos(struct TrainWheel *ptr, unsigned int a, float *b) {
    FUN_00429bb0(&ptr->walker, DAT_004b6408[a], *(int *)&ptr->t, 4.8f, b);
}

// FUNCTION: LEGOLAND 0x0042a670
float FUN_0042a670(struct TrainBogie *p, int i) {
    return FLOAT_004ab390;
}

// FUNCTION: LEGOLAND 0x0042a680
void DrawTrainBogie(struct TrainBogie *p) {
    struct TrainBogie *o = p;
    float E[4][4];
    unsigned int B[4][4];
    unsigned int A[9];
    unsigned int G[9];
    float D[4][4];
    float C[3];
    int i;

    FUN_00425c40();
    FUN_00429b60(&o->w0.walker, *(int *)&o->w0.t, A);
    Mat3ToMat4Transposed(A, B);
    for (i = 0; i <= 1; i++) {
        o->ang[i] = FUN_0042a670(p, i) + o->ang[i];
        GetTrainWheelPos(&p->w0, i, C);
        Mat4Identity(D);
        D[0][0] = (float)sin(o->ang[i]);
        D[2][0] = -(D[0][2] = (float)cos(o->ang[i]));
        D[2][2] = (float)sin(o->ang[i]);
        Mat4Multiply((float (*)[4])B, D, E);
        FUN_00426460(G, E);
        DrawLmsModel(CoasterTrainWheelLms, CoasterTrainWheelLfm, C, G, 0);
    }
    o->w1.t = o->w0.t;
    o->w1.walker = o->w0.walker;
}

// FUNCTION: LEGOLAND 0x0042a780
void LoadCoasterTrainWheelModel(void) {
    // STRING: LEGOLAND 0x004b6414
    const char *str = "coastertrain.wheel01";
    CoasterTrainWheelLms = GetLmsByName(str);
    CoasterTrainWheelLfm = GetLfmByName(str);
}
