#include "editor/editor_main.h"

//implementation of http://www.drdobbs.com/database/the-maximal-rectangle-problem/184410529

typedef struct pileCell_struct
{
	vect2D data;
	struct pileCell_struct* next;
}pileCell_struct;

typedef struct
{
	pileCell_struct* first;
	int num;
}pile_struct;

void initPile(pile_struct* p)
{
	p->first=NULL;
	p->num=0;
}

void pushPile(vect2D v, pile_struct* p)
{
	pileCell_struct* pc=(pileCell_struct*)malloc(sizeof(pileCell_struct));
	pc->next=p->first;
	pc->data=v;
	p->first=pc;
	p->num++;
}

vect2D popPile(pile_struct* p)
{
	p->num--;
	if(p->first)
	{
		pileCell_struct* pc=p->first;
		vect2D v=pc->data;
		p->first=pc->next;
		free(pc);
		return v;
	}
	return vect2(0,0); //not cool
}

int area(vect2D origin, vect2D corner)
{
	if(corner.x<origin.x || corner.y<origin.y)return 0;
	return (corner.x-origin.x+1)*(corner.y-origin.y+1);
}

void fillCache(int* c, u8* d, int w, int h, int x)
{
	int y;
	for(y=0;y<h;y++)
	{
		if(d[x+y*w])c[y]++;
		else c[y]=0;
	}
}

int* initCache(int h)
{
	int* c=(int*)malloc(sizeof(int)*(h+1));
	int i;
	for(i=0;i<=h;i++)c[i]=0;
	return c;
}

void getMaxRectangle(u8* data, int w, int h, vect2D* pos, vect2D* size)
{
	int x;
	vect2D originb=vect2(0,0),cornerb=vect2(-1,-1);
	pile_struct p;
	initPile(&p);
	int* c=initCache(h);
	for(x=w-1;x>=0;x--)
	{
		int y, width=0;
		fillCache(c,data,w,h,x);
		for(y=0;y<=h;y++)
		{
			if(c[y]>width)
			{
				pushPile(vect2(y,width),&p);
				width=c[y];
			}else if(c[y]<width)
			{
				int y0, w0;
				while(1)
				{
					vect2D v=popPile(&p);
					y0=v.x;w0=v.y;
					if(width*(y-y0)>area(originb,cornerb))
					{
						originb=vect2(x,y0);
						cornerb=vect2(x+width-1,y-1);
					}
					width=w0;
					if(c[y]>=width)break;
				}
				width=c[y];
				if(width)pushPile(vect2(y0,w0),&p);
			}
		}
	}
	if(c)free(c);
	*pos=originb;
	*size=vect2(cornerb.x-originb.x+1,cornerb.y-originb.y+1);
}

