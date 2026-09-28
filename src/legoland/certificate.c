#include "legoland.h"

#include <string.h>
#include <time.h>
#include "certificate.h"
#include "globals.h"

// FUNCTION: LEGOLAND 0x00451740
int FUN_00451740(char *param_1, char *param_2, char *param_3) { STUB(); }

// FUNCTION: LEGOLAND 0x00451e20
int FUN_00451e20(void) {
    time_t now;
    char *str;

    time(&now);
    str = asctime(localtime(&now));
    str[strlen(str) - 1] = '\0';
    // STRING: LEGOLAND 0x004b86fc
    return FUN_00451740("EGC.bmp", (char *)&DAT_0080ffa0, str) != 0;
}
