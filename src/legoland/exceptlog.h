#pragma once

#include <windows.h>

int stackdump(void *exc_info, const char *filename);
void FUN_00454380(HANDLE file, DWORD base);
void FUN_00454500(char *buffer, FILETIME ft);
void FUN_00454290(HANDLE file, const char *format, ...);
void FUN_004542e0(HANDLE file);
void FUN_004545a0(HANDLE file);
const char *FUN_00454700(unsigned int code);
char *FUN_004548f0(char *path);