void addEdgeRectangle(roomEdit_struct* re, vect2D* pos, vect2D* size, u8 edge, u8 reverse, int i, int k, int l, int old, material_struct* mat)
{
	switch(edge)
	{
		case 0:
			if(reverse)addRoomRectangle(&re->data,&re->entityCollection,(rectangle_struct){vect(i-l,k,pos->y),vect(l,old-k,0)},mat,true);
			else addRoomRectangle(&re->data,&re->entityCollection,(rectangle_struct){vect(i,k,pos->y),vect(-l,old-k,0)},mat,true);
			break;
		case 1:
			if(reverse)addRoomRectangle(&re->data,&re->entityCollection,(rectangle_struct){vect(i,k,pos->y+size->y),vect(-l,old-k,0)},mat,true);
			else addRoomRectangle(&re->data,&re->entityCollection,(rectangle_struct){vect(i-l,k,pos->y+size->y),vect(l,old-k,0)},mat,true);
			break;
		case 2:
			if(!reverse)addRoomRectangle(&re->data,&re->entityCollection,(rectangle_struct){vect(pos->x,k,i),vect(0,old-k,-l)},mat,true);
			else addRoomRectangle(&re->data,&re->entityCollection,(rectangle_struct){vect(pos->x,k,i-l),vect(0,old-k,l)},mat,true);
			break;
		case 3:
			if(!reverse)addRoomRectangle(&re->data,&re->entityCollection,(rectangle_struct){vect(pos->x+size->x,k,i-l),vect(0,old-k,l)},mat,true);
			else addRoomRectangle(&re->data,&re->entityCollection,(rectangle_struct){vect(pos->x+size->x,k,i),vect(0,old-k,-l)},mat,true);
			break;
		case 4:
			addRoomRectangle(&re->data,&re->entityCollection,(rectangle_struct){vect(i,k,pos->y),vect(-l,old-k,0)},mat,true);
			break;
		case 5:
			addRoomRectangle(&re->data,&re->entityCollection,(rectangle_struct){vect(i-l,k,pos->y+size->y),vect(l,old-k,0)},mat,true);
			break;
		case 6:
			addRoomRectangle(&re->data,&re->entityCollection,(rectangle_struct){vect(pos->x,k,i),vect(0,old-k,-l)},mat,true);
			break;
		case 7:
			addRoomRectangle(&re->data,&re->entityCollection,(rectangle_struct){vect(pos->x+size->x,k,i-l),vect(0,old-k,l)},mat,true);
			break;
	}
}

void scanEdge(roomEdit_struct* re, u8* data, int w, int h, vect2D* pos, vect2D* size, u8 edge, u8 reverse, bool ceil)
{
	material_struct* mat=re->data.materials[pos->x+pos->y*re->data.width];
	switch(edge)
	{
		case 0: //top
			if(pos->y>0)
			{
				int i;
				int l=0;
				u8 k=data[pos->x+(pos->y)*w];
				u8 old=data[pos->x+(pos->y-1)*w];
				for(i=pos->x;i<pos->x+size->x;i++)
				{
					u8 v=data[i+(pos->y-1)*w];
					if(v!=old)
					{
						//create rectangle
						if(l){addEdgeRectangle(re, pos, size, edge, reverse, i, k, l, old, mat);}
						l=0;
					}
					if(ceil){if(v>k)l++;}
					else if(v<k)l++;
					old=v;
				}
				if(l){addEdgeRectangle(re, pos, size, edge, reverse, i, k, l, old, mat);}
			}
			break;
		case 1: //bottom
			if(pos->y+size->y<h)
			{
				int i;
				int l=0;
				u8 k=data[pos->x+(pos->y)*w];
				u8 old=data[pos->x+(pos->y+size->y)*w];
				for(i=pos->x;i<pos->x+size->x;i++)
				{
					u8 v=data[i+(pos->y+size->y)*w];
					if(v!=old)
					{
						//create rectangle
						if(l){addEdgeRectangle(re, pos, size, edge, reverse, i, k, l, old, mat);}
						l=0;
					}
					if(ceil){if(v>k)l++;}
					else if(v<k)l++;
					old=v;
				}
				if(l){addEdgeRectangle(re, pos, size, edge, reverse, i, k, l, old, mat);}
			}
			break;
		case 2: //left
			if(pos->x>0)
			{
				int i;
				int l=0;
				u8 k=data[(pos->x)+(pos->y)*w];
				u8 old=data[pos->x-1+(pos->y)*w];
				for(i=pos->y;i<pos->y+size->y;i++)
				{
					u8 v=data[(pos->x-1)+(i)*w];
					if(v!=old)
					{
						//create rectangle
						if(l){addEdgeRectangle(re, pos, size, edge, reverse, i, k, l, old, mat);}
						l=0;
					}
					if(ceil){if(v>k)l++;}
					else if(v<k)l++;
					old=v;
				}
				if(l){addEdgeRectangle(re, pos, size, edge, reverse, i, k, l, old, mat);}
			}
			break;
		case 3: //right
			if(pos->x+size->x<w)
			{
				int i;
				int l=0;
				u8 k=data[(pos->x)+(pos->y)*w];
				u8 old=data[pos->x+size->x+(pos->y)*w];
				for(i=pos->y;i<pos->y+size->y;i++)
				{
					u8 v=data[(pos->x+size->x)+(i)*w];
					if(v!=old)
					{
						//create rectangle
						if(l){addEdgeRectangle(re, pos, size, edge, reverse, i, k, l, old, mat);}
						l=0;
					}
					if(ceil){if(v>k)l++;}
					else if(v<k)l++;
					old=v;
				}
				if(l){addEdgeRectangle(re, pos, size, edge, reverse, i, k, l, old, mat);}
			}
			break;
	}
}

