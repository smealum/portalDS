#ifndef SIMPLEGUI_H
#define SIMPLEGUI_H

#define NUMSIMPLEBUTTONS (64)
#define SIMPLEBUTTONMARGINX (6)
#define SIMPLEBUTTONMARGINY (3)

#define SIMPLEBUTTONSIZEY (SIMPLEBUTTONMARGINY*2+8)

struct sguiButton_struct;

typedef void(*buttonTargetFunction)(struct sguiButton_struct*);

struct sguiButton_struct
{
	vect3D position, size;
	char* string;
	mtlImg_struct* mtl;
	vect3D mtlOffset, mtlSize;
	buttonTargetFunction targetFunction;
	bool used, active;
};

typedef struct sguiButton_struct sguiButton_struct;


void initSimpleGui(void);
sguiButton_struct* createSimpleButton(vect3D p, const char* str, buttonTargetFunction f);
bool updateSimpleGui(s16 x, s16 y);
void drawSimpleGui(void);
void cleanUpSimpleButtons(void);
void simpleButtonSetImage(sguiButton_struct* b, mtlImg_struct* mtl, vect3D o, vect3D s);

#endif
