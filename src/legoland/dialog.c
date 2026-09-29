#include "dialog.h"
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "gfx.h"
#include "globals.h"
#include "image_sprite.h"
#include "input.h"
#include "legoland.h"
#include "llidb.h"
#include "render.h"
#include "text.h"

// FUNCTION: LEGOLAND 0x0043e930
int FUN_0043e930(RECT *rc, int min, int max, int value) {
    int pos;

    pos = (rc->bottom - rc->top) * value / (max - min);
    RenderThickBox(rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top, 2, 0);
    RenderBlock(rc->left, rc->top + pos - 1, rc->right - rc->left, 3, GetNearestColour(0xff, 0, 0));
    if ((DAT_00813ac4 & 4) && DAT_00813a44.x >= rc->left && DAT_00813a44.x <= rc->right &&
        DAT_00813a44.y >= rc->top && DAT_00813a44.y <= rc->bottom) {
        DAT_0062fea4 = 1;
    } else if (!DAT_0062fea4) {
        return value;
    }
    if (DAT_00813ac4 & 4) {
        pos = DAT_00813a44.y;
        if (pos < rc->top) {
            pos = rc->top;
        } else if (pos > rc->bottom) {
            pos = rc->bottom;
        }
        return (pos - rc->top) * (max - min) / (rc->bottom - rc->top);
    }
    DAT_0062fea4 = 0;
    return value;
}

// FUNCTION: LEGOLAND 0x0043ea30
int FUN_0043ea30(char **names, RECT *box, char *title, int sel, int unused, struct Sprite **icons, int w, int h, int flag) { STUB(); }

// FUNCTION: LEGOLAND 0x0043eee0
struct Element *FUN_0043eee0(RECT *box, char *title, int sel, unsigned int mask, int flag) {
    struct Sprite *happy;
    struct Sprite *poor;
    struct Element *e;
    struct Element *t;
    struct Element *u;
    struct Element **list;
    char **names;
    struct Sprite **icons;
    struct Element *result;
    int count;
    int n;
    int c;
    int i;
    int j;
    int k;
    int swapped;
    int r;

    count = LLIDB_GetCount();
    c = 0;
    // STRING: LEGOLAND 0x004b7a84
    happy = LoadSprite("happy.lls", 0);
    poor = LoadSprite("poor.lls", 0);
    for (i = 0; i < count; i++) {
        LLIDB_GetElement(i, &e);
        if (e->flags & mask) {
            c++;
        }
    }
    names = malloc(c * 4 + 4);
    list = malloc(c * 4);
    icons = malloc(c * 4);
    n = 0;
    for (k = 0; k < count; k++) {
        LLIDB_GetElement(k, &e);
        if (e->flags & mask) {
            list[n] = e;
            n++;
        }
    }
    names[n] = 0;
    for (i = 0; i < n - 1; i++) {
        swapped = 0;
        for (j = n - 2; j >= i; j--) {
            if (_strcmpi(list[j + 1]->name, list[j]->name) < 0) {
                t = list[j];
                u = list[j + 1];
                list[j + 1] = t;
                list[j] = u;
                swapped = 1;
            }
        }
        if (!swapped) {
            break;
        }
    }
    for (k = 0; k < n; k++) {
        names[k] = list[k]->name;
        if (list[k]->flags & 1) {
            icons[k] = happy;
        } else {
            icons[k] = poor;
        }
    }
    r = FUN_0043ea30(names, box, title, sel, 0, icons, 0x2e, 0x28, flag);
    if (r != -1) {
        result = list[r];
    } else {
        result = 0;
    }
    free(names);
    free(list);
    free(icons);
    KillSprite(happy);
    KillSprite(poor);
    return result;
}

// FUNCTION: LEGOLAND 0x0043f0b0
void FUN_0043f0b0(void) { STUB(); }

// FUNCTION: LEGOLAND 0x0043f460
int FUN_0043f460(RECT *rc, int unused, char *buf, int maxlen, int *pos) {
    char c;

    c = GetInputChar();
    if (c != 0) {
        switch (c) {
        case (char)0xfd:
            return 1;
        case (char)0xfe:
            return 0;
        case (char)0xff:
            if (*pos > 0) {
                (*pos)--;
            }
            buf[*pos] = 0;
            break;
        default:
            if (*pos < maxlen - 1) {
                buf[*pos] = c;
                (*pos)++;
                buf[*pos] = 0;
            }
            break;
        }
    }
    PrintLimitedText(rc->left + 4, rc->top + 4, rc->right - rc->left - 8, buf, 2, 0, 0);
    return 0;
}

// FUNCTION: LEGOLAND 0x0043f4f0
int FUN_0043f4f0(struct Sprite *bg, RECT *box, char *title, char *buf, int maxlen) {
    int pos;
    RECT rc;
    int done;

    pos = strlen(buf);
    rc.left = box->left + 8;
    rc.top = box->top + 0x20;
    rc.right = box->right + box->left - 9;
    rc.bottom = box->top + box->bottom - 9;
    while (!ProcessSystemEvents()) {
        ReadGameButtons();
        PushRenderingStatusAndLockVideoSurface();
        PrintSprite(bg, 0, 0, 0, 0);
        RenderBlock(box->left, box->top, box->right, box->bottom, GetNearestColour(0xef, 0xef, 0xef));
        RenderThickBox(box->left, box->top, box->right, box->bottom, 2, 0);
        RenderBlock(box->left + 2, box->top + 2, box->right - 4, 0x18, GetNearestColour(0, 0x3f, 0x7f));
        PrintLimitedText(box->left + 2, box->top + 2, box->right - 4, title, 0, 0xefefef, 0);
        RenderThickBox(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 2, 0);
        done = FUN_0043f460(&rc, 0, buf, maxlen, &pos);
        RenderingComplete();
        PopRenderingStatus();
        if (done) {
            break;
        }
    }
    return pos;
}
