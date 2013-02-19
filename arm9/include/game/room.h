#ifndef ROOM_H
#define ROOM_H

typedef struct
{
	vect3D position, size, lmSize, lmPos, normal;
	material_struct* material;
	s16 AARid;
	bool rot, portalable, hide, touched, collides;
}rectangle_struct;

typedef struct listCell_struct
{
	rectangle_struct data;
	struct listCell_struct* next;
}listCell_struct;

typedef struct
{
	listCell_struct* first;
	int num;
}rectangleList_struct;

typedef struct
{
	rectangle_struct** rectangles;
	light_struct* lights[3];
	int32 lightDistances[3];
	u8 numRectangles;
}gridCell_struct;

typedef struct
{
	material_struct** materials;
	vect3D position;
	vect3D lmSize;
	u16 width, height;
	u8* lightMapBuffer;
	mtlImg_struct* lightMap;
	lightMapSlots_struct* lmSlot;
	gridCell_struct* rectangleGrid;
	vect3D rectangleGridSize;
	rectangleList_struct rectangles;
}room_struct;

void drawRoomEdits(void);
void wipeMapEdit(void);
void writeMap(char* filename);
void readMap(char* filename, room_struct* r);
void newReadMap(char* filename, room_struct* r);

#endif