static inline u8 getVEdgeWall(roomEdit_struct* re, int w, u8* ceiling, int i, int j, u8* hh)
{
	u8 v=ceiling[i+j*w];
	door_struct* d;
	if((d=doorTouches(NULL, re, i, j, false))){v+=MAXHEIGHT;(*hh)=d->height;}
	else if((d=doorTouches(NULL, re, i, j, true))){v+=MAXHEIGHT;(*hh)=d->height;}
	return v;
}

// static inline void addEdgeRectangleWall(re, pos, size, edge, hh, old, l, i, k)
static inline void addEdgeRectangleWall(roomEdit_struct* re, vect2D* pos, vect2D* size, u8 edge, u8 hh, u8 old, int l, int i, u8 k)
{
	if(old>MAXHEIGHT){addEdgeRectangle(re, pos, size, edge+4, 0, i, 12+hh, l, old-MAXHEIGHT, NULL);addEdgeRectangle(re, pos, size, edge+4, 0, i, k, l, hh, NULL);}
	else addEdgeRectangle(re, pos, size, edge+4, 0, i, k, l, old, NULL);
}

void scanEdgeWall(roomEdit_struct* re, u8* floor, u8* ceiling, int w, int h, vect2D* pos, vect2D* size, u8 edge)
{
	u8 hh=0;
	switch(edge)
	{
		case 0: //top
			{
				if(pos->y==0)
				{
					int i;
					int l=0;
					u8 k=floor[pos->x+(pos->y)*w];
					u8 old=getVEdgeWall(re, w, ceiling, pos->x, pos->y, &hh);
					for(i=pos->x;i<pos->x+size->x;i++)
					{
						u8 v=getVEdgeWall(re, w, ceiling, i, pos->y, &hh);
						if(v!=old)
						{
							//create rectangle
							if(l)addEdgeRectangleWall(re, pos, size, edge, hh, old, l, i, k);
							l=0;
						}
						l++;
						old=v;
						NOGBA("%d,%d,%d,%d",l,v,k,old);
					}
					if(l)addEdgeRectangleWall(re, pos, size, edge, hh, old, l, i, k);
				}
			}
			break;
		case 1: //top
			{
				if(pos->y+size->y==h)
				{
					int i;
					int l=0;
					u8 k=floor[pos->x+(pos->y+size->y-1)*w];
					u8 old=getVEdgeWall(re, w, ceiling, pos->x, pos->y+size->y-1, &hh);
					for(i=pos->x;i<pos->x+size->x;i++)
					{
						u8 v=getVEdgeWall(re, w, ceiling, i, (pos->y+size->y-1), &hh);
						if(v!=old)
						{
							//create rectangle
							if(l)addEdgeRectangleWall(re, pos, size, edge, hh, old, l, i, k);
							l=0;
						}
						l++;
						old=v;
					}
					if(l)addEdgeRectangleWall(re, pos, size, edge, hh, old, l, i, k);
				}
			}
			break;
		case 2: //left
			{
				if(pos->x==0)
				{
					int i;
					int l=0;
					u8 k=floor[pos->x+(pos->y)*w];
					u8 old=getVEdgeWall(re, w, ceiling, pos->x, pos->y, &hh);
					for(i=pos->y;i<pos->y+size->y;i++)
					{
						u8 v=getVEdgeWall(re, w, ceiling, pos->x, i, &hh);
						if(v!=old)
						{
							//create rectangle
							if(l)addEdgeRectangleWall(re, pos, size, edge, hh, old, l, i, k);
							l=0;
						}
						l++;
						old=v;
					}
					if(l)addEdgeRectangleWall(re, pos, size, edge, hh, old, l, i, k);
				}
			}
			break;
		case 3: //right
			{
				if(pos->x+size->x==w)
				{
					int i;
					int l=0;
					u8 k=floor[pos->x+size->x-1+(pos->y)*w];
					u8 old=getVEdgeWall(re, w, ceiling, pos->x+size->x-1, pos->y, &hh);
					for(i=pos->y;i<pos->y+size->y;i++)
					{
						u8 v=getVEdgeWall(re, w, ceiling, pos->x+size->x-1, i, &hh);
						if(v!=old)
						{
							//create rectangle
							if(l)addEdgeRectangleWall(re, pos, size, edge, hh, old, l, i, k);
							l=0;
						}
						l++;
						old=v;
					}
					if(l)addEdgeRectangleWall(re, pos, size, edge, hh, old, l, i, k);
				}
			}
			break;
	}
}

