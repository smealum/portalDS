#ifndef ACTIVATOR_H
#define ACTIVATOR_H

#define NUMACTIVATORSLOTS (4)

typedef enum
{
	DISPENSER_TARGET,
	PLATFORM_TARGET,
	DOOR_TARGET,
	WALLDOOR_TARGET,
	NOT_TARGET
}activatorTarget_type;

typedef struct
{
	void* target;
	activatorTarget_type type;
}activatorSlot_struct;

typedef struct
{
	activatorSlot_struct slot[NUMACTIVATORSLOTS];
	u8 numSlots;
}activator_struct;

void initActivator(activator_struct* a);
void useActivator(activator_struct* a);
void unuseActivator(activator_struct* a);
void addActivatorTarget(activator_struct* a, void* target, activatorTarget_type type);

#endif