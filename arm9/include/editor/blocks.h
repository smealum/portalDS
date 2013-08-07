#ifndef BLOCKS_H
#define BLOCKS_H

#define BLOCKFACEPOOLSIZE (2048*4)

#define ROOMARRAYSIZEX (64)
#define ROOMARRAYSIZEY (64)
#define ROOMARRAYSIZEZ (64)

#define BLOCKMULT (2)

#define BLOCKMULTX (BLOCKMULT*1)
#define BLOCKMULTY (BLOCKMULT*4)
#define BLOCKMULTZ (BLOCKMULT*1)

#define BLOCKSIZEX (TILESIZE*2*BLOCKMULTX)
#define BLOCKSIZEY (HEIGHTUNIT*BLOCKMULTY)
#define BLOCKSIZEZ (TILESIZE*2*BLOCKMULTZ)

#define BLOCK_NOWALLS (1<<7)
#define BLOCK_SLUDGE (1<<8)

#define BLOCK_TYPE u16

//DIR : 0 X
//		1 -X
//		2 Y
//		3 -Y
//		4 Z
//		5 -Z

//BLOCK DATA STRUCTURE
// BIT 0 : WALL
// BITS 1-6 : PORTALABILITY (1 BIT PER DIRECTION)
// BIT 7 : NOWALLS
// BIT 8 : SLUDGE
// BITS 9-15 : unused

typedef struct blockFace_struct
{
	u8 x, y, z;
	u8 direction;
	bool draw;
	struct blockFace_struct* next;
}blockFace_struct;

typedef struct
{
	BLOCK_TYPE* blockArray;
	blockFace_struct* blockFaceList;
}editorRoom_struct;

extern vect3D faceNormals[6];
extern vect3D faceOrigin[6];
extern u32 packedVertex[6][4];
extern u8 oppositeDirection[6];

typedef void(*blockAttributeFunction)(BLOCK_TYPE* ba, u8 x, u8 y, u8 z, u16 mask, bool unset);

void initEditorRoom(editorRoom_struct* er);
void freeEditorRoom(editorRoom_struct* er);
void drawEditorRoom(editorRoom_struct* er);
void editorRoomTransform(void);

void initBlocks(void);
void freeBlockFacePool(void);
blockFace_struct* popBlockFace(blockFace_struct** l);
vect3D getBlockPosition(u8 x, u8 y, u8 z);
void freeBlockFace(blockFace_struct* bf);
void addBlockFace(blockFace_struct** l, blockFace_struct* bf);
blockFace_struct* findBlockFace(blockFace_struct* l, u8 x, u8 y, u8 z, u8 direction);
blockFace_struct* collideLineBlockFaceListClosest(blockFace_struct* l, vect3D o, vect3D v, int32* d);
vect3D adjustVectForNormal(u8 dir, vect3D v);

vect3D getMinBlockArray(BLOCK_TYPE* ba);
void fixOriginSize(vect3D* o, vect3D* s);
void freeBlockFaceList(blockFace_struct** l);
void generateBlockFacesRange(BLOCK_TYPE* ba, blockFace_struct** l, vect3D o, vect3D s, bool outskirts);
void emptyBlockArrayRange(BLOCK_TYPE* ba, blockFace_struct** l, vect3D o, vect3D s);
void fillBlockArrayRange(BLOCK_TYPE* ba, blockFace_struct** l, vect3D o, vect3D s);
void changePortalableBlockDirection(BLOCK_TYPE* ba, u8 x, u8 y, u8 z, u16 u, bool portalable);
void changeSludgeBlock(BLOCK_TYPE* ba, u8 x, u8 y, u8 z, u16 u, bool nosludge);
void changeAttributeBlockArrayRange(BLOCK_TYPE* ba, blockAttributeFunction f, blockFace_struct* l, vect3D o, vect3D s, bool unset);
void changeAttributeBlockArrayRangeDirection(BLOCK_TYPE* ba, blockAttributeFunction f, blockFace_struct* l, vect3D o, vect3D s, u8 dir, bool portalable);

rectangleList_struct generateOptimizedRectangles(BLOCK_TYPE* ba, rectangleList_struct* sludgeList);

#endif
