#ifndef BLOCKS_H
#define BLOCKS_H

#define BLOCKFACEPOOLSIZE (2048*4)

#define ROOMARRAYSIZEX (64)
#define ROOMARRAYSIZEY (64)
#define ROOMARRAYSIZEZ (64)

#define BLOCKMULTX (1)
#define BLOCKMULTY (2)
#define BLOCKMULTZ (1)

#define BLOCKSIZEX (TILESIZE*2*BLOCKMULTX)
#define BLOCKSIZEY (HEIGHTUNIT*2*BLOCKMULTY)
#define BLOCKSIZEZ (TILESIZE*2*BLOCKMULTZ)

//DIR : 0 X
//		1 -X
//		2 Y
//		3 -Y
//		4 Z
//		5 -Z

typedef struct blockFace_struct
{
	u8 x, y, z;
	u8 direction;
	bool draw;
	struct blockFace_struct* next;
}blockFace_struct;

typedef struct
{
	u8* blockArray;
	blockFace_struct* blockFaceList;
	rectangleList_struct rectangleList;
}editorRoom_struct;

extern vect3D faceNormals[6];
extern vect3D faceOrigin[6];
extern u32 packedVertex[6][4];
extern u8 oppositeDirection[6];

void initEditorRoom(editorRoom_struct* er);
void freeEditorRoom(editorRoom_struct* er);
void drawEditorRoom(editorRoom_struct* er);

void initBlocks(void);
void freeBlockFacePool(void);
blockFace_struct* popBlockFace(blockFace_struct** l);
vect3D getBlockPosition(u8 x, u8 y, u8 z);
void freeBlockFace(blockFace_struct* bf);
void addBlockFace(blockFace_struct** l, blockFace_struct* bf);
blockFace_struct* findBlockFace(blockFace_struct* l, u8 x, u8 y, u8 z, u8 direction);
blockFace_struct* collideLineBlockFaceListClosest(blockFace_struct* l, vect3D o, vect3D v);
vect3D adjustVectForNormal(u8 dir, vect3D v);


void fixOriginSize(vect3D* o, vect3D* s);
void freeBlockFaceList(blockFace_struct** l);
void generateBlockFacesRange(u8* ba, blockFace_struct** l, vect3D o, vect3D s, bool outskirts);
void emptyBlockArrayRange(u8* ba, blockFace_struct** l, vect3D o, vect3D s);
void fillBlockArrayRange(u8* ba, blockFace_struct** l, vect3D o, vect3D s);

rectangleList_struct generateOptimizedRectangles(u8* ba);

#endif
