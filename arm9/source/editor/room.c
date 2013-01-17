#include "editor/editor_main.h"

roomEdit_struct roomEdits[NUMROOMEDITS];
u8 roomEditorMode=0;
u8 oldEditorMode=0;
roomEdit_struct *currentRoom, *selectedRoom, *oldSelectedRoom;
door_struct *selectedDoor;
entity_struct *currentEntity;
vect3D selectionOrigin,selectionSize;
u8 selectionMode, oldSelectionMode;

void initRoomEditor(void)
{
	int i;
	for(i=0;i<NUMROOMEDITS;i++)
	{
		initRoomEdit(&roomEdits[i]);
		roomEdits[i].id=i;
	}
	roomEditorMode=0;
	oldEditorMode=0;
	currentEntity=NULL;
	currentRoom=NULL;
	selectedRoom=oldSelectedRoom=NULL;
	selectionMode=oldSelectionMode=0;
}

void initRoomEdit(roomEdit_struct* re)
{
	if(re)
	{
		re->position=vect(0,0,0);
		re->size=vect(0,0,0);
		re->used=false;
		re->quadsUpToDate=false;
		re->lightUpToDate=false;
	}
}

void swapData(roomEdit_struct* re)
{
	if(!re)re=selectedRoom;
	if(re)re->floor^=1;
}

void moveData(s8 v, roomEdit_struct* re, vect3D* so, vect3D* ss)
{
	if(!v || selectionMode!=2)return;
	if(!re)re=selectedRoom;
	if(!so || !ss)
	{
		so=&selectionOrigin;
		ss=&selectionSize;
	}
	if(re && so && ss)
	{
		int i;
		u8* data=re->data.floor;
		if(!re->floor)data=re->data.ceiling;
		for(i=so->x;i<so->x+ss->x;i++)
		{
			int j;
			for(j=so->y;j<so->y+ss->y;j++)
			{
				if(data[i+j*re->data.width]+v>=MAXHEIGHT)data[i+j*re->data.width]=MAXHEIGHT;
				else if(data[i+j*re->data.width]+v<=0)data[i+j*re->data.width]=0;
				else data[i+j*re->data.width]+=v;
			}		
		}
		changeRoom(re,true);
	}
}

void applyMaterial(material_struct* mat, roomEdit_struct* re, vect3D* so, vect3D* ss)
{
	if(selectionMode!=2)return;
	if(!re)re=selectedRoom;
	if(!so || !ss)
	{
		so=&selectionOrigin;
		ss=&selectionSize;
	}
	if(re && so && ss)
	{
		int i;
		material_struct** data=re->data.materials;
		for(i=so->x;i<so->x+ss->x;i++)
		{
			int j;
			for(j=so->y;j<so->y+ss->y;j++)
			{
				data[i+j*re->data.width]=mat;
			}		
		}
		changeRoom(re,true);
	}
}

void changeRoom(roomEdit_struct*re, bool both){if(!re)return;if(both){re->quadsUpToDate=false;removeRectangles(&re->data);}re->lightUpToDate=false;}

void makeWall(roomEdit_struct* re, vect3D* so, vect3D* ss)
{
	if(!re)re=selectedRoom;
	if(re)
	{
		bool r=re->floor;
		re->floor=false;
		moveData(-31,re,so,ss);
		re->floor=r;
		changeRoom(re,true);
	}
}

void getRoomDoorWays(void)
{
	int i;
	for(i=0;i<NUMROOMEDITS;i++)
	{
		if(roomEdits[i].used)getDoorWayData(NULL, &roomEdits[i].data);
	}
	
}

roomEdit_struct* createRoomEdit(vect3D position, vect3D size)
{
	int i;
	for(i=0;i<NUMROOMEDITS;i++)
	{
		if(!roomEdits[i].used)
		{
			roomEdit_struct* re=&roomEdits[i];
			re->used=true;
			re->floor=true;
			re->data.floor=NULL;
			re->data.ceiling=NULL;
			re->selected=false;
			re->position=position;
			re->quadsUpToDate=false;
			re->lightUpToDate=false;
			re->size=size;
			re->id=i;
			initEntityCollection(&re->entityCollection);
			
			return re;
		}
	}
	return NULL;
}

void deleteRoomEdit(roomEdit_struct* re)
{
	if(!re){re=selectedRoom;selectedRoom=NULL;}
	if(re)
	{
		wipeEntityCollection(&re->entityCollection);
		freeRoom(&re->data);
		re->used=false;
	}
}

void wipeMapEdit(void)
{
	int i;
	for(i=0;i<NUMROOMEDITS;i++)
	{
		if(roomEdits[i].used)deleteRoomEdit(&roomEdits[i]);
	}
	initDoorCollection(NULL); //TEMP?
}

