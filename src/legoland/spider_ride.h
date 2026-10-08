#pragma once

#include "legoland.h"
#include "objclass.h"

struct SpiderNode {
    unsigned short tile_id;
    unsigned char pad_2[2];
    char frame;
    unsigned char pad_5[0x27];
    struct SpiderNode *next;
};

int FUN_00415a90(struct SpiderNode *node);
void FUN_004161f0(struct SpiderNode *node);
struct SpiderNode *FindSpiderNode(TileId *key);
void SpiderRideUpdate(Element *obj);
LEGO_EXPORT int SaveSpider(void);
LEGO_EXPORT int LoadSpider(void);

void SpiderRide(struct ClassNode *head, struct CallbackTable *iface);
struct SlotOwner;
struct SlotArray;
int FUN_00416830(struct SlotOwner *owner, struct SlotArray *arr, signed char count);
