#include "game/game_main.h"

#define PATHCELLNUM (2048)
#define LISTNODENUM (2048)
// #define PNNETSIZE (128*128)
#define PNNETSIZE (64*64)

pathCell_struct pathCellPoolDATA[PATHCELLNUM];
listNode_struct listNodePoolDATA[LISTNODENUM];

pathCell_struct* pathCellPool;
listNode_struct* listNodePool;

pathNode_struct pathNodes[PNNETSIZE];

int lnCnt;
int pcCnt;

void initPathCellPool(void)
{
	int i;
	for(i=0;i<PATHCELLNUM-1;i++)
	{
		pathCellPoolDATA[i].next=&pathCellPoolDATA[i+1];
	}
	pathCellPoolDATA[i].next=NULL;
	pathCellPool=&pathCellPoolDATA[0];
	pcCnt=PATHCELLNUM;
}

pathCell_struct* getPathCell()
{
	if(pathCellPool)
	{
		pathCell_struct* pc=pathCellPool;
		pathCellPool=pathCellPool->next;
		pc->next=NULL;
		pcCnt--;
		return pc;
	}else return NULL;
}

void freePathCell(pathCell_struct* pc)
{
	if(!pc)return;
	pc->next=pathCellPool;
	pathCellPool=pc;
	pcCnt++;
}

void initListNodePool(void)
{
	int i;
	for(i=0;i<LISTNODENUM-1;i++)
	{
		listNodePoolDATA[i].next=&listNodePoolDATA[i+1];
	}
	listNodePoolDATA[i].next=NULL;
	listNodePool=&listNodePoolDATA[0];
	lnCnt=LISTNODENUM;
}

listNode_struct* getListNode()
{
	if(listNodePool)
	{
		listNode_struct* pc=listNodePool;
		listNodePool=listNodePool->next;
		pc->next=NULL;
		lnCnt++;
		return pc;
	}else return NULL;
}

void freeListNode(listNode_struct* ln)
{
	if(!ln)return;
	ln->next=listNodePool;
	listNodePool=ln;
	lnCnt--;
}

void initPathfindingGlobal(void)
{
	initPathCellPool();
	initListNodePool();
}

void initPath(path_struct* p)
{
	if(!p)return;
	p->first=NULL;
}

void addPathCell(path_struct* p, unsigned char x, unsigned char y)
{
	if(!p)return;
	pathCell_struct* pc=getPathCell();
	if(!pc)return;
	pc->X=x;
	pc->Y=y;
	pc->next=p->first;
	p->first=pc;
}

bool popPathCell(path_struct* p, int* x, int* y)
{
	if(!p || !p->first){if(x)*x=-1;if(y)*y=-1;return false;}
	pathCell_struct* pc=p->first;
	p->first=pc->next;
	if(x)*x=pc->X;
	if(y)*y=pc->Y;
	pc->next=NULL;
	freePathCell(pc);
	// NOGBA("*pop* %p",p->first);
	return true;
}

void initOpenList(openList_struct* ol)
{
	if(!ol)return;
	ol->first=NULL;
}

void addListNode(openList_struct* ol, pathNode_struct* pn)
{
	if(!ol || !pn || pn->inList!=noList)return;
	listNode_struct* ln=(listNode_struct*)getListNode();
	if(!ln)return;
	ln->data=pn;
	ln->next=ol->first;
	ol->first=ln;
}

pathNode_struct* getLowestFcost(openList_struct* ol)
{
	if(!ol)return NULL;
	listNode_struct* ln=ol->first;
	listNode_struct *lowest=ln, *previous=NULL;
	while(ln && ln->next)
	{
		if(ln->next->data->fCost < lowest->data->fCost)
		{
			lowest=ln->next;
			previous=ln;
		}
		ln=ln->next;
	}
	if(lowest)
	{
		pathNode_struct* pn=lowest->data;
		if(previous)previous->next=lowest->next;
		else ol->first=lowest->next;
		lowest->next=NULL;
		freeListNode(lowest);
		return pn;
	}else return NULL;
}

void freeOpenList(openList_struct* ol)
{
	if(!ol)return;
	listNode_struct* ln=ol->first;
	while(ln)
	{
		// NOGBA("freeing");
		listNode_struct* ln2=ln->next;
		ln->next=NULL;
		freeListNode(ln);
		ln=ln2;
	}
}

bool initPathfinding(pathfindingSystem_struct* pfs, unsigned char* data, unsigned char w, unsigned char h, vect2D o, vect2D t)
{
	if(!pfs || !data)return false;
	if(o.x==t.x&&o.y==t.y)return false;

	pfs->height=h;
	pfs->width=w;
	pfs->data=data;
	pfs->origin=o;
	pfs->target=t;
	
	pfs->path.first=NULL;
	pfs->openList.first=NULL;
	initPath(&pfs->path);
	initOpenList(&pfs->openList);
	// pfs->nodes=(pathNode_struct*)malloc(sizeof(pathNode_struct)*w*h);
	pfs->nodes=pathNodes;
	if(!pfs->nodes){NOGBA("TROUUUUUBLE");return false;}
	int i, j;
	for(i=0;i<w;i++)
	{
		for(j=0;j<h;j++)
		{
			pfs->nodes[i+j*w].fCost=0;
			pfs->nodes[i+j*w].inList=noList;
			pfs->nodes[i+j*w].parentX=0;
			pfs->nodes[i+j*w].parentY=0;
			pfs->nodes[i+j*w].X=i;
			pfs->nodes[i+j*w].Y=j;
		}
	}
	return true;
}

