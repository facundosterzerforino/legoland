#include <windows.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "imports.h"
#include "legoland.h"
#include "sound_music.h"

#include "stream.h"

// FUNCTION: LEGOLAND 0x00497f60
unsigned int FUN_00497f60(void) {
    int head;
    int tail;

    head = DAT_0079a7dc;
    tail = DAT_0079a7d8;
    if (head >= tail) {
        if (tail == 0) {
            return 0xffff - head;
        }
        return 0x10000 - head;
    }
    return tail - head - 1;
}

// FUNCTION: LEGOLAND 0x00497f90
int FUN_00497f90(void) {
    int head;
    int tail;

    head = DAT_0079a7dc;
    tail = DAT_0079a7d8;
    if (head < tail) {
        head = 0x10000;
    }
    return head - tail;
}

// FUNCTION: LEGOLAND 0x00497fb0
int FUN_00497fb0(void) {
    int used;
    int limit;

    limit = DAT_0079a7e4[0];
    used = (DAT_0079a7dc - DAT_0079a7d8) & 0xffff;
    if (limit == 0) {
        memcpy(DAT_0079a7e4, &DAT_0079a7e4[1], 0x13 * sizeof(unsigned int));
        DAT_0079a7e4[19] = 0;
        if (DAT_0079a7e0 != 0) {
            DAT_0079a7e0--;
        }
    }
    if (used <= limit) {
        return used;
    }
    return limit;
}

// FUNCTION: LEGOLAND 0x00498000
void FUN_00498000(void) {
    int n;
    int r;
    unsigned int idx;
    unsigned char mask;

    if (DAT_007cacac == 0) {
        return;
    }
    n = FUN_00497f60();
    if (n == 0) {
        return;
    }
    mask = 4;
    while (n != 0) {
        while (n != 0) {
            if (n > (int)DAT_007cacac) {
                r = _read(DAT_007caca8, &DAT_0079ac20[DAT_0079a7dc], DAT_007cacac);
            } else {
                r = _read(DAT_007caca8, &DAT_0079ac20[DAT_0079a7dc], n);
            }
            idx = DAT_0079a7e0;
            if (r != -1) {
                DAT_007cacac -= r;
                DAT_0079a7e4[idx] += r;
                DAT_0079a7dc = (DAT_0079a7dc + r) & 0xffff;
            }
            if (r < n) {
                if (!(DAT_0079a83c & mask)) {
                    DAT_0079a7e0 = idx + 1;
                    return;
                }
                if (DAT_007cacac == 0) {
                    FUN_00498100();
                    DAT_0079a7e0++;
                }
            }
            n -= r;
        }
        n = FUN_00497f60();
    }
}

// FUNCTION: LEGOLAND 0x00498100
void FUN_00498100(void) {
    FUN_00498120();
    DAT_0079a83c |= 8;
}

// FUNCTION: LEGOLAND 0x00498120
void FUN_00498120(void) {
    _lseek(DAT_007caca8, DAT_007cacb4, 0);
    DAT_007cacac = DAT_0079ac04;
}

// FUNCTION: LEGOLAND 0x00498150
int FUN_00498150(unsigned char *dst, int count) {
    int avail;
    int chunk;
    int n;

    avail = FUN_00497fb0();
    n = count;
    if (n > avail) {
        FUN_00498000();
        avail = FUN_00497fb0();
        if (n > avail) {
            n = avail;
        }
    }
    count = n;
    if (n != 0) {
        do {
            chunk = FUN_00497f90();
            if (chunk > n) {
                chunk = n;
            }
            memcpy(dst, &DAT_0079ac20[DAT_0079a7d8], chunk);
            n -= chunk;
            dst += chunk;
            DAT_0079a7d8 = (DAT_0079a7d8 + chunk) & 0xffff;
            DAT_0079a7e4[0] -= chunk;
        } while (n != 0);
    }
    FUN_00498000();
    return count;
}

