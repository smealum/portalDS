#include "game/game_main.h"

typedef enum
{
	CONTROL_FORWARD,
	CONTROL_BACK,
	CONTROL_STRAFEL,
	CONTROL_STRAFER,
	CONTROL_LOOKL,
	CONTROL_LOOKR,
	CONTROL_LOOKUP,
	CONTROL_LOOKDOWN,
	CONTROL_JUMP,
	CONTROL_SHOOTALL,
	CONTROL_SHOOTY,
	CONTROL_SHOOTB,
	CONTROL_USE,
	CONTROL_PAUSE,
	CONTROL_SCREENSHOT,
	CONTROL_NUMBER
}controlAction_type;

char* controlStrings[]={"CONTROL_FORWARD",
					"CONTROL_BACK",
					"CONTROL_STRAFEL",
					"CONTROL_STRAFER",
					"CONTROL_LOOKL",
					"CONTROL_LOOKR",
					"CONTROL_LOOKUP",
					"CONTROL_LOOKDOWN",
					"CONTROL_JUMP",
					"CONTROL_SHOOTALL",
					"CONTROL_SHOOTY",
					"CONTROL_SHOOTB",
					"CONTROL_USE",
					"CONTROL_PAUSE",
					"CONTROL_SCREENSHOT"};

typedef enum
{
	INPUT_A,
	INPUT_B,
	INPUT_X,
	INPUT_Y,
	INPUT_UP,
	INPUT_DOWN,
	INPUT_LEFT,
	INPUT_RIGHT,
	INPUT_R,
	INPUT_L,
	INPUT_SELECT,
	INPUT_START,
	INPUT_DOUBLETAP,
	INPUT_NUMBER
}controlInput_type;

char* inputStrings[]={"INPUT_A",
					"INPUT_B",
					"INPUT_X",
					"INPUT_Y",
					"INPUT_UP",
					"INPUT_DOWN",
					"INPUT_LEFT",
					"INPUT_RIGHT",
					"INPUT_R",
					"INPUT_L",
					"INPUT_SELECT",
					"INPUT_START",
					"INPUT_DOUBLETAP"};

KEYPAD_BITS inputMasks[]={KEY_A,
					KEY_B,
					KEY_X,
					KEY_Y,
					KEY_UP,
					KEY_DOWN,
					KEY_LEFT,
					KEY_RIGHT,
					KEY_R,
					KEY_L,
					KEY_SELECT,
					KEY_START,
					0};

char* inputDefaults[]={"CONTROL_STRAFER",
					"CONTROL_BACK",
					"CONTROL_FORWARD",
					"CONTROL_STRAFEL",
					"CONTROL_FORWARD",
					"CONTROL_BACK",
					"CONTROL_STRAFEL",
					"CONTROL_STRAFER",
					"CONTROL_SHOOTALL",
					"CONTROL_SHOOTALL",
					"CONTROL_SCREENSHOT",
					"CONTROL_PAUSE",
					"CONTROL_JUMP"};

controlAction_type currentConfiguration[INPUT_NUMBER];

typedef void(*controlActionFunction)(player_struct*,bool,bool);
controlActionFunction controlFunctions[];

extern SFX_struct *gunSFX1, *gunSFX2;

controlAction_type actionByString(char* str)
{
	if(!str)return CONTROL_NUMBER;

	int i; for(i=0;i<CONTROL_NUMBER;i++)
	{
		if(!strcmp(controlStrings[i], str))return i;
	}
	return CONTROL_NUMBER;
}

void loadControlConfiguration(char* filename)
{
	dictionary* dic=iniparser_load(filename);
	int i; for(i=0;i<INPUT_NUMBER;i++)
	{
		char str[255];
		sprintf(str,"controls:%s",inputStrings[i]);
		currentConfiguration[i]=actionByString(dictionary_get(dic, str, inputDefaults[i]));
		if(currentConfiguration[i]==CONTROL_NUMBER)currentConfiguration[i]=actionByString(inputDefaults[i]);
	}
	iniparser_freedict(dic);
}

u8 touchCnt;

void updateControl(controlInput_type ci)
{
	switch(ci)
	{
		case INPUT_DOUBLETAP:
			if(keysDown() & KEY_TOUCH)
			{
				if(!touchCnt)touchCnt=16;
				else if(controlFunctions[currentConfiguration[ci]]){controlFunctions[currentConfiguration[ci]](getPlayer(),true,true);touchCnt=0;}
			}
			if(touchCnt)touchCnt--;
			break;
		default:
			if((keysHeld() & inputMasks[ci]) && controlFunctions[currentConfiguration[ci]]){controlFunctions[currentConfiguration[ci]](getPlayer(),(keysDown()&inputMasks[ci]),(keysHeld()&inputMasks[ci]));}
			break;
	}
}

