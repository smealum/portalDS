#include "game/game_main.h"

void initActivator(activator_struct* a)
{
	if(!a)return;
	
	a->numSlots=0;
}

void useSlot(activatorSlot_struct* as)
{
	if(!as)return;
	
	switch(as->type)
	{
		case DISPENSER_TARGET:
			{
				cubeDispenser_struct* cd=(cubeDispenser_struct*)as->target;
				cd->active=true;
			}
			break;
		case PLATFORM_TARGET:
			{
				platform_struct* pf=(platform_struct*)as->target;
				pf->active=true;
			}
			break;
		case DOOR_TARGET:
			{
				door_struct* d=(door_struct*)as->target;
				d->active=true;
			}
			break;
		case WALLDOOR_TARGET:
			{
				wallDoor_struct* wd=(door_struct*)as->target;
				wd->override=true;
			}
			break;
	}
}

void unuseSlot(activatorSlot_struct* as)
{
	if(!as)return;
	
	switch(as->type)
	{
		case DISPENSER_TARGET:
			{
				cubeDispenser_struct* cd=(cubeDispenser_struct*)as->target;
				cd->active=false;
			}
			break;
		case PLATFORM_TARGET:
			{
				platform_struct* pf=(platform_struct*)as->target;
				pf->active=false;
			}
			break;
		case DOOR_TARGET:
			{
				door_struct* d=(door_struct*)as->target;
				d->active=false;
			}
			break;
		case WALLDOOR_TARGET:
			{
				wallDoor_struct* wd=(door_struct*)as->target;
				wd->override=false;
			}
			break;
	}
}

void useActivator(activator_struct* a)
{
	if(!a)return;
	int i;
	for(i=0;i<a->numSlots;i++)
	{
		useSlot(&a->slot[i]);
	}
}

void unuseActivator(activator_struct* a)
{
	if(!a)return;
	int i;
	for(i=0;i<a->numSlots;i++)
	{
		unuseSlot(&a->slot[i]);
	}
}

void addActivatorTarget(activator_struct* a, void* target, activatorTarget_type type)
{
	if(!a)return;
	if(a->numSlots>=NUMACTIVATORSLOTS)return;
	
	activatorSlot_struct* as=&a->slot[a->numSlots];
	
	as->target=target;
	as->type=type;
	
	a->numSlots++;
	unuseActivator(a);
}
