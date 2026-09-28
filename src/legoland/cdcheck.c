#include <windows.h>
#include "legoland.h"

#include <ctype.h>
#include <string.h>
#include "cdcheck.h"
#include "debug.h"
#include "globals.h"
#include "saveload.h"

// FUNCTION: LEGOLAND 0x00450f30
int FUN_00450f30(char *cd_volume) {
    DWORD drives;
    int result;
    DWORD bit;
    int i;
    char root_path[4];
    char volume_name[256];
    char fs_name[256];
    DWORD serial_number;
    DWORD max_component_length;
    DWORD fs_flags;

    // STRING: LEGOLAND 0x004b8630
    strcpy(root_path, "c:\\");
    result = 0;
    drives = GetLogicalDrives();
    // STRING: LEGOLAND 0x004b8610
    FUN_0047f870("Checking all drives (Mask = %d)", drives);
    FUN_0047f850();
    if (drives != 0) {
        bit = 1;
        i = 0;
        do {
            if (bit & drives) {
                root_path[0] = (char)('A' + i);
                if (GetDriveTypeA(root_path) == DRIVE_CDROM) {
                    // STRING: LEGOLAND 0x004b85f4
                    FUN_0047f870("Getting Info on drive %s", root_path);
                    FUN_0047f850();
                    if (GetVolumeInformationA(root_path, volume_name, 256, &serial_number, &max_component_length, &fs_flags, fs_name, 256)) {
                        // STRING: LEGOLAND 0x004b85ec
                        if (strcmp(fs_name, "CDFS") == 0) {
                            if (cd_volume == NULL || strcmp(volume_name, cd_volume) == 0) {
                                strcpy(DAT_00813b04, root_path);
                                // STRING: LEGOLAND 0x004b85c8
                                FUN_0047f870("Drive %s contains the correct CD", DAT_00813b04);
                                FUN_0047f850();
                                result = 1;
                            }
                        }
                    }
                }
            }
            i++;
            bit <<= 1;
        } while (i < 32);
    }

    return result;
}

// FUNCTION: LEGOLAND 0x004510e0
int FUN_004510e0(char *cd_volume) {
    int result;
    char root_path[4];
    char volume_name[256];
    char fs_name[256];
    DWORD serial_number;
    DWORD max_component_length;
    DWORD fs_flags;

    strcpy(root_path, "c:\\");
    result = 0;
    root_path[0] = DAT_00813b04[0];
    toupper(DAT_00813b04[0]);
    if (GetVolumeInformationA(root_path, volume_name, 256, &serial_number, &max_component_length, &fs_flags, fs_name, 256)) {
        if (strcmp(fs_name, "CDFS") == 0) {
            if (cd_volume == NULL || strcmp(volume_name, cd_volume) == 0) {
                return 1;
            }
        }
    }
    return result;
}

// FUNCTION: LEGOLAND 0x004511e0
void FUN_004511e0(void) { return; }

// FUNCTION: LEGOLAND 0x004511f0
void FUN_004511f0(int param_1) { return; }

// FUNCTION: LEGOLAND 0x00451200
void FUN_00451200(void) { return; }

// FUNCTION: LEGOLAND 0x00451210
void FUN_00451210(int param_1) {
    int drive;

    FUN_004511f0(param_1);
    if (DAT_004b85c4 != INVALID_HANDLE_VALUE) {
        *(char *)&drive = (char)toupper((char)param_1) - 0x40;
        FUN_00451280(DAT_004b85c4, drive);
        FUN_00451410(DAT_004b85c4, drive);
        FUN_00451550(DAT_004b85c4, drive);
        FUN_004514a0(DAT_004b85c4);
        DAT_004b85c4 = INVALID_HANDLE_VALUE;
    }
}

