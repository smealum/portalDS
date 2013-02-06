#ifndef BLOCKS_H
#define BLOCKS_H

#define BLOCKFACEPOOLSIZE (2048)

#define ROOMARRAYSIZEX (64)
#define ROOMARRAYSIZEY (64)
#define ROOMARRAYSIZEZ (64)

#define BLOCKSIZEX inttof32(10)//(TILESIZE*2)
#define BLOCKSIZEY inttof32(10)//(TILESIZE*2)
#define BLOCKSIZEZ inttof32(10)//(TILESIZE*2)

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
}editorRoom_struct;

extern vect3D faceNormals[6];
extern u32 packedVertex[6][4];

void initEditorRoom(editorRoom_struct* er);
void freeEditorRoom(editorRoom_struct* er);
void drawEditorRoom(editorRoom_struct* er);

void initBlocks(void);
blockFace_struct* popBlockFace(blockFace_struct** l);
void addBlockFace(blockFace_struct** l, blockFace_struct* bf);
void generateBlockFacesRange(u8* ba, blockFace_struct** l, vect3D o, vect3D s);
blockFace_struct* collideLineBlockFaceListClosest(blockFace_struct* l, vect3D o, vect3D v);

#endif
