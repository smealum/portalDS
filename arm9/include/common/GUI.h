#ifndef __API9__
#define __API9__

typedef void(*guiDrawFunction)(void* e);

typedef struct
{
	s32 x, y;
}coord2Ds;

typedef struct
{
	u32 x, y;
}coord2Du;

typedef struct guiEntity_struct
{
	u16 id, type;
	u8 timep;
	u8 times;
	u8 timea;
	s16 prio;
	s16 alpha;
	u16 color;
	u8 t_timep;
	u8 t_times;
	u8 t_timea;
	void* data;
	u8 outline;
	s16 t_alpha;
	bool halpha;
	bool shadow;
	bool bastard;
	u8 kidToDate;
	coord2Ds Size;
	coord2Ds Ratio;
	coord2Ds a_Size;
	coord2Ds o_Size;
	coord2Ds b_Size;
	coord2Ds t_Size;
	GLvector scaleVs;
	u16 outline_color;
	coord2Ds Position;
	coord2Ds a_Position;
	coord2Ds o_Position;
	coord2Ds t_Position;
	guiDrawFunction Draw;
	struct guiEntity_struct *father;
	struct guiEntity_struct *temp_son;
	struct guiEntity_struct *up, *down, *left, *right;
}guiEntity_struct;

typedef void(*guiFunction)(guiEntity_struct* e);

typedef struct
{
	mtlImg_struct *background;
}guiWindowData_struct;

typedef struct
{
	// mtlImg_struct *background;
	char* string;
	u16 width;
	u16 color;
}guiLabelData_struct;

typedef struct
{
	bool over, clicked, colorTakeover;
	mtlImg_struct *background;
	guiEntity_struct* label;
	guiFunction function;
	bool clickable, activated;
}guiButtonData_struct;

typedef struct
{
	bool over, checked, clickable;
	mtlImg_struct *background;
	guiEntity_struct* label;
	guiFunction function;
}guiCheckBoxData_struct;

typedef struct
{
	s16 position, oldpos, size;
	guiEntity_struct* label;
	bool over, selected;
	mtlImg_struct *background;
	guiFunction function;
}guiSliderData_struct;

typedef struct guiListElement_struct
{
	struct guiListElement_struct *previous, *next;
	guiEntity_struct *entity;
}guiListElement_struct;

typedef struct
{
	guiListElement_struct *first, *last;
	u16 count;
}guiEntityList_struct;

void guiCall(void);
void initGui(void);
void cleanUpGui(void);
void projectGui(void);
void toggleShadow(guiEntity_struct* e);
bool collideGui(guiEntityList_struct *cl, int px, int py);
void updateGui(guiEntityList_struct *cl, touchPosition* tp);
void setAlpha(guiEntity_struct* e, u8 alpha);
void setSize(guiEntity_struct* e, u16 x, u16 y);
void setAlphaSons(guiEntity_struct* f, u8 alpha);
void setPositionX(guiEntity_struct* e, s16 x);
void setPosition(guiEntity_struct* e, s16 x, s16 y);
void fadeSons(guiEntity_struct* f, u8 alpha, u8 time);
void fadeGuiEntity(guiEntity_struct* e, u8 alpha, u8 time);
void computeDirections(guiEntityList_struct *cl, u8 force);
void moveGuiEntity(guiEntity_struct* e, s16 x, s16 y, u8 time);
void resizeGuiEntity(guiEntity_struct* e, u16 x, u16 y, u8 time);
void deleteGuiEntitySons(guiEntityList_struct *cl, guiEntity_struct* father);
void computeGuiDirectionsEntity(guiEntityList_struct *cl, guiEntity_struct *e, u8 force);
void updateLabelText(guiEntity_struct* e, char* text);
void updateButtonText(guiEntity_struct* e, char* text);
void setSizeA(guiEntity_struct* e, u16 x, u16 y);
guiEntity_struct* createLabel(s16 x, s16 y, u16 color, char* text);
guiEntity_struct* createLabelFather(s16 x, s16 y, u16 color, guiEntity_struct* father, char* text, bool halpha);
guiEntity_struct* createCheckBoxFather(s16 x, s16 y, guiFunction function, guiEntity_struct* father, char* text, bool halpha);
guiEntity_struct* createSliderFather(s16 x, s16 y, u8 size, guiFunction function, guiEntity_struct* father, char* text, bool halpha);
guiEntity_struct* createVSliderFather(s16 x, s16 y, u8 size, guiFunction function, guiEntity_struct* father, char* text, bool halpha);
guiEntity_struct* createWindow(s16 x, s16 y, u16 sx, u16 sy, u8 alpha, u16 color, u8 outline, u16 outline_color, char* filename);
guiEntity_struct* createButtonFather(s16 x, s16 y, u16 color, guiFunction function, guiEntity_struct* father, char* text, char* filename, bool halpha);

#endif