#include "resource.h"
#include <windows.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdcheck.h"
#include "debug.h"
#include "globals.h"
#include "legoland.h"
#include "saveload.h"

struct ResVolume;
struct ResVolEntry;

struct ResVolume {
    struct ResVolume *next;
    struct ResVolEntry *dir;
    char name[0x14];
    HANDLE handle;
    struct ResFile *current;
    unsigned int refcount;
};
/* The __asm version of RES_OpenVolume names the fields through the typedef (inline asm can't see struct tags). */
typedef struct ResVolume ResVolume;

struct ResFile {
    int size;
    int base;
    struct ResVolume *volume;
    int position;
};

struct MasterDirNode {
    struct MasterDirNode *next;
    struct ResVolEntry *entries; /* the directory's first entry (through next_dir) */
    char *name;
};

struct MasterVolNode {
    struct MasterVolNode *next;
    struct ResVolEntry *entries; /* the volume's first entry */
    char name[1];
};

struct ResVolEntry;

struct ResVolEntry {
    struct ResVolEntry *next_all;
    struct ResVolEntry *next_dir;
    struct ResVolEntry *next;
    struct MasterDirNode *dir;
    struct ResVolume *volume;
    int size;
    int base;
    char *name;
};

struct ResDirRecord {
    int child;
    int sibling;
    int is_dir;
    int size;
    int base;
    char name[1];
};

// FUNCTION: LEGOLAND 0x00489440
struct MasterDirNode *FindOrAddMasterDir(char *name) {
    struct MasterDirNode *node;
    struct MasterDirNode *new_node;
    char *copy;

    for (node = MasterDirList; node != 0; node = node->next) {
        if (_stricmp(name, node->name) == 0) {
            return node;
        }
    }

    new_node = (struct MasterDirNode *)malloc(sizeof(struct MasterDirNode));
    copy = (char *)malloc(strlen(name) + 1);
    new_node->name = copy;
    strcpy(copy, name);

    new_node->next = MasterDirList;
    new_node->entries = 0;
    MasterDirList = new_node;
    return new_node;
}

