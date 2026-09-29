#include "dialog.h"
#include <windows.h>
#include "gfx.h"
#include "globals.h"
#include "input.h"
#include "legoland.h"
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
void FUN_0043ea30(void) { STUB(); }

// FUNCTION: LEGOLAND 0x0043eee0
void FUN_0043eee0(void) { STUB(); }

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
