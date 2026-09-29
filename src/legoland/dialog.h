#pragma once

#include <windows.h>

int FUN_0043e930(RECT *rc, int min, int max, int value, int step);
struct Sprite;
struct Element;
int FUN_0043ea30(char **names, char *title, struct Sprite *bg, RECT *box, void (*callback)(int), struct Sprite **icons, int w, int h, int flag);
struct Element *FUN_0043eee0(char *title, struct Sprite *bg, RECT *box, unsigned int mask, int flag);
struct FileNode {
    struct FileNode *next;
    char *name;
    unsigned int attrib;
};
char *FUN_0043f0b0(char *title, struct Sprite *bg, RECT *box, char *path);
int FUN_0043f4f0(struct Sprite *bg, RECT *box, char *title, char *buf, int maxlen);
int FUN_0043f460(RECT *rc, int unused, char *buf, int maxlen, int *pos);
