#include <stdlib.h>
#include "legoland.h"

#include "bloke.h"
#include "globals.h"
#include "llidb.h"
#include "math.h"
#include "ride_queue.h"

struct QueueItemInner {
    unsigned char pad_0[0xe];
    unsigned short field_e;
    unsigned char pad_10[0x24 - 0x10];
    int field_24;
    int field_28;
    unsigned char pad_2c[0x38 - 0x2c];
    short field_38;
    unsigned char pad_3a[0x60 - 0x3a];
    unsigned char field_60;
    unsigned char pad_61;
    unsigned short field_62;
    unsigned char pad_64[0x68 - 0x64];
    int field_68;
    int field_6c;
    unsigned char pad_70[0x73 - 0x70];
    unsigned char field_73;
    unsigned char pad_74[0x98 - 0x74];
    struct Navigator field_98;
};

struct QueueItemMid {
    unsigned char pad_0[0x8];
    struct QueueItemInner *field_8;
};

struct QueueNode {
    struct QueueNode *next;
    struct QueueItemMid *field_4;
};

struct QueueStep {
    int dx;
    int dy;
    int pad;
};

struct QueueTable {
    int count;
    struct QueueStep *steps;
};

struct RideSlotArg {
    unsigned short field_0;
};

struct RideSlot {
    unsigned char pad_0[0x34];
    unsigned char field_34;
    unsigned char pad_35[0x3];
    unsigned short field_38;
    unsigned char pad_3a[0x16];
    struct RideSlotArg *field_50;
    unsigned char pad_54[0xc];
    unsigned char field_60;
};

// FUNCTION: LEGOLAND 0x00411e30
void FUN_00411e30(struct Queue *queue, struct QueueNode *node) {
    if (queue->head == NULL && queue->tail == NULL) {
        queue->head = node;
        queue->tail = node;
    } else {
        queue->tail->next = node;
        queue->tail = node;
    }
}

