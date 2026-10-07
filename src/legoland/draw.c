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

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x00464480
__declspec(naked) void FUN_00464480(void) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x14
        mov eax, dword ptr [CurrentSurfaceDesc + 0x10]
        mov ecx, dword ptr [ebp + 8]
        mov dword ptr [DAT_007fe9a4], eax
        mov eax, dword ptr [OverrideFrame]
        push ebx
        mov edx, 1
        push esi
        push edi
        test eax, eax
        mov dword ptr [ebp - 8], edx
        jl L4644ae
        mov esi, eax
        mov dword ptr [ebp + 8], esi
        jmp L4644b6
L4644ae:
        movsx eax, word ptr [ecx]
        mov dword ptr [ebp + 8], eax
        mov esi, eax
L4644b6:
        movsx eax, word ptr [ecx + 0x10]
        cmp esi, eax
        jl L4644c4
        dec eax
        mov dword ptr [ebp + 8], eax
        mov esi, eax
L4644c4:
        test byte ptr [ecx + 0x14], dl
        je L4644d0
        mov dword ptr [ebp - 8], 2
L4644d0:
        mov eax, dword ptr [ebp - 8]
        mov dword ptr [ebp - 0x10], 0
        test eax, eax
        jle L464a7c
        add ecx, 0x18
        mov dword ptr [ebp - 0x14], ecx
        jmp L4644f2
L4644ea:
        mov esi, dword ptr [ebp + 8]
        mov edx, 1
L4644f2:
        mov ecx, dword ptr [ebp - 8]
        mov eax, dword ptr [ebp - 0x14]
        cmp ecx, edx
        jne L464547
        mov edx, dword ptr [OverridePalette]
        mov ecx, dword ptr [eax + 4]
        test edx, edx
        je L464511
        mov dword ptr [DAT_007fea20], edx
        jmp L46451b
L464511:
        lea ecx, [ecx + eax + 8]
        mov dword ptr [DAT_007fea20], ecx
L46451b:
        test esi, esi
        je L464528
        lea ecx, [esi]
L464521:
        mov edi, dword ptr [eax]
        add eax, edi
        dec ecx
        jne L464521
L464528:
        mov ecx, dword ptr [eax + 4]
        lea edx, [eax + 8]
        test esi, esi
        mov dword ptr [ebp - 0xc], edx
        jne L46453e
        lea eax, [ecx + eax + 0x208]
        jmp L464593
L46453e:
        lea ecx, [ecx + eax + 8]
        mov dword ptr [ebp - 4], ecx
        jmp L464596
L464547:
        mov ecx, dword ptr [ebp - 0x10]
        test ecx, ecx
        jne L464574
        mov ecx, dword ptr [eax + 4]
        lea edx, [eax + 8]
        mov dword ptr [ebp - 0xc], edx
        mov edx, dword ptr [OverridePalette]
        test edx, edx
        jne L464565
        lea edx, [ecx + eax + 8]
L464565:
        mov dword ptr [DAT_007fea20], edx
        lea eax, [ecx + eax + 0x208]
        jmp L464593
L464574:
        lea ecx, [esi + 1]
        mov edx, ecx
        dec ecx
        test edx, edx
        je L464586
        inc ecx
L46457f:
        mov esi, dword ptr [eax]
        add eax, esi
        dec ecx
        jne L46457f
L464586:
        mov edx, dword ptr [eax + 4]
        lea ecx, [eax + 8]
        mov dword ptr [ebp - 0xc], ecx
        lea eax, [edx + eax + 8]
L464593:
        mov dword ptr [ebp - 4], eax
L464596:
        push esi
        push edi
        push ebx
        push ecx
        push edx
        push ebp
        mov eax, dword ptr [ebp + 0x10]
        mov edi, dword ptr [CurrentSurfaceDesc + 0x24]
        add edi, dword ptr [eax]
        add edi, dword ptr [eax]
        mov ecx, dword ptr [eax + 4]
        imul ecx, dword ptr [CurrentSurfaceDesc + 0x10]
        add edi, ecx
        mov dword ptr [DAT_007feb18], edi
        mov eax, dword ptr [ebp + 0xc]
        mov ecx, dword ptr [eax]
        mov dword ptr [SpriteClipOrigin + 0x4], ecx
        mov ebx, dword ptr [eax + 8]
        sub ebx, ecx
        mov dword ptr [DrawClipExtent + 0x4], ebx
        mov ecx, dword ptr [eax + 4]
        mov dword ptr [SpriteClipOrigin], ecx
        mov edx, ecx
        mov ebx, dword ptr [eax + 0xc]
        sub ebx, ecx
        mov dword ptr [DrawClipExtent], ebx
        mov esi, dword ptr [ebp - 0xc]
        mov ebp, dword ptr [ebp - 4]
        xor eax, eax
        mov dword ptr [DAT_00668160], eax
        and edx, edx
        je L464693
L4645fc:
        mov eax, dword ptr [DAT_00668160]
        shr ebx, 2
        dec eax
        jns L464612
        mov eax, 0xf
        mov ebx, dword ptr [ebp]
        add ebp, 4
L464612:
        mov dword ptr [DAT_00668160], eax
        test ebx, 2
        je L46467e
        test ebx, 1
        je L4645fc
        mov eax, dword ptr [DAT_00668160]
        shr ebx, 2
        sub eax, 4
        movzx ecx, bl
        jns L464645
        mov eax, 0xc
        mov ebx, dword ptr [ebp]
        movzx ecx, bl
        add ebp, 4
L464645:
        mov dword ptr [DAT_00668160], eax
        shr ebx, 6
        test ecx, ecx
        je L46468c
        mov eax, dword ptr [DAT_00668160]
        shr ebx, 2
        dec eax
        jns L464667
        mov eax, 0xf
        mov ebx, dword ptr [ebp]
        add ebp, 4
L464667:
        mov dword ptr [DAT_00668160], eax
        test ebx, 2
        je L464676
        jmp L4645fc
L464676:
        test ebx, 1
        je L464684
L46467e:
        inc esi
        jmp L4645fc
L464684:
        lea esi, [esi + ecx]
        jmp L4645fc
L46468c:
        dec edx
        jne L4645fc
L464693:
        mov edx, dword ptr [DrawClipExtent]
        and edx, edx
        je L464a64
L4646a1:
        mov dword ptr [DAT_007fea10], edx
        mov edx, dword ptr [SpriteClipOrigin + 0x4]
        and edx, edx
        je L4647dc
L4646b5:
        mov eax, dword ptr [DAT_00668160]
        shr ebx, 2
        dec eax
        jns L4646cb
        mov eax, 0xf
        mov ebx, dword ptr [ebp]
        add ebp, 4
L4646cb:
        mov dword ptr [DAT_00668160], eax
        test ebx, 2
        je L464796
        test ebx, 1
        je L464797
        mov eax, dword ptr [DAT_00668160]
        shr ebx, 2
        sub eax, 4
        movzx ecx, bl
        jns L464706
        mov eax, 0xc
        mov ebx, dword ptr [ebp]
        movzx ecx, bl
        add ebp, 4
L464706:
        mov dword ptr [DAT_00668160], eax
        shr ebx, 6
        test ecx, ecx
        je L464a45
        mov eax, dword ptr [DAT_00668160]
        shr ebx, 2
        dec eax
        jns L46472c
        mov eax, 0xf
        mov ebx, dword ptr [ebp]
        add ebp, 4
L46472c:
        mov dword ptr [DAT_00668160], eax
        test ebx, 2
        je L46475b
        sub edx, ecx
        jns L4646b5
        neg edx
        lea edi, [edi + edx*2]
        mov ecx, dword ptr [DrawClipExtent + 0x4]
        sub ecx, edx
        js L4649b5
        mov edx, ecx
        jmp L4647e2
L46475b:
        test ebx, 1
        je L4647a6
        inc esi
        sub edx, ecx
        jns L4646b5
        neg edx
        mov ecx, edx
        mov edx, dword ptr [DrawClipExtent + 0x4]
        and edx, edx
        je L4649b5
        cmp edi, dword ptr [DAT_007fe9a8]
        setbe al
        mov byte ptr [DAT_007fea18], al
        and ecx, ecx
        jne L464884
        jmp L4647e2
L464796:
        inc esi
L464797:
        dec edx
        jne L4646b5
        mov edx, dword ptr [DrawClipExtent + 0x4]
        jmp L4647e2
L4647a6:
        add esi, ecx
        sub edx, ecx
        jns L4646b5
        neg edx
        mov ecx, edx
        mov edx, dword ptr [DrawClipExtent + 0x4]
        and edx, edx
        je L4649b5
        sub esi, ecx
        cmp edi, dword ptr [DAT_007fe9a8]
        setbe al
        mov byte ptr [DAT_007fea18], al
        and ecx, ecx
        jne L46492c
        jmp L4647e2
L4647dc:
        mov edx, dword ptr [DrawClipExtent + 0x4]
L4647e2:
        mov eax, dword ptr [DAT_00668160]
        shr ebx, 2
        dec eax
        jns L4647f8
        mov eax, 0xf
        mov ebx, dword ptr [ebp]
        add ebp, 4
L4647f8:
        mov dword ptr [DAT_00668160], eax
        test ebx, 2
        je L464919
        test ebx, 1
        je L4649aa
        mov eax, dword ptr [DAT_00668160]
        shr ebx, 2
        sub eax, 4
        movzx ecx, bl
        jns L464833
        mov eax, 0xc
        mov ebx, dword ptr [ebp]
        movzx ecx, bl
        add ebp, 4
L464833:
        mov dword ptr [DAT_00668160], eax
        shr ebx, 6
        test ecx, ecx
        je L464a45
        mov eax, dword ptr [DAT_00668160]
        shr ebx, 2
        dec eax
        jns L464859
        mov eax, 0xf
        mov ebx, dword ptr [ebp]
        add ebp, 4
L464859:
        mov dword ptr [DAT_00668160], eax
        cmp edi, dword ptr [DAT_007fe9a8]
        setbe al
        mov byte ptr [DAT_007fea18], al
        test ebx, 2
        je L464887
        sub edx, ecx
        js L4649b5
        lea edi, [edi + ecx*2]
        jmp L4647e2
L464884:
        sub esi, 1
L464887:
        test ebx, 1
        je L46492c
        movzx eax, byte ptr [esi]
        cmp edx, ecx
        ja L4648e3
        add eax, eax
        inc esi
        add eax, dword ptr [DAT_007fea20]
        mov ecx, edx
        movzx eax, word ptr [eax]
        push eax
        shl eax, 0x10
        add eax, dword ptr [esp]
        add esp, 4
        shr ecx, 1
        mov word ptr [edi], ax
        jae L4648bc
        add edi, 2
L4648bc:
        rep stosd
        cmp edi, dword ptr [DAT_007fe9a8]
        movzx eax, byte ptr [DAT_007fea18]
        sbb ecx, -1
        and eax, ecx
        movzx ecx, byte ptr [DAT_007feb14]
        or eax, ecx
        mov byte ptr [DAT_007feb14], al
        jmp L4649b5
L4648e3:
        add eax, eax
        inc esi
        add eax, dword ptr [DAT_007fea20]
        sub edx, ecx
        movzx eax, word ptr [eax]
        rep stosw
        cmp edi, dword ptr [DAT_007fe9a8]
        movzx eax, byte ptr [DAT_007fea18]
        sbb ecx, -1
        and eax, ecx
        movzx ecx, byte ptr [DAT_007feb14]
        or eax, ecx
        mov byte ptr [DAT_007feb14], al
        jmp L4647e2
