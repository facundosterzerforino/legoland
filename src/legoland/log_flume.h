#pragma once

struct CallbackTable;
struct ClassNode;
struct FlumeEntry;
struct FlumeDims;

struct FlumeDims FUN_004112c0(void);

struct Point FUN_0040cfd0(struct FlumeEntry *entry);

int FUN_004119c0(Element *obj, int filter);

void FUN_00410d60(struct ClassNode *flume, struct CallbackTable *vtbl);
