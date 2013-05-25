#include "game/game_main.h"

emancipator_struct emancipators[NUMEMANCIPATORS];
emancipationGrid_struct emancipationGrids[NUMEMANCIPATIONGRIDS];
md2Model_struct gridModel;
mtlImg_struct* gridMtl;
u32* gridPalettes[6];

void initEmancipation(void)
{
	int i;
	for(i=0;i<NUMEMANCIPATORS;i++)
	{
		emancipators[i].used=false;
	}
	for(i=0;i<NUMEMANCIPATIONGRIDS;i++)
	{
		emancipationGrids[i].used=false;
	}
	
	loadMd2Model("models/grid.md2","balllauncher.pcx",&gridModel);
	generateModelDisplayLists(&gridModel, false, 1);
	getVramStatus();
	gridMtl=createTexture("gridcolor1.pcx", "textures");
	gridPalettes[0]=loadPalettePCX("gridcolor1.pcx","textures");
	gridPalettes[1]=loadPalettePCX("gridcolor2.pcx","textures");
	gridPalettes[2]=loadPalettePCX("gridcolor3.pcx","textures");
	gridPalettes[3]=loadPalettePCX("gridcolor4.pcx","textures");
	gridPalettes[4]=loadPalettePCX("gridcolor5.pcx","textures");
	gridPalettes[5]=loadPalettePCX("gridcolor6.pcx","textures");

	if(gridMtl)gridMtl->param|=(1<<18)|(1<<19); //flip

	u16 paletteTest[16];
	getPaletteFromBank(gridMtl, paletteTest, 16*2);
	
	for(i=0;i<5;i++)
	{
		int j;
		for(j=0;j<6;j++)
		{
			editPalette((u16*)gridPalettes[j],i,paletteTest[(i+j)%5]);
		}
	}
}

void initEmancipator(emancipator_struct* e, modelInstance_struct* mi, vect3D pos, int32* m)
{
	if(!e)return;
	
	e->position=pos;
	e->modelInstance=*mi;
	if(m)memcpy(e->transformationMatrix,m,sizeof(int32)*9);
	else{
		m=e->transformationMatrix;
		m[0]=m[4]=m[8]=inttof32(1);
		m[1]=m[2]=m[3]=0;
		m[5]=m[6]=m[7]=0;
	}
	e->counter=0;
	e->angle=0;
	e->velocity=vect((rand()%inttof32(2))-inttof32(1),inttof32(1),(rand()%inttof32(2))-inttof32(1));
	e->velocity=vectDivInt(normalize(e->velocity),256);
	e->axis=vect((rand()%inttof32(2))-inttof32(1),(rand()%inttof32(2))-inttof32(1),(rand()%inttof32(2))-inttof32(1));
	e->axis=normalize(e->axis);
	
	e->used=true;
}

void createEmancipator(modelInstance_struct* mi, vect3D pos, int32* m)
{
	if(!mi)return;
	int i;
	for(i=0;i<NUMEMANCIPATORS;i++)
	{
		if(!emancipators[i].used)
		{
			initEmancipator(&emancipators[i],mi,pos,m);
			return;
		}
	}
}

void updateEmancipator(emancipator_struct* e)
{
	if(!e || !e->used)return;
	
	e->counter++;
	e->angle+=128;
	e->position=addVect(e->position,e->velocity);
	if(e->counter>BLACKENINGTIME+FADINGTIME)e->used=false;
}

void updateEmancipators(void)
{
	int i;
	for(i=0;i<NUMEMANCIPATORS;i++)
	{
		if(emancipators[i].used)
		{
			updateEmancipator(&emancipators[i]);
		}
	}
}

void drawEmancipator(emancipator_struct* e)
{
	if(!e || !e->used)return;
	
	u8 shade=max(31-(e->counter*31)/BLACKENINGTIME,0);
	u8 alpha=(e->counter<BLACKENINGTIME)?(31):(max(31-((e->counter-BLACKENINGTIME)*31)/FADINGTIME,1));
	
	glPushMatrix();
		glTranslatef32(e->position.x,e->position.y,e->position.z);
		multTMatrix(e->transformationMatrix);
		glRotatef32i(e->angle, e->axis.x, e->axis.y, e->axis.z);
		glLight(0, RGB15(shade,shade,shade), inttof32(1)-1, 0, 0);
		renderModelFrameInterp(e->modelInstance.currentFrame,e->modelInstance.nextFrame,0,e->modelInstance.model,POLY_ID(1)|POLY_ALPHA(alpha)|POLY_CULL_FRONT|POLY_FORMAT_LIGHT0,true,e->modelInstance.palette,RGB15(shade,shade,shade));
	glPopMatrix(1);
}

void drawEmancipators(void)
{
	int i;
	for(i=0;i<NUMEMANCIPATORS;i++)
	{
		if(emancipators[i].used)
		{
			drawEmancipator(&emancipators[i]);
		}
	}
}