void scanEdgeInnerWall(roomEdit_struct* re, u8* floor, u8* ceiling, int w, int h, vect2D* pos, vect2D* size, u8 edge)
{
	switch(edge)
	{
		case 0: //top
			if(pos->y>0)
			{
				int i;
				int l=0;
				u8 k=floor[pos->x+(pos->y)*w];
				u8 old=ceiling[pos->x+(pos->y)*w];
				for(i=pos->x;i<pos->x+size->x;i++)
				{
					u8 v=ceiling[i+(pos->y)*w];
					if(floor[i+(pos->y-1)*w]>=ceiling[i+(pos->y-1)*w])
					{
						if(v!=old)
						{
							//create rectangle
							if(l){addEdgeRectangle(re, pos, size, edge+4, 0, i, k, l, old, NULL);}
							l=0;
						}
						l++;
					}else if(l){addEdgeRectangle(re, pos, size, edge+4, 0, i, k, l, old, NULL);l=0;}
					old=v;
				}
				if(l){addEdgeRectangle(re, pos, size, edge+4, 0, i, k, l, old, NULL);}
			}
			break;
		case 1: //bottom
			if(pos->y+size->y<h)
			{
				int i;
				int l=0;
				u8 k=floor[pos->x+(pos->y+size->y-1)*w];
				u8 old=ceiling[pos->x+(pos->y+size->y-1)*w];
				for(i=pos->x;i<pos->x+size->x;i++)
				{
					u8 v=ceiling[i+(pos->y+size->y-1)*w];
					if(floor[i+(pos->y+size->y)*w]>=ceiling[i+(pos->y+size->y)*w])
					{
						if(v!=old)
						{
							//create rectangle
							if(l){addEdgeRectangle(re, pos, size, edge+4, 0, i, k, l, old, NULL);}
							l=0;
						}
						l++;
					}else if(l){addEdgeRectangle(re, pos, size, edge+4, 0, i, k, l, old, NULL);l=0;}
					old=v;
				}
				if(l){addEdgeRectangle(re, pos, size, edge+4, 0, i, k, l, old, NULL);}
			}
			break;
		case 2: //left
			if(pos->x>0)
			{
				int i;
				int l=0;
				u8 k=floor[pos->x+(pos->y)*w];
				u8 old=ceiling[pos->x+(pos->y)*w];
				for(i=pos->y;i<pos->y+size->y;i++)
				{
					u8 v=ceiling[pos->x+(i)*w];
					if(floor[pos->x-1+(i)*w]>=ceiling[pos->x-1+(i)*w])
					{
						if(v!=old)
						{
							//create rectangle
							if(l){addEdgeRectangle(re, pos, size, edge+4, 0, i, k, l, old, NULL);}
							l=0;
						}
						l++;
					}else if(l){addEdgeRectangle(re, pos, size, edge+4, 0, i, k, l, old, NULL);l=0;}
					old=v;
				}
				if(l){addEdgeRectangle(re, pos, size, edge+4, 0, i, k, l, old, NULL);}
			}
			break;
		case 3: //bottom
			if(pos->x+size->x<w)
			{
				int i;
				int l=0;
				u8 k=floor[(pos->x+size->x-1)+pos->x*w];
				u8 old=ceiling[(pos->x+size->x-1)+pos->x*w];
				for(i=pos->y;i<pos->y+size->y;i++)
				{
					u8 v=ceiling[(pos->x+size->x-1)+i*w];
					if(floor[(pos->x+size->x)+i*w]>=ceiling[(pos->x+size->x)+i*w])
					{
						if(v!=old)
						{
							//create rectangle
							if(l){addEdgeRectangle(re, pos, size, edge+4, 0, i, k, l, old, NULL);}
							l=0;
						}
						l++;
					}else if(l){addEdgeRectangle(re, pos, size, edge+4, 0, i, k, l, old, NULL);l=0;}
					old=v;
				}
				if(l){addEdgeRectangle(re, pos, size, edge+4, 0, i, k, l, old, NULL);}
			}
			break;
	}
}

