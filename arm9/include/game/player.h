#ifndef __PLAYER9__
#define __PLAYER9__

#define PLAYERRADIUS (256)
#define ERRORMARGIN (2)
#define SQPLAYERRADIUS ((PLAYERRADIUS*PLAYERRADIUS)>>12)

#define PLAYERGROUNDSPEED (inttof32(3)>>8)
#define PLAYERAIRSPEED (inttof32(1)>>9)

typedef struct
{
	vect3D relativePosition;
	vect3D relativePositionReal;
	vect3D tempAngle;
	s32 walkCnt;
	room_struct* currentRoom;
	physicsObject_struct* object;
	modelInstance_struct modelInstance;
	modelInstance_struct playerModelInstance;
	bool currentPortal;
	bool inPortal, oldInPortal;
	s8 life;
}player_struct;

room_struct* getCurrentRoom(void);
void initPlayer(player_struct* p);
void playerControls(player_struct* p);
void updatePlayer(player_struct* p);
void renderGun(player_struct*);
player_struct* getPlayer(void);
void freePlayer(void);
void drawCrosshair(void);
void damagePlayer(player_struct* p);
void drawPlayer(player_struct* p);

#endif
