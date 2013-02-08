#ifndef ROOMEDITOR_H
#define ROOMEDITOR_H

typedef struct
{
	blockFace_struct *firstFace, *secondFace, *currentFace;
	vect3D origin, size, currentPosition;
	bool active, selecting, planar;
}selection_struct;

void initRoomEditor(void);
void drawRoomEditor(void);
void freeRoomEditor(void);
void updateRoomEditor(void);

#endif