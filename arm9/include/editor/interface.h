#ifndef INTERFACE_H
#define INTERFACE_H

typedef struct
{
	u8 x, y;
	const char* imageName;
	struct gl_texture_t* imageData;
	u16 argument;
	bool down;
}interfaceButton_struct;

void initInterface(void);
void updateInterfaceButtons(u8 x, u8 y);
void pauseEditorInterface(void);
void freeInterface(void);

#endif
