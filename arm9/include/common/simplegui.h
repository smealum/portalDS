#ifndef SIMPLEGUI_H
#define SIMPLEGUI_H

#define NUMSIMPLEBUTTONS (16)
#define SIMPLEBUTTONMARGINX (6)
#define SIMPLEBUTTONMARGINY (3)

#define SIMPLEBUTTONSIZEY (SIMPLEBUTTONMARGINY*2+8)

typedef void(*buttonTargetFunction)(void);

typedef struct
{
	vect3D position, size;
	char* string;
	buttonTargetFunction targetFunction;
	bool used, active;
}sguiButton_struct;


void initSimpleGui(void);
sguiButton_struct* createSimpleButton(vect3D p, const char* str, buttonTargetFunction f);
bool updateSimpleGui(u8 x, u8 y);
void drawSimpleGui(void);
void cleanUpSimpleButtons(void);

#endif
