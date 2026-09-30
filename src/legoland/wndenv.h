#pragma once

#include <windows.h>
#include "legoland.h"

LEGO_EXPORT HWND WNDENV_Gethwnd(void);
BOOL FUN_0047fe70(void);
void FUN_0047fe80(void);
LEGO_EXPORT void *WNDENV_GethInstance(void);
LEGO_EXPORT LRESULT CALLBACK LegoLandWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LEGO_EXPORT int ProcessSystemEvents(void);

struct ResFile;
void FUN_00480150(struct ResFile *file, void *out);
void FUN_00480170(struct ResFile *file, void *out);
void *FUN_004801a0(struct ResFile *file);
LEGO_EXPORT void WNDENV_Sethwnd(HWND param_1);
