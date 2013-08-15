#ifndef TIMEDBUTTON_H
#define TIMEDBUTTON_H

#define NUMTIMEDBUTTONS (16)

typedef struct
{
	room_struct* room;
	activator_struct activator;
	modelInstance_struct modelInstance;
	vect3D position;
	u16 angle;
	u16 active;
	bool used;
	u8 id;
}timedButton_struct;

void initTimedButtons(void);
void freeTimedButtons(void);
void activateTimedButton(timedButton_struct* tb);
timedButton_struct* collideRayTimedButtons(vect3D o, vect3D v, int32 l);
timedButton_struct* createTimedButton(room_struct* r, vect3D position, u16 angle);
bool checkObjectTimedButtonsCollision(physicsObject_struct* o, room_struct* r);
void drawTimedButtons(void);
void updateTimedButtons(void);

#endif