void fillRectangle(u8* data, int w, int h, vect2D* pos, vect2D* size)
{
	int i;
	for(i=pos->x;i<pos->x+size->x;i++)
	{
		int j;
		for(j=pos->y;j<pos->y+size->y;j++)
		{
			data[i+j*w]=0;
		}
	}
}

bool collideLineRectangle(rectangle_struct* rec, vect3D o, vect3D v, int32 d, int32* kk, vect3D* ip)
{
	int32 p1=dotProduct(v,rec->normal);
	if(!equals(p1,0))
	{
		vect3D p=convertVect(rec->position); //CHECK lightmap generation ?
		vect3D s=vect(rec->size.x*TILESIZE*2,rec->size.y*HEIGHTUNIT,rec->size.z*TILESIZE*2);
		
		int32 p2=dotProduct(vectDifference(p,o),rec->normal);
		int32 k=divf32(p2,p1);
		if(kk)*kk=k;
		if(k<0 || k>d){return false;}
		vect3D i=addVect(o,vectMult(v,k));
		if(ip)*ip=i;
		// NOGBA("ip : %d %d %d",i.x,i.y,i.z);
		i=vectDifference(i,p);
		
		bool r=true;
		if(s.x)
		{
			if(s.x>0)r=r&&i.x<s.x&&i.x>=0;
			else r=r&&i.x>s.x&&i.x<=0;
		}
		if(s.y)
		{
			if(s.y>0)r=r&&i.y<s.y&&i.y>=0;
			else r=r&&i.y>s.y&&i.y<=0;
		}
		if(s.z)
		{
			if(s.z>0)r=r&&i.z<s.z&&i.z>=0;
			else r=r&&i.z>s.z&&i.z<=0;
		}
		return r;
	}
	return false;
}

