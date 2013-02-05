#ifndef BLOCKS_H
#define BLOCKS_H

#define ROOMARRAYSIZEX (64)
#define ROOMARRAYSIZEY (64)
#define ROOMARRAYSIZEZ (64)

typedef struct blockFace_struct
{
	u8 x, y, z;
	u8 direction;
	struct blockFace_struct* next;
}blockFace_struct;

typedef struct
{
	u8* blockArray;
	blockFace_struct* blockFaceList;
}editorRoom_struct;

void initEditorRoom(editorRoom_struct* er);

void initBlocks(void);
blockFace_struct* popBlockFace(blockFace_struct** l);
void addBlockFace(blockFace_struct** l, blockFace_struct* bf);

#endif