void drawRectangle(vect3D position, vect3D size, u16 width, bool cull)
{
	if(cull)glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);
	else glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
	glPushMatrix();
		glTranslate3f32(position.x*(1<<9),position.y*(1<<9),position.z);
		
		glPushMatrix();	
			glScalef32(size.x*(1<<9),inttof32(1),inttof32(1));
			glBegin(GL_QUADS);
				glVertex3v16(0, width, 0);
				glVertex3v16(inttof32(1), width, 0);
				glVertex3v16(inttof32(1), 0, 0);
				glVertex3v16(0, 0, 0);
		glPopMatrix(1);
		
		glPushMatrix();
			glTranslate3f32(0,size.y*(1<<9),0);
			glScalef32(size.x*(1<<9),inttof32(1),inttof32(1));	
			glBegin(GL_QUADS);
				glVertex3v16(0, 0, 0);
				glVertex3v16(inttof32(1), 0, 0);
				glVertex3v16(inttof32(1), -width, 0);
				glVertex3v16(0, -width, 0);
		glPopMatrix(1);
		
		glPushMatrix();	
			glScalef32(inttof32(1),size.y*(1<<9),inttof32(1));	
			glBegin(GL_QUADS);
				glVertex3v16(0, 0, 0);
				glVertex3v16(0, inttof32(1), 0);
				glVertex3v16(width, inttof32(1), 0);
				glVertex3v16(width, 0, 0);
		glPopMatrix(1);
		
		glPushMatrix();	
			glTranslate3f32(size.x*(1<<9),0,0);	
			glScalef32(inttof32(1),size.y*(1<<9),inttof32(1));	
			glBegin(GL_QUADS);
				glVertex3v16(-width, 0, 0);
				glVertex3v16(-width, inttof32(1), 0);
				glVertex3v16(0, inttof32(1), 0);
				glVertex3v16(0, 0, 0);
		glPopMatrix(1);
	glPopMatrix(1);
}

void drawRoomEdit(roomEdit_struct* re)
{
	if(re && re->used)
	{
		unbindMtl();
		if(re->selected)glColor3b(255,0,0);
		else glColor3b(255,201,14);
		
		drawRectangle(vect(re->position.x,re->position.y,32), re->size, OUTLINEWIDTH, true);
		
		drawRoomDoors(NULL, re);
		
		if(re->selected)
		{
			glPushMatrix();
				glTranslate3f32(re->position.x*(1<<9),re->position.y*(1<<9),0);
				int w=min(re->data.width,re->size.x);
				int h=min(re->data.height,re->size.y);
				int j;
				for(j=0;j<h;j++)
				{
					int i;
					for(i=0;i<w;i++)
					{
						if(re->data.floor[i+j*re->data.width]>=re->data.ceiling[i+j*re->data.width])
						{
							//wall
							drawTile(i, j, RGB15(22,0,0));
						}else{
							int v;
							if(re->floor)v=re->data.floor[i+j*re->data.width];
							else v=re->data.ceiling[i+j*re->data.width];
							drawTile(i, j, RGB15(v,v,v));
						}
					}
					for(;i<re->size.x;i++)
					{
						drawTile(i, j, RGB15(0,0,0));
					}
				}
				for(j=0;j<re->size.y;j++)
				{
					int i;
					for(i=0;i<re->size.x;i++)
					{
						drawTile(i, j, RGB15(0,0,0));
					}
				}
				drawEntityCollection(&re->entityCollection);
			glPopMatrix(1);
		}
		
		/*glBegin(GL_QUADS);
			glColor3b(200,200,0);
		
			glVertex3v16(0, inttof32(1), 0);
			glVertex3v16(inttof32(1), inttof32(1), 0);
			glVertex3v16(inttof32(1), 0, 0);
			glVertex3v16(0, 0, 0);
		
		glPopMatrix(1);*/
	}
}

void drawRoomEdits(void)
{
	int i;
	for(i=0;i<NUMROOMEDITS;i++)
	{
		if(roomEdits[i].used)drawRoomEdit(&roomEdits[i]);
	}
	if(selectionMode==2 && selectedRoom)
	{
		unbindMtl();
		GFX_COLOR=RGB15(0,31,0);
		drawRectangle(vect(selectionOrigin.x+selectedRoom->position.x,selectionOrigin.y+selectedRoom->position.y,48), selectionSize, SELECTIONOUTLINEWIDTH, true);
	}
}

void drawRoomsPreview(void)
{
	int i;
	unbindMtl();
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
	for(i=0;i<NUMROOMEDITS;i++)
	{
		if(roomEdits[i].used)
		{
			drawRoom(&roomEdits[i].data,((roomEdits[i].lightUpToDate&&roomEdits[i].selected)<<1)|(roomEdits[i].quadsUpToDate));
			glPushMatrix();
				glTranslate3f32(TILESIZE*2*roomEdits[i].data.position.x, 0, TILESIZE*2*roomEdits[i].data.position.y);
				renderEntityCollection(&roomEdits[i].entityCollection);
			glPopMatrix(1);
		}
	}
}

void drawRoomsGame(u8 mode)
{
	int i;
	unbindMtl();
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
	for(i=0;i<NUMROOMEDITS;i++)
	{
		if(roomEdits[i].used)
		{
			drawRoom(&roomEdits[i].data,((1)<<3)|((roomEdits[i].data.lmSlot!=0)<<2)|(1)|(mode));
			// glPushMatrix();
				// glTranslate3f32(TILESIZE*2*roomEdits[i].data.position.x, 0, TILESIZE*2*roomEdits[i].data.position.y);
				// renderEntityCollection(&roomEdits[i].entityCollection);
			// glPopMatrix(1);
			glPushMatrix();
				glTranslate3f32(TILESIZE*2*roomEdits[i].data.position.x, 0, TILESIZE*2*roomEdits[i].data.position.y);
				renderRoomDoors(NULL,&roomEdits[i].data);
			glPopMatrix(1);
		}
	}
}

static inline bool collideRectangles(vect3D p1, vect3D s1, vect3D p2, vect3D s2)
{
	return (!(p1.y+s1.y <= p2.y
			|| p2.y+s2.y <= p1.y
			|| p1.x+s1.x <= p2.x
			|| p2.x+s2.x <= p1.x));
}

