#pragma once

struct CallbackTable;
struct ClassNode;
struct FlumeEntry;
struct FlumeDims;
struct ParticleEmitter;
struct Context;
struct LinkList;

struct FlumeDims FUN_004112c0(void);

void FUN_0040da10(struct Context *a, struct LinkList *list);
void FUN_004119a0(struct ParticleEmitter *param_1, unsigned int param_2);

struct FlumeEntry *FUN_0040d210(int x, int y);

struct Point FUN_0040cfd0(struct FlumeEntry *entry);

int FUN_004119c0(Element *obj, int filter);

void FUN_00410d60(struct ClassNode *flume, struct CallbackTable *vtbl);
