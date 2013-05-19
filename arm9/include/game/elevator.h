#ifndef ELEVATOR_H
#define ELEVATOR_H

#define ELEVATOR_UPDOWNBIT (4)

typedef struct
{
	vect3D position;
	int32 progress;
	u8 direction;
	modelInstance_struct modelInstance;
}elevator_struct;

void initElevators(void);
void initElevator(elevator_struct* ev, room_struct* r, vect3D position, u8 direction, bool up);
void updateElevator(elevator_struct* ev);
void drawElevator(elevator_struct* ev);

#endif