bool collideEditorRoom(roomEdit_struct* re1)
{
	int i;
	for(i=0;i<NUMROOMEDITS;i++)
	{
		if(roomEdits[i].used && re1!=&roomEdits[i])
		{
			roomEdit_struct* re2=&roomEdits[i];
			if(collideRectangles(re1->position,re1->size,re2->position,re2->size))return true;			
		}
	}
	return false;
}

s16 getRoomID(room_struct* r)
{
	int i;
	for(i=0;i<NUMROOMEDITS;i++)
	{
		if(roomEdits[i].used && &roomEdits[i].data==r)return roomEdits[i].id;
	}
	return -1;
}

room_struct* getRoomByID(s16 id)
{
	int i;
	for(i=0;i<NUMROOMEDITS;i++)
	{
		if(roomEdits[i].used && roomEdits[i].id==id)return &roomEdits[i].data;
	}
	return NULL;
}

entityCollection_struct* getEntityCollection(room_struct* r)
{
	int i;
	for(i=0;i<NUMROOMEDITS;i++)
	{
		if(roomEdits[i].used && &roomEdits[i].data==r)return &roomEdits[i].entityCollection;
	}
	return NULL;
}

roomEdit_struct* roomEditorCollision(int x, int y, bool corner)
{
	int i;
	switch(corner)
	{
		case true:
			for(i=0;i<NUMROOMEDITS;i++)
			{
				if(roomEdits[i].used)
				{
					roomEdit_struct* re=&roomEdits[i];
					if(x==re->position.x && y==re->position.y)
					{
						re->origin.x=re->position.x+re->size.x-1;
						re->origin.y=re->position.y+re->size.y-1;
						return re;
					}else if(x==re->position.x+re->size.x-1 && y==re->position.y)
					{
						re->origin.x=re->position.x;
						re->origin.y=re->position.y+re->size.y-1;
						return re;
					}else if(x==re->position.x+re->size.x-1 && y==re->position.y+re->size.y-1)
					{
						re->origin.x=re->position.x;
						re->origin.y=re->position.y;
						return re;
					}else if(x==re->position.x && y==re->position.y+re->size.y-1)
					{
						re->origin.x=re->position.x+re->size.x-1;
						re->origin.y=re->position.y;
						return re;
					}
				}
			}
			break;
		default:
			for(i=0;i<NUMROOMEDITS;i++)
			{
				if(roomEdits[i].used)
				{
					roomEdit_struct* re=&roomEdits[i];
					if(x>=re->position.x && y>=re->position.y && x<re->position.x+re->size.x && y<re->position.y+re->size.y)return re;
				}
			}
			break;
	}
	return NULL;
}

void setRoomEditorMode(u8 mode)
{
	oldEditorMode=roomEditorMode;
	roomEditorMode=mode;
}

void optimizeRoom(roomEdit_struct* re)
{
	if(!re)return;
	// confirmRoomDoors(NULL, &re->data);
	confirmDoors(NULL);
	getSecondaryRooms(NULL);
	if(!re->quadsUpToDate)
	{
		removeRectangles(&re->data);
		u8 heights[(MAXHEIGHT+1)*NUMMATERIALS];
		int i, j;
		u8* f=re->data.floor;
		u8* c=re->data.ceiling;
		material_struct** m=re->data.materials;
		for(i=0;i<(MAXHEIGHT+1)*NUMMATERIALS;i++)heights[i]=0;
		for(i=0;i<re->data.width*re->data.height;i++){heights[(*f)+(getMaterialID(*m))*(MAXHEIGHT+1)]|=1;heights[(*c)+(getMaterialID(*m))*(MAXHEIGHT+1)]|=1<<1;f++;c++;m++;}
		f=malloc(re->data.width*re->data.height);c=malloc(re->data.width*re->data.height);
		for(j=0;j<NUMMATERIALS;j++)
		{
			for(i=0;i<=MAXHEIGHT;i++)
			{
				if(heights[i+j*(MAXHEIGHT+1)]&1)
				{
					int y;
					int k=0, n=0;
					for(y=0;y<re->data.height;y++)
					{
						int x;
						for(x=0;x<re->data.width;x++)
						{
							if(re->data.floor[k]==i && re->data.ceiling[k]>re->data.floor[k] && getMaterialID(re->data.materials[k])==j){f[k]=1;n++;}
							else f[k]=0;
							k++;
						}
					}
					while(n>0)
					{
						vect2D p, s;
						getMaxRectangle(f, re->data.width, re->data.height, &p, &s);
						addRoomRectangle(&re->data,&re->entityCollection,(rectangle_struct){vect(p.x,i,p.y),vect(s.x,0,s.y)},NULL,true);
						scanEdge(re, re->data.floor, re->data.width, re->data.height, &p, &s, 0, 0, false);
						scanEdge(re, re->data.floor, re->data.width, re->data.height, &p, &s, 1, 0, false);
						scanEdge(re, re->data.floor, re->data.width, re->data.height, &p, &s, 2, 0, false);
						scanEdge(re, re->data.floor, re->data.width, re->data.height, &p, &s, 3, 0, false);
						scanEdgeWall(re, re->data.floor, re->data.ceiling, re->data.width, re->data.height, &p, &s, 0);
						scanEdgeWall(re, re->data.floor, re->data.ceiling, re->data.width, re->data.height, &p, &s, 1);
						scanEdgeWall(re, re->data.floor, re->data.ceiling, re->data.width, re->data.height, &p, &s, 2);
						scanEdgeWall(re, re->data.floor, re->data.ceiling, re->data.width, re->data.height, &p, &s, 3);
						scanEdgeInnerWall(re, re->data.floor, re->data.ceiling, re->data.width, re->data.height, &p, &s, 0);
						scanEdgeInnerWall(re, re->data.floor, re->data.ceiling, re->data.width, re->data.height, &p, &s, 1);
						scanEdgeInnerWall(re, re->data.floor, re->data.ceiling, re->data.width, re->data.height, &p, &s, 2);
						scanEdgeInnerWall(re, re->data.floor, re->data.ceiling, re->data.width, re->data.height, &p, &s, 3);
						fillRectangle(f, re->data.width, re->data.height, &p, &s);
						n-=s.x*s.y;
					}
				}
				if(heights[i+j*(MAXHEIGHT+1)]&(1<<1))
				{
					int y;
					int k=0, n=0;
					for(y=0;y<re->data.height;y++)
					{
						int x;
						for(x=0;x<re->data.width;x++)
						{
							if(re->data.ceiling[k]==i && re->data.ceiling[k]>re->data.floor[k] && getMaterialID(re->data.materials[k])==j){f[k]=1;n++;}
							else f[k]=0;
							k++;
						}
					}
					while(n>0)
					{
						vect2D p, s;
						getMaxRectangle(f, re->data.width, re->data.height, &p, &s);
						addRoomRectangle(&re->data,&re->entityCollection,(rectangle_struct){vect(p.x+s.x,i,p.y),vect(-s.x,0,s.y)},NULL,true);
						scanEdge(re, re->data.ceiling, re->data.width, re->data.height, &p, &s, 0, 1, true);
						scanEdge(re, re->data.ceiling, re->data.width, re->data.height, &p, &s, 1, 1, true);
						scanEdge(re, re->data.ceiling, re->data.width, re->data.height, &p, &s, 2, 1, true);
						scanEdge(re, re->data.ceiling, re->data.width, re->data.height, &p, &s, 3, 1, true);
						fillRectangle(f, re->data.width, re->data.height, &p, &s);
						n-=s.x*s.y;
					}
				}
			}
		}
		if(f)free(f);
		if(c)free(c);
		re->quadsUpToDate=true;
	}
	generateLightmaps(re, &re->data,&re->entityCollection);
	NOGBA("mem free : %dko (%do)",getMemFree()/1024,getMemFree());
}

