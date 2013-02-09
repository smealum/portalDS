#include "editor/editor_main.h"

roomEdit_struct roomEdits[NUMROOMEDITS];
u8 roomEditorMode=0;
u8 oldEditorMode=0;
roomEdit_struct *currentRoom, *selectedRoom, *oldSelectedRoom;
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
		re->data.entityCollection=&re->entityCollection;
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
			drawRoom(&roomEdits[i].data,((roomEdits[i].lightUpToDate&&roomEdits[i].selected)<<1)|(roomEdits[i].quadsUpToDate), 0);
			glPushMatrix();
				glTranslate3f32(TILESIZE*2*roomEdits[i].data.position.x, 0, TILESIZE*2*roomEdits[i].data.position.y);
				renderEntityCollection(&roomEdits[i].entityCollection);
			glPopMatrix(1);
		}
	}
}

void drawRoomsGame(u8 mode, u16 color)
{
	int i;
	unbindMtl();
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
	for(i=0;i<NUMROOMEDITS;i++)
	{
		if(roomEdits[i].used)
		{
			drawRoom(&roomEdits[i].data,((1)<<3)|((roomEdits[i].data.lmSlot!=0)<<2)|(1)|(mode), color);
			// glPushMatrix();
				// glTranslate3f32(TILESIZE*2*roomEdits[i].data.position.x, 0, TILESIZE*2*roomEdits[i].data.position.y);
				// renderEntityCollection(&roomEdits[i].entityCollection);
			// glPopMatrix(1);
			glPushMatrix();
				glTranslate3f32(TILESIZE*2*roomEdits[i].data.position.x, 0, TILESIZE*2*roomEdits[i].data.position.y);
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

int oldpx, oldpy;
extern int32 gridScale;
bool moving=false;

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
			// createEn(r,vect(e->position.x,e->position.z,e->position.y));
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
	
	fclose(f);
}