// FUNCTION: LEGOLAND 0x004894d0
LEGO_EXPORT struct MasterDirNode *GetMasterDirPtr(const char *name) {
    struct MasterDirNode *node;

    for (node = MasterDirList; node != 0; node = node->next) {
        if (_stricmp(name, node->name) == 0) {
            return node;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00489510
LEGO_EXPORT struct MasterVolNode *GetMasterVolPtr(const char *name) {
    struct MasterVolNode *node;

    for (node = MasterVolList; node != 0; node = node->next) {
        if (_stricmp(name, node->name) == 0) {
            return node;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00489550
struct MasterDirNode *FUN_00489550(const char *vol_name, const char *file_name, struct ResVolEntry **out_entry) {
    struct MasterVolNode *master;
    struct ResVolEntry *entry;

    master = GetMasterVolPtr(vol_name);
    if (master != 0) {
        for (entry = (struct ResVolEntry *)master->entries; entry != 0; entry = entry->next) {
            if (_stricmp(file_name, entry->dir->name) == 0) {
                *out_entry = entry;
                return entry->dir;
            }
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x004895a0
void FUN_004895a0(struct ResDirRecord *node, struct ResVolume *volume, struct ResDirRecord *base, char *path) {
    char buffer[0x104];
    struct MasterDirNode *master;
    struct ResVolEntry *entry;

    strcpy(buffer, path);
    master = FindOrAddMasterDir(buffer);

    if (node->is_dir == 0) {
        entry = (struct ResVolEntry *)malloc(sizeof(struct ResVolEntry));
        entry->next_all = (struct ResVolEntry *)DAT_0079862c;
        DAT_0079862c = entry;
        entry->next_dir = (struct ResVolEntry *)master->entries;
        master->entries = entry;
        entry->next = volume->dir;
        volume->dir = entry;
        entry->size = node->size;
        entry->base = node->base;
        entry->name = (char *)malloc(strlen(node->name) + 1);
        strcpy(entry->name, node->name);
        entry->dir = (struct MasterDirNode *)master;
        entry->volume = volume;
    }

    if (node->sibling != -1) {
        node->sibling += (int)base;
        FUN_004895a0((struct ResDirRecord *)node->sibling, volume, base, buffer);
    }

    if (node->child != -1) {
        strcat(buffer, node->name);
        // STRING: LEGOLAND 0x004bdd7c
        strcat(buffer, "\\");
        node->child += (int)base;
        FUN_004895a0((struct ResDirRecord *)node->child, volume, base, buffer);
    }
}

// FUNCTION: LEGOLAND 0x00489740
LEGO_EXPORT unsigned int RES_GetResourcePath(void) {
    return 0;
}

// Strings used by RES_OpenVolume. They are named arrays rather than literals so the MSVC6 __asm version
// below can push their addresses (inline asm cannot take the address of a string literal).
// GLOBAL: LEGOLAND 0x004bde74
static const char ResOpenTraceFormat[] = "Attempting to open Resource %s";
// GLOBAL: LEGOLAND 0x004bddc8
static const char ResAlreadyOpenTrace[] = "Volume Already open";
// GLOBAL: LEGOLAND 0x004bde60
static const char ResLocalPathFormat[] = ".\\volumes\\%s.res";
// GLOBAL: LEGOLAND 0x004bde48
static const char ResTryPathTraceFormat[] = "Trying to open from %s";
// GLOBAL: LEGOLAND 0x004bde3c
static const char ResCdPathFormat[] = "%s%s.res";
// GLOBAL: LEGOLAND 0x004bde28
static const char ResOpenedTraceFormat[] = "Openned resource %s";
// GLOBAL: LEGOLAND 0x004bde04
static const char ResDirInfoTraceFormat[] = "FileSize = %x, Directory is at %x";
// GLOBAL: LEGOLAND 0x004bdddc
static const char ResDirAllocFailedTrace[] = "Failed to allocate space for directory";
// GLOBAL: LEGOLAND 0x004bdd94
static const char ResDirShortReadTraceFormat[] = "Failed to load directory fully (%x of %x loaded)";
// GLOBAL: LEGOLAND 0x004bdd80
static const char ResDirReadOkTrace[] = "Directory read OK";

// RES_OpenVolume: opens the resource volume <path's file name>.res, from .\volumes\ first and then from the CD.
// The volume's name is the file name upper-cased (20 chars, zero padded); if a volume with that name is already
// open it is returned instead. A volume file starts with the offset of its directory, which runs to the end of the
// file; the directory is read and turned into entries (FUN_004895a0), and the volume goes to the front of
// MasterVolList with one reference. Returns NULL (and frees everything) if the file can't be opened or read.
//
// The MSVC6 build uses an exact __asm transcription of the original. The C below (the #else branch, built by the
// port) is the same function, and the asm is commented block by block against it. Why asm: in the C version the
// "Failed to allocate space for directory" path jumps into the tail of the "Failed to load directory fully" path
// (CloseHandle; free; return NULL), as in the original, but then MSVC6 colours the temporaries of the
// SetFilePointer/ReadFile block with ecx/edx where the original has eax/ecx (and edx/eax for the later
// CloseHandle and MasterVolList loads): 97.82% ignoring registers, 92.14% exact. The only C that got those
// registers (an extra `close_handle` local loaded before the failure trace) loses the shared tail instead (97.18%).
// About 40 variants of the handle temporaries, the types of file_size/dir_offset/bytes_read/dir_data, the
// argument and comparison forms, and goto-based cleanup did not move the colouring
// (docs/attempts/0x00489750-RES_OpenVolume.md).
// clang-format off
#if defined(_MSC_VER) && (_MSC_VER <= 1200) && defined(_M_IX86)
// FUNCTION: LEGOLAND 0x00489750
__declspec(naked) LEGO_EXPORT struct ResVolume *RES_OpenVolume(const char *path) {
    __asm {
        // Frame after the four pushes: [esp+0x10] dir_offset, [esp+0x14] bytes_read, [esp+0x18] res_path[0x104],
        // [esp+0x11c] fname[0x100], path at [esp+0x220]. Pushed arguments move these up by 4 each.
        // esi = volume, ebp = cur (later file_size), edi = i (later dir_data), ebx = name_len (later SetFilePointer, 0).
        sub esp, 0x20c
        push ebx
        push ebp
        push esi
        push edi
        // volume = malloc(sizeof(struct ResVolume)); cur = MasterVolList;
        // _splitpath(path, NULL, NULL, fname, NULL); DebugTrace("Attempting to open Resource %s", fname);
        // FUN_0047f850();
        push 0x28
        call malloc
        mov ecx, dword ptr [esp + 0x224]
        mov ebp, dword ptr [MasterVolList]
        mov esi, eax
        lea eax, [esp + 0x120]
        push 0
        push eax
        push 0
        push 0
        push ecx
        call _splitpath
        lea edx, [esp + 0x134]
        push edx
        push offset ResOpenTraceFormat
        call DebugTrace
        add esp, 0x20
        call FUN_0047f850
        // name_len = strlen(fname);
        lea edi, [esp + 0x11c]
        or ecx, 0xffffffff
        xor eax, eax
        repne scasb
        not ecx
        dec ecx
        // for (i = 0; i < 0x14; i++)
        //     volume->name[i] = i < name_len ? (char)toupper(fname[i]) : '\0';
        xor edi, edi
        mov ebx, ecx
    name_next_char:
        cmp edi, ebx
        jge name_pad
        movsx eax, byte ptr [esp + edi + 0x11c]
        push eax
        call toupper
        add esp, 4
        mov byte ptr [edi + esi]ResVolume.name, al
        jmp name_char_done
    name_pad:
        mov byte ptr [edi + esi]ResVolume.name, 0
    name_char_done:
        inc edi
        cmp edi, 0x14
        jl name_next_char
        // for (; cur != NULL; cur = cur->next)
        //     if (_stricmp(cur->name, volume->name) == 0) goto already_open;
        test ebp, ebp
        je not_open_yet
        lea edi, [esi]ResVolume.name
    compare_next_volume:
        lea ecx, [ebp]ResVolume.name
        push edi
        push ecx
        call _stricmp
        add esp, 8
        test eax, eax
        je already_open
        mov ebp, dword ptr [ebp]ResVolume.next
        test ebp, ebp
        jne compare_next_volume

    not_open_yet:
        // sprintf(res_path, ".\\volumes\\%s.res", fname); DebugTrace("Trying to open from %s", res_path);
        // FUN_0047f850();
        lea edx, [esp + 0x11c]
        lea eax, [esp + 0x18]
        push edx
        push offset ResLocalPathFormat
        push eax
        call sprintf
        lea ecx, [esp + 0x24]
        push ecx
        push offset ResTryPathTraceFormat
        call DebugTrace
        add esp, 0x14
        call FUN_0047f850
        // volume->handle = CreateFileA(res_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
        //                              FILE_FLAG_SEQUENTIAL_SCAN, NULL);   (edi = CreateFileA)
        mov edi, dword ptr [CreateFileA]
        push 0
        push 0x8000000
        push 3
        push 0
        push 1
        lea edx, [esp + 0x2c]
        push 0x80000000
        push edx
        call edi
        cmp eax, -1
        mov dword ptr [esi]ResVolume.handle, eax
        jne check_handle
        // if (volume->handle == INVALID_HANDLE_VALUE) {
        //     sprintf(res_path, "%s%s.res", CdDrivePath, fname); DebugTrace("Trying to open from %s", res_path);
        //     FUN_0047f850(); volume->handle = CreateFileA(res_path, ... same arguments ...);
        // }
        lea eax, [esp + 0x11c]
        lea ecx, [esp + 0x18]
        push eax
        push offset CdDrivePath
        push offset ResCdPathFormat
        push ecx
        call sprintf
        lea edx, [esp + 0x28]
        push edx
        push offset ResTryPathTraceFormat
        call DebugTrace
        add esp, 0x18
        call FUN_0047f850
        push 0
        push 0x8000000
        push 3
        push 0
        push 1
        lea eax, [esp + 0x2c]
        push 0x80000000
        push eax
        call edi
        mov dword ptr [esi]ResVolume.handle, eax
    check_handle:
        // if (volume->handle == INVALID_HANDLE_VALUE) { free(volume); return NULL; }
        cmp dword ptr [esi]ResVolume.handle, -1
        je free_volume
        // DebugTrace("Openned resource %s", res_path);
        lea ecx, [esp + 0x18]
        push ecx
        push offset ResOpenedTraceFormat
        call DebugTrace
        // file_size = GetFileSize(volume->handle, NULL);   (ebp = file_size, ebx = SetFilePointer)
        mov edx, dword ptr [esi]ResVolume.handle
        add esp, 8
        push 0
        push edx
        call dword ptr [GetFileSize]
        mov ebx, dword ptr [SetFilePointer]
        mov ebp, eax
        // SetFilePointer(volume->handle, 0, NULL, FILE_BEGIN);
        mov eax, dword ptr [esi]ResVolume.handle
        push 0
        push 0
        push 0
        push eax
        call ebx
        // ReadFile(volume->handle, &dir_offset, 4, &bytes_read, NULL);
        mov eax, dword ptr [esi]ResVolume.handle
        lea ecx, [esp + 0x14]
        push 0
        push ecx
        lea edx, [esp + 0x18]
        push 4
        push edx
        push eax
        call dword ptr [ReadFile]
        // DebugTrace("FileSize = %x, Directory is at %x", file_size, dir_offset);
        mov ecx, dword ptr [esp + 0x10]
        push ecx
        push ebp
        push offset ResDirInfoTraceFormat
        call DebugTrace
        // dir_data = malloc(file_size - dir_offset);   (edi = dir_data)
        mov eax, dword ptr [esp + 0x1c]
        mov edx, ebp
        sub edx, eax
        push edx
        call malloc
        mov edi, eax
        add esp, 0x10
        test edi, edi
        jne dir_allocated
        // if (dir_data == NULL) { DebugTrace("Failed to allocate space for directory");
        //     CloseHandle(volume->handle); free(volume); return NULL; }   (shares the short-read path's tail)
        push offset ResDirAllocFailedTrace
        call DebugTrace
        add esp, 4
        jmp close_and_free

    already_open:
        // free(volume); DebugTrace("Volume Already open"); return cur;
        push esi
        call free
        push offset ResAlreadyOpenTrace
        call DebugTrace
        add esp, 8
        mov eax, ebp
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x20c
        ret

    dir_allocated:
        // SetFilePointer(volume->handle, dir_offset, NULL, FILE_BEGIN);
        mov eax, dword ptr [esp + 0x10]
        mov ecx, dword ptr [esi]ResVolume.handle
        push 0
        push 0
        push eax
        push ecx
        call ebx
        // ReadFile(volume->handle, dir_data, file_size - dir_offset, &bytes_read, NULL);   (ebx = 0 from here)
        mov ecx, dword ptr [esp + 0x10]
        xor ebx, ebx
        mov eax, ebp
        lea edx, [esp + 0x14]
        push ebx
        sub eax, ecx
        mov ecx, dword ptr [esi]ResVolume.handle
        push edx
        push eax
        push edi
        push ecx
        call dword ptr [ReadFile]
        // if (bytes_read != file_size - dir_offset) {
        //     DebugTrace("Failed to load directory fully (%x of %x loaded)", bytes_read, file_size - dir_offset);
        //     free(dir_data);
        mov ecx, dword ptr [esp + 0x14]
        mov eax, ebp
        sub eax, dword ptr [esp + 0x10]
        cmp ecx, eax
        je dir_read_ok
        push eax
        push ecx
        push offset ResDirShortReadTraceFormat
        call DebugTrace
        push edi
        call free
        add esp, 0x10
    close_and_free:
        //     CloseHandle(volume->handle);
        mov edx, dword ptr [esi]ResVolume.handle
        push edx
        call dword ptr [CloseHandle]
    free_volume:
        //     free(volume); return NULL;
        // }
        push esi
        call free
        add esp, 4
        xor eax, eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x20c
        ret

    dir_read_ok:
        // DebugTrace("Directory read OK"); volume->dir = NULL;
        // FUN_004895a0(dir_data, volume, dir_data, DAT_004d8bb0); free(dir_data);
        push offset ResDirReadOkTrace
        call DebugTrace
        push offset DAT_004d8bb0
        push edi
        push esi
        push edi
        mov dword ptr [esi]ResVolume.dir, ebx
        call FUN_004895a0
        push edi
        call free
        // volume->next = MasterVolList; MasterVolList = volume; volume->refcount = 1; volume->current = NULL;
        // return volume;
        mov eax, dword ptr [MasterVolList]
        add esp, 0x18
        mov dword ptr [esi]ResVolume.next, eax
        mov dword ptr [MasterVolList], esi
        mov dword ptr [esi]ResVolume.refcount, 1
        mov dword ptr [esi]ResVolume.current, ebx
        mov eax, esi
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x20c
        ret
    }
}
// clang-format on
#else
LEGO_EXPORT struct ResVolume *RES_OpenVolume(const char *path) {
    char fname[0x100];
    char res_path[0x104];
    struct ResVolume *volume;
    struct ResVolume *cur;
    unsigned int file_size;
    unsigned int dir_offset;
    DWORD bytes_read;
    void *dir_data;
    int i;
    int name_len;

    volume = (struct ResVolume *)malloc(sizeof(struct ResVolume));
    cur = MasterVolList;

    _splitpath(path, 0, 0, fname, 0);
    DebugTrace(ResOpenTraceFormat, fname);
    FUN_0047f850();

    name_len = strlen(fname);
    for (i = 0; i < 0x14; i++) {
        if (i < name_len) {
            volume->name[i] = (char)toupper(fname[i]);
        } else {
            volume->name[i] = '\0';
        }
    }

    while (cur != 0) {
        if (_stricmp(cur->name, volume->name) == 0) {
            free(volume);
            DebugTrace(ResAlreadyOpenTrace);
            return cur;
        }
        cur = cur->next;
    }

    sprintf(res_path, ResLocalPathFormat, fname);
    DebugTrace(ResTryPathTraceFormat, res_path);
    FUN_0047f850();

    volume->handle = CreateFileA(res_path, 0x80000000, 1, 0, 3, 0x8000000, 0);
    if (INVALID_HANDLE_VALUE == volume->handle) {
        sprintf(res_path, ResCdPathFormat, CdDrivePath, fname);
        DebugTrace(ResTryPathTraceFormat, res_path);
        FUN_0047f850();
        volume->handle = CreateFileA(res_path, 0x80000000, 1, 0, 3, 0x8000000, 0);
    }

    if (INVALID_HANDLE_VALUE == volume->handle) {
        free(volume);
        return 0;
    }

    DebugTrace(ResOpenedTraceFormat, res_path);

    file_size = GetFileSize(volume->handle, 0);
    SetFilePointer(volume->handle, 0, 0, 0);
    ReadFile(volume->handle, &dir_offset, 4, &bytes_read, 0);
    DebugTrace(ResDirInfoTraceFormat, file_size, dir_offset);

    dir_data = malloc(file_size - dir_offset);
    if (dir_data == 0) {
        DebugTrace(ResDirAllocFailedTrace);
        CloseHandle(volume->handle);
        free(volume);
        return 0;
    }

    SetFilePointer(volume->handle, dir_offset, 0, 0);
    ReadFile(volume->handle, dir_data, file_size - dir_offset, &bytes_read, 0);
    if (bytes_read != file_size - dir_offset) {
        DebugTrace(ResDirShortReadTraceFormat, bytes_read, file_size - dir_offset);
        free(dir_data);
        CloseHandle(volume->handle);
        free(volume);
        return 0;
    }

    DebugTrace(ResDirReadOkTrace);
    volume->dir = 0;
    FUN_004895a0(dir_data, volume, dir_data, DAT_004d8bb0);
    free(dir_data);

    volume->next = MasterVolList;
    MasterVolList = volume;
    volume->refcount = 1;
    volume->current = 0;
    return volume;
}
#endif

// FUNCTION: LEGOLAND 0x00489a00
LEGO_EXPORT struct ResFile *RES_OpenFileFromVolume(const char *path, const char *vol_name) {
    char prefix[0x104];
    char path_copy[0x104];
    struct ResVolEntry *entry;
    struct MasterDirNode *dir;
    struct ResFile *file;
    char *last_bs;
    char *scan;
    char *src;
    int prefix_len;

    last_bs = 0;
    prefix[0] = '\0';
    memset(prefix + 1, 0, sizeof(prefix) - 1);
    src = path_copy;
    scan = path_copy;
    strcpy(src, path);

    for (; scan != 0; scan++) {
        if (*scan == '\0') {
            break;
        }
        if (*scan == '\\') {
            last_bs = scan;
        }
    }

    if (last_bs != 0) {
        prefix_len = (int)(last_bs - path_copy) + 1;
        while (*src == '.' && src[1] == '\\') {
            src += 2;
            prefix_len -= 2;
        }
        memcpy(prefix, src, prefix_len);
        prefix[prefix_len] = '\0';
        last_bs++;
    } else {
        last_bs = path_copy;
    }

    dir = FUN_00489550(vol_name, prefix, &entry);
    if (dir == 0) {
        return 0;
    }

    while (entry != 0) {
        if (entry->dir == dir) {
            if (_stricmp(entry->name, last_bs) == 0) {
                file = (struct ResFile *)malloc(sizeof(struct ResFile));
                file->size = entry->size;
                file->base = entry->base;
                file->volume = entry->volume;
                file->volume->refcount++;
                file->position = 0;
                return file;
            }
        }
        entry = entry->next;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00489b60
LEGO_EXPORT struct ResFile *RES_OpenFile(const char *path) {
    char base_name[0x104];
    char orig_str[0x104];
    struct MasterDirNode *dir;
    struct ResVolEntry *entry;
    struct ResFile *file;
    char *scan;
    char *last_slash;
    char *prefix;
    char *file_name;
    int prefix_len;

    base_name[0] = '\0';
    memset(base_name + 1, 0, sizeof(base_name) - 1);
    last_slash = 0;
    prefix = orig_str;
    scan = orig_str;

    if (WaitForLegolandCd(0) == 0) {
        exit(1);
    }

    strcpy(orig_str, path);

    for (; scan != 0; scan++) {
        if (*scan == '\0') {
            break;
        }
        if (*scan == ':') {
            *scan = '\0';
            return RES_OpenFileFromVolume(scan + 1, orig_str);
        }
        if (*scan == '\\') {
            last_slash = scan;
        }
    }

    if (last_slash != 0) {
        prefix_len = (int)(last_slash - orig_str) + 1;
        while (*prefix == '.' && prefix[1] == '\\') {
            prefix += 2;
            prefix_len -= 2;
        }
        memcpy(base_name, prefix, prefix_len);
        base_name[prefix_len] = '\0';
        file_name = last_slash + 1;
    } else {
        file_name = orig_str;
    }

    dir = GetMasterDirPtr(base_name);
    if (dir == 0) {
        return 0;
    }

    for (entry = (struct ResVolEntry *)dir->entries; entry != 0; entry = entry->next_dir) {
        if (_stricmp(entry->name, file_name) == 0) {
            file = (struct ResFile *)malloc(sizeof(struct ResFile));
            file->size = entry->size;
            file->base = entry->base;
            file->volume = entry->volume;
            file->volume->refcount++;
            file->position = 0;
            return file;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00489ce0
LEGO_EXPORT unsigned int RES_GetFileSize(struct ResFile *file) {
    if (file == 0) {
        return 0xffffffff;
    }
    return file->size;
}

// FUNCTION: LEGOLAND 0x00489cf0
LEGO_EXPORT int RES_ReadFile(struct ResFile *file, void *buffer, int count) {
    int remaining;
    DWORD bytes_read;
    struct ResVolume *volume;

    if (file == 0) {
        return -1;
    }

    volume = file->volume;
    if (file != volume->current) {
        volume->current = file;
        SetFilePointer(volume->handle, file->position + file->base, 0, 0);
    }

    remaining = file->size - file->position;
    if (count > remaining) {
        count = remaining;
    }
    if (count == 0) {
        return 0;
    }

    ReadFile(volume->handle, buffer, count, &bytes_read, 0);
    file->position += bytes_read;
    return bytes_read;
}

// FUNCTION: LEGOLAND 0x00489d70
LEGO_EXPORT int RES_SetFilePointer(struct ResFile *file, int offset) {
    struct ResVolume *volume;

    if (file != 0) {
        if (offset >= 0) {
            if (offset < file->size) {
                file->position = offset;
                volume = file->volume;
                volume->current = file;
                SetFilePointer(volume->handle, file->base + file->position, 0, 0);
                return offset;
            }
        }
    }
    return -1;
}

// FUNCTION: LEGOLAND 0x00489db0
LEGO_EXPORT int RES_GetFilePointer(struct ResFile *file) {
    if (file == 0) {
        return -1;
    }
    return file->position;
}

// FUNCTION: LEGOLAND 0x00489dc0
LEGO_EXPORT unsigned int RES_CloseVolume(struct ResVolume *volume) {
    if (volume == 0) {
        return 0xffffffff;
    }
    volume->refcount--;
    return volume->refcount;
}

// FUNCTION: LEGOLAND 0x00489de0
LEGO_EXPORT int RES_CloseFile(struct ResFile *file) {
    struct ResVolume *volume;

    if (file == 0) {
        return -1;
    }

    volume = file->volume;
    if (volume->current == file) {
        volume->current = 0;
    }

    RES_CloseVolume(volume);
    free(file);
    return 0;
}

// FUNCTION: LEGOLAND 0x00489e10
LEGO_EXPORT struct ResFile *RES_OpenFileFromVolumePtr(void *param_1, struct MasterVolNode *entry) {
    if (entry == 0) {
        return 0;
    }
    return RES_OpenFileFromVolume(param_1, entry->name);
}

// FUNCTION: LEGOLAND 0x00489e30
LEGO_EXPORT int RES_FileExists(const char *path) {
    struct ResFile *file;

    file = RES_OpenFile(path);
    if (file == 0) {
        return 0;
    }
    RES_CloseFile(file);
    return 1;
}