// FUNCTION: LEGOLAND 0x00451280
int FUN_00451280(HANDLE h, int drive) {
    DIOC_REGISTERS regs = {0};
    unsigned char buf[8];
    int result;
    int i;

    buf[1] = 0;
    regs.reg_EBX = drive & 0xff;
    regs.reg_EDX = (DWORD)buf;
    buf[0] = 2;
    regs.reg_EAX = 0x440d;
    regs.reg_ECX = 0x848;
    result = DeviceIoControl(h, 1, &regs, sizeof(regs), &regs, sizeof(regs), (LPDWORD)&drive, NULL);
    if (!result) {
        return 0;
    }
    if (regs.reg_Flags & 1) {
        if (regs.reg_EAX != 0xb0 && regs.reg_EAX != 1) {
            return 0;
        }
        result = 1;
    }
    i = 0;
    while (buf[1] > 0) {
        regs.reg_EBX = drive & 0xff;
        regs.reg_EDX = (DWORD)buf;
        buf[0] = 1;
        regs.reg_EAX = 0x440d;
        regs.reg_ECX = 0x848;
        if (!DeviceIoControl(h, 1, &regs, sizeof(regs), &regs, sizeof(regs), (LPDWORD)&drive, NULL) || (regs.reg_Flags & 1)) {
            return 0;
        }
        i++;
        result = 1;
        if (i >= buf[1]) {
            break;
        }
    }
    return result;
}

// FUNCTION: LEGOLAND 0x00451390
int FUN_00451390(HANDLE h, int drive) {
    DIOC_REGISTERS regs = {0};
    char buf[2];

    regs.reg_EBX = drive & 0xff;
    buf[1] = 0;
    buf[0] = 0;
    regs.reg_EAX = 0x440d;
    regs.reg_ECX = 0x848;
    regs.reg_EDX = (DWORD)buf;
    if (DeviceIoControl(h, 1, &regs, sizeof(regs), &regs, sizeof(regs), (LPDWORD)&drive, NULL) && !(regs.reg_Flags & 1)) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00451410
int FUN_00451410(HANDLE h, int drive) {
    DIOC_REGISTERS regs = {0};

    regs.reg_EBX = drive & 0xff;
    regs.reg_EAX = 0x440d;
    regs.reg_ECX = 0x849;
    if (DeviceIoControl(h, 1, &regs, sizeof(regs), &regs, sizeof(regs), (LPDWORD)&drive, NULL) && !(regs.reg_Flags & 1)) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00451480
int FUN_00451480(void) {
    // STRING: LEGOLAND 0x004b8634
    HANDLE result = CreateFileA("\\\\.\\vwin32", 0, 0, NULL, 0, 0x4000000, NULL);
    return (int)result;
}

// FUNCTION: LEGOLAND 0x004514a0
BOOL __stdcall FUN_004514a0(HANDLE param_1) { return CloseHandle(param_1); }

// FUNCTION: LEGOLAND 0x004514b0
BOOL __stdcall FUN_004514b0(HANDLE h, int drive, int param_3, int param_4) {
    DIOC_REGISTERS regs = {0};
    unsigned char category = 0x48;
    BOOL result;

    for (;;) {
        regs.reg_ECX = MAKEWORD(0x4a, category);
        regs.reg_EAX = 0x440d;
        regs.reg_EBX = MAKEWORD(drive, param_3);
        regs.reg_EDX = param_4 & 0xffff;
        if (DeviceIoControl(h, 1, &regs, sizeof(regs), &regs, sizeof(regs), (LPDWORD)&drive, NULL) && !(regs.reg_Flags & 1)) {
            result = TRUE;
            break;
        }
        result = FALSE;
        if (category == 8) {
            break;
        }
        category = 8;
    }
    return result;
}

// FUNCTION: LEGOLAND 0x00451550
BOOL __stdcall FUN_00451550(HANDLE h, int drive) {
    DIOC_REGISTERS regs = {0};
    unsigned char category = 0x48;
    int d = drive & 0xff;
    BOOL result;

    for (;;) {
        regs.reg_ECX = MAKEWORD(0x6a, category);
        regs.reg_EAX = 0x440d;
        regs.reg_EBX = d;
        if (DeviceIoControl(h, 1, &regs, sizeof(regs), &regs, sizeof(regs), (LPDWORD)&drive, NULL) && !(regs.reg_Flags & 1)) {
            result = TRUE;
            break;
        }
        result = FALSE;
        if (category == 8) {
            break;
        }
        category = 8;
    }
    return result;
}

// FUNCTION: LEGOLAND 0x004515e0
int FUN_004515e0(int param_1) { STUB(); }
