#ifndef __DOORS9__
#define __DOORS9__

#define NUMDOORS (256)

#define DOORPERIOD 120

#define DOORSIZE 2
#define DOORHEIGHT 12

typedef enum
{
	doorUp,
	doorRight,
	doorDown,
	doorLeft,
}doorDirection;

typedef struct
{
	room_struct *primaryRoom, *secondaryRoom;
	rectangle_struct primaryRec, secondaryRec;
	doorDirection direction;
	u16 position, height;
	s16 percentage;
	u16 timer;
	bool selected;
	bool open;
	bool used;
}door_struct;

typedef struct
{
	door_struct door[NUMDOORS];
}doorCollection_struct;

void initDoor(door_struct* d);
void getSecondaryRooms(doorCollection_struct* dc);
void initDoorCollection(doorCollection_struct* dc);
void getDoorRectangle(door_struct* d, bool primary, rectangle_struct* rrrec);
void confirmDoors(doorCollection_struct* dc);
void confirmRoomDoors(doorCollection_struct* dc, room_struct* r);
bool collideSegmentDoors(doorCollection_struct* dc, room_struct* r, vect3D p1, vect3D p2);
door_struct* inDoorWay(doorCollection_struct* dc, room_struct* r, int x, int y, bool secondary, bool override);
door_struct* createDoor(doorCollection_struct* dc, room_struct* pr, u16 position, doorDirection dir, bool override);
void renderRoomDoors(doorCollection_struct* dc, room_struct* r);
void getRoomDoorRectangles(doorCollection_struct* dc, room_struct* r);
door_struct* getClosestDoorRoom(doorCollection_struct* dc, room_struct* r, vect2D p, int* distt);
void updateDoors(doorCollection_struct* dc);
bool toggleDoor(room_struct* r, door_struct* d);
void closeRoomDoors(doorCollection_struct* dc, room_struct* r, door_struct* d);
bool isDoorOpen(doorCollection_struct* dc, room_struct* r1, room_struct* r2);

void getDoorWayData(doorCollection_struct* dc, room_struct* r);

#ifdef __EDITORMAIN9__
	void drawDoor(roomEdit_struct* re, door_struct* d);
	void drawRoomDoors(doorCollection_struct* dc, roomEdit_struct* re);
	door_struct* doorTouches(doorCollection_struct* dc, roomEdit_struct* re, int x, int y, bool secondary);
	bool doorCollides(doorCollection_struct* dc, room_struct* r, doorDirection dd, u16 p, door_struct* d2);
#endif

#endif
