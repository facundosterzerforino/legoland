#include "exceptlog.h"
#include <windows.h>
#include <stdarg.h>
#include <string.h>
#include "legoland.h"

struct ExceptionEntry {
    unsigned int code;
    const char *message;
};

// FUNCTION: LEGOLAND 0x00453da0
int stackdump(void *exc_info, const char *filename) { STUB(); }

// FUNCTION: LEGOLAND 0x00454290
void FUN_00454290(HANDLE file, const char *format, ...) {
    DWORD written;
    char buffer[2000];
    va_list args;

    va_start(args, format);
    wvsprintfA(buffer, format, args);
    WriteFile(file, buffer, lstrlenA(buffer), &written, NULL);
}

// FUNCTION: LEGOLAND 0x004542e0
void FUN_004542e0(HANDLE file) {
    SYSTEM_INFO si;
    MEMORY_BASIC_INFORMATION mbi;
    DWORD pagesize;
    DWORD limit;
    DWORD i = 0;
    DWORD prev = 0;

    // STRING: LEGOLAND 0x004b8c90
    FUN_00454290(file, "\r\n\tModule list: names, addresses, sizes, time stamps and file times:\r\n");
    GetSystemInfo(&si);
    pagesize = si.dwPageSize;
    limit = (0x40000000 / pagesize) << 2;
    while (i < limit) {
        if (VirtualQuery((LPCVOID)(pagesize * i), &mbi, sizeof(mbi)) && mbi.RegionSize > 0) {
            i += mbi.RegionSize / pagesize;
            if (mbi.State == MEM_COMMIT && (DWORD)mbi.AllocationBase > prev) {
                prev = (DWORD)mbi.AllocationBase;
                FUN_00454380(file, prev);
            }
        } else {
            i += 0x10000 / pagesize;
        }
    }
}

// FUNCTION: LEGOLAND 0x00454380
void FUN_00454380(HANDLE file, DWORD base) { STUB(); }

// FUNCTION: LEGOLAND 0x00454500
void FUN_00454500(char *buffer, FILETIME ft) {
    WORD date;
    WORD time;

    if (FileTimeToLocalFileTime(&ft, &ft) && FileTimeToDosDateTime(&ft, &date, &time)) {
        // STRING: LEGOLAND 0x004b8d18
        wsprintfA(buffer, "%d/%d/%d %02d:%02d:%02d", (date >> 5) & 0xf, date & 0x1f, (date >> 9) + 1980,
            time >> 11, (time >> 5) & 0x3f, (time & 0x1f) * 2);
    } else {
        buffer[0] = 0;
    }
}

// FUNCTION: LEGOLAND 0x004545a0
void FUN_004545a0(void) { STUB(); }

// FUNCTION: LEGOLAND 0x00454700
const char *FUN_00454700(unsigned int code) {
    struct ExceptionEntry table[24] = {
        // STRING: LEGOLAND 0x004b8fd8
        {0x40010005, "a Control-C"},
        // STRING: LEGOLAND 0x004b8fc8
        {0x40010008, "a Control-Break"},
        // STRING: LEGOLAND 0x004b8fb0
        {0x80000002, "a Datatype Misalignment"},
        // STRING: LEGOLAND 0x004b8fa0
        {0x80000003, "a Breakpoint"},
        // STRING: LEGOLAND 0x004b8f8c
        {0xc0000005, "an Access Violation"},
        // STRING: LEGOLAND 0x004b8f78
        {0xc0000006, "an In Page Error"},
        // STRING: LEGOLAND 0x004b8f6c
        {0xc0000017, "a No Memory"},
        // STRING: LEGOLAND 0x004b8f54
        {0xc000001d, "an Illegal Instruction"},
        // STRING: LEGOLAND 0x004b8f38
        {0xc0000025, "a Noncontinuable Exception"},
        // STRING: LEGOLAND 0x004b8f20
        {0xc0000026, "an Invalid Disposition"},
        // STRING: LEGOLAND 0x004b8f08
        {0xc000008c, "a Array Bounds Exceeded"},
        // STRING: LEGOLAND 0x004b8eec
        {0xc000008d, "a Float Denormal Operand"},
        // STRING: LEGOLAND 0x004b8ed4
        {0xc000008e, "a Float Divide by Zero"},
        // STRING: LEGOLAND 0x004b8ebc
        {0xc000008f, "a Float Inexact Result"},
        // STRING: LEGOLAND 0x004b8ea0
        {0xc0000090, "a Float Invalid Operation"},
        // STRING: LEGOLAND 0x004b8e8c
        {0xc0000091, "a Float Overflow"},
        // STRING: LEGOLAND 0x004b8e78
        {0xc0000092, "a Float Stack Check"},
        // STRING: LEGOLAND 0x004b8e64
        {0xc0000093, "a Float Underflow"},
        // STRING: LEGOLAND 0x004b8e48
        {0xc0000094, "an Integer Divide by Zero"},
        // STRING: LEGOLAND 0x004b8e34
        {0xc0000095, "an Integer Overflow"},
        // STRING: LEGOLAND 0x004b8e18
        {0xc0000096, "a Privileged Instruction"},
        // STRING: LEGOLAND 0x004b8e04
        {0xc00000fd, "a Stack Overflow"},
        // STRING: LEGOLAND 0x004b8de8
        {0xc0000142, "a DLL Initialization Failed"},
        // STRING: LEGOLAND 0x004b8dcc
        {0xe06d7363, "a Microsoft C++ Exception"}};
    unsigned int i = 0;
    while (i < 24) {
        if (code == table[i].code) {
            return table[i].message;
        }
        i++;
    }
    // STRING: LEGOLAND 0x004b8db4
    return "Unknown exception type";
}

// FUNCTION: LEGOLAND 0x004548f0
char *FUN_004548f0(char *path) {
    char *last_backslash = strrchr(path, '\\');
    if (last_backslash != NULL) {
        return last_backslash + 1;
    }
    return path;
}