void getDoorPosition(roomEdit_struct* re, int x, int y, doorDirection* d, u16* p)
{
	u16 d1=abs(y);
	u16 d2=abs(re->size.y-y);
	u16 d3=abs(x);
	u16 d4=abs(re->size.x-x);
	if(d1<=d2 && d1<=d3 && d1<=d4)
	{
		*p=min(abs(x),re->size.x-1-DOORSIZE);
		*d=doorUp;
	}else if(d2<=d1 && d2<=d3 && d2<=d4)
	{
		*p=min(abs(x),re->size.x-1-DOORSIZE);
		*d=doorDown;
	}else if(d3<=d1 && d3<=d2 && d3<=d4)
	{
		*p=min(abs(y),re->size.y-1-DOORSIZE);
		*d=doorLeft;
	}else{
		*p=min(abs(y),re->size.y-1-DOORSIZE);
		*d=doorRight;
	}
	if(*p<1)*p=1;
}

int oldpx, oldpy;
extern int32 gridScale;
bool moving=false;

void updateRoomEditor(int px, int py)
{
	oldSelectedRoom=selectedRoom;
	oldSelectionMode=selectionMode;
	if(selectionMode!=3 && currentEntity){currentEntity->selected=false;currentEntity=NULL;}
	switch(roomEditorMode)
	{
		case ROOMCREATION:
			if(keysDown()&(KEY_TOUCH) && !collideGui(NULL,px,py))
			{
				int x, y;
				getGridCell(&x,&y,px,py);
				roomEdit_struct* re=roomEditorCollision(x,y,false);
				if(!re)
				{
					currentRoom=createRoomEdit(vect(x,y,0),vect(1,1,0));
					currentRoom->origin=currentRoom->position;
					initRoom(&currentRoom->data,0,0,currentRoom->position);
					setRoomEditorMode(ROOMSIZING);
				}
			}
			break;
		case ROOMSIZING:
			if(currentRoom && (keysHeld()&KEY_TOUCH))
			{
				int x, y, sx, sy;
				vect3D rPos=currentRoom->position;
				vect3D rSize=currentRoom->size;
				
				getGridCell(&x,&y,px,py);
				sx=x-currentRoom->origin.x;
				sy=y-currentRoom->origin.y;
				
				if(sx>0){sx++;currentRoom->position.x=currentRoom->origin.x;}
				else{
					sx=abs(sx)+1;
					currentRoom->position.x=x;
				}
				currentRoom->size.x=sx;
				if(collideEditorRoom(currentRoom)){currentRoom->position=rPos;currentRoom->size=rSize;}
				if(sy>0){sy++;currentRoom->position.y=currentRoom->origin.y;}
				else{
					sy=abs(sy)+1;
					currentRoom->position.y=y;
				}
				currentRoom->size.y=sy;
				if(collideEditorRoom(currentRoom)){currentRoom->position.y=rPos.y;currentRoom->size.y=rSize.y;}
			}else{selectionMode=0;changeRoom(currentRoom,true);resizeRoom(&currentRoom->data,currentRoom->size.x,currentRoom->size.y,currentRoom->position);roomEditorMode=oldEditorMode;}
			break;
		case ROOMSELECTMOVE:
			if(!collideGui(NULL,px,py))
			{
				if(keysDown()&(KEY_TOUCH))
				{
					int x, y;
					getGridCell(&x,&y,px,py);
					roomEdit_struct* re=roomEditorCollision(x,y,false);
					if(re)
					{
						currentRoom=re;
						currentRoom->origin.x=x-currentRoom->position.x;
						currentRoom->origin.y=y-currentRoom->position.y;
						setRoomEditorMode(ROOMMOVING);
						moving=false;
					}else moving=true;
					oldpx=px;
					oldpy=py;
				}else if(keysHeld()&(KEY_TOUCH) && moving)
				{
					int vx=divf32((px-oldpx)*(1<<6),gridScale),vy=divf32((py-oldpy)*(1<<6),gridScale);
					translateGrid(vect(vx,vy,0));
					oldpx=px;
					oldpy=py;
				}
			}else moving=false;
			break;
		case ROOMMOVING:
			if(currentRoom && (keysHeld()&KEY_TOUCH))
			{
				int x, y;
				getGridCell(&x,&y,px,py);
				vect3D rPos=currentRoom->position;
				currentRoom->position.x=x-currentRoom->origin.x;
				if(collideEditorRoom(currentRoom)){currentRoom->position=rPos;}
				currentRoom->position.y=y-currentRoom->origin.y;
				if(collideEditorRoom(currentRoom)){currentRoom->position.y=rPos.y;}
				currentRoom->data.position=currentRoom->position;
			}else roomEditorMode=oldEditorMode;
			break;
		case ROOMSELECTRESIZE:
			if(keysHeld()&(KEY_TOUCH))
			{
				int x, y;
				getGridCell(&x,&y,px,py);
				roomEdit_struct* re=roomEditorCollision(x,y,true);
				if(re)
				{
					currentRoom=re;
					setRoomEditorMode(ROOMSIZING);
				}
			}
			break;
		case ROOMSELECTION:
			if(keysDown()&(KEY_TOUCH) && !collideGui(NULL,px,py))
			{
				int x, y;
				getGridCell(&x,&y,px,py);
				roomEdit_struct* os=selectedRoom;
				if(selectedRoom)selectedRoom->selected=false;
				selectedRoom=roomEditorCollision(x,y,false);
				if(selectedRoom){selectedRoom->selected=true;if(os!=selectedRoom && selectedRoom->lightUpToDate)loadToBank(selectedRoom->data.lightMap,selectedRoom->data.lightMapBuffer);}
				else selectionMode=0;
			}
			break;
		case TILESELECTION:
			if(!selectedRoom){selectionMode=0;roomEditorMode=oldEditorMode;}
			else{
				if(keysDown()&(KEY_TOUCH) && !collideGui(NULL,px,py))
				{
					int x, y;
					getGridCell(&x,&y,px,py);
					selectionOrigin=vect(x,y,0);
					selectionSize=vect(1,1,0);
					selectionMode=1;					
				}else if(keysHeld()&(KEY_TOUCH)&&selectionMode==1)
				{
					int x, y;
					getGridCell(&x,&y,px,py);
					selectionSize.x=x-selectionOrigin.x;
					selectionSize.y=y-selectionOrigin.y;
					if(selectionSize.x>=0)selectionSize.x++;
					if(selectionSize.y>=0)selectionSize.y++;
					GFX_COLOR=RGB15(0,31,0);
					unbindMtl();
					drawRectangle(vect(selectionOrigin.x,selectionOrigin.y,48), selectionSize, SELECTIONOUTLINEWIDTH, false);
				}else if(keysUp()&(KEY_TOUCH)&&selectionMode==1)
				{
					selectionMode=2;
					if(selectionSize.x<0){selectionOrigin.x+=selectionSize.x;selectionSize.x=-selectionSize.x;}
					if(selectionSize.y<0){selectionOrigin.y+=selectionSize.y;selectionSize.y=-selectionSize.y;}
					
					// if(!collideRectangles(selectionOrigin,selectionSize,selectedRoom->position,selectedRoom->size)){optimizeRoom(selectedRoom);selectionMode=0;}
					if(!collideRectangles(selectionOrigin,selectionSize,selectedRoom->position,selectedRoom->size)){selectionMode=0;}
					else
					{
						if(selectionSize.x+selectionOrigin.x>selectedRoom->position.x+selectedRoom->size.x)selectionSize.x+=selectedRoom->position.x+selectedRoom->size.x-(selectionSize.x+selectionOrigin.x);
						if(selectionSize.y+selectionOrigin.y>selectedRoom->position.y+selectedRoom->size.y)selectionSize.y+=selectedRoom->position.y+selectedRoom->size.y-(selectionSize.y+selectionOrigin.y);
						if(selectionOrigin.x<selectedRoom->position.x)
						{
							selectionSize.x-=selectedRoom->position.x-selectionOrigin.x;
							selectionOrigin.x=selectedRoom->position.x;
						}
						if(selectionOrigin.y<selectedRoom->position.y)
						{
							selectionSize.y-=selectedRoom->position.y-selectionOrigin.y;
							selectionOrigin.y=selectedRoom->position.y;
						}
						selectionOrigin.x-=selectedRoom->position.x;
						selectionOrigin.y-=selectedRoom->position.y;
					}
				}
			}
			break;
		case ENTITYSELECTION:
			if(!selectedRoom){selectionMode=0;roomEditorMode=oldEditorMode;}
			else{
				if(keysHeld()&(KEY_TOUCH) && !collideGui(NULL,px,py))
				{
					int x, y;
					getGridCell(&x,&y,px,py);
					selectedDoor=false;
					if(currentEntity)currentEntity->selected=false;
					currentEntity=collideEntityCollection(&selectedRoom->entityCollection,x-selectedRoom->position.x,y-selectedRoom->position.y);
					if(currentEntity){currentEntity->selected=true;setRoomEditorMode(ENTITYMOVING);selectionMode=3;}
					else{
						selectionMode=0;
						x-=selectedRoom->position.x;
						y-=selectedRoom->position.y;
						selectedDoor=doorTouches(NULL,selectedRoom,x,y,false);
						if(selectedDoor)
						{
							setRoomEditorMode(DOORMOVING);
						}
					}
				}
			}
			break;
		case ENTITYMOVING:
			if(!selectedRoom||!currentEntity){selectionMode=0;roomEditorMode=oldEditorMode;}
			else{
				if(keysHeld()&(KEY_TOUCH))
				{
					int x, y;
					getGridCell(&x,&y,px,py);
					vect3D op=currentEntity->position;
					currentEntity->position.x=x-selectedRoom->position.x;
					currentEntity->position.y=y-selectedRoom->position.y;
					if(currentEntity->position.x<0)currentEntity->position.x=0;
					if(currentEntity->position.y<0)currentEntity->position.y=0;
					if(currentEntity->position.x>=selectedRoom->size.x)currentEntity->position.x=selectedRoom->size.x-1;
					if(currentEntity->position.y>=selectedRoom->size.y)currentEntity->position.y=selectedRoom->size.y-1;
					if(op.x!=currentEntity->position.x||op.y!=currentEntity->position.y)changeRoom(selectedRoom,false);
				}else roomEditorMode=oldEditorMode;
			}
			break;
		case DOORMOVING:
			if(!selectedRoom||!selectedDoor){selectionMode=0;roomEditorMode=oldEditorMode;}
			else{
				if(keysHeld()&(KEY_TOUCH))
				{
					int x, y;
					getGridCell(&x,&y,px,py);
					x-=selectedRoom->position.x;
					y-=selectedRoom->position.y;
					doorDirection d=doorUp;
					u16 p=0;
					getDoorPosition(selectedRoom, x, y, &d, &p);
					if(!doorCollides(NULL, &selectedRoom->data, d, p, selectedDoor))
					{
						selectedDoor->position=p;
						selectedDoor->direction=d;
						changeRoom(selectedRoom,true);
					}
				}else roomEditorMode=oldEditorMode;
			}
			break;
		case LIGHTCREATION:
			if(!selectedRoom){selectionMode=0;roomEditorMode=oldEditorMode;}
			else{
				if(keysDown()&(KEY_TOUCH) && !collideGui(NULL,px,py))
				{
					int x, y;
					getGridCell(&x,&y,px,py);
					x-=selectedRoom->position.x;
					y-=selectedRoom->position.y;
					if(x>=0&&y>=0&&x<selectedRoom->size.x&&y<selectedRoom->size.y)
					{
						createLight(&selectedRoom->entityCollection,vect(x,y,16),TILESIZE*24);
						changeRoom(selectedRoom,false);
					}
				}
			}
			break;
		case ENEMYCREATION:
			if(!selectedRoom){selectionMode=0;roomEditorMode=oldEditorMode;}
			else{
				if(keysDown()&(KEY_TOUCH) && !collideGui(NULL,px,py))
				{
					int x, y;
					getGridCell(&x,&y,px,py);
					x-=selectedRoom->position.x;
					y-=selectedRoom->position.y;
					if(x>=0&&y>=0&&x<selectedRoom->size.x&&y<selectedRoom->size.y)
					{
						createEnemy(&selectedRoom->entityCollection,vect(x,y,16),0);
					}
				}
			}
			break;
		case DOORCREATION:
			if(!selectedRoom){selectionMode=0;roomEditorMode=oldEditorMode;}
			else{
				if(keysDown()&(KEY_TOUCH) && !collideGui(NULL,px,py))
				{
					int x, y;
					getGridCell(&x,&y,px,py);
					x-=selectedRoom->position.x;
					y-=selectedRoom->position.y;
					if(x>=0&&y>=0&&x<selectedRoom->size.x&&y<selectedRoom->size.y)
					{
						doorDirection d=doorUp;
						u16 p=0;
						getDoorPosition(selectedRoom, x, y, &d, &p);
						createDoor(NULL, &selectedRoom->data, p, d, false);
						changeRoom(selectedRoom,true);
					}
				}
			}
			break;
		default:
			roomEditorMode=0;
			break;
	}
}

