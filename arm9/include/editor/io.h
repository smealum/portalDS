#ifndef EDITORIO_H
#define EDITORIO_H

#define NUMENTITIES (64)

typedef struct
{
	u32 dataSize;
	u32 dataPosition;
	u32 rectanglesPosition;
	u32 entityPosition;
	u32 lightPosition;
}mapHeader_struct;

void readHeader(mapHeader_struct* h, FILE* f);

#ifdef BLOCKS_H
	void writeMapEditor(editorRoom_struct* er, const char* str);
	void loadMapEditor(editorRoom_struct* er, const char* str);
#endif

#endif
