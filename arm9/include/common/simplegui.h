#ifndef SIMPLEGUI_H
#define SIMPLEGUI_H

typedef struct
{
	vect3D position, size;
	char* string;
	bool used;
}sguiButton_struct;


void initSimpleGui(void);
sguiButton_struct* createSimpleButton(vect3D p, const char* str);
void drawSimpleButtons(void);
void drawSimpleGui(void);

#endif
