#ifndef __MAP9__
#define __MAP9__

#define TILESIZE (256)
#define HEIGHTUNIT (128)
#define SCALEFACT (inttof32(150))

#define CELLSIZE (4)

// #define MAXHEIGHT 31
#define MAXHEIGHT 47
#define STARTHEIGHT 16 //TEMP

#define DEFAULTFLOOR 8
#define DEFAULTCEILING 24

#define LIGHTMAPRESOLUTION 6

#define LIGHTCONST (0)
#define AMBIENTLIGHT (8) //portal is pretty bright, right ?

typedef struct
{
	vect3D position, size, lmSize, lmPos, normal;
	material_struct* material;
	s16 AARid;
	bool rot, portalable, hide, touched;
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
	entity_struct* lights[3];
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
	u8* pathfindingData;
	void** doorWay;
	mtlImg_struct* lightMap;
	lightMapSlots_struct* lmSlot;
	gridCell_struct* rectangleGrid;
	vect3D rectangleGridSize;
	entityCollection_struct* entityCollection;
	rectangleList_struct rectangles;
}room_struct;

static inline vect3D reverseConvertVect(vect3D v)
{
	return vect((v.x+TILESIZE)/(TILESIZE*2),v.y/HEIGHTUNIT,(v.z+TILESIZE)/(TILESIZE*2));
}

static inline vect3D convertVect(vect3D v)
{
	return vect(v.x*TILESIZE*2-TILESIZE,v.y*HEIGHTUNIT,v.z*TILESIZE*2-TILESIZE);
}

static inline vect3D convertSize(vect3D v)
{
	return vect(v.x*TILESIZE*2,v.y*HEIGHTUNIT,v.z*TILESIZE*2);
}

void initRectangleList(rectangleList_struct* p);
rectangle_struct* addRectangle(rectangle_struct r, rectangleList_struct* p);
void initRoom(room_struct* r, u16 w, u16 h, vect3D p);
void resizeRoom(room_struct* r, u16 l, u16 w, vect3D p);
// void addRoomRectangle(room_struct* r, entityCollection_struct* ec, rectangle_struct rec, bool portalable);
void initRectangle(rectangle_struct* rec, vect3D pos, vect3D size);
rectangle_struct createRectangle(vect3D pos, vect3D size);
void removeRectangles(room_struct* r);
void drawRectangleList(rectangleList_struct* rl);
void drawRoom(room_struct* r, u8 mode, u16 color);
void freeRoom(room_struct* r);
void drawRect(rectangle_struct rec, vect3D pos, vect3D size, bool c);
bool collideLineMap(room_struct* r, rectangle_struct* rec, vect3D l, vect3D u, int32 d, vect3D* i, vect3D* n);
rectangle_struct* collideGridCell(gridCell_struct* gc, rectangle_struct* rec, vect3D l, vect3D u, int32 d, vect3D* i, vect3D* n);
rectangle_struct* collideLineMapClosest(room_struct* r, rectangle_struct* rec, vect3D l, vect3D u, int32 d, vect3D* i);

u8 getHeightValue(room_struct* r, vect3D pos, bool floor);

u32* generateRoomDisplayList(room_struct* r, vect3D pos, vect3D normal, bool cull);

void setupObjectLighting(room_struct* r, vect3D pos, u32* params);

void generateRoomGrid(room_struct* r);
gridCell_struct* getCurrentCell(room_struct* r, vect3D o);

//lightmaps.h
void loadLightMap(room_struct* r);
void unloadLightMap(room_struct* r);
void unloadAllLightMaps(room_struct* r);
void unloadLightMaps(room_struct* r, room_struct* r2);

#endif
