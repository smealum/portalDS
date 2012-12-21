#include "editor/editor_main.h"

mtlImg_struct *lightMtl, *enemyMtl;

void initEntity(entity_struct* e)
{
	if(e)
	{
		e->position=vect(0,0,0);
		e->type=0;
		e->data=NULL;
		e->selected=false;
		e->used=false;
	}
}

void initLight(entity_struct* e, int32 intensity)
{
	if(e)
	{
		e->type=lightEntity;
		e->mtl=lightMtl;
		lightData_struct* d=e->data=malloc(sizeof(lightData_struct));
		if(d)
		{
			d->intensity=intensity;
		}
	}
}

void initEnemy(entity_struct* e, u8 type)
{
	if(e)
	{
		e->type=enemyEntity;
		e->mtl=enemyMtl;
		enemyData_struct* d=e->data=malloc(sizeof(enemyData_struct));
		if(d)
		{
			d->type=type;
		}
	}
}

void initEntityCollection(entityCollection_struct* ec)
{
	if(ec)
	{
		int i;
		for(i=0;i<ENTITYCOLLECTIONNUM;i++)
		{
			initEntity(&ec->entity[i]);
		}
		ec->num=0;
		lightMtl=createTexture("light.pcx", "gui");
		enemyMtl=createTexture("enemy.pcx", "gui");
	}
}

void drawCircle(vect3D p, int32 r, u8 precision)
{
	glPushMatrix();
		glTranslate3f32(p.x,p.y,p.z);
		glScalef32(r,r,r);
		int i;
		glBegin(GL_TRIANGLES);
		for(i=0;i<precision;i++)
		{
			u16 a=(i*(1<<15))/precision;
			u16 a2=((i+1)*(1<<15))/precision;
			glVertex3v16(cosLerp(a),sinLerp(a),0);
			glVertex3v16(cosLerp(a2),sinLerp(a2),0);
			glVertex3v16(cosLerp(a2),sinLerp(a2),0);
		}
	glPopMatrix(1);
}

entity_struct* createEntity(entityCollection_struct* ec, vect3D position)
{
	if(ec)
	{
		int i;
		for(i=0;i<ENTITYCOLLECTIONNUM;i++)
		{
			if(!ec->entity[i].used)
			{
				entity_struct* e=&ec->entity[i];
				e->used=true;
				e->data=NULL;
				e->selected=false;
				e->position=position;
				ec->num++;
				return e;
			}
		}
	}
	return NULL;
}

void removeEntity(entityCollection_struct* ec, entity_struct* e)
{
	if(ec&&e&&e->used)
	{
		int i;
		for(i=0;i<ENTITYCOLLECTIONNUM;i++)
		{
			if(&ec->entity[i]==e)
			{
				e->used=false;
				if(e->data)free(e->data);
				e->data=false;
				ec->num--;
				return;
			}
		}
	}
}

entity_struct* createLight(entityCollection_struct* ec, vect3D position, int32 intensity)
{
	entity_struct* e=createEntity(ec,position);
	initLight(e,intensity);
	return e;
}

entity_struct* createEnemy(entityCollection_struct* ec, vect3D position, u8 type)
{
	entity_struct* e=createEntity(ec,position);
	initEnemy(e,type);
	return e;
}

static inline bool collideEntity(entity_struct* e, int px, int py)
{
	return e->position.x==px&&e->position.y==py;
}

entity_struct* collideEntityCollection(entityCollection_struct* ec, int px, int py)
{
	if(ec)
	{
		int i;
		for(i=0;i<ENTITYCOLLECTIONNUM;i++)
		{
			if(ec->entity[i].used)
			{
				if(collideEntity(&ec->entity[i],px,py))return &ec->entity[i];
			}
		}
	}
	return NULL;
}

void drawEntity(entity_struct* e)
{
	if(e)
	{
		glPushMatrix();
		
			if(e->selected)
			{
				u8 v=e->position.z;
				u16 c=RGB15(v,v,v);
				glTranslate3f32(0,0,16);
				unbindMtl();
				drawTile(e->position.x-1,e->position.y-1,c);
				drawTile(e->position.x-1,e->position.y,c);
				drawTile(e->position.x-1,e->position.y+1,c);
				drawTile(e->position.x,e->position.y-1,c);
				drawTile(e->position.x+1,e->position.y-1,c);
				drawTile(e->position.x,e->position.y,c);
				drawTile(e->position.x,e->position.y+1,c);
				drawTile(e->position.x+1,e->position.y,c);
				drawTile(e->position.x+1,e->position.y+1,c);
				glColor3b(255,255,255);
				lightData_struct* d=e->data;
				drawCircle(vect(e->position.x*(1<<9)+(1<<8),e->position.y*(1<<9)+(1<<8),64),sqrtf32(sqrtf32((d->intensity*(1<<9))/(TILESIZE*2))),32);
			}
			glTranslate3f32(0,0,64);
			applyMTL(e->mtl);
			glTranslate3f32(e->position.x*(1<<9)-(1<<8),e->position.y*(1<<9)-(1<<8),0);
			glScalef32(2*(1<<9),2*(1<<9),inttof32(1));
			glBegin(GL_QUADS);
				if(e->selected)glColor3b(255,0,0);
				else glColor3b(255,255,255);
				
				GFX_TEX_COORD=TEXTURE_PACK(0,16*16);
				glVertex3v16(0, inttof32(1), 0);
				GFX_TEX_COORD=TEXTURE_PACK(16*16,16*16);
				glVertex3v16(inttof32(1), inttof32(1), 0);
				GFX_TEX_COORD=TEXTURE_PACK(16*16,0);
				glVertex3v16(inttof32(1), 0, 0);
				GFX_TEX_COORD=TEXTURE_PACK(0,0);
				glVertex3v16(0, 0, 0);
			
		glPopMatrix(1);
	}
}

