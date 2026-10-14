#pragma once

#include "objclass.h"

struct SpaceTowerRideNode;
struct SpaceTowerCar;

struct AnimEntry {
    /* 0x00 */ int count;
    /* 0x04 */ struct AnimStep *steps;
};

struct AnimLayout {
    /* 0x00 */ int count;
    /* 0x04 */ struct AnimEntry **entries;
};

unsigned int FUN_0043acb0(struct SpaceTowerRideNode *param_1, struct SpaceTowerCar *param_2);
void SpaceTowerUpdate(void);

struct Ride;
struct ClassNode;
void SpaceTowerRide(struct ClassNode *head, struct Ride *iface);