void addOpenList(pathfindingSystem_struct* pfs, pathNode_struct* ppn, int X, int Y)
{
	// NOGBA("adding : %d %d ?",X,Y);
	if(!pfs || !ppn || X<0 || Y<0 || X>pfs->width-1 || Y>pfs->height-1)return;
	pathNode_struct* pn=&pfs->nodes[X+Y*pfs->width];
	// NOGBA("%d %d, %d",X,Y,pn->inList);
	if(pn->inList==noList)
	{
		addListNode(&pfs->openList,pn);
		pn->inList=openList;
		pn->parentX=ppn->X;
		pn->parentY=ppn->Y;
		pn->gCost=ppn->gCost+1;
		pn->fCost=abs(pn->X-pfs->target.x)+abs(pn->Y-pfs->target.y)+pn->gCost;
		// NOGBA("added : %d %d",pn->X,pn->Y);
	}else if(pn->inList==openList && pn->gCost-1>ppn->gCost)
	{
		pn->fCost-=pn->gCost;
		pn->gCost=ppn->gCost+1;
		pn->fCost+=pn->gCost;
		pn->parentX=ppn->X;
		pn->parentY=ppn->Y;
		// NOGBA("revised : %d %d",pn->X,pn->Y);
	}
}

bool findPath(pathfindingSystem_struct* pfs)
{
	if(!pfs)return false;
	addListNode(&pfs->openList,&pfs->nodes[pfs->origin.x+pfs->origin.y*pfs->width]);
	pfs->nodes[pfs->origin.x+pfs->origin.y*pfs->width].inList=openList;
	pfs->nodes[pfs->origin.x+pfs->origin.y*pfs->width].gCost=0;
	pfs->nodes[pfs->origin.x+pfs->origin.y*pfs->width].fCost=0;
	pathNode_struct* pn;
	while(1)
	{
		pn=getLowestFcost(&pfs->openList);
		if(!pn || (pn->X==pfs->target.x && pn->Y==pfs->target.y))break;
		unsigned char v=pfs->data[pn->X+pn->Y*pfs->width];
		if(v&DIRUP)addOpenList(pfs, pn, pn->X, pn->Y-1);
		if(v&DIRDOWN)addOpenList(pfs, pn, pn->X, pn->Y+1);
		if(v&DIRLEFT)addOpenList(pfs, pn, pn->X-1, pn->Y);
		if(v&DIRRIGHT)addOpenList(pfs, pn, pn->X+1, pn->Y);
		pn->inList=closedList;
		// NOGBA("mem free : %dko (%do)",getMemFree()/1024,getMemFree());
		// NOGBA("doing : %d %d %d (%d %d) (%d)",v,pn->X,pn->Y,lnCnt,pcCnt,pfs->nodes[0+11*pfs->width].inList);
	}
	// NOGBA("%p %d %d",pn,pn->X,pn->Y);
	if(pn)
	{
		while(!(pn->X==pfs->origin.x && pn->Y==pfs->origin.y))
		{
			addPathCell(&pfs->path, pn->X, pn->Y);
			pn=&pfs->nodes[pn->parentX+pn->parentY*pfs->width];
			// NOGBA("not popping");
		}
		return true;
	}
	return false;
}

void freePath(path_struct* p)
{
	if(!p)return;
	while(popPathCell(p,NULL,NULL));//NOGBA("popping");
	p->first=NULL;
	// free(p);
}

void freePathfinding(pathfindingSystem_struct* pfs)
{
	if(!pfs)return;
	
		// NOGBA("mem free : %dko (%do)",getMemFree()/1024,getMemFree());
	freeOpenList(&pfs->openList);
	// if(pfs->nodes)free(pfs->nodes);
}

void printPath(path_struct* p)
{
	if(!p)return;
	pathCell_struct* pc=p->first;
	while(pc)
	{
		NOGBA("%d %d",pc->X,pc->Y);
		pc=pc->next;
	}
}

pathCell_struct* getPath(room_struct* r, vect2D origin, vect2D target)
{
	if(!r)return NULL;
	if((origin.x==target.x&&origin.y==target.y)||origin.x<0||origin.y<0||target.x<0||target.y<0||origin.x>r->width-1||origin.y>r->height-1||target.x>r->width-1||target.y>r->height-1)return NULL;
	pathfindingSystem_struct pfs;
	if(!r->pathfindingData)getPathfindingData(r);
	if(!initPathfinding(&pfs, r->pathfindingData, r->width, r->height, origin, target)){freePathfinding(&pfs);return NULL;}
	// NOGBA("finding path... %d %d -> %d %d)",origin.x,origin.y,target.x,target.y);
	if(!findPath(&pfs)){freePathfinding(&pfs);return NULL;};
	// NOGBA("found path !");
	printPath(&pfs.path);
	freePathfinding(&pfs);
	return pfs.path.first;
}