// FUNCTION: LEGOLAND 0x00411e60
unsigned int FUN_00411e60(struct Queue *queue) {
    unsigned int count;
    struct QueueNode *node;

    if (queue->head != NULL) {
        count = 0;
        node = queue->head;
        while (node != NULL) {
            count++;
            node = node->next;
        }
        if (count == (unsigned int)queue->count->count) {
            return 1;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00411e90
unsigned int FUN_00411e90(struct Queue *queue) {
    return queue->head != NULL;
}

// FUNCTION: LEGOLAND 0x00411ea0
int FUN_00411ea0(struct Queue *queue) {
    struct QueueNode *head = queue->head;
    if (head != NULL) {
        short value = head->field_4->field_8->field_38;
        if (value == (queue->count->count - 1)) {
            return 1;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00411ed0
void FUN_00411ed0(struct Queue *queue) {
    struct QueueNode *node = queue->head;
    while (node != NULL) {
        FUN_00411f00(queue);
        free(node);
        node = queue->head;
    }
}

// FUNCTION: LEGOLAND 0x00411f00
void FUN_00411f00(struct Queue *queue) {
    struct QueueNode *head = queue->head;
    if (head != NULL) {
        queue->head = head->next;
        if (queue->tail == head) {
            queue->tail = NULL;
        }
    }
}

// FUNCTION: LEGOLAND 0x00411f20
void FUN_00411f20(struct Queue *queue, struct QueueItemMid *mid) {
    struct QueueNode *node = (struct QueueNode *)malloc(8);
    if (node != NULL) {
        memset(node, 0, sizeof(*node));
        node->field_4 = mid;
        mid->field_8->field_62 |= 0x40;
        mid->field_8->field_38 = 0;
        mid->field_8->field_60++;
        FUN_00411e30(queue, node);
    }
}

// FUNCTION: LEGOLAND 0x00411f70
int FUN_00411f70(struct Queue *queue, struct QueueItemInner *inner) {
    struct QueueNode *head = queue->head;
    if (head != NULL && head->field_4->field_8 == inner) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00411fa0
void FUN_00411fa0(struct Queue *queue, int param_2, int param_3, struct QueueItemInner *inner) {
    struct QueueTable *table = queue->count;
    struct QueueStep *step = &table->steps[inner->field_38];
    struct QueueNode *node;
    short cur;
    char dir;
    int x;
    int y;

    x = (step->dx + param_2) << 8;
    y = (step->dy + param_3) << 8;
    inner->field_24 = x;
    inner->field_28 = y;
    dir = CalcMoveLine(*(struct Point *)&inner->field_68, *(struct Point *)&inner->field_24, &inner->field_98);
    inner->field_e = 7;
    inner->field_73 = dir + 0x10;
    NewDirForAction((Bloke *)inner, (unsigned char)(inner->field_73 >> 5) + 3);
    inner->field_38++;
    cur = inner->field_38;
    if (cur >= table->count) {
        inner->field_38 = (short)(table->count - 1);
        return;
    }
    for (node = queue->head; node != NULL; node = node->next) {
        struct QueueItemInner *other = node->field_4->field_8;
        if (other->field_38 == cur && other != inner) {
            inner->field_38 = cur - 1;
            return;
        }
    }
}

// FUNCTION: LEGOLAND 0x00412060
void FUN_00412060(struct Queue *queue, struct QueueItemMid **out) {
    struct QueueNode *node = queue->head;
    if (node != NULL) {
        struct QueueItemInner *inner = node->field_4->field_8;
        inner->field_62 &= ~0x40;
        inner->field_60++;
        *out = node->field_4;
        FUN_00411f00(queue);
        free(node);
    }
}

// FUNCTION: LEGOLAND 0x004120a0
void FUN_004120a0(struct Queue *queue, unsigned int param_2, unsigned int param_3) {
    struct QueueNode *node = queue->head;
    while (node != NULL) {
        struct QueueItemInner *inner = node->field_4->field_8;
        if (inner->field_e == 0) {
            FUN_00411fa0(queue, param_2, param_3, inner);
        }
        node = node->next;
    }
}

// FUNCTION: LEGOLAND 0x00412100
int FUN_00412100(struct PathTable *param_1) {
    int total = 0;
    int px;
    int py;
    int i;
    struct QueueTable *table;
    int k;
    int j;
    int sx;
    int sy;
    int dx;
    int dy;
    int len;
    int cx;
    int cy;

    for (i = 0; i < param_1->count; i++) {
        int x = param_1->pairs[i].a;
        int y = param_1->pairs[i].b;
        total += (int)sqrt((double)(y * y + x * x));
    }
    table = (struct QueueTable *)malloc(total * 12 + 8);
    if (table != NULL) {
        memset(table, 0, total * 12 + 8);
        table->count = total;
        table->steps = (struct QueueStep *)(table + 1);
    }
    sx = 0;
    sy = 0;
    k = 0;
    for (i = 0; i < param_1->count; i++) {
        struct PathPair *p = &param_1->pairs[i];
        dx = p->a;
        dy = p->b;
        len = (int)sqrt((double)(dy * dy + dx * dx));
        px = 0;
        py = 0;
        cx = sx;
        cy = sy;
        for (j = len; j > 0; j--) {
            table->steps[k].dx = cx;
            table->steps[k].dy = cy;
            k++;
            cx = sx + px / len;
            cy = sy + py / len;
            px += dx;
            py += dy;
        }

        sx += dx;
        sy += dy;
    }
    return (int)table;
}

// FUNCTION: LEGOLAND 0x00412290
void FUN_00412290(void *param_1) {
    if (param_1 != NULL) {
        free(param_1);
    }
}

// FUNCTION: LEGOLAND 0x004122a0
void FUN_004122a0(struct RideSlotArg *param_1, struct RideSlot *slot) {
    slot->field_50 = param_1;
    slot->field_38 = param_1->field_0 - 1;
    slot->field_34 = 0xff;
    slot->field_60++;
}

// FUNCTION: LEGOLAND 0x004122d0
void FUN_004122d0(struct RideSlotArg *param_1, struct RideSlot *slot) {
    slot->field_50 = param_1;
    slot->field_38 = 0;
    slot->field_34 = 1;
    slot->field_60++;
}

// FUNCTION: LEGOLAND 0x004122f0
struct RideSlotArg *FUN_004122f0(struct RideSlot *slot) {
    return slot->field_50;
}

// FUNCTION: LEGOLAND 0x00412300
void FUN_00412300(struct QueueTable *table, int x, int y, struct Bloke *bloke) {
    struct Point p;
    char dir;

    p.x = x + table->steps[bloke->field_38].dx;
    p.y = table->steps[bloke->field_38].dy + y;
    bloke->dest.x = p.x << 8;
    bloke->dest.y = p.y << 8;
    dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
    bloke->field_e = 7;
    bloke->field_73 = dir + 0x10;
    NewDirForAction(bloke, (unsigned char)(bloke->field_73 >> 5) + 3);
    bloke->field_38 += (signed char)bloke->field_34;
    if ((signed char)bloke->field_34 < 0) {
        if (bloke->field_38 < 0) {
            bloke->param_action++;
        }
    } else if (bloke->field_38 >= table->count) {
        bloke->param_action++;
    }
}

// FUNCTION: LEGOLAND 0x004123a0
unsigned int FUN_004123a0(struct QueueNode *start, struct QueueNode *stop) {
    unsigned int count = 0;
    struct QueueNode *current = start;
    if (current != NULL) {
        while (1) {
            if (current == stop) {
                break;
            }
            current = current->next;
            count++;
            if (current == NULL) {
                break;
            }
        }
    }
    return count;
}

// FUNCTION: LEGOLAND 0x004123c0
void FUN_004123c0(struct QueueNode *start, struct Queue *queue) {
    struct QueueNode *node;
    int n;

    SaveGameWrite(queue->count, 4);
    for (n = 0; n < queue->count->count; n++) {
        SaveGameWrite(&queue->count->steps[n], 12);
    }
    n = 0;
    for (node = queue->head; node != NULL; node = node->next) {
        n++;
    }
    SaveGameWrite(&n, 4);
    for (node = queue->head; node != NULL; node = node->next) {
        n = FUN_004123a0(start, (struct QueueNode *)node->field_4);
        SaveGameWrite(&n, 4);
    }
}

// FUNCTION: LEGOLAND 0x00412470
struct QueueNode *FUN_00412470(struct QueueNode *node, int n) {
    int i = n;
    while (i-- != 0) {
        node = node->next;
    }
    return node;
}

// FUNCTION: LEGOLAND 0x00412490
void FUN_00412490(struct QueueNode *start, struct Queue *queue) {
    int n;
    int idx;

    SaveGameRead(&n, 4);
    queue->count = (struct QueueTable *)malloc(n * 12 + 8);
    queue->count->count = n;
    queue->count->steps = (struct QueueStep *)(queue->count + 1);
    for (n = 0; n < queue->count->count; n++) {
        SaveGameRead(&queue->count->steps[n], 12);
    }
    queue->head = NULL;
    queue->tail = NULL;
    SaveGameRead(&n, 4);
    while (n-- != 0) {
        if (queue->tail == NULL) {
            queue->tail = (struct QueueNode *)malloc(8);
            queue->head = queue->tail;
        } else {
            queue->tail->next = (struct QueueNode *)malloc(8);
            queue->tail = queue->tail->next;
        }
        queue->tail->next = NULL;
        SaveGameRead(&idx, 4);
        queue->tail->field_4 = (struct QueueItemMid *)FUN_00412470(start, idx);
    }
}

// FUNCTION: LEGOLAND 0x004125a0
struct RideQueueEntry *FUN_004125a0(int x, int y) {
    struct RideQueueEntry *entry = DAT_004cbeac;
    if (x < 0) {
        return NULL;
    }
    if (x >= (unsigned short)lpConfig->width) {
        return NULL;
    }
    if (y < 0) {
        return NULL;
    }
    if (y >= (unsigned short)lpConfig->height) {
        return NULL;
    }
    if (entry == NULL) {
        return NULL;
    }
    while (entry != NULL) {
        if (entry->x == x && entry->y == y) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x004125f0
void *FUN_004125f0(unsigned int a, unsigned int b) {
    struct RideQueueEntry *entry = DAT_004cbeac;
    int x = a;
    int y = b;
    if (x < 0) {
        return NULL;
    }
    if (x >= (unsigned short)lpConfig->width) {
        return NULL;
    }
    if (y < 0) {
        return NULL;
    }
    if (y >= (unsigned short)lpConfig->height) {
        return NULL;
    }
    if (entry == NULL) {
        return NULL;
    }
    while (entry != NULL) {
        if ((unsigned int)(x - entry->x) < 4 && (unsigned int)(y - entry->y) < 4) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x00412650
struct RideQueueEntry *FUN_00412650(unsigned short param_1) {
    struct RideQueueEntry *entry = DAT_004cbeac;
    if (entry == NULL) {
        return NULL;
    }
    while (entry != NULL) {
        if (entry->field_8 == param_1 && (entry->field_14 & 0xf) == 6) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x00412680
void FUN_00412680(int x, int y, int param_3, int param_4) { STUB(); }