vect3D getClosestPointRectangle(rectangle_struct* rec, vect3D o)
{
	vect3D p=vect(rec->position.x*TILESIZE*2,rec->position.y*HEIGHTUNIT,rec->position.z*TILESIZE*2);
	vect3D s=vect(rec->size.x*TILESIZE*2,rec->size.y*HEIGHTUNIT,rec->size.z*TILESIZE*2);
	vect3D u1, u2;
	int32 x,y,sx,sy;
	
	// NOGBA("p: %d %d %d",p.x,p.y,p.z);
	// NOGBA("o: %d %d %d",o.x,o.y,o.z);
	
	if(s.x){sx=abs(s.x);u1=vect((s.x>0)?inttof32(1):(-inttof32(1)),0,0);}
	else{sx=abs(s.y);u1=vect(0,(s.y>0)?inttof32(1):(-inttof32(1)),0);}
	
	if(s.z){sy=abs(s.z);u2=vect(0,0,(s.z>0)?inttof32(1):(-inttof32(1)));}
	else{sy=abs(s.y);u2=vect(0,(s.y>0)?inttof32(1):(-inttof32(1)),0);}
	
	o=vectDifference(o,p);
	
	x=dotProduct(o,u1);y=dotProduct(o,u2);
	
	// NOGBA("x, y: %d %d",x,y);
	// NOGBA("sx, sy: %d %d",sx,sy);
	
	bool r=true;
	r=r&&x<sx&&x>=0;
	r=r&&y<sy&&y>=0;
	
	if(r)return addVect(p,vect(mulf32(x,u1.x)+mulf32(y,u2.x), mulf32(x,u1.y)+mulf32(y,u2.y), mulf32(x,u1.z)+mulf32(y,u2.z)));
	
	if(x<0)
	{
		x=0;
		if(y<0)y=0;
		else if(y>sy)y=sy;
	}else if(x>sx)
	{
		x=sx;
		if(y<0)y=0;
		else if(y>sy)y=sy;
	}else if(y<0)
	{
		y=0;
		if(x<0)x=0;
		else if(x>sx)y=sx;
	}else if(y>sy)
	{
		y=sy;
		if(x<0)x=0;
		else if(x>sx)x=sx;
	}
	
	return addVect(p,vect(mulf32(x,u1.x)+mulf32(y,u2.x), mulf32(x,u1.y)+mulf32(y,u2.y), mulf32(x,u1.z)+mulf32(y,u2.z)));
}

int area2D(rectangle2D_struct rec)
{
	return (rec.size.x*rec.size.y);
}

void initRectangle2DList(rectangle2DList_struct* l)
{
	l->first=NULL;
	l->surface=0;
	l->num=0;
}

listCell2D_struct* createListCell()
{
	listCell2D_struct* lc=(listCell2D_struct*)malloc(sizeof(listCell2D_struct));
	lc->next=NULL;
	return lc;
}

void insertRectangle2DList(rectangle2DList_struct* l, rectangle2D_struct rec)
{
	int a=rec.size.x*rec.size.y;
	listCell2D_struct* n=createListCell();
	n->data=rec;
	listCell2D_struct** plcn=&l->first;
	listCell2D_struct* lc=l->first;
	l->num++;
	l->surface+=a;
	while(lc)
	{
		int a2=lc->data.size.x*lc->data.size.y;
		if(a>a2)
		{
			*plcn=n;
			n->next=lc;
			return;
		}
		plcn=&lc->next;
		lc=lc->next;
	}
	*plcn=n;
	n->next=NULL;
}

void freeRectangle2DList(rectangle2DList_struct* l)
{
	listCell2D_struct* lc=l->first;
	listCell2D_struct* lcn=lc->next;
	while(lc)
	{
		lcn=lc->next;
		free(lc);
		lc=lcn;
	}
	l->first=NULL;
}

void initTreeNode(treeNode_struct* tn)
{
	tn->son[0]=tn->son[1]=NULL;
	tn->type=EMPTY;
	tn->data=0;
}

treeNode_struct* createNode()
{
	treeNode_struct* tn=(treeNode_struct*)malloc(sizeof(treeNode_struct));
	initTreeNode(tn);
	return tn;
}

void initTree(tree_struct* t, short w, short h)
{
	t->root=createNode();
	t->width=w;
	t->height=h;
}

void freeTreeBranch(treeNode_struct* tn)
{
	if(tn)
	{
		freeTreeBranch(tn->son[0]);
		freeTreeBranch(tn->son[1]);
		free(tn);
	}
}

void freeTree(tree_struct* t)
{
	if(t)
	{
		freeTreeBranch(t->root);
	}
	t->root=NULL;
}

