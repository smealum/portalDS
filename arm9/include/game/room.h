#ifndef ROOM_H
#define ROOM_H

typedef struct
{
	vect3D position, size, normal;
	material_struct* material;
	s16 AARid;
	union{
		vertexLightingData_struct* vertex;
		lightMapCoordinates_struct* lightMap;
	}lightData;
	bool portalable, hide, touched, collides;
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
	gridCell_struct* rectangleGrid;
	vect3D rectangleGridSize;
	rectangleList_struct rectangles;
	lightingData_struct lightingData;
	u32* displayList;
}room_struct;

void drawRoomEdits(void);
void wipeMapEdit(void);
void writeMap(char* filename);
void readMap(char* filename, room_struct* r);
void newReadMap(char* filename, room_struct* r, u8 flags);

void roomResetOrigin(room_struct* r);
void roomOriginSize(room_struct* r, vect3D* o, vect3D* s);
void insertRoom(room_struct* r1, room_struct* r2, vect3D v, u8 orientation);

void generateLightmaps(room_struct* r, lightingData_struct* ld);

void generateVertexLighting(room_struct* r, lightingData_struct* ld);

void drawRoomsGame(u8 mode, u16 color);
#endif