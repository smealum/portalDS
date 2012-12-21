#include "common/general.h"

u8 state_id;
state_struct *currentState, *nextState;

state_struct* getCurrentState(void)
{
	return currentState;
}

void changeState(state_struct* s)
{
	currentState->used=0;
	nextState=s;
}

void initHardware(void)
{
	defaultExceptionHandler();
	glInit();
}

void createState(state_struct* s, function i, function f, function k, function vbl)
{
	s->init=(function)i;
	s->frame=(function)f;
	s->kill=(function)k;
	s->vbl=(function)vbl;
	
	s->id=state_id;
	s->mc_id=0;
	state_id++;
}

void setState(state_struct* s)
{
	currentState=s;
	currentState->used=1;
	//irqSet(IRQ_VBLANK, CurrentState->VBlank);
}

void applyState()
{
	currentState=nextState;
	currentState->used=1;
	currentState->mc_id=0;
	initMalloc();
	irqSet(IRQ_VBLANK, currentState->vbl);
}
