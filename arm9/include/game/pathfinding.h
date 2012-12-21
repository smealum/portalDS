#ifndef __PATHFINDING9__
#define __PATHFINDING9__

#define DIRUP 1
#define DIRDOWN (1<<1)
#define DIRLEFT (1<<2)
#define DIRRIGHT (1<<3)

typedef enum
{
	noList=0,
	openList=1,
	closedList=2,
}listType;

typedef struct
{
	listType inList;
	unsigned char X, Y;
	unsigned char parentX, parentY;
	unsigned short gCost;
	unsigned short fCost;
}pathNode_struct;

typedef struct listNode_struct
{
	pathNode_struct* data;
	struct listNode_struct* next;
}listNode_struct;

typedef struct
{
	listNode_struct* first;
}openList_struct;

typedef struct pathCell_struct
{
	unsigned char X, Y;
	struct pathCell_struct* next;
}pathCell_struct;

typedef struct
{
	pathCell_struct* first;
}path_struct;

typedef struct
{
	unsigned char* data;
	pathNode_struct* nodes;
	unsigned char width, height;
	openList_struct openList;
	vect2D origin, target;
	path_struct path;
}pathfindingSystem_struct;

void initPathfindingGlobal(void);
pathCell_struct* getPath(room_struct* r, vect2D origin, vect2D target);
bool popPathCell(path_struct* p, int* x, int* y);
void freePath(path_struct* p);

#endif
