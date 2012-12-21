#ifndef __PLAYER9__
#define __PLAYER9__

typedef struct
{
	vect3D relativePosition;
	vect3D relativePositionReal;
	room_struct* currentRoom;
	physicsObject_struct* object;
	modelInstance_struct modelInstance;
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

#endif
