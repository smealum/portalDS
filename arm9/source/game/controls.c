#include "game/game_main.h"


u8 touchCnt;

//extern bool currentPortalColor;
// extern SFX_struct *gunSFX1, *gunSFX2;

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

static char* controlStrings[]={"CONTROL_FORWARD",
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

static char* inputStrings[]={"INPUT_A",
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

static KEYPAD_BITS inputMasks[]={KEY_A,
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

static char* inputDefaults[]={"CONTROL_STRAFER",
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


#define CONTROLS_NB_CONTROL_FN  14

/** Guard to prevent the same control function to
 * be called twice in case of allocation to two
 * different button that may be pressed at the same time. */
static bool controlWasCalled[CONTROLS_NB_CONTROL_FN];

static controlAction_type currentConfiguration[INPUT_NUMBER];

typedef void(*controlActionFunction)(player_struct*,bool,bool);


static void controlForward(player_struct* p, bool down, bool held);
static void controlBack(player_struct* p, bool down, bool held);
static void controlStrafeLeft(player_struct* p, bool down, bool held);
static void controlStrafeRight(player_struct* p, bool down, bool held);
static void controlPause(player_struct* p, bool down, bool held);
static void controlShootAll(player_struct* p, bool down, bool held);
static void controlShootYellow(player_struct* p, bool down, bool held);
static void controlShootBlue(player_struct* p, bool down, bool held);
static void controlUse(player_struct* p, bool down, bool held);
static void controlJump(player_struct* p, bool down, bool held);
static void controlLookRight(player_struct* p, bool down, bool held);
static void controlLookLeft(player_struct* p, bool down, bool held);
static void controlLookUp(player_struct* p, bool down, bool held);
static void controlLookDown(player_struct* p, bool down, bool held);


static controlActionFunction controlFunctions[]={(controlActionFunction)controlForward,
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


static controlAction_type actionByString(char* str)
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
	char str[255];
	sprintf(str,"%s/%s",basePath,filename);
	dictionary* dic=iniparser_load(str);
	if(!dic)dic=iniparser_load(filename);
	int i; for(i=0;i<INPUT_NUMBER;i++)
	{
		char str[255];
		sprintf(str,"controls:%s",inputStrings[i]);
		currentConfiguration[i]=actionByString(dictionary_get(dic, str, inputDefaults[i]));
		if(currentConfiguration[i]==CONTROL_NUMBER)currentConfiguration[i]=actionByString(inputDefaults[i]);
	}
	iniparser_freedict(dic);
}

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

	int i;
	for(i=0;i<CONTROLS_NB_CONTROL_FN;i++)controlWasCalled[i]=false;
	for(i=0;i<INPUT_NUMBER;i++)updateControl(i);
}

static void controlForward(player_struct* p, bool down, bool held)
{
	if(!p)return;

	if(controlWasCalled[0])
		return;

	controlWasCalled[0]=true;


	if(p->object->contact)
	{
		moveCamera(NULL, vect(0,0,-(PLAYERGROUNDSPEED)));
		p->walkCnt+=2500;
		changeAnimation(&p->playerModelInstance,3,false);
		idle=false;
	}else moveCamera(NULL, vect(0,0,-PLAYERAIRSPEED));
}

static void controlBack(player_struct* p, bool down, bool held)
{
	if(!p)return;

	if(controlWasCalled[1])
		return;

	controlWasCalled[1]=true;

	if(p->object->contact)
	{
		moveCamera(NULL, vect(0,0,PLAYERGROUNDSPEED));
		p->walkCnt+=2500;
		changeAnimation(&p->playerModelInstance,3,false);
		idle=false;
	}else moveCamera(NULL, vect(0,0,PLAYERAIRSPEED));
}

static void controlStrafeLeft(player_struct* p, bool down, bool held)
{
	if(!p)return;

	if(controlWasCalled[2])
		return;

	controlWasCalled[2]=true;

	if(p->object->contact)
	{
		moveCamera(NULL, vect(-(PLAYERGROUNDSPEED),0,0));
		p->walkCnt+=2500;
		changeAnimation(&p->playerModelInstance,4,false);
		idle=false;
	}else moveCamera(NULL, vect(-PLAYERAIRSPEED,0,0));
}

static void controlStrafeRight(player_struct* p, bool down, bool held)
{
	if(!p)return;

	if(controlWasCalled[3])
		return;

	controlWasCalled[3]=true;

	if(p->object->contact)
	{
		moveCamera(NULL, vect(PLAYERGROUNDSPEED,0,0));
		p->walkCnt+=2500;
		changeAnimation(&p->playerModelInstance,4,false);
		idle=false;
	}else moveCamera(NULL, vect(PLAYERAIRSPEED,0,0));
}

static void controlPause(player_struct* p, bool down, bool held)
{
	if(!p || !down)return;

	if(controlWasCalled[4])
		return;

	controlWasCalled[4]=true;

	doPause(NULL);
}

static void controlShootAll(player_struct* p, bool down, bool held)
{
	if(!p || !down)return;

	if(controlWasCalled[5])
		return;

	controlWasCalled[5]=true;

	if(!p->modelInstance.oneshot)
	{
		playSFX(currentPortalColor?gunSFX1:gunSFX2);
		shootPlayerGun(p,currentPortalColor,255);
		changeAnimation(&p->modelInstance,1,true);
	}
}

static void controlShootYellow(player_struct* p, bool down, bool held)
{
	if(!p || !down)return;

	if(controlWasCalled[6])
		return;

	controlWasCalled[6]=true;

	if(!p->modelInstance.oneshot)
	{
		playSFX(gunSFX1);
		shootPlayerGun(p,true,255);
		changeAnimation(&p->modelInstance,1,true);
	}
}

static void controlShootBlue(player_struct* p, bool down, bool held)
{
	if(!p || !down)return;

	if(controlWasCalled[7])
		return;

	controlWasCalled[7]=true;

	if(!p->modelInstance.oneshot)
	{
		playSFX(gunSFX2);
		shootPlayerGun(p,false,255);
		changeAnimation(&p->modelInstance,1,true);
	}
}

static void controlUse(player_struct* p, bool down, bool held)
{
	if(!p || !down)return;

	if(controlWasCalled[8])
		return;

	controlWasCalled[8]=true;

	if(!p->modelInstance.oneshot)
	{
		playSFX(gunSFX2);
		shootPlayerGun(p,false,1|2);
		changeAnimation(&p->modelInstance,1,true);
	}
}

static void controlJump(player_struct* p, bool down, bool held)
{
	if(!p || !down)return;

	if(controlWasCalled[9])
		return;

	controlWasCalled[9]=true;

	if(p->object->contact)p->object->speed=addVect(p->object->speed,vectMult(normGravityVector,-(inttof32(1)>>5)));
}

static void controlLookRight(player_struct* p, bool down, bool held)
{
	if(!p)return;

	if(controlWasCalled[10])
		return;

	controlWasCalled[10]=true;

	rotateCamera(NULL, vect(0,16,0));
}

static void controlLookLeft(player_struct* p, bool down, bool held)
{
	if(!p)return;

	if(controlWasCalled[11])
		return;

	controlWasCalled[11]=true;

	rotateCamera(NULL, vect(0,-16,0));
}

static void controlLookUp(player_struct* p, bool down, bool held)
{
	if(!p)return;

	if(controlWasCalled[12])
		return;

	controlWasCalled[12]=true;

	rotateCamera(NULL, vect(16,0,0));
}

static void controlLookDown(player_struct* p, bool down, bool held)
{
	if(!p)return;
	if(controlWasCalled[13])
		return;

	controlWasCalled[13]=true;

	rotateCamera(NULL, vect(-16,0,0));
}