// FUNCTION: LEGOLAND 0x004981e0
unsigned int FUN_004981e0(void) {
    int head;
    int tail;

    head = DAT_0079a838;
    tail = DAT_0079a834;
    if (head >= tail) {
        if (tail == 0) {
            return 0x1ffff - head;
        }
        return 0x20000 - head;
    }
    return tail - head - 1;
}

// FUNCTION: LEGOLAND 0x00498210
int FUN_00498210(void) {
    int head;
    int tail;

    head = DAT_0079a838;
    tail = DAT_0079a834;
    if (head < tail) {
        head = 0x20000;
    }
    return head - tail;
}

// FUNCTION: LEGOLAND 0x00498230
unsigned int FUN_00498230(void) {
    return (DAT_0079a838 - DAT_0079a834) & 0x1ffff;
}

// FUNCTION: LEGOLAND 0x00498250
void FUN_00498250(void) {
    int n;
    int chunk;
    int len;

    FUN_00498000();
    n = FUN_004981e0();
    while (n != 0) {
        while (n != 0) {
            chunk = DAT_007caca4 - DAT_007aac24;
            if (chunk == 0) {
                if (n != 0) {
                    len = DAT_0079a7e4[0];
                    if (len >= DAT_007caca0) {
                        len = DAT_007caca0;
                    }
                    DAT_007aac40.cbSrcLength = len;
                    FUN_00498150(DAT_0079ac0c, len);
                    acmStreamConvert(DAT_007cacb8, &DAT_007aac40, 0x10);
                    DAT_007aac24 = 0;
                    DAT_007caca4 = DAT_007aac40.cbDstLengthUsed;
                    if (DAT_007aac40.cbDstLengthUsed < n) {
                        memcpy(&DAT_007aaca0[DAT_0079a838], DAT_0079ac08, DAT_007aac40.cbDstLengthUsed);
                        DAT_007aac24 = DAT_007aac40.cbDstLengthUsed;
                        DAT_0079a838 += DAT_007aac40.cbDstLengthUsed;
                        return;
                    }
                    memcpy(&DAT_007aaca0[DAT_0079a838], DAT_0079ac08, n);
                    DAT_007aac24 = n;
                    DAT_0079a838 = (DAT_0079a838 + n) & 0x1ffff;
                }
                break;
            }
            if (n < chunk) {
                chunk = n;
            }
            memcpy(&DAT_007aaca0[DAT_0079a838], DAT_0079ac08 + DAT_007aac24, chunk);
            DAT_0079a838 = (DAT_0079a838 + chunk) & 0x1ffff;
            n -= chunk;
            DAT_007aac24 += chunk;
        }
        n = FUN_004981e0();
    }
    FUN_00498000();
}

// FUNCTION: LEGOLAND 0x004983a0
int FUN_004983a0(unsigned char *dst, int count) {
    int avail;
    int chunk;
    int n;

    avail = FUN_00498230();
    n = count;
    if (n > avail) {
        FUN_00498250();
        avail = FUN_00498230();
        if (n > avail) {
            n = avail;
        }
    }
    count = n;
    if (n != 0) {
        do {
            chunk = FUN_00498210();
            if (chunk > n) {
                chunk = n;
            }
            memcpy(dst, &DAT_007aaca0[DAT_0079a834], chunk);
            n -= chunk;
            dst += chunk;
            DAT_0079a834 = (DAT_0079a834 + chunk) & 0x1ffff;
        } while (n != 0);
    }
    FUN_00498250();
    return count;
}