void initEmancipationGrid(room_struct* r, emancipationGrid_struct* eg, vect3D pos, int32 l, bool dir)
{
	if(!eg || !r)return;
	
	pos=vect(pos.x+r->position.x, pos.y, pos.z+r->position.y);
	eg->position=convertVect(pos);
	eg->length=l;
	eg->direction=dir;
	
	eg->used=true;
}

void createEmancipationGrid(room_struct* r, vect3D pos, int32 l, bool dir)
{
	if(!r)r=getPlayer()->currentRoom;
	int i;
	for(i=0;i<NUMEMANCIPATIONGRIDS;i++)
	{
		if(!emancipationGrids[i].used)
		{
			initEmancipationGrid(r, &emancipationGrids[i],pos,l,dir);
			return;
		}
	}
}

void updateEmancipationGrid(emancipationGrid_struct* eg)
{
	if(!eg)return;
	
	player_struct* pl=getPlayer();
	vect3D pos, sp;
	getEmancipationGridAAR(eg,&pos,&sp);
	if(intersectAABBAAR(pl->object->position, vect(PLAYERRADIUS,PLAYERRADIUS*5,PLAYERRADIUS), pos, sp))
	{
		resetPortals();
	}
}

void updateEmancipationGrids(void)
{
	int i;
	for(i=0;i<NUMEMANCIPATIONGRIDS;i++)
	{
		if(emancipationGrids[i].used)
		{
			updateEmancipationGrid(&emancipationGrids[i]);
		}
	}
}

u16 counter=0;

void drawEmancipationGrid(emancipationGrid_struct* eg)
{
	if(!eg)return;
	
	int32 l=abs(eg->length);
	
	glPushMatrix();
		glTranslatef32(eg->position.x,eg->position.y,eg->position.z);
		if(eg->direction)glRotateYi(-8192);
		if(eg->length<0)glRotateYi(8192*2);
		renderModelFrameInterp(0, 0, 0, &gridModel, POLY_ALPHA(31) | POLY_ID(20) | POLY_CULL_FRONT | POLY_TOON_HIGHLIGHT | POLY_FORMAT_LIGHT0, false, NULL, RGB15(31,31,31));
		glPushMatrix();
			glTranslatef32(l,0,0);
			glRotateYi(8192*2);
			renderModelFrameInterp(0, 0, 0, &gridModel, POLY_ALPHA(31) | POLY_ID(20) | POLY_CULL_FRONT | POLY_TOON_HIGHLIGHT | POLY_FORMAT_LIGHT0, false, NULL, RGB15(31,31,31));
		glPopMatrix(1);
		
		applyMTL(gridMtl);
		bindPaletteAddr(gridPalettes[(((counter++)/4)%6)]);
		GFX_COLOR=RGB15(31,31,31);
		glPolyFmt(POLY_ALPHA(12) | POLY_ID(21) | POLY_CULL_NONE);
		glScalef32(l,EMANCIPATIONGRIDHEIGHT/2,inttof32(1));
		glBegin(GL_QUADS);
			GFX_TEX_COORD = TEXTURE_PACK(0*16, 0*16);
			glVertex3v16(0, inttof32(1), 0);
			GFX_TEX_COORD = TEXTURE_PACK(l*32*16/TILESIZE, 0*16);
			glVertex3v16(inttof32(1), inttof32(1), 0);
			GFX_TEX_COORD = TEXTURE_PACK(l*32*16/TILESIZE, 256*16);
			glVertex3v16(inttof32(1), -inttof32(1), 0);
			GFX_TEX_COORD = TEXTURE_PACK(0*16, 256*16);
			glVertex3v16(0, -inttof32(1), 0);
	glPopMatrix(1);
}

void drawEmancipationGrids(void)
{
	int i;
	for(i=0;i<NUMEMANCIPATIONGRIDS;i++)
	{
		if(emancipationGrids[i].used)
		{
			drawEmancipationGrid(&emancipationGrids[i]);
		}
	}
}

void getEmancipationGridAAR(emancipationGrid_struct* eg, vect3D* pos, vect3D* sp)
{
	if(!eg || !pos || !sp)return;
	
	*sp=vect(eg->direction?(0):(eg->length/2),0,eg->direction?(eg->length/2):(0));
	*pos=addVect(eg->position,*sp);
	sp->y=EMANCIPATIONGRIDHEIGHT/2;
	sp->x=abs(sp->x);sp->z=abs(sp->z);
}

bool emancipationGridBoxCollision(emancipationGrid_struct* eg, OBB_struct* o)
{
	if(!eg || !o)return false;
	
	vect3D s, pos, sp;
	getBoxAABB(o,&s);
	getEmancipationGridAAR(eg,&pos,&sp);
	
	return intersectAABBAAR(vectDivInt(o->position,4), s, pos, sp);
}

bool collideBoxEmancipationGrids(OBB_struct* o)
{
	if(!o)return false;
	int i;
	for(i=0;i<NUMEMANCIPATIONGRIDS;i++)
	{
		if(emancipationGrids[i].used)
		{
			if(emancipationGridBoxCollision(&emancipationGrids[i], o))return true;
		}
	}
	return false;
}
