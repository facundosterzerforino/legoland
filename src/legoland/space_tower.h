#pragma once

#include "objclass.h"

struct SpaceTowerRideNode;
struct SpaceTowerCar;

unsigned int FUN_0043acb0(struct SpaceTowerRideNode *param_1, struct SpaceTowerCar *param_2);
void SpaceTowerUpdate(void);

void SpaceTowerRide(struct ClassNode *head, struct CallbackTable *iface);