// FUNCTION: LEGOLAND 0x00498420
int FUN_00498420(void) {
    unsigned int size;
    unsigned int tag;
    unsigned int *p;

    _lseek(DAT_007caca8, 0, 0);
    if (_read(DAT_007caca8, &tag, 4) != 4) {
        return 0;
    }
    if (tag != 0x46464952) {
        return 0;
    }
    if (_read(DAT_007caca8, &size, 4) != 4) {
        return 0;
    }
    if (_read(DAT_007caca8, &tag, 4) != 4) {
        return 0;
    }
    if (tag != 0x45564157) {
        return 0;
    }
    if (_read(DAT_007caca8, &tag, 4) != 4) {
        return 0;
    }
    if (_read(DAT_007caca8, &size, 4) != 4) {
        return 0;
    }
    if (size < 0x12) {
        DAT_007cacb0 = malloc(0x12);
    } else {
        DAT_007cacb0 = malloc(size);
    }
    if (_read(DAT_007caca8, DAT_007cacb0, size) != size) {
        return 0;
    }
    if (size <= 0x12) {
        *(short *)((char *)DAT_007cacb0 + 0x10) = 0;
    }
    while (_read(DAT_007caca8, &tag, 4) == 4) {
        if (tag == 0x61746164) {
            break;
        }
        if (_read(DAT_007caca8, &size, 4) != 4) {
            return 0;
        }
        p = (unsigned int *)malloc(size);
        if (_read(DAT_007caca8, p, size) != size) {
            free(p);
            return 0;
        }
        free(p);
    }
    if (tag != 0x61746164) {
        return 0;
    }
    if (_read(DAT_007caca8, &DAT_0079ac04, 4) != 4) {
        return 0;
    }
    DAT_007cacb4 = _tell(DAT_007caca8);
    return 1;
}

// FUNCTION: LEGOLAND 0x00498630
void FUN_00498630(const char *param_1) { STUB(); }

// FUNCTION: LEGOLAND 0x00498870
void FUN_00498870(void) {
    DAT_0079a7d8 = 0;
    DAT_0079a7dc = 0;
    DAT_0079a7e0 = 0;
    memset(DAT_0079a7e4, 0, sizeof(DAT_0079a7e4));
    DAT_0079a834 = 0;
    DAT_0079a838 = 0;
    DAT_0079a840 = 0;
    DAT_0079a844 = 0;
    DAT_007caca4 = 0;
    DAT_007aac24 = 0;
}
// FUNCTION: LEGOLAND 0x004988c0
int FUN_004988c0(void) {
    if (DAT_0079a84c == 0 || DAT_0079a84c == 1) {
        return 0;
    }
    ((struct KLIBAUDIO_Stop *)((struct KLIBAUDIO_Object *)DAT_0079a848)->vtable)->func_48(DAT_0079a848);
    FUN_00498870();
    FUN_00498120();
    DAT_0079a84c = 1;
    return 1;
}

// FUNCTION: LEGOLAND 0x00498900
void FUN_00498900(unsigned int param_1) {
    DAT_0079a7d0 = param_1;
    if (DAT_0079a848 != NULL) {
        ((struct KLIBAUDIO_Vtbl *)((struct KLIBAUDIO_Object *)DAT_0079a848)->vtable)->func_3c(DAT_0079a848, param_1);
    }
}

// FUNCTION: LEGOLAND 0x00498920
int FUN_00498920(void) {
    if (DAT_0079a84c == 0) {
        return 0;
    }
    FUN_004988c0();
    acmStreamUnprepareHeader(DAT_007cacb8, &DAT_007aac40, 0);
    acmStreamClose(DAT_007cacb8, 0);
    _close(DAT_007caca8);
    KLIBAUDIO_DestroyAVISoundBuffer((struct AVISoundBuffer *)DAT_0079a848);
    DAT_0079a848 = NULL;
    free(DAT_0079ac0c);
    free(DAT_0079ac08);
    free(DAT_007cacb0);
    DAT_0079a84c = 0;
    return 1;
}

