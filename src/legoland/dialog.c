#include <windows.h>
#include "globals.h"
#include "legoland.h"
#include "dialog.h"
#include "gfx.h"
#include "input.h"
#include "render.h"
#include "text.h"

// FUNCTION: LEGOLAND 0x0043e930
void FUN_0043e930(void) { STUB(); }

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
void FUN_0043f4f0(void) { STUB(); }
