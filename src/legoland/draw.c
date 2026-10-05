#include <windows.h>
#include <ddraw.h>
#include "legoland.h"

#include <stdlib.h>
#include <string.h>
#include "clipping.h"
#include "debug_alloc.h"
#include "globals.h"

#include "draw.h"
#include "gfx.h"
#include "llidb.h"
#include "math.h"
#include "print_sprite.h"
#include "text.h"
#include "timer.h"
#include "wndenv.h"

#include "image_sprite.h"

// FUNCTION: LEGOLAND 0x004636f0
LEGO_EXPORT int InstallDirectDraw(void) { return 0; }

// FUNCTION: LEGOLAND 0x00463700
LEGO_EXPORT int InitHostSystemGPU(void) {
    LPDIRECTDRAW ddraw;
    LPDIRECTDRAW2 ddraw2;

    if (DDRAWENV.ddraw2 != 0) {
        return 1;
    }
    if (DirectDrawCreate(NULL, &DDRAWENV.ddraw, NULL) == 0) {
        ddraw = DDRAWENV.ddraw;
        if (IDirectDraw_QueryInterface(ddraw, &IID_IDirectDraw2_Guid, &DDRAWENV.ddraw2) != 0) {
            IDirectDraw_Release(DDRAWENV.ddraw);
            // STRING: LEGOLAND 0x004b9cd0
            DBPrintf("Can't Create DDCOM");
            return 0;
        }
        DDRAWENV.caps.dwSize = 0x17c;
        DDRAWENV.hel_caps.dwSize = 0x17c;
        ddraw2 = DDRAWENV.ddraw2;
        if (IDirectDraw2_GetCaps(ddraw2, &DDRAWENV.caps, &DDRAWENV.hel_caps) != 0) {
            IDirectDraw2_Release(DDRAWENV.ddraw2);
            IDirectDraw_Release(DDRAWENV.ddraw);
            // STRING: LEGOLAND 0x004b9cb8
            DBPrintf("Can't get DDCOM caps");
            return 0;
        }
        // STRING: LEGOLAND 0x004b9cac
        AddFontResourceA("Lego.ttf");
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x004637c0
LEGO_EXPORT int CheckHostSystemGPU(void) {
    memset(&DDRAWENV, 0, sizeof(DDRAWENV));
    return InitHostSystemGPU();
}

// FUNCTION: LEGOLAND 0x004637e0
LEGO_EXPORT void KillHostSystemGPU(void) {
    // STRING: LEGOLAND 0x004b9ce4
    RemoveFontResourceA("lego.ttf");
    if (DDRAWENV.ddraw2 != 0) {
        IDirectDraw2_Release(DDRAWENV.ddraw2);
        DDRAWENV.ddraw2 = 0;
    }
    DeleteObject((HGDIOBJ)LegoFont24Bold);
    DeleteObject((HGDIOBJ)LegoFont28Normal);
    DeleteObject((HGDIOBJ)LegoFont20Bold);
    DeleteObject((HGDIOBJ)LegoFont18SemiBold);
    if (DDRAWENV.ddraw != 0) {
        IDirectDraw_Release(DDRAWENV.ddraw);
        DDRAWENV.ddraw = 0;
    }
}

// FUNCTION: LEGOLAND 0x00463850
LEGO_EXPORT unsigned int SetPointer(unsigned int param_1) {
    unsigned int old = DAT_0066814c;
    DAT_0066814c = param_1;
    DAT_00668148 = PointerSprites[param_1];
    return old;
}

// FUNCTION: LEGOLAND 0x00463870
LEGO_EXPORT int InitScreen(void) {
    LOGFONTA font;
    WNDCLASSEXA wc;
    DDSURFACEDESC desc;
    RECT rect_slot;
    RECT window_rect;
    RGNDATA *rgn;

    rect_slot.left = 0;
    rect_slot.top = 0;
    rect_slot.right = lpConfig->screen_width;
    rect_slot.bottom = lpConfig->screen_height;
    FrameNumber = 0;

    wc.cbSize = sizeof(wc);
    wc.style = 0;
    wc.lpfnWndProc = LegoLandWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = WNDENV_GethInstance();
    wc.hIcon = LoadIconA(WNDENV_GethInstance(), (LPCSTR)0x65);
    wc.hCursor = LoadCursorA(WNDENV_GethInstance(), (LPCSTR)0x7d);
    wc.hbrBackground = (HBRUSH)GetStockObject(5);
    wc.lpszMenuName = NULL;
    // STRING: LEGOLAND 0x004b9cfc
    wc.lpszClassName = "LEGOLANDMAIN";
    wc.hIconSm = LoadIconA(WNDENV_GethInstance(), (LPCSTR)0x65);
    RegisterClassExA(&wc);

    font.lfHeight = 0x18;
    font.lfWidth = 0;
    font.lfEscapement = 0;
    font.lfOrientation = 0;
    font.lfWeight = 0x2bc;
    font.lfItalic = 0;
    font.lfUnderline = 0;
    font.lfStrikeOut = 0;
    font.lfCharSet = 1;
    font.lfOutPrecision = 0;
    font.lfClipPrecision = 0;
    font.lfQuality = 2;
    font.lfPitchAndFamily = 0;
    // STRING: LEGOLAND 0x004b86e0
    strcpy(font.lfFaceName, "Lego");
    LegoFont24Bold = CreateFontIndirectA(&font);

    font.lfWeight = 0x190;
    font.lfHeight = 0x1c;
    font.lfWidth = 0;
    LegoFont28Normal = CreateFontIndirectA(&font);
    font.lfHeight = 0x14;
    font.lfWidth = 0;
    font.lfWeight = 0x2bc;
    LegoFont20Bold = CreateFontIndirectA(&font);
    font.lfWeight = 0x258;
    font.lfHeight = 0x12;
    font.lfWidth = 0;
    strcpy(font.lfFaceName, "Lego");
    LegoFont18SemiBold = CreateFontIndirectA(&font);

    if (WinDebugMode == 0) {
        DisplayPixelFormat = 2;
        WNDENV_Sethwnd(CreateWindowExA(8, "LEGOLANDMAIN",
            // STRING: LEGOLAND 0x004b86d0
            "LEGOLAND", 0x90000000, 0, 0, lpConfig->screen_width, lpConfig->screen_height, GetDesktopWindow(), NULL, WNDENV_GethInstance(), NULL));
        if (WNDENV_Gethwnd() == NULL) {
            return 0;
        }
        if (IDirectDraw2_SetCooperativeLevel(DDRAWENV.ddraw2, WNDENV_Gethwnd(), 0x11) != 0) {
            return 0;
        }
        if (SetDisplayModeAndDetectPixelFormat() == 0) {
            return 0;
        }
        while ((lpConfig->field_1c & 1) != 0) {
            ProcessSystemEvents();
            ShowWindow(WNDENV_Gethwnd(), 3);
        }
        if (lpConfig->field_1e == 0) {
            ShowCursor(0);
        }
        BlitFrameFunc = BlitFrameToWindow;
        desc.dwSize = sizeof(desc);
        desc.dwFlags = 1;
        desc.ddsCaps.dwCaps = 0x4200;
        if (IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc, &PrimarySurface, NULL) != 0) {
            return 0;
        }
        LoadColourTable();
        desc.dwSize = sizeof(desc);
        desc.dwFlags = 7;
        desc.ddsCaps.dwCaps = 0x800;
        desc.dwWidth = lpConfig->screen_width;
        desc.dwHeight = lpConfig->screen_height;
        if (IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc, &OffscreenSurface, NULL) != 0) {
            return 0;
        }
        renderEngine = OffscreenSurface;
        desc.dwSize = sizeof(desc);
        desc.dwFlags = 7;
        desc.ddsCaps.dwCaps = 0x4000;
        desc.dwWidth = lpConfig->screen_width;
        desc.dwHeight = lpConfig->screen_height;
        if (IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc, &DAT_00668074, NULL) != 0) {
            return 0;
        }
        IDirectDraw2_CreateClipper(DDRAWENV.ddraw2, 0, &DDrawClipper, NULL);
        SetClipping(&rect_slot);
        rgn = malloc(sizeof(RGNDATA) - 1 + sizeof(RECT));
        rgn->rdh.dwSize = 0x20;
        rgn->rdh.iType = 1;
        rgn->rdh.nCount = 1;
        rgn->rdh.nRgnSize = 0x10;
        rgn->rdh.rcBound = SPRITE_ClipRect;
        memcpy(rgn->Buffer, &SPRITE_ClipRect, sizeof(RECT));
        IDirectDrawClipper_SetClipList(DDrawClipper, (LPRGNDATA)rgn, 0);
        free(rgn);
    } else {
        window_rect.left = 0;
        window_rect.top = 0;
        window_rect.right = lpConfig->screen_width - 1;
        window_rect.bottom = lpConfig->screen_height - 1;
        AdjustWindowRect(&window_rect, 0x10cf0000, 0);
        WNDENV_Sethwnd(CreateWindowExA(0, "LEGOLANDMAIN",
            // STRING: LEGOLAND 0x004b9cf0
            "Lego Land", 0x10cf0000, 0, 0, window_rect.right - window_rect.left + 1, window_rect.bottom - window_rect.top + 1, NULL, NULL, WNDENV_GethInstance(), NULL));
        if (WNDENV_Gethwnd() == NULL) {
            return 0;
        }
        if (IDirectDraw2_SetCooperativeLevel(DDRAWENV.ddraw2, WNDENV_Gethwnd(), 8) != 0) {
            DestroyWindow(WNDENV_Gethwnd());
            return 0;
        }
        if (SetDisplayModeAndDetectPixelFormat() == 0) {
            DestroyWindow(WNDENV_Gethwnd());
            return 0;
        }
        IDirectDraw2_CreateClipper(DDRAWENV.ddraw2, 0, &DDrawClipper, NULL);
        IDirectDrawClipper_SetHWnd(DDrawClipper, 0, WNDENV_Gethwnd());
        desc.dwSize = sizeof(desc);
        desc.dwFlags = 1;
        desc.ddsCaps.dwCaps = 0x200;
        if (IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc, &PrimarySurface, NULL) != 0) {
            DestroyWindow(WNDENV_Gethwnd());
            return 0;
        }
        IDirectDrawSurface_SetClipper(PrimarySurface, DDrawClipper);
        desc.dwSize = sizeof(desc);
        desc.dwFlags = 7;
        desc.ddsCaps.dwCaps = 0x40;
        desc.dwWidth = lpConfig->screen_width;
        desc.dwHeight = lpConfig->screen_height;
        if (IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc, &OffscreenSurface, NULL) != 0) {
            IDirectDrawSurface_Release(PrimarySurface);
            DestroyWindow(WNDENV_Gethwnd());
            return 0;
        }
        renderEngine = OffscreenSurface;
        desc.dwSize = sizeof(desc);
        desc.dwFlags = 7;
        desc.ddsCaps.dwCaps = 0x40;
        desc.dwWidth = lpConfig->screen_width;
        desc.dwHeight = lpConfig->screen_height;
        if (IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc, &DAT_00668074, NULL) != 0) {
            return 0;
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00463ef0
int SetDisplayModeAndDetectPixelFormat(void) {
    DDSURFACEDESC desc;
    LPDIRECTDRAW2 ddraw2;

    if (WinDebugMode == 0) {
        ddraw2 = DDRAWENV.ddraw2;
        if (IDirectDraw2_SetDisplayMode(ddraw2, lpConfig->screen_width, lpConfig->screen_height, 0x10, 0, 0) != 0) {
            ddraw2 = DDRAWENV.ddraw2;
            if (IDirectDraw2_SetDisplayMode(ddraw2, lpConfig->screen_width, lpConfig->screen_height, 8, 0, 0) != 0) {
                return 0;
            }
        }
    }
    desc.dwSize = 0x6c;
    ddraw2 = DDRAWENV.ddraw2;
    IDirectDraw2_GetDisplayMode(ddraw2, &desc);
    if (desc.ddpfPixelFormat.dwRGBBitCount != 8) {
        if (desc.ddpfPixelFormat.dwRGBBitCount != 0x10) {
            return 0;
        }
        DisplayPixelFormat = (desc.ddpfPixelFormat.dwGBitMask == 0x7e0) + 1;
        return 1;
    }
    DisplayPixelFormat = 0;
    return 1;
}

// FUNCTION: LEGOLAND 0x00463fc0
LEGO_EXPORT void PushRenderingStatusAndLockVideoSurface(void) {
    RECT local;
    LPDIRECTDRAWSURFACE surface;
    int value;

    value = VideoSurfaceLocked;
    DAT_00668164[RenderingStatusStackDepth] = VideoSurfaceLocked;
    RenderingStatusStackDepth = RenderingStatusStackDepth + 1;
    if (value == 0) {
        local.left = value;
        local.top = value;
        local.right = lpConfig->screen_width - 1;
        local.bottom = lpConfig->screen_height - 1;
        CurrentSurfaceDesc.dwSize = 0x6c;
        IntersectRect(&DAT_00668108, &local, &SPRITE_ClipRect);
        surface = renderEngine;
        if (IDirectDrawSurface_Lock(surface, NULL, &CurrentSurfaceDesc, 0x21, NULL) == 0x887601c2) {
            IDirectDrawSurface_Restore(renderEngine);
            IDirectDrawSurface_Lock(renderEngine, NULL, &CurrentSurfaceDesc, 0x21, NULL);
        }
        StoredTransparentColour = GetTransparentColour();
    }
    VideoSurfaceLocked = 1;
}

// FUNCTION: LEGOLAND 0x00464080
LEGO_EXPORT void PushRenderingStatusAndUnlockVideoSurface(void) {
    LPDIRECTDRAWSURFACE surface;

    DAT_00668164[RenderingStatusStackDepth] = VideoSurfaceLocked;
    RenderingStatusStackDepth = RenderingStatusStackDepth + 1;
    if (VideoSurfaceLocked != 0) {
        surface = renderEngine;
        if (IDirectDrawSurface_Unlock(surface, CurrentSurfaceDesc.lpSurface) == 0x887601c2) {
            IDirectDrawSurface_Restore(renderEngine);
            IDirectDrawSurface_Unlock(renderEngine, CurrentSurfaceDesc.lpSurface);
        }
    }
    VideoSurfaceLocked = 0;
}

// FUNCTION: LEGOLAND 0x004640f0
void FUN_004640f0(void) {
    RECT local;
    LPDIRECTDRAWSURFACE surface;
    int wasLocked;

    wasLocked = VideoSurfaceLocked;
    local.left = 0;
    local.top = 0;
    local.right = lpConfig->screen_width - 1;
    local.bottom = lpConfig->screen_height - 1;
    DAT_00668164[RenderingStatusStackDepth] = VideoSurfaceLocked;
    RenderingStatusStackDepth = RenderingStatusStackDepth + 1;
    if (wasLocked != 0) {
        surface = renderEngine;
        if (IDirectDrawSurface_Unlock(surface, CurrentSurfaceDesc.lpSurface) == 0x887601c2) {
            IDirectDrawSurface_Restore(renderEngine);
            IDirectDrawSurface_Unlock(renderEngine, CurrentSurfaceDesc.lpSurface);
        }
    }
    CurrentSurfaceDesc.dwSize = 0x6c;
    IntersectRect(&DAT_00668108, &local, &SPRITE_ClipRect);
    surface = renderEngine;
    if (IDirectDrawSurface_Lock(surface, NULL, &CurrentSurfaceDesc, 0x21, NULL) == 0x887601c2) {
        IDirectDrawSurface_Restore(renderEngine);
        IDirectDrawSurface_Lock(renderEngine, NULL, &CurrentSurfaceDesc, 0x21, NULL);
    }
    StoredTransparentColour = GetTransparentColour();
    VideoSurfaceLocked = 1;
}

// FUNCTION: LEGOLAND 0x004641f0
LEGO_EXPORT void PopRenderingStatus(void) {
    RECT local;
    LPDIRECTDRAWSURFACE surface;

    RenderingStatusStackDepth = RenderingStatusStackDepth - 1;
    if (DAT_00668164[RenderingStatusStackDepth] != 0) {
        if (VideoSurfaceLocked == 0) {
            local.left = 0;
            local.top = 0;
            local.right = lpConfig->screen_width - 1;
            local.bottom = lpConfig->screen_height - 1;
            CurrentSurfaceDesc.dwSize = 0x6c;
            IntersectRect(&DAT_00668108, &local, &SPRITE_ClipRect);
            surface = renderEngine;
            if (IDirectDrawSurface_Lock(surface, NULL, &CurrentSurfaceDesc, 0x21, NULL) == 0x887601c2) {
                IDirectDrawSurface_Restore(renderEngine);
                IDirectDrawSurface_Lock(renderEngine, NULL, &CurrentSurfaceDesc, 0x21, NULL);
            }
            StoredTransparentColour = GetTransparentColour();
            VideoSurfaceLocked = 1;
        }
        return;
    }
    if (VideoSurfaceLocked != 0) {
        surface = renderEngine;
        if (IDirectDrawSurface_Unlock(surface, CurrentSurfaceDesc.lpSurface) == 0x887601c2) {
            IDirectDrawSurface_Restore(renderEngine);
            IDirectDrawSurface_Unlock(renderEngine, CurrentSurfaceDesc.lpSurface);
        }
        VideoSurfaceLocked = 0;
    }
}

// FUNCTION: LEGOLAND 0x00464310
LEGO_EXPORT int GetVideoSurface(struct VideoArg *arg) {
    if (VideoSurfaceLocked == 0) {
        return 0;
    }
    arg->pitch = CurrentSurfaceDesc.lPitch;
    arg->width = lpConfig->screen_width;
    arg->height = lpConfig->screen_height;
    arg->bits = CurrentSurfaceDesc.lpSurface;
    arg->field_14 = 2;
    return 1;
}

// FUNCTION: LEGOLAND 0x00464360
LEGO_EXPORT void PrintBackground(int x, int y) { return; }

// FUNCTION: LEGOLAND 0x00464370
LEGO_EXPORT void CommitCliprectToHardware(void) {
    RGNDATA *rgn;
    LPDIRECTDRAWCLIPPER clipper;

    rgn = malloc(sizeof(RGNDATA) - 1 + sizeof(RECT));
    rgn->rdh.iType = 1;
    rgn->rdh.nCount = 1;
    rgn->rdh.dwSize = 0x20;
    rgn->rdh.nRgnSize = 0x10;
    rgn->rdh.rcBound = SPRITE_ClipRect;
    memcpy(rgn->Buffer, &SPRITE_ClipRect, sizeof(RECT));
    clipper = DDrawClipper;
    IDirectDrawClipper_SetClipList(clipper, (LPRGNDATA)rgn, 0);
    free(rgn);
}

// FUNCTION: LEGOLAND 0x00464400
LEGO_EXPORT void SetOverridePalette(unsigned int param_1) { OverridePalette = param_1; }

// FUNCTION: LEGOLAND 0x00464410
LEGO_EXPORT unsigned int GetOverridePalette(void) { return OverridePalette; }

// FUNCTION: LEGOLAND 0x00464420
LEGO_EXPORT void SetOverrideFrame(unsigned int param_1) { OverrideFrame = param_1; }

// FUNCTION: LEGOLAND 0x00464430
LEGO_EXPORT unsigned int GetOverrideFrame(void) { return OverrideFrame; }

// FUNCTION: LEGOLAND 0x00464440
LEGO_EXPORT void ClearOverrideFrame(void) { OverrideFrame = 0xffffffff; }

// FUNCTION: LEGOLAND 0x00464450
LEGO_EXPORT void ClearOverridePalette(void) { OverridePalette = 0; }

// FUNCTION: LEGOLAND 0x00464460
LEGO_EXPORT void ClearSpriteOverrides(void) {
    OverrideFrame = 0xffffffff;
    OverridePalette = 0;
}

// Hand-written assembly in the original (ebp frame + xchg, rep movsw/stosw): not reproducible in pure C, left as STUB().
// FUNCTION: LEGOLAND 0x00464480
void FUN_00464480(void) { STUB(); }

// Hand-written assembly in the original (ebp frame + pusha/popa, shrd, xchg): not reproducible in pure C, left as STUB().
// FUNCTION: LEGOLAND 0x00464a90
LEGO_EXPORT void ZBufferHelper(unsigned int *param_1, int *param_2, int *param_3, void *param_4) { STUB(); }

// Hand-written assembly in the original (ebp frame + pusha/popa): not reproducible in pure C, left as STUB().
// FUNCTION: LEGOLAND 0x00464ee0
void __fastcall FUN_00464ee0(struct Sprite *sprite, RECT *rect, int *off) { STUB(); }

// The original fills the rows with an inline __asm block (pusha; rep stosw per row; popa) that
// reads its loop bounds from the globals below. This is the C equivalent: same effect, but it
// cannot byte-match without __asm.
// FUNCTION: LEGOLAND 0x004651d0
LEGO_EXPORT void SoftPrint_Clear(void) {
    unsigned short colour = GetTransparentColour();
    unsigned short *row;
    int y;
    int x;

    DAT_007fea14 = CurrentSurfaceDesc.dwHeight;
    DAT_007fea1c = CurrentSurfaceDesc.dwWidth;
    DAT_007fe9a4 = DAT_007fea1c;
    row = CurrentSurfaceDesc.lpSurface;
    for (y = DAT_007fea14; y != 0; y--) {
        for (x = 0; x < DAT_007fe9a4; x++) {
            row[x] = colour;
        }
        row = (unsigned short *)((char *)row + CurrentSurfaceDesc.lPitch);
    }
}

// Hand-written assembly in the original (ebp frame + pusha/popa, shrd, xchg): not reproducible in pure C, left as STUB().
// FUNCTION: LEGOLAND 0x00465240
void FUN_00465240(void) { STUB(); }

// FUNCTION: LEGOLAND 0x00465850
void FUN_00465850(struct AviFrame *frame) {
    unsigned char *dst;
    unsigned char *next;
    unsigned short *row;
    unsigned short *s;
    unsigned short *p0;
    unsigned short *p1;
    unsigned short v;
    int width;
    int height;
    int half;
    int rest;
    int y;
    int x;

    dst = CurrentSurfaceDesc.lpSurface;
    DAT_006681ec = (DAT_006681ec != dst) ? dst : 0;
    height = frame->height;
    width = frame->width;
    rest = lpConfig->screen_height - height * 2;
    half = rest / 2;
    rest = rest - half;
    row = frame->pixels + (height - 1) * width;
    for (y = half; y > 0; y--) {
        memset(dst, 0, 0x500);
        dst += CurrentSurfaceDesc.lPitch;
    }
    for (y = height; y != 0; y--) {
        next = dst + CurrentSurfaceDesc.lPitch;
        s = row;
        p0 = (unsigned short *)dst;
        p1 = (unsigned short *)next;
        for (x = width; x > 0; x--) {
            v = *s;
            if (DisplayPixelFormat == 2) {
                v = (v & 0x1f) | (v & 0xffe0) << 1;
            }
            p0[0] = v;
            p1[0] = v;
            p0[1] = v;
            p1[1] = v;
            s++;
            p0 += 2;
            p1 += 2;
        }
        row -= width;
        dst += CurrentSurfaceDesc.lPitch * 2;
    }
    dst = next + CurrentSurfaceDesc.lPitch;
    for (y = rest; y > 0; y--) {
        memset(dst, 0, 0x500);
        dst += CurrentSurfaceDesc.lPitch;
    }
}

// FUNCTION: LEGOLAND 0x004659a0
void FUN_004659a0(struct AviFrame *param_1, int param_2, int param_3) {
    int height;
    int width;
    int last;
    unsigned short *src;
    unsigned short *dst;
    int offset;
    int i;
    int j;
    unsigned short *p;
    short v;

    height = param_1->height;
    width = param_1->width;
    dst = (unsigned short *)((char *)CurrentSurfaceDesc.lpSurface + CurrentSurfaceDesc.lPitch * param_3 + param_2 * 2);
    last = height - 1;
    src = param_1->pixels + last * width;
    if (height != 0) {
        i = last + 1;
        do {
            if (width > 0) {
                offset = (char *)src - (char *)dst;
                p = dst;
                j = width;
                do {
                    v = *(short *)(offset + (char *)p);
                    if (DisplayPixelFormat == 2) {
                        v = (v & 0x1f) | (v & ~0x1f) << 1;
                    }
                    *p = v;
                    p++;
                    j--;
                } while (j != 0);
            }
            dst = (unsigned short *)((char *)dst + CurrentSurfaceDesc.lPitch);
            src -= width;
            i--;
        } while (i != 0);
    }
}

// Hand-written assembly in the original (ebp frame + pusha/popa): not reproducible in pure C, left as STUB().
// FUNCTION: LEGOLAND 0x00465a40
LEGO_EXPORT void SoftPrint_XBltFast(struct Sprite *sprite, RECT *a, RECT *b, unsigned int param_4) { STUB(); }

// FUNCTION: LEGOLAND 0x00465ee0
void FUN_00465ee0(struct DrawLLS *lls, RECT *clip, struct Point *pos) {
    struct DrawLLSFrame *frame = (struct DrawLLSFrame *)(lls + 1);
    unsigned short *dst;
    unsigned short *pixels;
    unsigned char *runs;
    unsigned int *mask;
    int frame_index;
    int i;
    int off;

    DAT_007fe9a4 = CurrentSurfaceDesc.lPitch;
    SpriteClipOrigin.left = clip->left;
    DrawClipExtent.width = clip->right - clip->left;
    SpriteClipOrigin.top = clip->top;
    DrawClipExtent.height = clip->bottom - clip->top;
    if ((int)OverrideFrame < 0) {
        frame_index = lls->frame;
    } else {
        frame_index = (int)OverrideFrame;
    }
    if (frame_index >= lls->frame_count) {
        frame_index = lls->frame_count - 1;
    }
    DAT_007fe9a8 = (unsigned int)((unsigned char *)CurrentSurfaceDesc.lpSurface + MousePos.y * CurrentSurfaceDesc.lPitch + MousePos.x * 2);
    dst = (unsigned short *)((unsigned char *)CurrentSurfaceDesc.lpSurface + pos->y * CurrentSurfaceDesc.lPitch + (pos->x - SpriteClipOrigin.left) * 2);
    if (lls->flags & 1) {
        pixels = frame->pixels;
        off = frame->pixel_count * 2 + 0x10;
        runs = (unsigned char *)frame + off;
        mask = (unsigned int *)((unsigned char *)frame + (frame->run_bytes + off));
        FUN_00468040(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
        i = lls->frame + 1;
        while (i-- != 0) {
            frame = (struct DrawLLSFrame *)((unsigned char *)frame + frame->size);
        }
        pixels = frame->pixels;
        off = frame->pixel_count * 2 + 0x10;
        runs = (unsigned char *)frame + off;
        mask = (unsigned int *)((unsigned char *)frame + (frame->run_bytes + off));
        FUN_00468040(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
    } else {
        i = frame_index;
        while (i-- != 0) {
            frame = (struct DrawLLSFrame *)((unsigned char *)frame + frame->size);
        }
        pixels = frame->pixels;
        off = frame->pixel_count * 2 + 0x10;
        runs = (unsigned char *)frame + off;
        mask = (unsigned int *)((unsigned char *)frame + (frame->run_bytes + off));
        FUN_00468040(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
    }
}

// FUNCTION: LEGOLAND 0x00466080
int FlipFrame(void) {
    DWORD tick;
    int result;
    int frames;
    LPDIRECTDRAWSURFACE primary;
    LPDIRECTDRAWSURFACE back;

    LLSAuto();
    if (lpConfig->field_1e != 0 && DAT_00668148 != 0) {
        PushRenderingStatusAndLockVideoSurface();
        PrintSprite(DAT_00668148, MousePos.x, MousePos.y, 0, 0);
        PopRenderingStatus();
    }
    tick = GetTickCount();
    while (tick - LastPresentTicks < 0x1c) {
        tick = GetTickCount();
    }
    LastPresentTicks = GetTickCount();
    primary = PrimarySurface;
    result = IDirectDrawSurface_Flip(primary, NULL, 1);
    while (result != 0) {
        if (result == 0x887601c2) {
            IDirectDrawSurface_Restore(PrimarySurface);
            IDirectDrawSurface_Restore(OffscreenSurface);
            return 0;
        }
        if (result != 0x887601ae && result != 0x8876021c) {
            return 0;
        }
        primary = PrimarySurface;
        result = IDirectDrawSurface_Flip(primary, NULL, 1);
    }
    back = OffscreenSurface;
    result = IDirectDrawSurface_GetFlipStatus(back, 2);
    while (result != 0) {
        back = OffscreenSurface;
        result = IDirectDrawSurface_GetFlipStatus(back, 2);
    }
    tick = GetTickCount();
    FrameNumber = FrameNumber + 1;
    FramesThisSecond = FramesThisSecond + 1;
    if (tick - FpsWindowStartTicks >= 0x3e8) {
        FramesPerSecond = FramesThisSecond;
        FramesThisSecond = 0;
        FpsWindowStartTicks = tick;
    }
    LastFrameMS = tick - LastFrameTicks;
    LastFrameTicks = tick;
    return 1;
}

// FUNCTION: LEGOLAND 0x004661d0
int BlitFrameToWindow(void) {
    RECT dst;
    union RectPoints client;
    DWORD tick;
    LPDIRECTDRAWSURFACE surface;
    int result;
    int frames;

    dst.left = 0;
    dst.top = 0;
    dst.right = 0x280;
    dst.bottom = 0x1e0;
    LLSAuto();
    if (lpConfig->field_1e != 0 && DAT_00668148 != 0) {
        PushRenderingStatusAndLockVideoSurface();
        PrintSprite(DAT_00668148, MousePos.x, MousePos.y, 0, 0);
        PopRenderingStatus();
    }
    tick = GetTickCount();
    while (tick - LastPresentTicks < 0x1c) {
        tick = GetTickCount();
    }
    LastPresentTicks = GetTickCount();
    GetClientRect(WNDENV_Gethwnd(), &client.rect);
    ClientToScreen(WNDENV_Gethwnd(), &client.pt[0]);
    OffsetRect(&dst, client.rect.left, client.rect.top);
    surface = PrimarySurface;
    result = IDirectDrawSurface_Blt(surface, &dst, OffscreenSurface, NULL, 0x1000000, NULL);
    if (result == 0x887601c2) {
        IDirectDrawSurface_Restore(PrimarySurface);
        result = IDirectDrawSurface_Blt(PrimarySurface, &dst, OffscreenSurface, NULL, 0x1000000, NULL);
    }
    if (result != 0) {
        return 0;
    }
    tick = GetTickCount();
    FrameNumber = FrameNumber + 1;
    FramesThisSecond = FramesThisSecond + 1;
    if (tick - FpsWindowStartTicks >= 0x3e8) {
        FramesPerSecond = FramesThisSecond;
        FramesThisSecond = 0;
        FpsWindowStartTicks = tick;
    }
    LastFrameMS = tick - LastFrameTicks;
    LastFrameTicks = tick;
    return 1;
}

// FUNCTION: LEGOLAND 0x00466360
void LoadWatchSprite(int a, int b) {
    short w;
    short h;

    if (WatchSprite == NULL) {
        // STRING: LEGOLAND 0x004b9d30
        WatchSprite = LoadSprite("Watch.lls", 4);
        if (WatchSprite == NULL) {
            return;
        }
    }
    WatchActive = 1;
    w = WatchSprite->width;
    h = WatchSprite->height;
    WatchRect.left = a;
    WatchRect.top = b;
    WatchRect.right = w + a;
    WatchRect.bottom = h + b;
}

// FUNCTION: LEGOLAND 0x004663c0
void UnloadWatchSprite(void) {
    if (WatchSprite != 0) {
        KillSprite(WatchSprite);
        WatchSprite = 0;
    }
    WatchActive = 0;
}

// FUNCTION: LEGOLAND 0x004663f0
void DrawWatchSprite(void) {
    union RectPoints cursor;
    LPDIRECTDRAWSURFACE surface;
    struct Image *image;

    if (WatchActive != 0) {
        if ((int)(GetTicks() - LastWatchDrawTicks) > 0xc8) {
            cursor.rect.left = WatchRect.left;
            cursor.rect.top = WatchRect.top;
            cursor.rect.right = WatchRect.right;
            cursor.rect.bottom = WatchRect.bottom;
            LastWatchDrawTicks = GetTicks();
            image = WatchSprite->image;
            LLSAdvanceFrame((struct LLS *)image->data);
            PushRenderingStatusAndLockVideoSurface();
            PrintSprite(WatchSprite, WatchRect.left, WatchRect.top, 0, 0);
            PopRenderingStatus();
            ClientToScreen(WNDENV_Gethwnd(), &cursor.pt[0]);
            ClientToScreen(WNDENV_Gethwnd(), &cursor.pt[1]);
            surface = PrimarySurface;
            if (IDirectDrawSurface_Blt(surface, &cursor.rect, OffscreenSurface, &WatchRect, 0x1000000, NULL) == 0x887601c2) {
                IDirectDrawSurface_Restore(PrimarySurface);
                IDirectDrawSurface_Blt(PrimarySurface, &cursor, OffscreenSurface, NULL, 0x1000000, NULL);
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x00466500
LEGO_EXPORT int RenderingComplete(void) {
    int result;

    ProcessSystemEvents();
    FlushTextCells(0);
#if defined(_MSC_VER) && (_MSC_VER <= 1200) && defined(_M_IX86)
    __asm {
        pushad
        rdtsc
        mov ebx, DAT_00813a18
        sub eax, ebx
        mov DAT_00813a18, eax
        add DAT_00813a2c, eax
        popad
    }
#else
    {
        int tsc = (int)GetTickCount();
        tsc -= DAT_00813a18;
        DAT_00813a18 = tsc;
        DAT_00813a2c += tsc;
    }
#endif
    LastRenderingCompleteTick = GetTickCount();
    result = BlitFrameFunc();
    DAT_00813a2c = 0;
#if defined(_MSC_VER) && (_MSC_VER <= 1200) && defined(_M_IX86)
    __asm {
        pushad
        rdtsc
        mov DAT_00813a18, eax
        popad
    }
#else
    DAT_00813a18 = (int)GetTickCount();
#endif
    return result;
}

// FUNCTION: LEGOLAND 0x00466560
LEGO_EXPORT void PushSetTarget(struct Sprite *sprite) {
    LPDIRECTDRAWSURFACE surface;
    int locked;

    locked = VideoSurfaceLocked;
    DAT_00668164[RenderingStatusStackDepth] = VideoSurfaceLocked;
    RenderingStatusStackDepth = RenderingStatusStackDepth + 1;
    if (locked != 0) {
        surface = renderEngine;
        if (IDirectDrawSurface_Unlock(surface, CurrentSurfaceDesc.lpSurface) == 0x887601c2) {
            IDirectDrawSurface_Restore(renderEngine);
            IDirectDrawSurface_Unlock(renderEngine, CurrentSurfaceDesc.lpSurface);
        }
    }
    VideoSurfaceLocked = 0;
    renderEngineTargets[renderEngineTargetIdx] = renderEngine;
    renderEngine = sprite->surface;
    renderEngineTargetIdx++;
    FUN_004640f0();
}

// FUNCTION: LEGOLAND 0x00466600
LEGO_EXPORT void PopTarget(void) {
    PopRenderingStatus();
    renderEngineTargetIdx--;
    renderEngine = renderEngineTargets[renderEngineTargetIdx];
    PopRenderingStatus();
    if (renderEngineTargetIdx >= 0) {
        return;
    }
    exit(2);
}

// FUNCTION: LEGOLAND 0x00466640
LEGO_EXPORT int RecreateSprite(struct Sprite *sprite) {
    DDSURFACEDESC desc;
    DDCOLORKEY colorkey;
    LPDIRECTDRAW2 ddraw2;
    LPDIRECTDRAWSURFACE surface;

    desc.dwSize = 0x6c;
    desc.dwWidth = (short)sprite->width;
    desc.dwFlags = 7;
    desc.ddsCaps.dwCaps = 0x40;
    desc.dwHeight = (short)sprite->height;
    for (;;) {
        if ((sprite->flags & 0x10) == 0) {
            ddraw2 = DDRAWENV.ddraw2;
            if (IDirectDraw2_CreateSurface(ddraw2, &desc, &sprite->surface, NULL) == 0) {
                break;
            }
        }
        desc.dwWidth = (short)sprite->width;
        desc.dwHeight = (short)sprite->height;
        desc.dwSize = 0x6c;
        desc.dwFlags = 7;
        desc.ddsCaps.dwCaps = 0x840;
        ddraw2 = DDRAWENV.ddraw2;
        if (IDirectDraw2_CreateSurface(ddraw2, &desc, &sprite->surface, NULL) != 0) {
            ddraw2 = DDRAWENV.ddraw2;
            if (IDirectDraw2_Compact(ddraw2) == 0) {
                IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc, &sprite->surface, NULL);
            }
            return 0;
        }
        break;
    }
    if ((sprite->flags & 0x80) != 0) {
        colorkey.dwColorSpaceLowValue = GetNearestColour(0xff, 0, 0xff);
        surface = sprite->surface;
        colorkey.dwColorSpaceHighValue = colorkey.dwColorSpaceLowValue;
        IDirectDrawSurface_SetColorKey(surface, 8, &colorkey);
        return 1;
    }
    colorkey.dwColorSpaceLowValue = GetTransparentColour();
    surface = sprite->surface;
    colorkey.dwColorSpaceHighValue = colorkey.dwColorSpaceLowValue;
    IDirectDrawSurface_SetColorKey(surface, 8, &colorkey);
    return 1;
}

// FUNCTION: LEGOLAND 0x00466770
void FUN_00466770(struct DrawLLS *lls, RECT *clip, struct Point *pos) {
    struct DrawLLSFrame *frame = (struct DrawLLSFrame *)(lls + 1);
    unsigned short *dst;
    unsigned short *pixels;
    unsigned char *runs;
    unsigned int *mask;
    int frame_index;
    int hit;
    int i;

    DAT_007fe9a4 = CurrentSurfaceDesc.lPitch;
    SpriteClipOrigin.left = clip->left;
    DrawClipExtent.width = clip->right - clip->left;
    SpriteClipOrigin.top = clip->top;
    DrawClipExtent.height = clip->bottom - clip->top;
    frame_index = (int)OverrideFrame;
    if (frame_index < 0) {
        frame_index = lls->frame;
    }
    if (frame_index >= lls->frame_count) {
        frame_index = lls->frame_count - 1;
    }
    hit = 0;
    DAT_007fe9a8 = (unsigned int)((unsigned char *)CurrentSurfaceDesc.lpSurface + MousePos.y * CurrentSurfaceDesc.lPitch + MousePos.x * 2);
    if (MousePos.x >= pos->x && MousePos.x <= pos->x + lls->width && MousePos.y >= pos->y && MousePos.y <= pos->y + lls->height) {
        hit = 1;
    }
    dst = (unsigned short *)((unsigned char *)CurrentSurfaceDesc.lpSurface + pos->y * CurrentSurfaceDesc.lPitch + (pos->x - SpriteClipOrigin.left) * 2);
    if (lls->flags & 1) {
        pixels = frame->pixels;
        runs = (unsigned char *)frame + frame->pixel_count * 2 + 0x10;
        mask = (unsigned int *)(runs + frame->run_bytes);
        if (hit) {
            if (lls->width <= DrawClipExtent.width - SpriteClipOrigin.left) {
                FUN_00467640(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
            } else if (SpriteClipOrigin.left != 0) {
                if (lls->width - SpriteClipOrigin.left > DrawClipExtent.width) {
                    FUN_00466d80(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                } else {
                    FUN_00467180(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                }
            } else {
                FUN_004673f0(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
            }
        } else {
            if (lls->width <= DrawClipExtent.width - SpriteClipOrigin.left) {
                FUN_00467f00(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top);
            } else if (SpriteClipOrigin.left != 0) {
                if (lls->width - SpriteClipOrigin.left > DrawClipExtent.width) {
                    FUN_004677b0(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                } else {
                    FUN_00467b00(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                }
            } else {
                FUN_00467d10(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
            }
        }
        i = lls->frame + 1;
        while (i-- != 0) {
            frame = (struct DrawLLSFrame *)((unsigned char *)frame + frame->size);
        }
        pixels = frame->pixels;
        runs = (unsigned char *)frame + frame->pixel_count * 2 + 0x10;
        mask = (unsigned int *)(runs + frame->run_bytes);
        if (hit) {
            if (lls->width <= DrawClipExtent.width - SpriteClipOrigin.left) {
                FUN_00467640(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
            } else if (SpriteClipOrigin.left != 0) {
                if (lls->width - SpriteClipOrigin.left > DrawClipExtent.width) {
                    FUN_00466d80(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                } else {
                    FUN_00467180(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                }
            } else {
                FUN_004673f0(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
            }
        } else {
            if (lls->width <= DrawClipExtent.width - SpriteClipOrigin.left) {
                FUN_00467f00(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top);
            } else if (SpriteClipOrigin.left != 0) {
                if (lls->width - SpriteClipOrigin.left > DrawClipExtent.width) {
                    FUN_004677b0(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                } else {
                    FUN_00467b00(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                }
            } else {
                FUN_00467d10(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
            }
        }
    } else {
        i = frame_index;
        while (i-- != 0) {
            frame = (struct DrawLLSFrame *)((unsigned char *)frame + frame->size);
        }
        pixels = frame->pixels;
        runs = (unsigned char *)frame + frame->pixel_count * 2 + 0x10;
        mask = (unsigned int *)(runs + frame->run_bytes);
        if (hit) {
            if (lls->width <= DrawClipExtent.width - SpriteClipOrigin.left) {
                FUN_00467640(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
            } else if (SpriteClipOrigin.left != 0) {
                if (lls->width - SpriteClipOrigin.left > DrawClipExtent.width) {
                    FUN_00466d80(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                } else {
                    FUN_00467180(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                }
            } else {
                FUN_004673f0(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
            }
        } else {
            if (lls->width <= DrawClipExtent.width - SpriteClipOrigin.left) {
                FUN_00467f00(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top);
            } else if (SpriteClipOrigin.left != 0) {
                if (lls->width - SpriteClipOrigin.left > DrawClipExtent.width) {
                    FUN_004677b0(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                } else {
                    FUN_00467b00(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                }
            } else {
                FUN_00467d10(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
            }
        }
    }
}

// Hand-written assembly in the original (rol, rep movsw/stosw): not reproducible in pure C, left as STUB().
// FUNCTION: LEGOLAND 0x00466d80
void FUN_00466d80(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) { STUB(); }

// Hand-written assembly in the original (rol, rep movsw/stosw): not reproducible in pure C, left as STUB().
// FUNCTION: LEGOLAND 0x00467180
void FUN_00467180(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) { STUB(); }

// Hand-written assembly in the original (rol, rep movsw/stosw): not reproducible in pure C, left as STUB().
// FUNCTION: LEGOLAND 0x004673f0
void FUN_004673f0(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) { STUB(); }

// FUNCTION: LEGOLAND 0x00467640
void FUN_00467640(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) {
    unsigned int m;
    unsigned int c;
    unsigned int n;
    unsigned short *p;
    unsigned short v;

    m = 3;
    if (skip != 0) {
        do {
            for (;;) {
                c = *mask & m;
                m = _rotl(m, 2);
                mask += m & 1;
                if (!(c & 0xaaaaaaaa)) {
                    src++;
                }
                if (c & 0x55555555) {
                    n = *runs++;
                    if (n == 0) {
                        break;
                    }
                    c = *mask & m;
                    m = _rotl(m, 2);
                    mask += m & 1;
                    if (!(c & 0xaaaaaaaa)) {
                        if (c & 0x55555555) {
                            src++;
                        } else {
                            src += n;
                        }
                    }
                }
            }
        } while (--skip > 0);
    }
    p = dst;
    do {
        for (;;) {
            c = *mask & m;
            m = _rotl(m, 2);
            mask += m & 1;
            if (c & 0xaaaaaaaa) {
                p++;
                if (c & 0x55555555) {
                    p--;
                    n = *runs++;
                    if (n == 0) {
                        break;
                    }
                    c = *mask & m;
                    m = _rotl(m, 2);
                    mask += m & 1;
                    if (!(c & 0xaaaaaaaa)) {
                        if ((unsigned int)(cursor - p) < n) {
                            DAT_007feb14 |= 1;
                        }
                        if (!(c & 0x55555555)) {
                            do {
                                *p++ = *src++;
                            } while (--n);
                        } else {
                            v = *src++;
                            do {
                                *p++ = v;
                            } while (--n);
                        }
                    } else {
                        p += n;
                    }
                }
            } else {
                if (cursor == p) {
                    DAT_007feb14 |= 1;
                }
                *p++ = *src++;
            }
        }
        dst = (unsigned short *)((char *)dst + stride);
        p = dst;
    } while (--h != 0);
}

// Hand-written assembly in the original (rol, rep movsw/stosw): not reproducible in pure C, left as STUB().
// FUNCTION: LEGOLAND 0x004677b0
void FUN_004677b0(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) { STUB(); }

// Hand-written assembly in the original (rol, rep movsw/stosw): not reproducible in pure C, left as STUB().
// FUNCTION: LEGOLAND 0x00467b00
void FUN_00467b00(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) { STUB(); }

// Hand-written assembly in the original (rol, rep movsw/stosw): not reproducible in pure C, left as STUB().
// FUNCTION: LEGOLAND 0x00467d10
void FUN_00467d10(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) { STUB(); }

// FUNCTION: LEGOLAND 0x00467f00
void FUN_00467f00(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip) {
    unsigned int m;
    unsigned int c;
    unsigned int n;
    unsigned short *p;
    unsigned short v;

    m = 3;
    if (skip != 0) {
        do {
            for (;;) {
                c = *mask & m;
                m = _rotl(m, 2);
                mask += m & 1;
                if (!(c & 0xaaaaaaaa)) {
                    src++;
                } else if (c & 0x55555555) {
                    n = *runs++;
                    if (n == 0) {
                        break;
                    }
                    c = *mask & m;
                    m = _rotl(m, 2);
                    mask += m & 1;
                    if (!(c & 0xaaaaaaaa)) {
                        if (c & 0x55555555) {
                            src++;
                        } else {
                            src += n;
                        }
                    }
                }
            }
        } while (--skip > 0);
    }
    p = dst;
    do {
        for (;;) {
            c = *mask & m;
            m = _rotl(m, 2);
            mask += m & 1;
            if (c & 0xaaaaaaaa) {
                p++;
                if (c & 0x55555555) {
                    p--;
                    n = *runs++;
                    if (n == 0) {
                        break;
                    }
                    c = *mask & m;
                    m = _rotl(m, 2);
                    mask += m & 1;
                    if (!(c & 0xaaaaaaaa)) {
                        if (!(c & 0x55555555)) {
                            do {
                                *p++ = *src++;
                            } while (--n);
                        } else {
                            v = *src++;
                            do {
                                *p++ = v;
                            } while (--n);
                        }
                    } else {
                        p += n;
                    }
                }
            } else {
                *p++ = *src++;
            }
        }
        dst = (unsigned short *)((char *)dst + stride);
        p = dst;
    } while (--h != 0);
}

// Hand-written assembly in the original (rol, rep movsw/stosw): not reproducible in pure C, left as STUB().
// FUNCTION: LEGOLAND 0x00468040
void FUN_00468040(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) { STUB(); }

// Hand-written assembly in the original (rol, rep movsw/stosw): not reproducible in pure C, left as STUB().
// FUNCTION: LEGOLAND 0x00468410
void FUN_00468410(void) { STUB(); }

// FUNCTION: LEGOLAND 0x004687f0
void FUN_004687f0(const char *param_1) {
    strncpy(DAT_0066861c, param_1, 0x80);
    DAT_0066861c[0x7f] = 0;
}