// FUNCTION: LEGOLAND 0x004989b0
int FUN_004989b0(void) {
    unsigned char buf[0x1000];
    void *ptr1;
    unsigned int size1;
    int n;

    if (DAT_0079a84c == 2 || DAT_0079a84c == 0) {
        return 0;
    }
    ((struct KLIBAUDIO_Buf *)((struct KLIBAUDIO_Object *)DAT_0079a848)->vtable)->Stop(DAT_0079a848);
    ((struct KLIBAUDIO_Buf *)((struct KLIBAUDIO_Object *)DAT_0079a848)->vtable)->SetPos(DAT_0079a848, 0);
    DAT_0079a840 = 0;
    if (((struct KLIBAUDIO_Buf *)((struct KLIBAUDIO_Object *)DAT_0079a848)->vtable)->Lock(DAT_0079a848, 0, 0xa000, &ptr1, &size1, NULL, NULL, 0) == 0) {
        if (DAT_0079a840 < 10) {
            do {
                n = FUN_004983a0(buf, 0x1000);
                memcpy((unsigned char *)ptr1 + DAT_0079a840 * 0x1000, buf, n);
                if (n < 0x1000) {
                    memset((unsigned char *)ptr1 + DAT_0079a840 * 0x1000 + n, 0, 0x1000 - n);
                }
                if (n != 0) {
                    DAT_0079a844++;
                }
                DAT_0079a840++;
            } while (DAT_0079a840 < 10);
        }
        DAT_0079a840 = 0;
        ((struct KLIBAUDIO_Buf *)((struct KLIBAUDIO_Object *)DAT_0079a848)->vtable)->Unlock(DAT_0079a848, ptr1, size1, NULL, 0);
    }
    DAT_0079a84c = 2;
    return 1;
}

// FUNCTION: LEGOLAND 0x00498b00
int FUN_00498b00(void) {
    if (DAT_0079a84c == 0 || DAT_0079a84c == 3) {
        return 0;
    }
    FUN_004989b0();
    ((struct KLIBAUDIO_Vtbl *)((struct KLIBAUDIO_Object *)DAT_0079a848)->vtable)->func_30(DAT_0079a848, 0, 0, 1);
    DAT_0079a84c = 3;
    return 1;
}

// FUNCTION: LEGOLAND 0x00498b40
int FUN_00498b40(void) {
    unsigned char buf[0x1000];
    void *ptr1;
    int play;
    unsigned int size1;
    unsigned int write;
    int n;

    if (DAT_0079a84c == 0 || DAT_0079a84c == 1) {
        return 0;
    }
    if (DAT_0079a84c == 2) {
        FUN_00498250();
        return 0;
    }
    if (!(((struct KLIBAUDIO_Buf *)((struct KLIBAUDIO_Object *)DAT_0079a848)->vtable)->GetCurrentPosition(DAT_0079a848, &play, &write) != 0 || play < DAT_0079a840 * 0x1000 || play >= (DAT_0079a840 + 1) * 0x1000)) {
        return 1;
    }
    do {
        if (((struct KLIBAUDIO_Buf *)((struct KLIBAUDIO_Object *)DAT_0079a848)->vtable)->Lock(DAT_0079a848, DAT_0079a840 * 0x1000, 0x1000, &ptr1, &size1, NULL, NULL, 0) == 0) {
            n = FUN_004983a0(buf, size1);
            if (n == 0x1000) {
                memcpy(ptr1, buf, 0x1000);
                DAT_0079a844++;
            } else {
                memcpy(ptr1, buf, n);
                memset((unsigned char *)ptr1 + n, 0, 0x1000 - n);
                if (n != 0) {
                    DAT_0079a844++;
                }
            }
            ((struct KLIBAUDIO_Buf *)((struct KLIBAUDIO_Object *)DAT_0079a848)->vtable)->Unlock(DAT_0079a848, ptr1, size1, NULL, 0);
            DAT_0079a840++;
            if (DAT_0079a840 >= 10) {
                DAT_0079a840 = 0;
            }
        }
        DAT_0079a844--;
        if (DAT_0079a844 == 0) {
            FUN_004988c0();
            return 0;
        }
    } while (((struct KLIBAUDIO_Buf *)((struct KLIBAUDIO_Object *)DAT_0079a848)->vtable)->GetCurrentPosition(DAT_0079a848, &play, &write) != 0 || play < DAT_0079a840 * 0x1000 || play >= (DAT_0079a840 + 1) * 0x1000);
    return 1;
}

// FUNCTION: LEGOLAND 0x00498cf0
int FUN_00498cf0(void) {
    return DAT_0079a84c == 3;
}
