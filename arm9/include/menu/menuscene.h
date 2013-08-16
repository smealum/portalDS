#ifndef MENUSCENE_H
#define MENUSCENE_H

#define NUMMENUBOXES (8)
#define MENUBOXSPEED (32)
#define MENUBOXFREQUENCY (20)
#define MENUBOXANGLESPEED (1024)

#define MENUSCREENLINES 6
#define MENUSCREENCHARS 14

typedef struct
{
	vect3D angle, anglespeed;
	u16 progress;
	bool used;
}menuBox_struct;

extern camera_struct menuCamera;

extern char menuScreenText[MENUSCREENLINES][MENUSCREENCHARS];

void initMenuScene(void);
void freeMenuScene(void);
void updateMenuScene(void);
void drawMenuScene(void);

void resetSceneScreen(void);

#endif