//WRITE AREA

void writeRectangle(rectangle_struct* rec, FILE* f)
{
	if(!rec || !f)return;
	
	writeVect(&rec->position,f);
	writeVect(&rec->size,f);
	writeVect(&rec->lmSize,f);
	writeVect(&rec->lmPos,f);
	writeVect(&rec->normal,f);
	
	u16 mid=getMaterialID(rec->material);
	
	fwrite(&mid,sizeof(u16),1,f);
	fwrite(&rec->rot,sizeof(bool),1,f);
}

void writeRectangles(room_struct* r, FILE* f)
{
	if(!r || !f)return;
	listCell_struct *lc=r->rectangles.first;
	while(lc)
	{
		writeRectangle(&lc->data, f);
		lc=lc->next;
	}
}

void writeEntity(entity_struct* e, FILE* f)
{
	if(!e || !f)return;
	
	fwrite(&e->type,sizeof(entity_type),1,f);
	writeVect(&e->position,f);
	
	switch(e->type)
	{
		case lightEntity:
			fwrite(e->data,sizeof(lightData_struct),1,f);
			break;
		case enemyEntity:
			fwrite(e->data,sizeof(enemyData_struct),1,f);
			break;
	}
}

void writeEntityCollection(entityCollection_struct* ec, FILE* f)
{
	if(!ec || !f)return;
	int i;
	for(i=0;i<ENTITYCOLLECTIONNUM;i++)
	{
		if(ec->entity[i].used)
		{
			writeEntity(&ec->entity[i], f);
		}
	}
}