L464919:
        mov ecx, 1
        cmp edi, dword ptr [DAT_007fe9a8]
        sete al
        mov byte ptr [DAT_007fea18], al
L46492c:
        cmp edx, ecx
        ja L464970
        xchg ecx, edx
        sub edx, ecx
L464934:
        and ecx, ecx
        je L46496b
        movzx eax, byte ptr [esi]
        add eax, eax
        add eax, dword ptr [DAT_007fea20]
        movzx eax, word ptr [eax]
        inc esi
        stosw
        loop L464934
        cmp edi, dword ptr [DAT_007fe9a8]
        movzx eax, byte ptr [DAT_007fea18]
        sbb ecx, -1
        and eax, ecx
        movzx ecx, byte ptr [DAT_007feb14]
        or eax, ecx
        mov byte ptr [DAT_007feb14], al
L46496b:
        lea esi, [esi + edx]
        jmp L4649b5
L464970:
        sub edx, ecx
L464972:
        movzx eax, byte ptr [esi]
        add eax, eax
        add eax, dword ptr [DAT_007fea20]
        movzx eax, word ptr [eax]
        inc esi
        stosw
        loop L464972
        cmp edi, dword ptr [DAT_007fe9a8]
        movzx eax, byte ptr [DAT_007fea18]
        sbb ecx, -1
        and eax, ecx
        movzx ecx, byte ptr [DAT_007feb14]
        or eax, ecx
        mov byte ptr [DAT_007feb14], al
        jmp L4647e2
L4649aa:
        dec edx
        add edi, 2
        je L4649b5
        jmp L4647e2
L4649b5:
        mov eax, dword ptr [DAT_00668160]
        shr ebx, 2
        dec eax
        jns L4649cb
        mov eax, 0xf
        mov ebx, dword ptr [ebp]
        add ebp, 4
L4649cb:
        mov dword ptr [DAT_00668160], eax
        test ebx, 2
        je L464a37
        test ebx, 1
        je L4649b5
        mov eax, dword ptr [DAT_00668160]
        shr ebx, 2
        sub eax, 4
        movzx ecx, bl
        jns L4649fe
        mov eax, 0xc
        mov ebx, dword ptr [ebp]
        movzx ecx, bl
        add ebp, 4
L4649fe:
        mov dword ptr [DAT_00668160], eax
        shr ebx, 6
        test ecx, ecx
        je L464a45
        mov eax, dword ptr [DAT_00668160]
        shr ebx, 2
        dec eax
        jns L464a20
        mov eax, 0xf
        mov ebx, dword ptr [ebp]
        add ebp, 4
L464a20:
        mov dword ptr [DAT_00668160], eax
        test ebx, 2
        je L464a2f
        jmp L4649b5
L464a2f:
        test ebx, 1
        je L464a3d
L464a37:
        inc esi
        jmp L4649b5
L464a3d:
        lea esi, [esi + ecx]
        jmp L4649b5
L464a45:
        mov edi, dword ptr [DAT_007feb18]
        mov edx, dword ptr [DAT_007fea10]
        add edi, dword ptr [DAT_007fe9a4]
        dec edx
        mov dword ptr [DAT_007feb18], edi
        jne L4646a1
L464a64:
        pop ebp
        pop edx
        pop ecx
        pop ebx
        pop edi
        pop esi
        mov eax, dword ptr [ebp - 0x10]
        mov ecx, dword ptr [ebp - 8]
        inc eax
        cmp eax, ecx
        mov dword ptr [ebp - 0x10], eax
        jl L4644ea