void updateControls(void)
{
	int i; for(i=0;i<INPUT_NUMBER;i++)updateControl(i);
}

void controlForward(player_struct* p, bool down, bool held)
{
	if(!p)return;

	if(p->object->contact)
	{
		moveCamera(NULL, vect(0,0,-(PLAYERGROUNDSPEED)));
		p->walkCnt+=2500;
		changeAnimation(&p->playerModelInstance,3,false);
	}else moveCamera(NULL, vect(0,0,-PLAYERAIRSPEED));
}

void controlBack(player_struct* p, bool down, bool held)
{
	if(!p)return;

	if(p->object->contact)
	{
		moveCamera(NULL, vect(0,0,PLAYERGROUNDSPEED));
		p->walkCnt+=2500;
		changeAnimation(&p->playerModelInstance,3,false);
	}else moveCamera(NULL, vect(0,0,PLAYERAIRSPEED));
}

void controlStrafeLeft(player_struct* p, bool down, bool held)
{
	if(!p)return;

	if(p->object->contact)
	{
		moveCamera(NULL, vect(-(PLAYERGROUNDSPEED),0,0));
		p->walkCnt+=2500;
		changeAnimation(&p->playerModelInstance,4,false);
	}else moveCamera(NULL, vect(-PLAYERAIRSPEED,0,0));
}

void controlStrafeRight(player_struct* p, bool down, bool held)
{
	if(!p)return;

	if(p->object->contact)
	{
		moveCamera(NULL, vect(PLAYERGROUNDSPEED,0,0));
		p->walkCnt+=2500;
		changeAnimation(&p->playerModelInstance,4,false);
	}else moveCamera(NULL, vect(PLAYERAIRSPEED,0,0));
}

void controlPause(player_struct* p, bool down, bool held)
{
	if(!p || !down)return;

	doPause(NULL);
}

extern bool currentPortalColor;

void controlShootAll(player_struct* p, bool down, bool held)
{
	if(!p || !down)return;

	if(!p->modelInstance.oneshot)
	{
		playSFX(currentPortalColor?gunSFX1:gunSFX2);
		shootPlayerGun(p,currentPortalColor,255);
		changeAnimation(&p->modelInstance,1,true);
	}
}

void controlShootYellow(player_struct* p, bool down, bool held)
{
	if(!p || !down)return;

	if(!p->modelInstance.oneshot)
	{
		playSFX(gunSFX1);
		shootPlayerGun(p,true,255);
		changeAnimation(&p->modelInstance,1,true);
	}
}

void controlShootBlue(player_struct* p, bool down, bool held)
{
	if(!p || !down)return;

	if(!p->modelInstance.oneshot)
	{
		playSFX(gunSFX2);
		shootPlayerGun(p,false,255);
		changeAnimation(&p->modelInstance,1,true);
	}
}

void controlUse(player_struct* p, bool down, bool held)
{
	if(!p || !down)return;

	if(!p->modelInstance.oneshot)
	{
		playSFX(gunSFX2);
		shootPlayerGun(p,false,1|2);
		changeAnimation(&p->modelInstance,1,true);
	}
}

void controlJump(player_struct* p, bool down, bool held)
{
	if(!p || !down)return;

	p->object->speed=addVect(p->object->speed,vectMult(normGravityVector,-(inttof32(1)>>5)));
}

void controlLookRight(player_struct* p, bool down, bool held)
{
	if(!p)return;

	rotateCamera(NULL, vect(0,16,0));
}

void controlLookLeft(player_struct* p, bool down, bool held)
{
	if(!p)return;

	rotateCamera(NULL, vect(0,-16,0));
}

void controlLookUp(player_struct* p, bool down, bool held)
{
	if(!p)return;

	rotateCamera(NULL, vect(16,0,0));
}

void controlLookDown(player_struct* p, bool down, bool held)
{
	if(!p)return;

	rotateCamera(NULL, vect(-16,0,0));
}

controlActionFunction controlFunctions[]={(controlActionFunction)controlForward,
										(controlActionFunction)controlBack,
										(controlActionFunction)controlStrafeLeft,
										(controlActionFunction)controlStrafeRight,
										(controlActionFunction)controlLookLeft,
										(controlActionFunction)controlLookRight,
										(controlActionFunction)controlLookUp,
										(controlActionFunction)controlLookDown,
										(controlActionFunction)controlJump,
										(controlActionFunction)controlShootAll,
										(controlActionFunction)controlShootYellow,
										(controlActionFunction)controlShootBlue,
										(controlActionFunction)controlUse,
										(controlActionFunction)controlPause,
										NULL};
