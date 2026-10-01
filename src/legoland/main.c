#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include "legoland.h"

#include "debug.h"
#include "exceptlog.h"
#include "globals.h"
#include "main.h"
#include "screens.h"

// FUNCTION: LEGOLAND 0x00453cd0
void LogOutput(char *text) {}

// FUNCTION: LEGOLAND 0x00453ce0
void LogPrintf(const char *format, ...) {
    va_list argptr;

    va_start(argptr, format);
    vsprintf(LogBuffer, format, argptr);
    va_end(argptr);
    LogOutput(LogBuffer);
}

// FUNCTION: LEGOLAND 0x00453d10
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int result;

    result = -1;
    __try {
        FUN_00458830((char *)&DAT_0066752c);
        result = wWinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
        // STRING: LEGOLAND 0x004b8a94
    } __except (stackdump((void *)GetExceptionInformation(), "main thread")) {
    }

    return result;
}
