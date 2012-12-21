#ifndef __FONT9__
#define __FONT9__

#define CHARSIZE 16

typedef struct
{
	mtlImg_struct tex;
	int32 charsizef32;
	u8 charsize;
}font_struct;

void initText(void);
void setFont(font_struct* f);
void loadFont(font_struct* f, u8 charsize);
void drawChar(char c, u16 color, int32 x, int32 y);
void drawString(char* s, u16 color, int32 size, int32 x, int32 y);

#endif