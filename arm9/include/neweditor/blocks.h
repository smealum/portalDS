#ifndef BLOCKS_H
#define BLOCKS_H

typedef struct blockFace_struct
{
	u8 x, y, z;
	u8 direction;
	struct blockFace_struct* next;
}blockFace_struct;

void initBlocks(void);
blockFace_struct* popBlockFace(blockFace_struct** l);
void addBlockFace(blockFace_struct** l, blockFace_struct* bf);

#endif
