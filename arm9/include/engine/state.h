#ifndef __STATE9__
#define __STATE9__


typedef void(*function)();

typedef struct{
	function init,frame,kill,vbl;
	u16 mc_id;
	u8 id;
	bool used;
}state_struct;

extern state_struct* currentState;

void applyState(void);
void initHardware(void);
state_struct* getCurrentState(void);
void setState(state_struct* s);
void changeState(state_struct* s);
void createState(state_struct* s, function i, function f, function k, function vbl);

#endif