bool insertRectangle(treeNode_struct* tn, rectangle2D_struct* rec, short w, short h, short x, short y, int a)
{
	bool r;
	if(a>w*h)return false;
	// NOGBA("%d %d %d %d",x,y,w,h);
	switch(tn->type)
	{
		case EMPTY:
			if((rec->size.x<=w && rec->size.y<=h) || (rec->size.y<=w && rec->size.x<=h))
			{
				// NOGBA("empty");
				if(!(rec->size.x<=w && rec->size.y<=h))
				{
					//SUPERROTATIOOOON
					int z=rec->size.x;
					rec->size.x=rec->size.y;
					rec->size.y=z;
					rec->rot=true;
				}else rec->rot=false;
				if(rec->size.x>=rec->size.y)
				{
					tn->type=VERTICAL;
					tn->data=rec->size.x;
					tn->son[0]=createNode();
						tn->son[0]->type=HORIZONTAL;
						tn->son[0]->data=rec->size.y;
						tn->son[0]->son[0]=createNode();
							tn->son[0]->son[0]->type=RECTANGLE;
						tn->son[0]->son[1]=createNode();
					tn->son[1]=createNode();
				}else{
					tn->type=HORIZONTAL;
					tn->data=rec->size.y;
					tn->son[0]=createNode();
						tn->son[0]->type=VERTICAL;
						tn->son[0]->data=rec->size.x;
						tn->son[0]->son[0]=createNode();
							tn->son[0]->son[0]->type=RECTANGLE;
						tn->son[0]->son[1]=createNode();
					tn->son[1]=createNode();
				}
				rec->position.x=x;
				rec->position.y=y;
				return true;
			}
			break;
		case VERTICAL:
			// NOGBA("vertical %d", tn->data);
			r=insertRectangle(tn->son[0], rec, tn->data, h, x, y, a);
			if(!r)r=insertRectangle(tn->son[1], rec, w-tn->data, h, x+tn->data, y, a);
			return r;
			break;
		case HORIZONTAL:
			// NOGBA("horizontal %d", tn->data);
			r=insertRectangle(tn->son[0], rec, w, tn->data, x, y, a);
			if(!r)r=insertRectangle(tn->son[1], rec, w, h-tn->data, x, y+tn->data, a);
			return r;
			break;
		case RECTANGLE:
			// NOGBA("rectangle");
			return false;
			break;
	}
	return false;
}

bool insertRectangles(tree_struct* t, rectangle2DList_struct* l)
{
	listCell2D_struct*lc=l->first;
	bool rr=true;
	while(lc)
	{
		bool r=insertRectangle(t->root, &lc->data, t->width, t->height, 0, 0, area2D(lc->data));
		NOGBA("r : %d",(int)r);
		if(r)
		{
			lc->data.real->lmPos.x=lc->data.position.x;
			lc->data.real->lmPos.y=lc->data.position.y;
			lc->data.real->rot=lc->data.rot;
			NOGBA("pos %p %d %d %d %d (%d)",lc->data.real,lc->data.real->lmPos.x,lc->data.real->lmPos.y,lc->data.size.x,lc->data.size.y,lc->data.rot);
		}else rr=false;
		lc=lc->next;
	}
	return rr;
}

void unrotateRectangles(rectangle2DList_struct* l)
{
	if(!l)return;
	listCell2D_struct*lc=l->first;
	while(lc)
	{
		rectangle2D_struct* rec=&lc->data;
		if(rec->rot)
		{
			int z=rec->size.x;
			rec->size.x=rec->size.y;
			rec->size.y=z;
			rec->rot=false;
			rec->real->rot=false;
		}
		lc=lc->next;
	}
}

void packRectangles(rectangle2DList_struct* l, short w, short h)
{
	tree_struct t;
	initTree(&t,w,h);
	insertRectangles(&t,l);
	freeTree(&t);
}

