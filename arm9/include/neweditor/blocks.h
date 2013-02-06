#ifndef BLOCKS_H
#define BLOCKS_H

#define BLOCKFACEPOOLSIZE (2048)

#define ROOMARRAYSIZEX (64)
#define ROOMARRAYSIZEY (64)
#define ROOMARRAYSIZEZ (64)

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

void initEditorRoom(editorRoom_struct* er);
void freeEditorRoom(editorRoom_struct* er);
void drawEditorRoom(editorRoom_struct* er);

void initBlocks(void);
blockFace_struct* popBlockFace(blockFace_struct** l);
void addBlockFace(blockFace_struct** l, blockFace_struct* bf);
void generateBlockFacesRange(u8* ba, blockFace_struct** l, vect3D o, vect3D s);
void collideLineBlockFaceList(blockFace_struct* l, vect3D o, vect3D v, int32 d);

#endif
