#pragma once

#include "legoland.h"
#include "objclass.h"

struct SpiderNode {
    unsigned short field_0;
    unsigned char pad_2[2];
    char field_4;
    unsigned char pad_5[0x27];
    struct SpiderNode *next;
};

int FUN_00415a90(struct SpiderNode *node);
void FUN_004161f0(struct SpiderNode *node);
struct SpiderNode *FUN_004159b0(TileId *key);
void FUN_00416330(Element *obj);
LEGO_EXPORT int SaveSpider(void);
LEGO_EXPORT int LoadSpider(void);

void SpiderRide(struct ClassNode *head, struct CallbackTable *iface);