void convertMaterialArray(roomEdit_struct* re, u16* mat)
{
	int i, j;
	for(i=0;i<re->size.x;i++)
	{
		for(j=0;j<re->size.y;j++)
		{
			mat[i+j*re->size.x]=getMaterialID(re->data.materials[i+j*re->size.x]);
		}
	}
}

void writeRoom(roomEdit_struct* re, FILE* f)
{
	if(!re || !f)return;
	if(!re->quadsUpToDate || !re->lightUpToDate){return;}//temp
	
	fwrite(&re->id, sizeof(s16), 1, f);
	
	writeVect(&re->position,f);
	writeVect(&re->size,f);
	writeVect(&re->data.lmSize,f);
	
	fwrite(&re->data.rectangles.num, sizeof(int), 1, f);
	fwrite(&re->entityCollection.num, sizeof(u16), 1, f);
	
	u16* mat=malloc(sizeof(u16)*re->size.x*re->size.y);
	
	if(mat)convertMaterialArray(re, mat);
	
	fwrite(re->data.floor,sizeof(u8),re->size.x*re->size.y,f);
	fwrite(re->data.ceiling,sizeof(u8),re->size.x*re->size.y,f);
	fwrite(mat,sizeof(u16),re->size.x*re->size.y,f);
	fwrite(re->data.lightMapBuffer,sizeof(u8),re->data.lmSize.x*re->data.lmSize.y,f);
	
	if(mat)free(mat);
	
	writeRectangles(&re->data, f);
	writeEntityCollection(&re->entityCollection, f);
}

