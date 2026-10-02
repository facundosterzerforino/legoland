#pragma once

#include "legoland.h"

#include <windows.h>

union RectPoints {
    RECT rect;
    POINT pt[2];
};

struct VideoArg {
    /* 0x00 */ int field_0;
    /* 0x04 */ int field_4;
    /* 0x08 */ int field_8;
    /* 0x0c */ void *field_c;
    /* 0x10 */ int field_10;
    /* 0x14 */ int field_14;
};
struct Sprite;

struct AviFrame {
    /* 0x00 */ unsigned char pad_0[4];
    /* 0x04 */ int width;
    /* 0x08 */ int height;
    /* 0x0c */ unsigned char pad_c[0x1c];
    /* 0x28 */ unsigned short pixels[1];
};

LEGO_EXPORT int InitHostSystemGPU(void);
LEGO_EXPORT void KillHostSystemGPU(void);
LEGO_EXPORT int InitScreen(void);
LEGO_EXPORT unsigned int SetPointer(unsigned int param_1);
LEGO_EXPORT void PushRenderingStatusAndLockVideoSurface(void);
LEGO_EXPORT void PushRenderingStatusAndUnlockVideoSurface(void);
LEGO_EXPORT void PopRenderingStatus(void);
LEGO_EXPORT void PrintBackground(int x, int y);
LEGO_EXPORT int GetVideoSurface(struct VideoArg *arg);
LEGO_EXPORT void SetOverridePalette(unsigned int param_1);
LEGO_EXPORT void SetOverrideFrame(unsigned int param_1);
LEGO_EXPORT unsigned int GetOverridePalette(void);
LEGO_EXPORT unsigned int GetOverrideFrame(void);
LEGO_EXPORT void ClearSpriteOverrides(void);
LEGO_EXPORT void ZBufferHelper(unsigned int *param_1, int *param_2, int *param_3, void *param_4);
LEGO_EXPORT void ClearOverrideFrame(void);
LEGO_EXPORT void ClearOverridePalette(void);
void FUN_00465850(struct AviFrame *frame);
void FUN_004659a0(struct AviFrame *param_1, int param_2, int param_3);
void LoadWatchSprite(int a, int b);
void UnloadWatchSprite(void);
LEGO_EXPORT void CommitCliprectToHardware(void);
LEGO_EXPORT int RenderingComplete(void);
LEGO_EXPORT void PushSetTarget(struct Sprite *sprite);
LEGO_EXPORT void PopTarget(void);
LEGO_EXPORT int RecreateSprite(struct Sprite *sprite);
void FUN_004687f0(const char *param_1);
LEGO_EXPORT int CheckHostSystemGPU(void);
int BlitFrameToWindow(void);
void __fastcall FUN_00464ee0(struct Sprite *sprite, RECT *rect, int *off);
LEGO_EXPORT void SoftPrint_Clear(void);
LEGO_EXPORT void SoftPrint_XBltFast(struct Sprite *sprite, RECT *a, RECT *b, unsigned int param_4);
void DrawWatchSprite(void);
int SetDisplayModeAndDetectPixelFormat(void);

struct DrawLLS {
    short frame;
    unsigned short delay;
    int width;
    int height;
    unsigned int field_c;
    short frame_count;
    short loop_delay;
    unsigned int flags;
};

struct DrawLLSFrame {
    unsigned int size;
    unsigned int pixel_count;
    unsigned int run_bytes;
    unsigned int field_c;
    unsigned short pixels[1];
};

void FUN_00466770(struct DrawLLS *lls, RECT *clip, struct Point *pos);
void FUN_00466d80(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor);
void FUN_00467180(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor);
void FUN_004673f0(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor);
void FUN_00467640(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor);
void FUN_004677b0(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor);
void FUN_00467b00(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor);
void FUN_00467d10(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor);
void FUN_00467f00(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip);
void FUN_00465ee0(struct DrawLLS *lls, RECT *clip, struct Point *pos);
void FUN_00468040(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor);