bool packRectanglesSize(rectangle2DList_struct* l, short* w, short* h)
{
	tree_struct t;
	bool tr=true;
	bool rr=true;
	*w=32;
	*h=32;
	while(rr && *w<=512 && *h<=256)
	{
		if(l->surface<=(*w)*(*h))
		{
			NOGBA("doing : %d %d",(*w),(*h));
			initTree(&t,*w,*h);
			bool r=insertRectangles(&t,l);
			freeTree(&t);
			if(r){rr=false;break;}
			unrotateRectangles(l);
		}
		if(tr)(*w)*=2;
		else (*h)*=2;
		tr^=1;
	}
	NOGBA("surface : %d <= %d",l->surface,(*w)*(*h));
	return !rr;
}

int32 pointSegmentDistanceAbs(vect3D o, vect3D p, vect3D v)
{
	if(v.x)
	{
		int32 k=o.x-p.x;
		if(k<0)k=p.x;
		else if(k>v.x)k=p.x+v.x;
		else k=o.x;
		return ((k-o.x)*(k-o.x)+(o.y-p.y)*(o.y-p.y)+(o.z-p.z)*(o.z-p.z));
	}else if(v.y)
	{
		int32 k=o.y-p.y;
		if(k<0)k=p.y;
		else if(k>v.y)k=p.y+v.y;
		else k=o.y;
		return ((k-o.y)*(k-o.y)+(o.x-p.x)*(o.x-p.x)+(o.z-p.z)*(o.z-p.z));
	}else if(v.z)
	{
		int32 k=o.z-p.z;
		if(k<0)k=p.z;
		else if(k>v.z)k=p.z+v.z;
		else k=o.z;
		return ((k-o.z)*(k-o.z)+(o.y-p.y)*(o.y-p.y)+(o.x-p.x)*(o.x-p.x));
	}
	return 0;
}

int32 pointSegmentDistance(vect3D o, vect3D p, vect3D v)
{
	if(v.x>=0 && v.y>=0 && v.z>=0)return pointSegmentDistanceAbs(o, p, v);
	else return pointSegmentDistanceAbs(o, addVect(p,v), vectMultInt(v,-1));
}

int32 pointRectangleDistance(vect3D o, rectangle_struct* rec)
{
	if(!rec->size.x)
	{
		int32 d1=pointSegmentDistance(o, rec->position, vect(0,rec->size.y,0));
		int32 d2=pointSegmentDistance(o, rec->position, vect(0,0,rec->size.z));
		int32 d3=pointSegmentDistance(o, addVect(rec->position,vect(0,0,rec->size.z)), vect(0,rec->size.y,0));
		int32 d4=pointSegmentDistance(o, addVect(rec->position,vect(0,rec->size.y,0)), vect(0,0,rec->size.z));
		return min(d1,min(d2,min(d3,d4)));
	}else if(!rec->size.y)
	{
		int32 d1=pointSegmentDistance(o, rec->position, vect(rec->size.x,0,0));
		int32 d2=pointSegmentDistance(o, rec->position, vect(0,0,rec->size.z));
		int32 d3=pointSegmentDistance(o, addVect(rec->position,vect(0,0,rec->size.z)), vect(rec->size.x,0,0));
		int32 d4=pointSegmentDistance(o, addVect(rec->position,vect(rec->size.x,0,0)), vect(0,0,rec->size.z));
		return min(d1,min(d2,min(d3,d4)));
	}else{
		int32 d1=pointSegmentDistance(o, rec->position, vect(rec->size.x,0,0));
		int32 d2=pointSegmentDistance(o, rec->position, vect(0,rec->size.y,0));
		int32 d3=pointSegmentDistance(o, addVect(rec->position,vect(0,rec->size.y,0)), vect(rec->size.x,0,0));
		int32 d4=pointSegmentDistance(o, addVect(rec->position,vect(rec->size.x,0,0)), vect(0,rec->size.y,0));
		return min(d1,min(d2,min(d3,d4)));
	}
}
