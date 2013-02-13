#ifndef EDITORIO_H
#define EDITORIO_H

typedef struct
{
	u32 dataSize;
	u32 dataPosition;
	u32 rectanglesPosition;
	u32 entityPosition;
	u32 lightPosition;
}mapHeader_struct;

void writeMapEditor(editorRoom_struct* er, const char* str);

#endif