void writeDoor(door_struct* d, FILE* f)
{
	if(!d || !d->primaryRoom || !d->used)return;
	s16 id1=getRoomID(d->primaryRoom);
	s16 id2=-1;
	if(d->secondaryRoom)id2=getRoomID(d->secondaryRoom);
	
	fwrite(&id1,sizeof(s16),1,f);
	fwrite(&id2,sizeof(s16),1,f);
	fwrite(&d->direction,sizeof(doorDirection),1,f);
	fwrite(&d->position,sizeof(u16),1,f);
	fwrite(&d->height,sizeof(u16),1,f);
	
	rectangle_struct* rec=&d->primaryRec;
	writeVect(&rec->lmSize,f);
	writeVect(&rec->lmPos,f);
	fwrite(&rec->rot,sizeof(bool),1,f);
	
	NOGBA("%d %d, %d %d",rec->lmPos.x,rec->lmPos.y,rec->lmSize.x,rec->lmSize.y);
	
	rec=&d->secondaryRec;
	writeVect(&rec->lmSize,f);
	writeVect(&rec->lmPos,f);
	fwrite(&rec->rot,sizeof(bool),1,f);
	
	NOGBA("%d %d, %d %d",rec->lmPos.x,rec->lmPos.y,rec->lmSize.x,rec->lmSize.y);
}

extern doorCollection_struct doorCollection;

void writeDoors(doorCollection_struct* dc, FILE* f)
{
	if(!dc)dc=&doorCollection;
	int i;
	int k=0;for(i=0;i<NUMDOORS;i++)if(dc->door[i].used)k++;
	
	fwrite(&k,sizeof(int),1,f);
	
	for(i=0;i<NUMDOORS;i++)
	{
		if(dc->door[i].used)writeDoor(&dc->door[i], f);
	}
}

extern char* basePath;

void writeMap(char* filename)
{
	int i;
	char fn[1024];
	sprintf(fn,"%sfpsm/maps/%s",basePath,filename);
	FILE* f=fopen(fn,"wb");
	if(!f)return;
	
	int k=0;for(i=0;i<NUMROOMEDITS;i++)if(roomEdits[i].used)k++;
	
	fwrite(&k,sizeof(int),1,f);
	
	for(i=0;i<NUMROOMEDITS;i++)
	{
		if(roomEdits[i].used)writeRoom(&roomEdits[i], f);
	}
	
	writeDoors(NULL, f);
	
	fclose(f);
}

//READ AREA

void readRectangle(rectangle_struct* rec, FILE* f)
{
	if(!rec || !f)return;
	
	readVect(&rec->position,f);
	readVect(&rec->size,f);
	readVect(&rec->lmSize,f);
	readVect(&rec->lmPos,f);
	readVect(&rec->normal,f);
	
	u16 mid=0;
	fread(&mid,sizeof(u16),1,f);
	rec->material=getMaterial(mid);
	
	fread(&rec->rot,sizeof(bool),1,f);
}

void readRectangles(room_struct* r, FILE* f)
{
	if(!r || !f)return;
	int i;
	int k=r->rectangles.num;
	r->rectangles.num=0;
	for(i=0;i<k;i++)
	{
		rectangle_struct rec;
		readRectangle(&rec, f);
		addRoomRectangle(r, NULL, rec, rec.material, true);
	}
}

void readEntity(room_struct* r, entity_struct* e, FILE* f, bool game)
{
	if(!e || !f)return;
	
	fread(&e->type,sizeof(entity_type),1,f);
	readVect(&e->position,f);
	
	switch(e->type)
	{
		case lightEntity:
			initLight(e,0);
			fread(e->data,sizeof(lightData_struct),1,f);
			break;
		case enemyEntity:
			initEnemy(e,0);
			e->data=malloc(sizeof(enemyData_struct));
			fread(e->data,sizeof(enemyData_struct),1,f);
			createEn(r,vect(e->position.x,e->position.z,e->position.y));
			break;
	}
}

