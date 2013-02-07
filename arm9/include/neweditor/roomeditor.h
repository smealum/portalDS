#ifndef ROOMEDITOR_H
#define ROOMEDITOR_H

typedef struct
{
	blockFace_struct *firstFace, *secondFace;
	vect3D origin, size;
	bool active, planar;
}selection_struct;

void initRoomEditor(void);
void drawRoomEditor(void);
void freeRoomEditor(void);
void updateRoomEditor(void);

#endif