L464a7c:
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x00464a90
__declspec(naked) LEGO_EXPORT void ZBufferHelper(unsigned int *param_1, int *param_2, int *param_3, void *param_4) {
    __asm {
        push ebp
        mov ebp, esp
        push ecx
        mov ecx, dword ptr [OverrideFrame]
        mov edx, dword ptr [ebp + 8]
        push ebx
        push esi
        test ecx, ecx
        push edi
        mov dword ptr [DAT_007fe9a4], 0x200
        jge L464ab1
        movsx ecx, word ptr [edx]
L464ab1:
        movsx eax, word ptr [edx + 0x10]
        cmp ecx, eax
        jl L464abc
        lea ecx, [eax - 1]
L464abc:
        test ecx, ecx
        lea eax, [edx + 0x18]
        je L464acc
        lea edx, [ecx]
L464ac5:
        mov edi, dword ptr [eax]
        add eax, edi
        dec edx
        jne L464ac5
L464acc:
        mov edx, dword ptr [eax + 4]
        lea esi, [eax + 8]
        test ecx, ecx
        mov dword ptr [ebp - 4], esi
        jne L464ae5
        lea eax, [edx + eax + 0x208]
        mov dword ptr [ebp + 8], eax
        jmp L464aec
L464ae5:
        lea ecx, [edx + eax + 8]
        mov dword ptr [ebp + 8], ecx
L464aec:
        pushad
        mov eax, dword ptr [ebp + 0x10]
        mov edi, dword ptr [ebp + 0x14]
        mov ecx, dword ptr [eax]
        shl ecx, 2
        add edi, ecx
        mov ecx, dword ptr [eax + 4]
        imul ecx, ecx, 0x200
        add edi, ecx
        mov dword ptr [DAT_007feb18], edi
        mov eax, dword ptr [ebp + 0xc]
        mov ecx, dword ptr [eax]
        mov dword ptr [SpriteClipOrigin + 0x4], ecx
        mov ebx, dword ptr [eax + 8]
        sub ebx, ecx
        mov dword ptr [DrawClipExtent + 0x4], ebx
        mov ecx, dword ptr [eax + 4]
        mov dword ptr [SpriteClipOrigin], ecx
        mov edx, ecx
        mov ebx, dword ptr [eax + 0xc]
        sub ebx, ecx
        mov dword ptr [DrawClipExtent], ebx
        mov esi, dword ptr [ebp - 4]
        mov ebp, dword ptr [ebp + 8]
        xor eax, eax
        mov dword ptr [DAT_00668160], eax
        and edx, edx
        je L464bf9
L464b4c:
        shr ebx, 2
        and dword ptr [DAT_00668160], 0xf
        jne L464b68
        mov ebx, dword ptr [ebp]
        add ebp, 4
        mov dword ptr [DAT_00668160], 0x10
L464b68:
        dec dword ptr [DAT_00668160]
        test ebx, 2
        je L464be4
        test ebx, 1
        je L464b4c
        shr ebx, 2
        cmp dword ptr [DAT_00668160], 4
        jae L464b9a
        mov ebx, dword ptr [ebp]
        add ebp, 4
        mov dword ptr [DAT_00668160], 0x10
L464b9a:
        shrd ecx, ebx, 8
        shr ebx, 6
        sub dword ptr [DAT_00668160], 4
        shr ecx, 0x18
        je L464bf2
        shr ebx, 2
        and dword ptr [DAT_00668160], 0xf
        jne L464bc9
        mov ebx, dword ptr [ebp]
        add ebp, 4
        mov dword ptr [DAT_00668160], 0x10
L464bc9:
        dec dword ptr [DAT_00668160]
        test ebx, 2
        je L464bdc
        jmp L464b4c
L464bdc:
        test ebx, 1
        je L464bea
L464be4:
        inc esi
        jmp L464b4c
L464bea:
        lea esi, [esi + ecx]
        jmp L464b4c
L464bf2:
        dec edx
        jne L464b4c
L464bf9:
        mov edx, dword ptr [DrawClipExtent]
        and edx, edx
        je L464ed3
L464c07:
        mov dword ptr [DAT_007fea10], edx
        mov edx, dword ptr [SpriteClipOrigin + 0x4]
        and edx, edx
        je L464d2f
L464c1b:
        shr ebx, 2
        and dword ptr [DAT_00668160], 0xf
        jne L464c37
        mov ebx, dword ptr [ebp]
        add ebp, 4
        mov dword ptr [DAT_00668160], 0x10
L464c37:
        dec dword ptr [DAT_00668160]
        test ebx, 2
        je L464cfa
        test ebx, 1
        je L464cfb
        shr ebx, 2
        cmp dword ptr [DAT_00668160], 4
        jae L464c71
        mov ebx, dword ptr [ebp]
        add ebp, 4
        mov dword ptr [DAT_00668160], 0x10
L464c71:
        shrd ecx, ebx, 8
        shr ebx, 6
        sub dword ptr [DAT_00668160], 4
        shr ecx, 0x18
        je L464eb4
        shr ebx, 2
        and dword ptr [DAT_00668160], 0xf
        jne L464ca4
        mov ebx, dword ptr [ebp]
        add ebp, 4
        mov dword ptr [DAT_00668160], 0x10
L464ca4:
        dec dword ptr [DAT_00668160]
        test ebx, 2
        je L464cd1
        sub edx, ecx
        jns L464c1b
        neg edx
        lea edi, [edi + edx*2]
        mov ecx, dword ptr [DrawClipExtent + 0x4]
        sub ecx, edx
        js L464e0e
        mov edx, ecx
        jmp L464d35
L464cd1:
        test ebx, 1
        je L464d0a
        inc esi
        sub edx, ecx
        jns L464c1b
        neg edx
        dec esi
        mov ecx, edx
        mov edx, dword ptr [DrawClipExtent + 0x4]
        and edx, edx
        je L464e0e
        jmp L464db0
L464cfa:
        inc esi
L464cfb:
        dec edx
        jne L464c1b
        mov edx, dword ptr [DrawClipExtent + 0x4]
        jmp L464d35
L464d0a:
        lea esi, [esi + ecx]
        sub edx, ecx
        jns L464c1b
        lea esi, [esi + edx]
        neg edx
        mov ecx, edx
        mov edx, dword ptr [DrawClipExtent + 0x4]
        and edx, edx
        je L464e0e
        jmp L464dd7
L464d2f:
        mov edx, dword ptr [DrawClipExtent + 0x4]
L464d35:
        mov eax, dword ptr [DAT_00668160]
        shr ebx, 2
        dec eax
        jns L464d46
        mov ebx, dword ptr [ebp]
        add ebp, 4
L464d46:
        and eax, 0xf
        test ebx, 2
        mov dword ptr [DAT_00668160], eax
        je L464dd2
        test ebx, 1
        je L464e03
        shr ebx, 2
        sub eax, 4
        jns L464d75
        mov ebx, dword ptr [ebp]
        add ebp, 4
        mov eax, 0xc
L464d75:
        movzx ecx, bl
        shr ebx, 6
        and eax, 0xf
        test ecx, ecx
        mov dword ptr [DAT_00668160], eax
        je L464eb4
        shr ebx, 2
        dec eax
        jns L464d97
        mov ebx, dword ptr [ebp]
        add ebp, 4
L464d97:
        and eax, 0xf
        mov dword ptr [DAT_00668160], eax
        test ebx, 2
        je L464db0
        sub edx, ecx
        js L464e0e
        lea edi, [edi + ecx*4]
        jmp L464d35
L464db0:
        test ebx, 1
        je L464dd7
        movzx eax, byte ptr [esi]
        shl eax, 0x18
        inc esi
        cmp edx, ecx
        ja L464dc9
        mov ecx, edx
        rep stosd
        jmp L464e0e
L464dc9:
        sub edx, ecx
        rep stosd
        jmp L464d35
L464dd2:
        mov ecx, 1
L464dd7:
        cmp edx, ecx
        ja L464df2
        xchg ecx, edx
        sub edx, ecx
L464ddf:
        and ecx, ecx
        je L464ded
        movzx eax, byte ptr [esi]
        shl eax, 0x18
        inc esi
        stosd
        loop L464ddf
L464ded:
        lea esi, [esi + edx]
        jmp L464e0e
L464df2:
        sub edx, ecx
L464df4:
        movzx eax, byte ptr [esi]
        shl eax, 0x18
        inc esi
        stosd
        loop L464df4
        jmp L464d35
L464e03:
        dec edx
        je L464e0e
        add edi, 4
        jmp L464d35
L464e0e:
        shr ebx, 2
        and dword ptr [DAT_00668160], 0xf
        jne L464e2a
        mov ebx, dword ptr [ebp]
        add ebp, 4
        mov dword ptr [DAT_00668160], 0x10
L464e2a:
        dec dword ptr [DAT_00668160]
        test ebx, 2
        je L464ea6
        test ebx, 1
        je L464e0e
        shr ebx, 2
        cmp dword ptr [DAT_00668160], 4
        jae L464e5c
        mov ebx, dword ptr [ebp]
        add ebp, 4
        mov dword ptr [DAT_00668160], 0x10
L464e5c:
        shrd ecx, ebx, 8
        shr ebx, 6
        sub dword ptr [DAT_00668160], 4
        shr ecx, 0x18
        je L464eb4
        shr ebx, 2
        and dword ptr [DAT_00668160], 0xf
        jne L464e8b
        mov ebx, dword ptr [ebp]
        add ebp, 4
        mov dword ptr [DAT_00668160], 0x10
L464e8b:
        dec dword ptr [DAT_00668160]
        test ebx, 2
        je L464e9e
        jmp L464e0e
L464e9e:
        test ebx, 1
        je L464eac
L464ea6:
        inc esi
        jmp L464e0e
L464eac:
        lea esi, [esi + ecx]
        jmp L464e0e
L464eb4:
        mov edi, dword ptr [DAT_007feb18]
        mov edx, dword ptr [DAT_007fea10]
        add edi, dword ptr [DAT_007fe9a4]
        dec edx
        mov dword ptr [DAT_007feb18], edi
        jne L464c07
L464ed3:
        popad
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

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

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x00465240
__declspec(naked) void FUN_00465240(void) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x18
        mov ecx, dword ptr [OverrideFrame]
        mov eax, dword ptr [CurrentSurfaceDesc + 0x10]
        push ebx
        mov ebx, 1
        push esi
        push edi
        test ecx, ecx
        mov dword ptr [ebp - 0xc], ebx
        mov dword ptr [DAT_007fe9a4], eax
        jl L46526d
        mov edx, dword ptr [ebp + 8]
        mov dword ptr [ebp - 4], ecx
        jmp L465276
L46526d:
        mov edx, dword ptr [ebp + 8]
        movsx ecx, word ptr [edx]
        mov dword ptr [ebp - 4], ecx
L465276:
        movsx eax, word ptr [edx + 0x10]
        cmp ecx, eax
        jl L465284
        dec eax
        mov dword ptr [ebp - 4], eax
        mov ecx, eax
L465284:
        test byte ptr [edx + 0x14], bl
        je L465290
        mov dword ptr [ebp - 0xc], 2
L465290:
        mov eax, dword ptr [ebp - 0xc]
        mov dword ptr [ebp - 0x14], 0
        test eax, eax
        jle L465841
        lea eax, [edx + 0x18]
        mov dword ptr [ebp - 0x18], eax
        jmp L4652b5
L4652aa:
        mov edx, dword ptr [ebp + 8]
        mov ecx, dword ptr [ebp - 4]
        mov ebx, 1
L4652b5:
        mov esi, dword ptr [ebp - 0xc]
        mov eax, dword ptr [ebp - 0x18]
        cmp esi, ebx
        jne L4652ef
        mov esi, dword ptr [eax + 4]
        test ecx, ecx
        lea esi, [esi + eax + 8]
        mov dword ptr [DAT_007fea20], esi
        je L4652d7
L4652d0:
        mov edi, dword ptr [eax]
        add eax, edi
        dec ecx
        jne L4652d0
L4652d7:
        cmp word ptr [edx], 0
        mov ecx, dword ptr [eax + 4]
        lea esi, [eax + 8]
        mov dword ptr [ebp - 0x10], esi
        je L4652ff
        lea ecx, [ecx + eax + 8]
        mov dword ptr [ebp - 8], ecx
        jmp L465335
L4652ef:
        mov ecx, dword ptr [ebp - 0x14]
        test ecx, ecx
        jne L465312
        mov ecx, dword ptr [eax + 4]
        lea edx, [eax + 8]
        mov dword ptr [ebp - 0x10], edx
L4652ff:
        lea edx, [ecx + eax + 8]
        lea eax, [ecx + eax + 0x208]
        mov dword ptr [DAT_007fea20], edx
        jmp L465332
L465312:
        movsx ecx, word ptr [edx]
        inc ecx
        mov edx, ecx
        dec ecx
        test edx, edx
        je L465325
        inc ecx
L46531e:
        mov esi, dword ptr [eax]
        add eax, esi
        dec ecx
        jne L46531e
L465325:
        mov edx, dword ptr [eax + 4]
        lea ecx, [eax + 8]
        mov dword ptr [ebp - 0x10], ecx
        lea eax, [edx + eax + 8]
L465332:
        mov dword ptr [ebp - 8], eax
L465335:
        pushad
        mov eax, dword ptr [ebp + 0x10]
        mov edi, dword ptr [CurrentSurfaceDesc + 0x24]
        add edi, dword ptr [eax]
        add edi, dword ptr [eax]
        mov ecx, dword ptr [eax + 4]
        imul ecx, dword ptr [CurrentSurfaceDesc + 0x10]
        add edi, ecx
        mov dword ptr [DAT_007feb18], edi
        mov eax, dword ptr [ebp + 0xc]
        mov ecx, dword ptr [eax]
        mov dword ptr [SpriteClipOrigin + 0x4], ecx
        mov ebx, dword ptr [eax + 8]
        sub ebx, ecx
        mov dword ptr [DrawClipExtent + 0x4], ebx
        mov ecx, dword ptr [eax + 4]
        mov dword ptr [SpriteClipOrigin], ecx
        mov edx, ecx
        mov ebx, dword ptr [eax + 0xc]
        sub ebx, ecx
        mov dword ptr [DrawClipExtent], ebx
        mov esi, dword ptr [ebp - 0x10]
        mov ebp, dword ptr [ebp - 8]
        xor eax, eax
        mov dword ptr [DAT_00668160], eax
        and edx, edx
        je L465443
L465396:
        shr ebx, 2
        and dword ptr [DAT_00668160], 0xf
        jne L4653b2
        mov ebx, dword ptr [ebp]
        add ebp, 4
        mov dword ptr [DAT_00668160], 0x10
L4653b2:
        dec dword ptr [DAT_00668160]
        test ebx, 2
        je L46542e
        test ebx, 1
        je L465396
        shr ebx, 2
        cmp dword ptr [DAT_00668160], 4
        jae L4653e4
        mov ebx, dword ptr [ebp]
        add ebp, 4
        mov dword ptr [DAT_00668160], 0x10
L4653e4:
        shrd ecx, ebx, 8
        shr ebx, 6
        sub dword ptr [DAT_00668160], 4
        shr ecx, 0x18
        je L46543c
        shr ebx, 2
        and dword ptr [DAT_00668160], 0xf
        jne L465413
        mov ebx, dword ptr [ebp]
        add ebp, 4
        mov dword ptr [DAT_00668160], 0x10
L465413:
        dec dword ptr [DAT_00668160]
        test ebx, 2
        je L465426
        jmp L465396
L465426:
        test ebx, 1
        je L465434
L46542e:
        inc esi
        jmp L465396
L465434:
        lea esi, [esi + ecx]
        jmp L465396
L46543c:
        dec edx
        jne L465396
L465443:
        mov edx, dword ptr [DrawClipExtent]
        and edx, edx
        je L46582e
L465451:
        mov dword ptr [DAT_007fea10], edx
        mov edx, dword ptr [SpriteClipOrigin + 0x4]
        and edx, edx
        je L4655a1
L465465:
        shr ebx, 2
        and dword ptr [DAT_00668160], 0xf
        jne L465481
        mov ebx, dword ptr [ebp]
        add ebp, 4
        mov dword ptr [DAT_00668160], 0x10
L465481:
        dec dword ptr [DAT_00668160]
        test ebx, 2
        je L465559
        test ebx, 1
        je L46555a
        shr ebx, 2
        cmp dword ptr [DAT_00668160], 4
        jae L4654bb
        mov ebx, dword ptr [ebp]
        add ebp, 4
        mov dword ptr [DAT_00668160], 0x10
L4654bb:
        shrd ecx, ebx, 8
        shr ebx, 6
        sub dword ptr [DAT_00668160], 4
        shr ecx, 0x18
        je L46580f
        shr ebx, 2
        and dword ptr [DAT_00668160], 0xf
        jne L4654ee
        mov ebx, dword ptr [ebp]
        add ebp, 4
        mov dword ptr [DAT_00668160], 0x10
L4654ee:
        dec dword ptr [DAT_00668160]
        test ebx, 2
        je L46551e
        sub edx, ecx
        ja L465465
        neg edx
        lea edi, [edi + edx*2]
        mov ecx, dword ptr [DrawClipExtent + 0x4]
        sub ecx, edx
        jbe L465769
        mov edx, ecx
        jmp L4655a7
L46551e:
        test ebx, 1
        je L465569
        inc esi
        sub edx, ecx
        ja L465465
        neg edx
        mov ecx, edx
        mov edx, dword ptr [DrawClipExtent + 0x4]
        and edx, edx
        je L465769
        cmp edi, dword ptr [DAT_007fe9a8]
        setbe al
        mov byte ptr [DAT_007fea18], al
        and ecx, ecx
        jne L46563b
        jmp L4655a7
L465559:
        inc esi
L46555a:
        dec edx
        jne L465465
        mov edx, dword ptr [DrawClipExtent + 0x4]
        jmp L4655a7
L465569:
        lea esi, [esi + ecx]
        sub edx, ecx
        ja L465465
        lea esi, [esi + edx]
        neg edx
        mov ecx, edx
        mov edx, dword ptr [DrawClipExtent + 0x4]
        and edx, edx
        je L465769
        cmp edi, dword ptr [DAT_007fe9a8]
        setbe al
        mov byte ptr [DAT_007fea18], al
        and ecx, ecx
        jne L4656da
        jmp L4655a7
L4655a1:
        mov edx, dword ptr [DrawClipExtent + 0x4]
L4655a7:
        mov eax, dword ptr [DAT_00668160]
        shr ebx, 2
        dec eax
        jns L4655b8
        mov ebx, dword ptr [ebp]
        add ebp, 4
L4655b8:
        and eax, 0xf
        test ebx, 2
        mov dword ptr [DAT_00668160], eax
        je L4656c7
        test ebx, 1
        je L46575e
        shr ebx, 2
        sub eax, 4
        jns L4655eb
        mov ebx, dword ptr [ebp]
        add ebp, 4
        mov eax, 0xc
L4655eb:
        movzx ecx, bl
        shr ebx, 6
        and eax, 0xf
        test ecx, ecx
        mov dword ptr [DAT_00668160], eax
        je L46580f
        shr ebx, 2
        dec eax
        jns L46560d
        mov ebx, dword ptr [ebp]
        add ebp, 4
L46560d:
        and eax, 0xf
        mov dword ptr [DAT_00668160], eax
        cmp edi, dword ptr [DAT_007fe9a8]
        setbe al
        mov byte ptr [DAT_007fea18], al
        test ebx, 2
        je L46563c
        sub edx, ecx
        js L465769
        lea edi, [edi + ecx*2]
        jmp L4655a7
L46563b:
        dec esi
L46563c:
        test ebx, 1
        je L4656da
        movzx eax, byte ptr [esi]
        add eax, eax
        add eax, dword ptr [DAT_007fea20]
        movzx eax, word ptr [eax]
        inc esi
        cmp edx, ecx
        ja L465691
        mov ecx, edx
        or ecx, ecx
        je L465769
        and ax, word ptr [DAT_007fe998]
L46566c:
        mov word ptr [edi], ax
        add edi, 2
        loop L46566c
        cmp edi, dword ptr [DAT_007fe9a8]
        movzx ecx, byte ptr [DAT_007fea18]
        seta al
        and eax, ecx
        or dword ptr [DAT_007feb14], eax
        jmp L465769
L465691:
        sub edx, ecx
        or ecx, ecx
        je L4655a7
        and ax, word ptr [DAT_007fe998]
L4656a2:
        mov word ptr [edi], ax
        add edi, 2
        loop L4656a2
        cmp edi, dword ptr [DAT_007fe9a8]
        movzx ecx, byte ptr [DAT_007fea18]
        seta al
        and eax, ecx
        or dword ptr [DAT_007feb14], eax
        jmp L4655a7
L4656c7:
        mov ecx, 1
        cmp edi, dword ptr [DAT_007fe9a8]
        sete al
        mov byte ptr [DAT_007fea18], al
L4656da:
        cmp edx, ecx
        ja L465721
        xchg ecx, edx
        sub edx, ecx
L4656e2:
        and ecx, ecx
        je L46571c
        movzx eax, byte ptr [esi]
        add eax, eax
        add eax, dword ptr [DAT_007fea20]
        movzx eax, word ptr [eax]
        inc esi
        and ax, word ptr [DAT_007fe998]
        mov word ptr [edi], ax
        add edi, 2
        loop L4656e2
        cmp edi, dword ptr [DAT_007fe9a8]
        movzx ecx, byte ptr [DAT_007fea18]
        seta al
        and eax, ecx
        or dword ptr [DAT_007feb14], eax
L46571c:
        lea esi, [esi + edx]
        jmp L465769
L465721:
        sub edx, ecx
L465723:
        movzx eax, byte ptr [esi]
        add eax, eax
        add eax, dword ptr [DAT_007fea20]
        movzx eax, word ptr [eax]
        inc esi
        and ax, word ptr [DAT_007fe998]
        mov word ptr [edi], ax
        add edi, 2
        loop L465723
        cmp edi, dword ptr [DAT_007fe9a8]
        movzx ecx, byte ptr [DAT_007fea18]
        seta al
        and eax, ecx
        or dword ptr [DAT_007feb14], eax
        jmp L4655a7
L46575e:
        dec edx
        je L465769
        add edi, 2
        jmp L4655a7
L465769:
        shr ebx, 2
        and dword ptr [DAT_00668160], 0xf
        jne L465785
        mov ebx, dword ptr [ebp]
        add ebp, 4
        mov dword ptr [DAT_00668160], 0x10
L465785:
        dec dword ptr [DAT_00668160]
        test ebx, 2
        je L465801
        test ebx, 1
        je L465769
        shr ebx, 2
        cmp dword ptr [DAT_00668160], 4
        jae L4657b7
        mov ebx, dword ptr [ebp]
        add ebp, 4
        mov dword ptr [DAT_00668160], 0x10
L4657b7:
        shrd ecx, ebx, 8
        shr ebx, 6
        sub dword ptr [DAT_00668160], 4
        shr ecx, 0x18
        je L46580f
        shr ebx, 2
        and dword ptr [DAT_00668160], 0xf
        jne L4657e6
        mov ebx, dword ptr [ebp]
        add ebp, 4
        mov dword ptr [DAT_00668160], 0x10
L4657e6:
        dec dword ptr [DAT_00668160]
        test ebx, 2
        je L4657f9
        jmp L465769
L4657f9:
        test ebx, 1
        je L465807
L465801:
        inc esi
        jmp L465769
L465807:
        lea esi, [esi + ecx]
        jmp L465769
L46580f:
        mov edi, dword ptr [DAT_007feb18]
        mov edx, dword ptr [DAT_007fea10]
        add edi, dword ptr [DAT_007fe9a4]
        dec edx
        mov dword ptr [DAT_007feb18], edi
        jne L465451
L46582e:
        popad
        mov eax, dword ptr [ebp - 0x14]
        mov ecx, dword ptr [ebp - 0xc]
        inc eax
        cmp eax, ecx
        mov dword ptr [ebp - 0x14], eax
        jl L4652aa
L465841:
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

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

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x00466d80
__declspec(naked) void FUN_00466d80(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) {
    __asm {
        push edi
        mov edx, dword ptr [esp + 0x14]
        mov edi, dword ptr [esp + 0x20]
        push esi
        mov esi, dword ptr [esp + 0x10]
        push ebx
        mov ebx, 3
        push ebp
        test edi, edi
        je L466dfe
L466d99:
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        jne L466db7
        add esi, 2
        jmp L466d99
L466db7:
        test ebp, 0x55555555
        je L466d99
        xor ecx, ecx
        mov eax, dword ptr [esp + 0x1c]
        mov cl, byte ptr [eax]
        inc eax
        test ecx, ecx
        mov dword ptr [esp + 0x1c], eax
        je L466dfb
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, eax
        jne L466d99
        test ebp, 0x55555555
        je L466df6
        add esi, 2
        jmp L466d99
L466df6:
        lea esi, [esi + ecx*2]
        jmp L466d99
L466dfb:
        dec edi
        jg L466d99
L466dfe:
        mov eax, dword ptr [esp + 0x30]
        mov edi, dword ptr [esp + 0x14]
        mov dword ptr [esp + 0x2c], eax
        mov eax, dword ptr [esp + 0x34]
        mov dword ptr [esp + 0x38], eax
L466e12:
        cmp dword ptr [esp + 0x30], 0
        jle L466fa7
        dec dword ptr [esp + 0x30]
        add edi, 2
        add esi, 2
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        je L466f91
        test ebp, 0x55555555
        lea esi, [esi - 2]
        je L466f91
        inc dword ptr [esp + 0x30]
        xor ecx, ecx
        sub edi, 2
        mov eax, dword ptr [esp + 0x1c]
        mov cl, byte ptr [eax]
        inc eax
        test ecx, ecx
        mov dword ptr [esp + 0x1c], eax
        je L46713f
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, eax
        je L466ea2
        lea edi, [edi + ecx*2]
        sub dword ptr [esp + 0x30], ecx
        jge L466f91
        mov eax, dword ptr [esp + 0x30]
        add dword ptr [esp + 0x34], eax
        jmp L466f91
L466ea2:
        test ebp, 0x55555555
        jne L466f22
        sub dword ptr [esp + 0x30], ecx
        jl L466ebb
        lea esi, [esi + ecx*2]
        lea edi, [edi + ecx*2]
        jmp L466f91
L466ebb:
        mov eax, dword ptr [esp + 0x30]
        add dword ptr [esp + 0x34], eax
        jle L466ef0
        add eax, ecx
        lea esi, [esi + eax*2]
        lea edi, [edi + eax*2]
        mov eax, dword ptr [esp + 0x30]
        neg eax
        mov ecx, eax
        mov eax, dword ptr [esp + 0x3c]
        sub eax, edi
        sar eax, 1
        cmp eax, ecx
        jae L466ee8
        or dword ptr [DAT_007feb14], 1
L466ee8:
        rep movsw
        jmp L466f91
L466ef0:
        add eax, ecx
        lea esi, [esi + eax*2]
        lea edi, [edi + eax*2]
        sub ecx, eax
        add ecx, dword ptr [esp + 0x34]
        mov eax, dword ptr [esp + 0x3c]
        sub eax, edi
        sar eax, 1
        cmp eax, ecx
        jae L466f11
        or dword ptr [DAT_007feb14], 1
L466f11:
        rep movsw
        mov eax, dword ptr [esp + 0x34]
        neg eax
        lea esi, [esi + eax*2]
        jmp L4670dd
L466f22:
        sub dword ptr [esp + 0x30], ecx
        jl L466f30
        add esi, 2
        lea edi, [edi + ecx*2]
        jmp L466f91
L466f30:
        mov eax, dword ptr [esp + 0x30]
        add dword ptr [esp + 0x34], eax
        jle L466f65
        add eax, ecx
        lea edi, [edi + eax*2]
        mov eax, dword ptr [esp + 0x30]
        neg eax
        mov ecx, eax
        mov eax, dword ptr [esp + 0x3c]
        sub eax, edi
        sar eax, 1
        cmp eax, ecx
        jae L466f5a
        or dword ptr [DAT_007feb14], 1
L466f5a:
        mov ax, word ptr [esi]
        add esi, 2
        rep stosw
        jmp L466f91
L466f65:
        add eax, ecx
        lea edi, [edi + eax*2]
        sub ecx, eax
        add ecx, dword ptr [esp + 0x34]
        mov eax, dword ptr [esp + 0x3c]
        sub eax, edi
        sar eax, 1
        cmp eax, ecx
        jae L466f83
        or dword ptr [DAT_007feb14], 1
L466f83:
        mov ax, word ptr [esi]
        add esi, 2
        rep stosw
        jmp L4670dd
L466f91:
        cmp dword ptr [esp + 0x30], 0
        jg L466e12
        cmp dword ptr [esp + 0x34], 0
        jle L4670dd
L466fa7:
        dec dword ptr [esp + 0x34]
        add edi, 2
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        jne L466fea
        mov eax, dword ptr [esp + 0x3c]
        sub edi, 2
        cmp eax, edi
        jne L466fd9
        or dword ptr [DAT_007feb14], 1
L466fd9:
        mov ax, word ptr [esi]
        add esi, 2
        mov word ptr [edi], ax
        add edi, 2
        jmp L4670d2
L466fea:
        test ebp, 0x55555555
        je L4670d2
        inc dword ptr [esp + 0x34]
        xor ecx, ecx
        sub edi, 2
        mov eax, dword ptr [esp + 0x1c]
        mov cl, byte ptr [eax]
        inc eax
        test ecx, ecx
        mov dword ptr [esp + 0x1c], eax
        je L46713f
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, eax
        je L467037
        lea edi, [edi + ecx*2]
        sub dword ptr [esp + 0x34], ecx
        jmp L4670d2
L467037:
        test ebp, 0x55555555
        jne L467086
        mov eax, dword ptr [esp + 0x34]
        sub eax, ecx
        jl L467063
        mov dword ptr [esp + 0x34], eax
        mov eax, dword ptr [esp + 0x3c]
        sub eax, edi
        sar eax, 1
        cmp eax, ecx
        jae L46705e
        or dword ptr [DAT_007feb14], 1
L46705e:
        rep movsw
        jmp L4670d2
L467063:
        neg eax
        mov ecx, dword ptr [esp + 0x34]
        push eax
        mov eax, dword ptr [esp + 0x40]
        sub eax, edi
        sar eax, 1
        cmp eax, ecx
        jae L46707d
        or dword ptr [DAT_007feb14], 1
L46707d:
        rep movsw
        pop eax
        lea esi, [esi + eax*2]
        jmp L4670dd
L467086:
        mov eax, dword ptr [esp + 0x34]
        sub eax, ecx
        jl L4670b0
        mov dword ptr [esp + 0x34], eax
        mov eax, dword ptr [esp + 0x3c]
        sub eax, edi
        sar eax, 1
        cmp eax, ecx
        jae L4670a5
        or dword ptr [DAT_007feb14], 1
L4670a5:
        mov ax, word ptr [esi]
        add esi, 2
        rep stosw
        jmp L4670d2
L4670b0:
        mov ecx, dword ptr [esp + 0x34]
        mov eax, dword ptr [esp + 0x3c]
        sub eax, edi
        sar eax, 1
        cmp eax, ecx
        jae L4670c7
        or dword ptr [DAT_007feb14], 1
L4670c7:
        mov ax, word ptr [esi]
        add esi, 2
        rep stosw
        jmp L4670dd
L4670d2:
        cmp dword ptr [esp + 0x34], 0
        jg L466fa7
L4670dd:
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        jne L4670fb
        add esi, 2
        jmp L4670dd
L4670fb:
        test ebp, 0x55555555
        je L4670dd
        xor ecx, ecx
        mov eax, dword ptr [esp + 0x1c]
        mov cl, byte ptr [eax]
        inc eax
        test ecx, ecx
        mov dword ptr [esp + 0x1c], eax
        je L46713f
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, eax
        jne L4670dd
        test ebp, 0x55555555
        je L46713a
        add esi, 2
        jmp L4670dd
L46713a:
        lea esi, [esi + ecx*2]
        jmp L4670dd
L46713f:
        mov eax, dword ptr [esp + 0x14]
        mov ecx, dword ptr [esp + 0x24]
        add eax, dword ptr [esp + 0x28]
        dec ecx
        mov dword ptr [esp + 0x14], eax
        mov edi, eax
        mov dword ptr [esp + 0x24], ecx
        mov eax, dword ptr [esp + 0x2c]
        mov dword ptr [esp + 0x30], eax
        mov eax, dword ptr [esp + 0x38]
        mov dword ptr [esp + 0x34], eax
        test ecx, ecx
        jne L466e12
        pop ebp
        pop ebx
        pop esi
        pop edi
        ret
    }
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x00467180
__declspec(naked) void FUN_00467180(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) {
    __asm {
        push edi
        mov edx, dword ptr [esp + 0x14]
        mov edi, dword ptr [esp + 0x20]
        push esi
        mov esi, dword ptr [esp + 0x10]
        nop
        push ebx
        mov ebx, 3
        nop
        push ebp
        test edi, edi
        je L4671fd
L46719b:
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        test ebp, 0xaaaaaaaa
        jne L4671b7
        add esi, 2
L4671b7:
        test ebp, 0x55555555
        je L46719b
        mov eax, dword ptr [esp + 0x1c]
        movzx ecx, byte ptr [eax]
        inc eax
        mov dword ptr [esp + 0x1c], eax
        test ecx, ecx
        je L4671fa
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, eax
        test ebp, 0xaaaaaaaa
        jne L46719b
        test ebp, 0x55555555
        je L4671f5
        add esi, 2
        jmp L46719b
L4671f5:
        lea esi, [esi + ecx*2]
        jmp L46719b
L4671fa:
        dec edi
        jg L46719b
L4671fd:
        mov edi, dword ptr [esp + 0x14]
        mov eax, dword ptr [esp + 0x30]
        mov dword ptr [esp + 0x2c], eax
L467209:
        dec dword ptr [esp + 0x30]
        add edi, 2
        add esi, 2
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        test ebp, 0xaaaaaaaa
        je L467305
        sub esi, 2
        test ebp, 0x55555555
        je L467305
        inc dword ptr [esp + 0x30]
        sub edi, 2
        mov eax, dword ptr [esp + 0x1c]
        movzx ecx, byte ptr [eax]
        inc eax
        mov dword ptr [esp + 0x1c], eax
        test ecx, ecx
        je L4673b7
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, eax
        test ebp, 0xaaaaaaaa
        je L467286
        lea edi, [edi + ecx*2]
        sub dword ptr [esp + 0x30], ecx
        jge L467305
        mov eax, dword ptr [esp + 0x30]
        jmp L467305
L467286:
        test ebp, 0x55555555
        jne L4672c8
        sub dword ptr [esp + 0x30], ecx
        jl L46729c
        lea esi, [esi + ecx*2]
        lea edi, [edi + ecx*2]
        jmp L467305
L46729c:
        mov eax, dword ptr [esp + 0x30]
        add eax, ecx
        lea esi, [esi + eax*2]
        lea edi, [edi + eax*2]
        mov eax, dword ptr [esp + 0x30]
        neg eax
        mov ecx, eax
        mov eax, dword ptr [esp + 0x3c]
        sub eax, edi
        sar eax, 1
        cmp eax, ecx
        jae L4672c3
        or dword ptr [DAT_007feb14], 1
L4672c3:
        rep movsw
        jmp L467305
L4672c8:
        sub dword ptr [esp + 0x30], ecx
        jl L4672d6
        add esi, 2
        lea edi, [edi + ecx*2]
        jmp L467305
L4672d6:
        mov eax, dword ptr [esp + 0x30]
        add eax, ecx
        lea edi, [edi + eax*2]
        mov eax, dword ptr [esp + 0x30]
        neg eax
        mov ecx, eax
        mov eax, dword ptr [esp + 0x3c]
        sub eax, edi
        sar eax, 1
        cmp eax, ecx
        jae L4672fa
        or dword ptr [DAT_007feb14], 1
L4672fa:
        mov ax, word ptr [esi]
        add esi, 2
        rep stosw
        jmp L467305
L467305:
        cmp dword ptr [esp + 0x30], 0
        jg L467209
L467310:
        add edi, 2
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        test ebp, 0xaaaaaaaa
        jne L46734d
        mov eax, dword ptr [esp + 0x3c]
        sub edi, 2
        cmp eax, edi
        jne L46733e
        or dword ptr [DAT_007feb14], 1
L46733e:
        mov ax, word ptr [esi]
        add esi, 2
        add edi, 2
        mov word ptr [edi - 2], ax
        jmp L467310
L46734d:
        test ebp, 0x55555555
        je L467310
        sub edi, 2
        mov eax, dword ptr [esp + 0x1c]
        movzx ecx, byte ptr [eax]
        inc eax
        mov dword ptr [esp + 0x1c], eax
        test ecx, ecx
        je L4673b7
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, eax
        test ebp, 0xaaaaaaaa
        je L467386
        lea edi, [edi + ecx*2]
        jmp L467310
L467386:
        mov eax, dword ptr [esp + 0x3c]
        sub eax, edi
        sar eax, 1
        cmp eax, ecx
        jae L467399
        or dword ptr [DAT_007feb14], 1
L467399:
        test ebp, 0x55555555
        jne L4673a9
        rep movsw
        jmp L467310
L4673a9:
        mov ax, word ptr [esi]
        add esi, 2
        rep stosw
        jmp L467310
L4673b7:
        mov eax, dword ptr [esp + 0x14]
        add eax, dword ptr [esp + 0x28]
        mov dword ptr [esp + 0x14], eax
        mov edi, eax
        mov ecx, dword ptr [esp + 0x24]
        dec ecx
        mov dword ptr [esp + 0x24], ecx
        mov eax, dword ptr [esp + 0x2c]
        mov dword ptr [esp + 0x30], eax
        test ecx, ecx
        jne L467209
        pop ebp
        pop ebx
        pop esi
        pop edi
        ret
    }
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x004673f0
__declspec(naked) void FUN_004673f0(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) {
    __asm {
        push edi
        mov edx, dword ptr [esp + 0x14]
        mov edi, dword ptr [esp + 0x20]
        push esi
        mov esi, dword ptr [esp + 0x10]
        nop
        push ebx
        mov ebx, 3
        nop
        push ebp
        test edi, edi
        je L46746d
L46740b:
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        test ebp, 0xaaaaaaaa
        jne L467427
        add esi, 2
L467427:
        test ebp, 0x55555555
        je L46740b
        mov eax, dword ptr [esp + 0x1c]
        movzx ecx, byte ptr [eax]
        inc eax
        mov dword ptr [esp + 0x1c], eax
        test ecx, ecx
        je L46746a
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, eax
        test ebp, 0xaaaaaaaa
        jne L46740b
        test ebp, 0x55555555
        je L467465
        add esi, 2
        jmp L46740b
L467465:
        lea esi, [esi + ecx*2]
        jmp L46740b
L46746a:
        dec edi
        jg L46740b
L46746d:
        mov edi, dword ptr [esp + 0x14]
        mov eax, dword ptr [esp + 0x34]
        mov dword ptr [esp + 0x38], eax
L467479:
        dec dword ptr [esp + 0x34]
        add edi, 2
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        test ebp, 0xaaaaaaaa
        jne L4674bd
        mov eax, dword ptr [esp + 0x3c]
        sub edi, 2
        cmp eax, edi
        jne L4674ab
        or dword ptr [DAT_007feb14], 1
L4674ab:
        mov ax, word ptr [esi]
        add esi, 2
        add edi, 2
        mov word ptr [edi - 2], ax
        jmp L4675a4
L4674bd:
        test ebp, 0x55555555
        je L4675a4
        inc dword ptr [esp + 0x34]
        sub edi, 2
        mov eax, dword ptr [esp + 0x1c]
        movzx ecx, byte ptr [eax]
        inc eax
        mov dword ptr [esp + 0x1c], eax
        test ecx, ecx
        je L467610
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, eax
        test ebp, 0xaaaaaaaa
        je L467509
        lea edi, [edi + ecx*2]
        sub dword ptr [esp + 0x34], ecx
        jmp L4675a4
L467509:
        test ebp, 0x55555555
        jne L467558
        mov eax, dword ptr [esp + 0x34]
        sub eax, ecx
        jl L467535
        mov dword ptr [esp + 0x34], eax
        mov eax, dword ptr [esp + 0x3c]
        sub eax, edi
        sar eax, 1
        cmp eax, ecx
        jae L467530
        or dword ptr [DAT_007feb14], 1
L467530:
        rep movsw
        jmp L4675a4
L467535:
        neg eax
        mov ecx, dword ptr [esp + 0x34]
        push eax
        mov eax, dword ptr [esp + 0x40]
        sub eax, edi
        sar eax, 1
        cmp eax, ecx
        jae L46754f
        or dword ptr [DAT_007feb14], 1
L46754f:
        rep movsw
        pop eax
        lea esi, [esi + eax*2]
        jmp L4675af
L467558:
        mov eax, dword ptr [esp + 0x34]
        sub eax, ecx
        jl L467582
        mov dword ptr [esp + 0x34], eax
        mov eax, dword ptr [esp + 0x3c]
        sub eax, edi
        sar eax, 1
        cmp eax, ecx
        jae L467577
        or dword ptr [DAT_007feb14], 1
L467577:
        mov ax, word ptr [esi]
        add esi, 2
        rep stosw
        jmp L4675a4
L467582:
        mov ecx, dword ptr [esp + 0x34]
        mov eax, dword ptr [esp + 0x3c]
        sub eax, edi
        sar eax, 1
        cmp eax, ecx
        jae L467599
        or dword ptr [DAT_007feb14], 1
L467599:
        mov ax, word ptr [esi]
        add esi, 2
        rep stosw
        jmp L4675af
L4675a4:
        cmp dword ptr [esp + 0x34], 0
        jg L467479
L4675af:
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        test ebp, 0xaaaaaaaa
        jne L4675cd
        add esi, 2
        jmp L4675af
L4675cd:
        test ebp, 0x55555555
        je L4675af
        mov eax, dword ptr [esp + 0x1c]
        movzx ecx, byte ptr [eax]
        inc eax
        mov dword ptr [esp + 0x1c], eax
        test ecx, ecx
        je L467610
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, eax
        test ebp, 0xaaaaaaaa
        jne L4675af
        test ebp, 0x55555555
        je L46760b
        add esi, 2
        jmp L4675af
L46760b:
        lea esi, [esi + ecx*2]
        jmp L4675af
L467610:
        mov eax, dword ptr [esp + 0x14]
        add eax, dword ptr [esp + 0x28]
        mov dword ptr [esp + 0x14], eax
        mov edi, eax
        mov ecx, dword ptr [esp + 0x24]
        dec ecx
        mov dword ptr [esp + 0x24], ecx
        mov eax, dword ptr [esp + 0x38]
        mov dword ptr [esp + 0x34], eax
        test ecx, ecx
        jne L467479
        pop ebp
        pop ebx
        pop esi
        pop edi
        ret
    }
}

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

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x004677b0
__declspec(naked) void FUN_004677b0(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) {
    __asm {
        push edi
        mov edx, dword ptr [esp + 0x14]
        mov edi, dword ptr [esp + 0x20]
        push esi
        mov esi, dword ptr [esp + 0x10]
        push ebx
        mov ebx, 3
        push ebp
        test edi, edi
        je L46782e
L4677c9:
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        jne L4677e7
        add esi, 2
        jmp L4677c9
L4677e7:
        test ebp, 0x55555555
        je L4677c9
        xor ecx, ecx
        mov eax, dword ptr [esp + 0x1c]
        mov cl, byte ptr [eax]
        inc eax
        test ecx, ecx
        mov dword ptr [esp + 0x1c], eax
        je L46782b
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, eax
        jne L4677c9
        test ebp, 0x55555555
        je L467826
        add esi, 2
        jmp L4677c9
L467826:
        lea esi, [esi + ecx*2]
        jmp L4677c9
L46782b:
        dec edi
        jg L4677c9
L46782e:
        mov eax, dword ptr [esp + 0x30]
        mov edi, dword ptr [esp + 0x14]
        mov dword ptr [esp + 0x2c], eax
        mov eax, dword ptr [esp + 0x34]
        mov dword ptr [esp + 0x38], eax
L467842:
        cmp dword ptr [esp + 0x30], 0
        jle L467988
        mov ebp, dword ptr [edx]
        dec dword ptr [esp + 0x30]
        add esi, 2
        and ebp, ebx
        rol ebx, 2
        add edi, 2
        mov ecx, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        je L467972
        test ebp, 0x55555555
        lea esi, [esi - 2]
        je L467972
        xor ecx, ecx
        inc dword ptr [esp + 0x30]
        sub edi, 2
        mov eax, dword ptr [esp + 0x1c]
        mov cl, byte ptr [eax]
        inc eax
        test ecx, ecx
        mov dword ptr [esp + 0x1c], eax
        je L467abd
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, eax
        je L4678d2
        lea edi, [edi + ecx*2]
        sub dword ptr [esp + 0x30], ecx
        jge L467972
        mov eax, dword ptr [esp + 0x30]
        add dword ptr [esp + 0x34], eax
        jmp L467972
L4678d2:
        test ebp, 0x55555555
        jne L467929
        sub dword ptr [esp + 0x30], ecx
        jl L4678eb
        lea esi, [esi + ecx*2]
        lea edi, [edi + ecx*2]
        jmp L467972
L4678eb:
        mov eax, dword ptr [esp + 0x30]
        add dword ptr [esp + 0x34], eax
        jle L46790a
        add eax, ecx
        lea esi, [esi + eax*2]
        lea edi, [edi + eax*2]
        mov eax, dword ptr [esp + 0x30]
        neg eax
        mov ecx, eax
        rep movsw
        jmp L467972
L46790a:
        add eax, ecx
        lea esi, [esi + eax*2]
        lea edi, [edi + eax*2]
        sub ecx, eax
        add ecx, dword ptr [esp + 0x34]
        rep movsw
        mov eax, dword ptr [esp + 0x34]
        neg eax
        lea esi, [esi + eax*2]
        jmp L467a5b
L467929:
        sub dword ptr [esp + 0x30], ecx
        jl L467937
        add esi, 2
        lea edi, [edi + ecx*2]
        jmp L467972
L467937:
        mov eax, dword ptr [esp + 0x30]
        add dword ptr [esp + 0x34], eax
        jle L467959
        add eax, ecx
        lea edi, [edi + eax*2]
        mov eax, dword ptr [esp + 0x30]
        neg eax
        mov ecx, eax
        mov ax, word ptr [esi]
        add esi, 2
        rep stosw
        jmp L467972
L467959:
        add eax, ecx
        lea edi, [edi + eax*2]
        sub ecx, eax
        add ecx, dword ptr [esp + 0x34]
        mov ax, word ptr [esi]
        add esi, 2
        rep stosw
        jmp L467a5b
L467972:
        cmp dword ptr [esp + 0x30], 0
        jg L467842
        cmp dword ptr [esp + 0x34], 0
        jle L467a5b
L467988:
        mov ebp, dword ptr [edx]
        dec dword ptr [esp + 0x34]
        add edi, 2
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        jne L4679b7
        mov ax, word ptr [esi]
        add esi, 2
        mov word ptr [edi - 2], ax
        jmp L467a50
L4679b7:
        test ebp, 0x55555555
        je L467a50
        xor ecx, ecx
        inc dword ptr [esp + 0x34]
        sub edi, 2
        mov eax, dword ptr [esp + 0x1c]
        mov cl, byte ptr [eax]
        inc eax
        test ecx, ecx
        mov dword ptr [esp + 0x1c], eax
        je L467abd
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, eax
        je L467a01
        lea edi, [edi + ecx*2]
        sub dword ptr [esp + 0x34], ecx
        jmp L467a50
L467a01:
        test ebp, 0x55555555
        jne L467a2a
        mov eax, dword ptr [esp + 0x34]
        sub eax, ecx
        jl L467a1a
        mov dword ptr [esp + 0x34], eax
        rep movsw
        jmp L467a50
L467a1a:
        neg eax
        mov ecx, dword ptr [esp + 0x34]
        push eax
        rep movsw
        pop eax
        lea esi, [esi + eax*2]
        jmp L467a5b
L467a2a:
        mov eax, dword ptr [esp + 0x34]
        sub eax, ecx
        jl L467a41
        mov dword ptr [esp + 0x34], eax
        mov ax, word ptr [esi]
        add esi, 2
        rep stosw
        jmp L467a50
L467a41:
        mov ecx, dword ptr [esp + 0x34]
        mov ax, word ptr [esi]
        add esi, 2
        rep stosw
        jmp L467a5b
L467a50:
        cmp dword ptr [esp + 0x34], 0
        jg L467988
L467a5b:
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        jne L467a79
        add esi, 2
        jmp L467a5b
L467a79:
        test ebp, 0x55555555
        je L467a5b
        xor ecx, ecx
        mov eax, dword ptr [esp + 0x1c]
        mov cl, byte ptr [eax]
        inc eax
        test ecx, ecx
        mov dword ptr [esp + 0x1c], eax
        je L467abd
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, eax
        jne L467a5b
        test ebp, 0x55555555
        je L467ab8
        add esi, 2
        jmp L467a5b
L467ab8:
        lea esi, [esi + ecx*2]
        jmp L467a5b
L467abd:
        mov eax, dword ptr [esp + 0x14]
        mov ecx, dword ptr [esp + 0x24]
        add eax, dword ptr [esp + 0x28]
        dec ecx
        mov dword ptr [esp + 0x14], eax
        mov dword ptr [esp + 0x24], ecx
        mov edi, eax
        mov eax, dword ptr [esp + 0x2c]
        test ecx, ecx
        mov dword ptr [esp + 0x30], eax
        mov eax, dword ptr [esp + 0x38]
        mov dword ptr [esp + 0x34], eax
        jne L467842
        pop ebp
        pop ebx
        pop esi
        pop edi
        ret
    }
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x00467b00
__declspec(naked) void FUN_00467b00(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) {
    __asm {
        push edi
        mov edx, dword ptr [esp + 0x14]
        mov edi, dword ptr [esp + 0x20]
        push esi
        mov esi, dword ptr [esp + 0x10]
        push ebx
        mov ebx, 3
        push ebp
        test edi, edi
        je L467b7e
L467b19:
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        jne L467b37
        add esi, 2
        jmp L467b19
L467b37:
        test ebp, 0x55555555
        je L467b19
        xor ecx, ecx
        mov eax, dword ptr [esp + 0x1c]
        mov cl, byte ptr [eax]
        inc eax
        test ecx, ecx
        mov dword ptr [esp + 0x1c], eax
        je L467b7b
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, eax
        jne L467b19
        test ebp, 0x55555555
        je L467b76
        add esi, 2
        jmp L467b19
L467b76:
        lea esi, [esi + ecx*2]
        jmp L467b19
L467b7b:
        dec edi
        jg L467b19
L467b7e:
        mov eax, dword ptr [esp + 0x30]
        mov edi, dword ptr [esp + 0x14]
        mov dword ptr [esp + 0x2c], eax
L467b8a:
        mov ebp, dword ptr [edx]
        dec dword ptr [esp + 0x30]
        add edi, 2
        and ebp, ebx
        rol ebx, 2
        add esi, 2
        mov ecx, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        je L467c5d
        test ebp, 0x55555555
        lea esi, [esi - 2]
        je L467c5d
        xor ecx, ecx
        inc dword ptr [esp + 0x30]
        sub edi, 2
        mov eax, dword ptr [esp + 0x1c]
        mov cl, byte ptr [eax]
        inc eax
        test ecx, ecx
        mov dword ptr [esp + 0x1c], eax
        je L467ce2
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, eax
        je L467c04
        lea edi, [edi + ecx*2]
        sub dword ptr [esp + 0x30], ecx
        jge L467c5d
        mov eax, dword ptr [esp + 0x30]
        jmp L467c5d
L467c04:
        test ebp, 0x55555555
        jne L467c33
        sub dword ptr [esp + 0x30], ecx
        jl L467c1a
        lea esi, [esi + ecx*2]
        lea edi, [edi + ecx*2]
        jmp L467c5d
L467c1a:
        mov eax, dword ptr [esp + 0x30]
        add eax, ecx
        lea esi, [esi + eax*2]
        lea edi, [edi + eax*2]
        mov eax, dword ptr [esp + 0x30]
        neg eax
        mov ecx, eax
        rep movsw
        jmp L467c5d
L467c33:
        sub dword ptr [esp + 0x30], ecx
        jl L467c41
        add esi, 2
        lea edi, [edi + ecx*2]
        jmp L467c5d
L467c41:
        mov eax, dword ptr [esp + 0x30]
        add eax, ecx
        lea edi, [edi + eax*2]
        mov eax, dword ptr [esp + 0x30]
        neg eax
        mov ecx, eax
        mov ax, word ptr [esi]
        add esi, 2
        rep stosw
        jmp L467c5d
L467c5d:
        cmp dword ptr [esp + 0x30], 0
        jg L467b8a
L467c68:
        mov ebp, dword ptr [edx]
        add edi, 2
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        jne L467c90
        mov ax, word ptr [esi]
        add esi, 2
        mov word ptr [edi - 2], ax
        jmp L467c68
L467c90:
        test ebp, 0x55555555
        je L467c68
        xor ecx, ecx
        sub edi, 2
        mov eax, dword ptr [esp + 0x1c]
        mov cl, byte ptr [eax]
        inc eax
        test ecx, ecx
        mov dword ptr [esp + 0x1c], eax
        je L467ce2
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, eax
        je L467cca
        lea edi, [edi + ecx*2]
        jmp L467c68
L467cca:
        test ebp, 0x55555555
        jne L467cd7
        rep movsw
        jmp L467c68
L467cd7:
        mov ax, word ptr [esi]
        add esi, 2
        rep stosw
        jmp L467c68
L467ce2:
        mov eax, dword ptr [esp + 0x14]
        mov ecx, dword ptr [esp + 0x24]
        add eax, dword ptr [esp + 0x28]
        dec ecx
        mov dword ptr [esp + 0x14], eax
        mov dword ptr [esp + 0x24], ecx
        mov edi, eax
        mov eax, dword ptr [esp + 0x2c]
        test ecx, ecx
        mov dword ptr [esp + 0x30], eax
        jne L467b8a
        pop ebp
        pop ebx
        pop esi
        pop edi
        ret
    }
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x00467d10
__declspec(naked) void FUN_00467d10(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) {
    __asm {
        push edi
        mov edx, dword ptr [esp + 0x14]
        mov edi, dword ptr [esp + 0x20]
        push esi
        mov esi, dword ptr [esp + 0x10]
        push ebx
        mov ebx, 3
        push ebp
        test edi, edi
        je L467d8e
L467d29:
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        jne L467d47
        add esi, 2
        jmp L467d29
L467d47:
        test ebp, 0x55555555
        je L467d29
        xor ecx, ecx
        mov eax, dword ptr [esp + 0x1c]
        mov cl, byte ptr [eax]
        inc eax
        test ecx, ecx
        mov dword ptr [esp + 0x1c], eax
        je L467d8b
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, eax
        jne L467d29
        test ebp, 0x55555555
        je L467d86
        add esi, 2
        jmp L467d29
L467d86:
        lea esi, [esi + ecx*2]
        jmp L467d29
L467d8b:
        dec edi
        jg L467d29
L467d8e:
        mov eax, dword ptr [esp + 0x34]
        mov edi, dword ptr [esp + 0x14]
        mov dword ptr [esp + 0x38], eax
L467d9a:
        dec dword ptr [esp + 0x34]
        mov ebp, dword ptr [edx]
        add edi, 2
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        jne L467dc9
        mov ax, word ptr [esi]
        add esi, 2
        mov word ptr [edi - 2], ax
        jmp L467e60
L467dc9:
        test ebp, 0x55555555
        je L467e60
        inc dword ptr [esp + 0x34]
        xor ecx, ecx
        mov eax, dword ptr [esp + 0x1c]
        sub edi, 2
        mov cl, byte ptr [eax]
        inc eax
        test ecx, ecx
        mov dword ptr [esp + 0x1c], eax
        je L467ecd
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, eax
        je L467e13
        lea edi, [edi + ecx*2]
        sub dword ptr [esp + 0x34], ecx
        jmp L467e60
L467e13:
        test ebp, 0x55555555
        jne L467e3a
        mov eax, dword ptr [esp + 0x34]
        sub eax, ecx
        jl L467e2c
        mov dword ptr [esp + 0x34], eax
        rep movsw
        jmp L467e60
L467e2c:
        neg eax
        mov ecx, dword ptr [esp + 0x34]
        rep movsw
        lea esi, [esi + eax*2]
        jmp L467e6b
L467e3a:
        mov eax, dword ptr [esp + 0x34]
        sub eax, ecx
        jl L467e51
        mov dword ptr [esp + 0x34], eax
        mov ax, word ptr [esi]
        add esi, 2
        rep stosw
        jmp L467e60
L467e51:
        mov ecx, dword ptr [esp + 0x34]
        mov ax, word ptr [esi]
        add esi, 2
        rep stosw
        jmp L467e6b
L467e60:
        cmp dword ptr [esp + 0x34], 0
        jg L467d9a
L467e6b:
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        jne L467e89
        add esi, 2
        jmp L467e6b
L467e89:
        test ebp, 0x55555555
        je L467e6b
        xor ecx, ecx
        mov eax, dword ptr [esp + 0x1c]
        mov cl, byte ptr [eax]
        inc eax
        test ecx, ecx
        mov dword ptr [esp + 0x1c], eax
        je L467ecd
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        test ebp, 0xaaaaaaaa
        lea edx, [edx + ebx*4]
        mov ebx, eax
        jne L467e6b
        test ebp, 0x55555555
        je L467ec8
        add esi, 2
        jmp L467e6b
L467ec8:
        lea esi, [esi + ecx*2]
        jmp L467e6b
L467ecd:
        mov eax, dword ptr [esp + 0x14]
        mov ecx, dword ptr [esp + 0x24]
        add eax, dword ptr [esp + 0x28]
        dec ecx
        mov dword ptr [esp + 0x14], eax
        mov dword ptr [esp + 0x24], ecx
        mov edi, eax
        mov eax, dword ptr [esp + 0x38]
        test ecx, ecx
        mov dword ptr [esp + 0x34], eax
        jne L467d9a
        pop ebp
        pop ebx
        pop esi
        pop edi
        ret
    }
}

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

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x00468040
__declspec(naked) void FUN_00468040(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) {
    __asm {
        push edi
        mov edx, dword ptr [esp + 0x14]
        mov edi, dword ptr [esp + 0x20]
        push esi
        mov esi, dword ptr [esp + 0x10]
        nop
        push ebx
        mov ebx, 3
        nop
        push ebp
        test edi, edi
        je L4680bd
L46805b:
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        test ebp, 0xaaaaaaaa
        jne L468077
        add esi, 2
L468077:
        test ebp, 0x55555555
        je L46805b
        mov eax, dword ptr [esp + 0x1c]
        movzx ecx, byte ptr [eax]
        inc eax
        mov dword ptr [esp + 0x1c], eax
        test ecx, ecx
        je L4680ba
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, eax
        test ebp, 0xaaaaaaaa
        jne L46805b
        test ebp, 0x55555555
        je L4680b5
        add esi, 2
        jmp L46805b
L4680b5:
        lea esi, [esi + ecx*2]
        jmp L46805b
L4680ba:
        dec edi
        jg L46805b
L4680bd:
        mov edi, dword ptr [esp + 0x14]
        mov eax, dword ptr [esp + 0x30]
        mov dword ptr [esp + 0x2c], eax
        mov eax, dword ptr [esp + 0x34]
        mov dword ptr [esp + 0x38], eax
L4680d1:
        cmp dword ptr [esp + 0x30], 0
        jle L46824d
        dec dword ptr [esp + 0x30]
        add edi, 2
        add esi, 2
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        test ebp, 0xaaaaaaaa
        je L468237
        sub esi, 2
        test ebp, 0x55555555
        je L468237
        inc dword ptr [esp + 0x30]
        sub edi, 2
        mov eax, dword ptr [esp + 0x1c]
        movzx ecx, byte ptr [eax]
        inc eax
        mov dword ptr [esp + 0x1c], eax
        test ecx, ecx
        je L4683d1
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, eax
        test ebp, 0xaaaaaaaa
        je L468160
        lea edi, [edi + ecx*2]
        sub dword ptr [esp + 0x30], ecx
        jge L468237
        mov eax, dword ptr [esp + 0x30]
        add dword ptr [esp + 0x34], eax
        jmp L468237
L468160:
        test ebp, 0x55555555
        jne L4681e0
        sub dword ptr [esp + 0x30], ecx
        jl L468179
        lea esi, [esi + ecx*2]
        lea edi, [edi + ecx*2]
        jmp L468237
L468179:
        mov eax, dword ptr [esp + 0x30]
        add dword ptr [esp + 0x34], eax
        jle L4681ae
        add eax, ecx
        lea esi, [esi + eax*2]
        lea edi, [edi + eax*2]
        mov eax, dword ptr [esp + 0x30]
        neg eax
        mov ecx, eax
L468193:
        mov ax, word ptr [esi]
        and ax, word ptr [DAT_007fe998]
        mov word ptr [edi], ax
        add esi, 2
        add edi, 2
        dec ecx
        jne L468193
        jmp L468237
L4681ae:
        add eax, ecx
        lea esi, [esi + eax*2]
        lea edi, [edi + eax*2]
        sub ecx, eax
        add ecx, dword ptr [esp + 0x34]
L4681bc:
        mov ax, word ptr [esi]
        and ax, word ptr [DAT_007fe998]
        mov word ptr [edi], ax
        add esi, 2
        add edi, 2
        dec ecx
        jne L4681bc
        mov eax, dword ptr [esp + 0x34]
        neg eax
        lea esi, [esi + eax*2]
        jmp L468370
L4681e0:
        sub dword ptr [esp + 0x30], ecx
        jl L4681ee
        add esi, 2
        lea edi, [edi + ecx*2]
        jmp L468237
L4681ee:
        mov eax, dword ptr [esp + 0x30]
        add dword ptr [esp + 0x34], eax
        jle L468217
        add eax, ecx
        lea edi, [edi + eax*2]
        mov eax, dword ptr [esp + 0x30]
        neg eax
        mov ecx, eax
        mov ax, word ptr [esi]
        add esi, 2
        and ax, word ptr [DAT_007fe998]
        rep stosw
        jmp L468237
L468217:
        add eax, ecx
        lea edi, [edi + eax*2]
        sub ecx, eax
        add ecx, dword ptr [esp + 0x34]
        mov ax, word ptr [esi]
        add esi, 2
        and ax, word ptr [DAT_007fe998]
        rep stosw
        jmp L468370
L468237:
        cmp dword ptr [esp + 0x30], 0
        jg L4680d1
        cmp dword ptr [esp + 0x34], 0
        jle L468370
L46824d:
        dec dword ptr [esp + 0x34]
        add edi, 2
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        test ebp, 0xaaaaaaaa
        jne L468283
        mov ax, word ptr [esi]
        add esi, 2
        and ax, word ptr [DAT_007fe998]
        mov word ptr [edi - 2], ax
        jmp L468365
L468283:
        test ebp, 0x55555555
        je L468365
        inc dword ptr [esp + 0x34]
        sub edi, 2
        mov eax, dword ptr [esp + 0x1c]
        movzx ecx, byte ptr [eax]
        inc eax
        mov dword ptr [esp + 0x1c], eax
        test ecx, ecx
        je L4683d1
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, eax
        test ebp, 0xaaaaaaaa
        je L4682cf
        lea edi, [edi + ecx*2]
        sub dword ptr [esp + 0x34], ecx
        jmp L468365
L4682cf:
        mov eax, dword ptr [esp + 0x3c]
        sub eax, edi
        sar eax, 1
        cmp eax, ecx
        jae L4682e2
        or dword ptr [DAT_007feb14], 1
L4682e2:
        test ebp, 0x55555555
        jne L468331
        mov eax, dword ptr [esp + 0x34]
        sub eax, ecx
        jl L46830e
        mov dword ptr [esp + 0x34], eax
L4682f6:
        mov ax, word ptr [esi]
        and ax, word ptr [DAT_007fe998]
        mov word ptr [edi], ax
        add esi, 2
        add edi, 2
        dec ecx
        jne L4682f6
        jmp L468365
L46830e:
        neg eax
        mov ecx, dword ptr [esp + 0x34]
        push eax
L468315:
        mov ax, word ptr [esi]
        and ax, word ptr [DAT_007fe998]
        mov word ptr [edi], ax
        add esi, 2
        add edi, 2
        dec ecx
        jne L468315
        pop eax
        lea esi, [esi + eax*2]
        jmp L468370
L468331:
        mov eax, dword ptr [esp + 0x34]
        sub eax, ecx
        jl L46834f
        mov dword ptr [esp + 0x34], eax
        mov ax, word ptr [esi]
        add esi, 2
        and ax, word ptr [DAT_007fe998]
        rep stosw
        jmp L468365
L46834f:
        mov ecx, dword ptr [esp + 0x34]
        mov ax, word ptr [esi]
        add esi, 2
        and ax, word ptr [DAT_007fe998]
        rep stosw
        jmp L468370
L468365:
        cmp dword ptr [esp + 0x34], 0
        jg L46824d
L468370:
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        test ebp, 0xaaaaaaaa
        jne L46838e
        add esi, 2
        jmp L468370
L46838e:
        test ebp, 0x55555555
        je L468370
        mov eax, dword ptr [esp + 0x1c]
        movzx ecx, byte ptr [eax]
        inc eax
        mov dword ptr [esp + 0x1c], eax
        test ecx, ecx
        je L4683d1
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, eax
        test ebp, 0xaaaaaaaa
        jne L468370
        test ebp, 0x55555555
        je L4683cc
        add esi, 2
        jmp L468370
L4683cc:
        lea esi, [esi + ecx*2]
        jmp L468370
L4683d1:
        mov eax, dword ptr [esp + 0x14]
        add eax, dword ptr [esp + 0x28]
        mov dword ptr [esp + 0x14], eax
        mov edi, eax
        mov ecx, dword ptr [esp + 0x24]
        dec ecx
        mov dword ptr [esp + 0x24], ecx
        mov eax, dword ptr [esp + 0x2c]
        mov dword ptr [esp + 0x30], eax
        mov eax, dword ptr [esp + 0x38]
        mov dword ptr [esp + 0x34], eax
        test ecx, ecx
        jne L4680d1
        pop ebp
        pop ebx
        pop esi
        pop edi
        ret
    }
}

// Hand-written assembly in the original (MSVC6 never emits it from C); transcribed as naked __asm.
// FUNCTION: LEGOLAND 0x00468410
__declspec(naked) void FUN_00468410(void) {
    __asm {
        push edi
        mov edx, dword ptr [esp + 0x14]
        mov edi, dword ptr [esp + 0x20]
        push esi
        mov esi, dword ptr [esp + 0x10]
        nop
        push ebx
        mov ebx, 3
        nop
        push ebp
        test edi, edi
        je L46848d
L46842b:
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        test ebp, 0xaaaaaaaa
        jne L468447
        add esi, 2
L468447:
        test ebp, 0x55555555
        je L46842b
        mov eax, dword ptr [esp + 0x1c]
        movzx ecx, byte ptr [eax]
        inc eax
        mov dword ptr [esp + 0x1c], eax
        test ecx, ecx
        je L46848a
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, eax
        test ebp, 0xaaaaaaaa
        jne L46842b
        test ebp, 0x55555555
        je L468485
        add esi, 2
        jmp L46842b
L468485:
        lea esi, [esi + ecx*2]
        jmp L46842b
L46848a:
        dec edi
        jg L46842b
L46848d:
        mov edi, dword ptr [esp + 0x14]
        mov eax, dword ptr [esp + 0x30]
        mov dword ptr [esp + 0x2c], eax
        mov eax, dword ptr [esp + 0x34]
        mov dword ptr [esp + 0x38], eax
L4684a1:
        cmp dword ptr [esp + 0x30], 0
        jle L468629
        dec dword ptr [esp + 0x30]
        add edi, 2
        add esi, 2
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        test ebp, 0xaaaaaaaa
        je L468613
        sub esi, 2
        test ebp, 0x55555555
        je L468613
        inc dword ptr [esp + 0x30]
        sub edi, 2
        mov eax, dword ptr [esp + 0x1c]
        movzx ecx, byte ptr [eax]
        inc eax
        mov dword ptr [esp + 0x1c], eax
        test ecx, ecx
        je L4687bc
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, eax
        test ebp, 0xaaaaaaaa
        je L468530
        lea edi, [edi + ecx*2]
        sub dword ptr [esp + 0x30], ecx
        jge L468613
        mov eax, dword ptr [esp + 0x30]
        add dword ptr [esp + 0x34], eax
        jmp L468613
L468530:
        test ebp, 0x55555555
        jne L4685b6
        sub dword ptr [esp + 0x30], ecx
        jl L468549
        lea esi, [esi + ecx*2]
        lea edi, [edi + ecx*2]
        jmp L468613
L468549:
        mov eax, dword ptr [esp + 0x30]
        add dword ptr [esp + 0x34], eax
        jle L468581
        add eax, ecx
        lea esi, [esi + eax*2]
        lea edi, [edi + eax*2]
        mov eax, dword ptr [esp + 0x30]
        neg eax
        mov ecx, eax
L468563:
        mov ax, word ptr [esi]
        and ax, word ptr [DAT_007fe998]
        shr ax, 1
        mov word ptr [edi], ax
        add esi, 2
        add edi, 2
        dec ecx
        jne L468563
        jmp L468613
L468581:
        add eax, ecx
        lea esi, [esi + eax*2]
        lea edi, [edi + eax*2]
        sub ecx, eax
        add ecx, dword ptr [esp + 0x34]
L46858f:
        mov ax, word ptr [esi]
        and ax, word ptr [DAT_007fe998]
        shr ax, 1
        mov word ptr [edi], ax
        add esi, 2
        add edi, 2
        dec ecx
        jne L46858f
        mov eax, dword ptr [esp + 0x34]
        neg eax
        lea esi, [esi + eax*2]
        jmp L46875b
L4685b6:
        sub dword ptr [esp + 0x30], ecx
        jl L4685c4
        add esi, 2
        lea edi, [edi + ecx*2]
        jmp L468613
L4685c4:
        mov eax, dword ptr [esp + 0x30]
        add dword ptr [esp + 0x34], eax
        jle L4685f0
        add eax, ecx
        lea edi, [edi + eax*2]
        mov eax, dword ptr [esp + 0x30]
        neg eax
        mov ecx, eax
        mov ax, word ptr [esi]
        add esi, 2
        and ax, word ptr [DAT_007fe998]
        shr ax, 1
        rep stosw
        jmp L468613
L4685f0:
        add eax, ecx
        lea edi, [edi + eax*2]
        sub ecx, eax
        add ecx, dword ptr [esp + 0x34]
        mov ax, word ptr [esi]
        add esi, 2
        and ax, word ptr [DAT_007fe998]
        shr ax, 1
        rep stosw
        jmp L46875b
L468613:
        cmp dword ptr [esp + 0x30], 0
        jg L4684a1
        cmp dword ptr [esp + 0x34], 0
        jle L46875b
L468629:
        dec dword ptr [esp + 0x34]
        add edi, 2
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        test ebp, 0xaaaaaaaa
        jne L468662
        mov ax, word ptr [esi]
        add esi, 2
        and ax, word ptr [DAT_007fe998]
        shr ax, 1
        mov word ptr [edi - 2], ax
        jmp L468750
L468662:
        test ebp, 0x55555555
        je L468750
        inc dword ptr [esp + 0x34]
        sub edi, 2
        mov eax, dword ptr [esp + 0x1c]
        movzx ecx, byte ptr [eax]
        inc eax
        mov dword ptr [esp + 0x1c], eax
        test ecx, ecx
        je L4687bc
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, eax
        test ebp, 0xaaaaaaaa
        je L4686ae
        lea edi, [edi + ecx*2]
        sub dword ptr [esp + 0x34], ecx
        jmp L468750
L4686ae:
        mov eax, dword ptr [esp + 0x3c]
        sub eax, edi
        sar eax, 1
        cmp eax, ecx
        jae L4686c1
        or dword ptr [DAT_007feb14], 1
L4686c1:
        test ebp, 0x55555555
        jne L468716
        mov eax, dword ptr [esp + 0x34]
        sub eax, ecx
        jl L4686f0
        mov dword ptr [esp + 0x34], eax
L4686d5:
        mov ax, word ptr [esi]
        and ax, word ptr [DAT_007fe998]
        shr ax, 1
        mov word ptr [edi], ax
        add esi, 2
        add edi, 2
        dec ecx
        jne L4686d5
        jmp L468750
L4686f0:
        neg eax
        mov ecx, dword ptr [esp + 0x34]
        push eax
L4686f7:
        mov ax, word ptr [esi]
        and ax, word ptr [DAT_007fe998]
        shr ax, 1
        mov word ptr [edi], ax
        add esi, 2
        add edi, 2
        dec ecx
        jne L4686f7
        pop eax
        lea esi, [esi + eax*2]
        jmp L46875b
L468716:
        mov eax, dword ptr [esp + 0x34]
        sub eax, ecx
        jl L468737
        mov dword ptr [esp + 0x34], eax
        mov ax, word ptr [esi]
        add esi, 2
        and ax, word ptr [DAT_007fe998]
        shr ax, 1
        rep stosw
        jmp L468750
L468737:
        mov ecx, dword ptr [esp + 0x34]
        mov ax, word ptr [esi]
        add esi, 2
        and ax, word ptr [DAT_007fe998]
        shr ax, 1
        rep stosw
        jmp L46875b
L468750:
        cmp dword ptr [esp + 0x34], 0
        jg L468629
L46875b:
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov ecx, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, ecx
        test ebp, 0xaaaaaaaa
        jne L468779
        add esi, 2
        jmp L46875b
L468779:
        test ebp, 0x55555555
        je L46875b
        mov eax, dword ptr [esp + 0x1c]
        movzx ecx, byte ptr [eax]
        inc eax
        mov dword ptr [esp + 0x1c], eax
        test ecx, ecx
        je L4687bc
        mov ebp, dword ptr [edx]
        and ebp, ebx
        rol ebx, 2
        mov eax, ebx
        and ebx, 1
        lea edx, [edx + ebx*4]
        mov ebx, eax
        test ebp, 0xaaaaaaaa
        jne L46875b
        test ebp, 0x55555555
        je L4687b7
        add esi, 2
        jmp L46875b
L4687b7:
        lea esi, [esi + ecx*2]
        jmp L46875b
L4687bc:
        mov eax, dword ptr [esp + 0x14]
        add eax, dword ptr [esp + 0x28]
        mov dword ptr [esp + 0x14], eax
        mov edi, eax
        mov ecx, dword ptr [esp + 0x24]
        dec ecx
        mov dword ptr [esp + 0x24], ecx
        mov eax, dword ptr [esp + 0x2c]
        mov dword ptr [esp + 0x30], eax
        mov eax, dword ptr [esp + 0x38]
        mov dword ptr [esp + 0x34], eax
        test ecx, ecx
        jne L4684a1
        pop ebp
        pop ebx
        pop esi
        pop edi
        ret
    }
}

// FUNCTION: LEGOLAND 0x004687f0
void FUN_004687f0(const char *param_1) {
    strncpy(DAT_0066861c, param_1, 0x80);
    DAT_0066861c[0x7f] = 0;
}