void readEntityCollection(room_struct* r, entityCollection_struct* ec, FILE* f, bool game)
{
	if(!ec || !f)return;
	int i;
	int k=ec->num;
	ec->num=0;
	for(i=0;i<k;i++)
	{
		entity_struct* e=createEntity(ec,vect(0,0,0));
		readEntity(r, e, f, game);
	}
}

void convertBackMaterialArray(roomEdit_struct* re, u16* mat)
{
	int i, j;
	for(i=0;i<re->size.x;i++)
	{
		for(j=0;j<re->size.y;j++)
		{
			re->data.materials[i+j*re->size.x]=getMaterial(mat[i+j*re->size.x]);
		}
	}
}

void readDoor(door_struct* d, FILE* f)
{
	if(!d)return;
	
	s16 id1, id2;
	
	fread(&id1,sizeof(s16),1,f);
	fread(&id2,sizeof(s16),1,f);
	
	d->primaryRoom=getRoomByID(id1);
	d->secondaryRoom=getRoomByID(id2);
	
	fread(&d->direction,sizeof(doorDirection),1,f);
	fread(&d->position,sizeof(u16),1,f);
	fread(&d->height,sizeof(u16),1,f);
	
	getDoorRectangle(d,true,&d->primaryRec);
	getDoorRectangle(d,false,&d->secondaryRec);	
	
	rectangle_struct* rec=&d->primaryRec;
	readVect(&rec->lmSize,f);
	readVect(&rec->lmPos,f);
	fread(&rec->rot,sizeof(bool),1,f);
	
	NOGBA("%d %d, %d %d",rec->lmPos.x,rec->lmPos.y,rec->lmSize.x,rec->lmSize.y);
	
	rec=&d->secondaryRec;
	readVect(&rec->lmSize,f);
	readVect(&rec->lmPos,f);
	fread(&rec->rot,sizeof(bool),1,f);
	
	NOGBA("%d %d, %d %d",rec->lmPos.x,rec->lmPos.y,rec->lmSize.x,rec->lmSize.y);
}

void readDoors(doorCollection_struct* dc, FILE* f)
{
	if(!dc)dc=&doorCollection;
	int i, k=0;
	
	fread(&k,sizeof(int),1,f);
	
	for(i=0;i<k;i++)
	{
		readDoor(createDoor(NULL,NULL,0,doorUp,true), f);
	}
}

void readRoom(FILE* f, bool game)
{
	if(!f)return;
	
	roomEdit_struct* re=createRoomEdit(vect(0,0,0),vect(0,0,0));
	
	fread(&re->id, sizeof(s16), 1, f);
	
	readVect(&re->position,f);
	readVect(&re->size,f);
	readVect(&re->data.lmSize,f);
	
	initRoom(&re->data, re->size.x, re->size.y, re->position);
	
	fread(&re->data.rectangles.num, sizeof(int), 1, f);
	fread(&re->entityCollection.num, sizeof(u16), 1, f);
	
	u16* mat=malloc(sizeof(u16)*re->size.x*re->size.y);
	
	fread(re->data.floor,sizeof(u8),re->size.x*re->size.y,f);
	fread(re->data.ceiling,sizeof(u8),re->size.x*re->size.y,f);
	fread(mat,sizeof(u16),re->size.x*re->size.y,f);
	
	convertBackMaterialArray(re,mat);
	
	free(mat);
	
	re->data.lightMapBuffer=malloc(sizeof(u8)*re->data.lmSize.x*re->data.lmSize.y);
	fread(re->data.lightMapBuffer,sizeof(u8),re->data.lmSize.x*re->data.lmSize.y,f);

	{
		int i;
		u16 palette[8];
		for(i=0;i<8;i++){u8 v=(i*31)/7;palette[i]=RGB15(v,v,v);}
		re->data.lightMap=createReservedTextureBufferA5I3(NULL,palette,re->data.lmSize.x,re->data.lmSize.y,(void*)(0x6800000+0x0020000));
	} //TEMP
	
	NOGBA("%d %d %d",re->position.x,re->position.y,re->position.z);
	NOGBA("%d %d %d",re->size.x,re->size.y,re->size.z);
	NOGBA("%d %d %d",re->data.lmSize.x,re->data.lmSize.y,re->data.lmSize.z);
	
	NOGBA("%d",re->data.rectangles.num);
	NOGBA("%d",re->entityCollection.num);
	
	readRectangles(&re->data, f);
	readEntityCollection(&re->data,&re->entityCollection, f, game);
	
	re->quadsUpToDate=re->lightUpToDate=true;
}

void readMap(char* filename, bool game)
{
	char fn[1024];
	FILE* f=NULL;
	if(fsMode==1||fsMode==2)
	{
		if(fsMode==2)
		{
			sprintf(fn,"%sfpsm/maps/%s",basePath,filename);
			NOGBA("lala : %s",fn);
			f=fopen(fn,"rb");
		}
		if(!f)
		{
			sprintf(fn,"nitro:/fpsm/maps/%s",filename);
			NOGBA("lala : %s",fn);
			f=fopen(fn,"rb");
		}
	}else if(fsMode==3)
	{
		sprintf(fn,"%sfpsm/maps/%s",basePath,filename);
		NOGBA("lala : %s",fn);
		f=fopen(fn,"rb");
	}
	if(!f)return;
	int k;
	fread(&k,sizeof(int),1,f);
	
	NOGBA("%d rooms",k);
	
	int i;
	for(i=0;i<k;i++)readRoom(f, game);
	
	readDoors(NULL,f);
	
	getRoomDoorWays();
	
	fclose(f);
}
