#pragma once

#include <windows.h>

int FUN_0043e930(RECT *rc, int min, int max, int value);
struct Sprite;
struct Element;
int FUN_0043ea30(char **names, RECT *box, char *title, int sel, int unused, struct Sprite **icons, int w, int h, int flag);
struct Element *FUN_0043eee0(RECT *box, char *title, int sel, unsigned int mask, int flag);
int FUN_0043f4f0(struct Sprite *bg, RECT *box, char *title, char *buf, int maxlen);
int FUN_0043f460(RECT *rc, int unused, char *buf, int maxlen, int *pos);
