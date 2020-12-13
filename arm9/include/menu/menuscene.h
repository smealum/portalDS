#ifndef MENUSCENE_H
#define MENUSCENE_H

#define NUMMENUBOXES (8)
#define MENUBOXSPEED (32)
#define MENUBOXFREQUENCY (20)
#define MENUBOXANGLESPEED (1024)

/* Number of lines displayed in the screen terminal */
#define MENUSCREENLINES 6
/* Number of columns displayed in the screen terminal */
#define MENUSCREENCHARS 14

typedef struct
{
	vect3D angle, anglespeed;
	u16 progress;
	bool used;
}menuBox_struct;

extern camera_struct menuCamera;

extern char menuScreenText[MENUSCREENLINES][MENUSCREENCHARS];

/**
 * Inits menu scene, so the animation in the background of the menu.
 * Must only be called once before freeing ressources using #freeMenuScene.
 */
void initMenuScene(void);

/**
 * Free menu scene graphics elements.
 * Must only be called once before allocating again ressources using #initMenuScene.
 */
void freeMenuScene(void);

/**
 * Updates background animation of the menu.
 * Must be called every frame.
 */
void updateMenuScene(void);

/**
 * Draws background animation of the menu.
 * Must be called every frame.
 */
void drawMenuScene(void);

/**
 * Reset background screen (terminal that displays level lists)
 */
void resetSceneScreen(void);

typedef struct
{
	char title[MENUSCREENCHARS];
	char** list;
	int length, offset, cursor;
}screenList_struct;

/**
 * Initialises background terminal display list
 *
 * \param[out] sl 		screen list
 * \param[in]  title	title of the list (copied)
 * \param[in]  list  	list of string    (not deep copied)
 * \param[in]  l        length of list
 *
 */
void initScreenList(screenList_struct* sl, char* title, char** list, int l);
void screenListMove(screenList_struct* sl, s8 move);
void updateScreenList(screenList_struct* sl);



#endif
