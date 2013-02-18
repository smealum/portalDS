#ifndef DOOR_H
#define DOOR_H

#define NUMDOORS (16)

typedef struct
{
	vect3D position;
	modelInstance_struct modelInstance;
	bool orientation;
	bool used;
	u8 id;
}door_struct;


void initDoors(void);
door_struct* createDoor(room_struct* r, vect3D position, bool orientation);
void updateDoors(void);
void drawDoors(void);

#endif