#ifndef EDITORIO_H
#define EDITORIO_H


#define MAPHEADER_SIZE (256)

typedef struct
{
	u32 dataSize;
	u32 dataPosition;
	u32 rectanglesPosition;
	u32 entityPosition;
	u32 lightPosition;
	u32 sludgePosition;

	u8 blank[MAPHEADER_SIZE-6*4]; //for future use
}mapHeader_struct;

void readHeader(mapHeader_struct* h, FILE* f);

#ifdef BLOCKS_H
	void writeMapEditor(editorRoom_struct* er, const char* str);
	bool loadMapEditor(editorRoom_struct* er, const char* str);
#endif

#endif