void drawEntityCollection(entityCollection_struct* ec)
{
	if(ec)
	{
		int i;
		for(i=0;i<ENTITYCOLLECTIONNUM;i++)
		{
			if(ec->entity[i].used)
			{
				drawEntity(&ec->entity[i]);
			}
		}
	}
}

void renderEntity(entity_struct* e)
{
	if(e)
	{
		glPushMatrix();
		
			applyMTL(e->mtl);
			glTranslate3f32(e->position.x*(TILESIZE*2),e->position.z*HEIGHTUNIT,e->position.y*(TILESIZE*2));
			glScalef32((1<<8),(1<<8),(1<<8));
			glBegin(GL_QUADS);
				if(e->selected)glColor3b(255,0,0);
				else glColor3b(255,255,255);
				
				GFX_TEX_COORD=TEXTURE_PACK(0,0);
				glVertex3v16(-inttof32(1), inttof32(1), 0);
				GFX_TEX_COORD=TEXTURE_PACK(16*16,0*16);
				glVertex3v16(inttof32(1), inttof32(1), 0);
				GFX_TEX_COORD=TEXTURE_PACK(16*16,16*16);
				glVertex3v16(inttof32(1), -inttof32(1), 0);
				GFX_TEX_COORD=TEXTURE_PACK(0,16*16);
				glVertex3v16(-inttof32(1), -inttof32(1), 0);
				
				GFX_TEX_COORD=TEXTURE_PACK(0,0);
				glVertex3v16(0, inttof32(1), -inttof32(1));
				GFX_TEX_COORD=TEXTURE_PACK(16*16,0*16);
				glVertex3v16(0, inttof32(1), inttof32(1));
				GFX_TEX_COORD=TEXTURE_PACK(16*16,16*16);
				glVertex3v16(0, -inttof32(1), inttof32(1));
				GFX_TEX_COORD=TEXTURE_PACK(0,16*16);
				glVertex3v16(0, -inttof32(1), -inttof32(1));
			
		glPopMatrix(1);
	}
}

void renderEntityCollection(entityCollection_struct* ec)
{
	if(ec)
	{
		int i;
		glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
		for(i=0;i<ENTITYCOLLECTIONNUM;i++)
		{
			if(ec->entity[i].used)
			{
				renderEntity(&ec->entity[i]);
			}
		}
	}
}

void wipeEntityCollection(entityCollection_struct* ec)
{
	if(ec)
	{
		int i;
		for(i=0;i<ENTITYCOLLECTIONNUM;i++)
		{
			if(ec->entity[i].used)removeEntity(ec, &ec->entity[i]);
		}
	}
}

void getClosestLights(entityCollection_struct* ec, enemy_struct* e, entity_struct** ll1, entity_struct** ll2, entity_struct** ll3, int32* dd1, int32* dd2, int32* dd3)
{
	if(!ec)return;
	entity_struct *l1, *l2, *l3;
	int32 d1, d2, d3;
	d1=d2=d3=inttof32(300);
	l1=l2=l3=NULL;
	int i;
	for(i=0;i<ENTITYCOLLECTIONNUM;i++)
	{
		if(ec->entity[i].used && ec->entity[i].type==lightEntity)
		{
			entity_struct* ent=&ec->entity[i];
			int32 d=(e->tilePosition.x-ent->position.x)*(e->tilePosition.x-ent->position.x)+((e->tilePosition.y-ent->position.y)*(e->tilePosition.y-ent->position.y))/16+(e->tilePosition.z-ent->position.z)*(e->tilePosition.z-ent->position.z);
			if(d<d1){d3=d2;d2=d1;d1=d;
					l3=l2;l2=l1;l1=ent;}
			else if(d<d2){d3=d2;d2=d;
					l3=l2;l2=ent;}
			else if(d<d3){d3=d;
					l3=ent;}
		}
	}
	*ll1=l1;
	*ll2=l2;
	*ll3=l3;
	*dd1=d1;
	*dd2=d2;
	*dd3=d3;
}
