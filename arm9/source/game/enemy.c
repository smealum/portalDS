#include "game/game_main.h"

enemy_struct enemy[NUMENEMIES];

md2Model_struct enemyModel; //TEMP

void initEn(enemy_struct* e)
{
	if(!e)return;
	e->object.position=vect(0,0,0);
	e->object.speed=vect(0,0,0);
	e->modelInstance.currentAnim=0;
	e->modelInstance.currentFrame=0;
	e->used=false;
}

void initEnemies(void)
{
	int i;
	for(i=0;i<NUMENEMIES;i++)initEn(&enemy[i]);
	loadMd2Model("models/enemy.md2", "enemy1.pcx", &enemyModel);
}

void createEn(room_struct* r, vect3D p)
{
	int i;
	for(i=0;i<NUMENEMIES;i++)
	{
		if(!enemy[i].used)
		{
			enemy_struct* e=&enemy[i];
			e->used=true;
			e->tilePosition=vect(p.x,p.y,p.z);
			e->nextTilePosition=vect(p.x,p.y,p.z);
			e->progress=0;
			e->object.position=vect(p.x*(TILESIZE*2)+TILESIZE,p.y*HEIGHTUNIT,p.z*(TILESIZE*2)+TILESIZE);
			e->object.speed=vect(0,0,0);
			e->r=r;
			e->angle=0;
			e->path.first=NULL;
			e->life=100;
			e->task=0;
			e->detected=false;
			e->pathed=false;
			e->targetPosition=vect(0,0,0);
			
			e->modelInstance.model=&enemyModel;
			e->modelInstance.currentAnim=0;
			e->modelInstance.currentFrame=0;
			e->modelInstance.nextFrame=1;
			e->modelInstance.interpCounter=0;
			
			e->ec=getEntityCollection(r);
			break;
		}
	}
}

vect3D getVector(enemy_struct* e, entity_struct* ent)
{
	vect3D p=ent->position;
	vect3D v=vectDifference(e->object.position,vect(p.x*(TILESIZE*2),p.z*HEIGHTUNIT,p.y*(TILESIZE*2)));
	v=vectMultInt(v,100);
	int32 dist=magnitude(v);
	v=divideVect(v,dist);
	// dist/=100;
	return vect(f32tov10(v.x),f32tov10(v.y),f32tov10(v.z));
}

void setUpEnemyLighting(enemy_struct* e, u32* params)
{
	entity_struct *l1, *l2, *l3;
	int32 d1, d2, d3;
	getClosestLights(e->ec, e, &l1, &l2, &l3, &d1, &d2, &d3);
	*params=POLY_ALPHA(31) | POLY_CULL_FRONT;
	
	glMaterialf(GL_AMBIENT, RGB15(2,2,2));
	glMaterialf(GL_DIFFUSE, RGB15(31,31,31));
	glMaterialf(GL_SPECULAR, RGB15(0,0,0));
	glMaterialf(GL_EMISSION, RGB15(0,0,0));
	
	if(l1)
	{
		*params|=POLY_FORMAT_LIGHT0;
		vect3D v=getVector(e, l1);
		d1*=64;
		int32 v2=31-((31*d1)*(((lightData_struct*)l1->data)->intensity));
		glLight(0, RGB15(v2,v2,v2), v.x, v.y, v.z);
		if(l2)
		{
			*params|=POLY_FORMAT_LIGHT1;
			vect3D v=getVector(e, l2);
			int32 v2=31-((31*d2)*(((lightData_struct*)l2->data)->intensity));
			glLight(1, RGB15(v2,v2,v2), v.x, v.y, v.z);
			if(l3)
			{
				*params|=POLY_FORMAT_LIGHT2;
				vect3D v=getVector(e, l3);
				int32 v2=31-((31*d3)*(((lightData_struct*)l3->data)->intensity));
				glLight(2, RGB15(v2,v2,v2), v.x, v.y, v.z);
			}
		}
	}
}

void drawEnemy(enemy_struct* e)
{
	if(!e || !e->used || !e->r || !e->r->lmSlot || !e->r->lmSlot->used)return;
	glPushMatrix();
		glTranslate3f32(TILESIZE*2*e->r->position.x, 0, TILESIZE*2*e->r->position.y);
		glTranslate3f32(-TILESIZE*2, 0, -TILESIZE*2);
		glTranslate3f32(e->object.position.x,e->object.position.y,e->object.position.z);
		bool r=BoxTest(0,0,0,TILESIZE*2,HEIGHTUNIT*16,TILESIZE*2);
		glTranslate3f32(TILESIZE*2, 0, TILESIZE*2);
		// NOGBA("boxtest %d",(int)r);
		if(r)
		{
			u32 params=0;
			setUpEnemyLighting(e, &params);
			glRotateYi(e->angle);
			glScalef32(3<<10,3<<10,3<<10);
			renderModelFrameInterp(e->modelInstance.currentFrame,e->modelInstance.nextFrame,e->modelInstance.interpCounter,e->modelInstance.model,params,false,e->modelInstance.palette);
		}
	glPopMatrix(1);
}

void drawEnemies(void)
{
	int i;
	for(i=0;i<NUMENEMIES;i++)
	{
		if(enemy[i].used)drawEnemy(&enemy[i]);
	}
}

void moveEnemyTo(enemy_struct* e, int x, int y, u8 task)
{
	if(!e)return;
	if(x<0 || y<0 || x>e->r->width-1 || y>e->r->height-1)return;
	if(x==e->tilePosition.x && y==e->tilePosition.y)return;
	if(e->path.first){freePath(&e->path);}
	e->task=task;
	e->targetPosition=vect(x,0,y);
	e->path.first=getPath(e->r,vect2(e->tilePosition.x,e->tilePosition.z),vect2(x,y));
}

void moveRoomEnemiesTo(room_struct* r, int x, int y, u8 task)
{
	if(!r)return;
	int i;
	for(i=0;i<NUMENEMIES;i++)
	{
		if(enemy[i].used && enemy[i].r==r)
		{
			moveEnemyTo(&enemy[i], x, y, task);
		}
	}
}

bool collideLineEnemy(enemy_struct* e, vect3D l, vect3D u, int32 d, int32* k, vect3D* ip)
{
	if(!e)return false;
	rectangle_struct rec;
	rec.position=e->tilePosition;
	rec.size=vect(1,10,0);
	rec.normal=vect(0,0,inttof32(1));
	if(collideLineRectangle(&rec,vect(l.x-TILESIZE*2,l.y,l.z-TILESIZE*2),u,d,k,ip))return true;
	rec.position=e->tilePosition;
	rec.size=vect(0,10,1);
	rec.normal=vect(inttof32(1),0,0);
	if(collideLineRectangle(&rec,vect(l.x-TILESIZE*2,l.y,l.z-TILESIZE*2),u,d,k,ip))return true;
	return false;
}

enemy_struct* collideLineRoomEnemies(room_struct* r, vect3D l, vect3D u, int32 d, int32* k, vect3D* ip)
{
	if(!r)return NULL;
	int i;
	for(i=0;i<NUMENEMIES;i++)
	{
		if(enemy[i].used && enemy[i].life>0 && enemy[i].r==r)
		{
			if(collideLineEnemy(&enemy[i], l, u, d, k, ip))return &enemy[i];
		}
	}
	return NULL;
}

void updateEnemy(enemy_struct* e)
{
	if(!e || !e->used || !e->r || !e->r->lmSlot || !e->r->lmSlot->used)return;
	
	player_struct* p=getPlayer();
	// if(e->r!=p->currentRoom)return;
	int dist=100;
	if(e->life<=0)
	{
		if(e->modelInstance.currentFrame!=enemyModel.animations[ANIMDEATH].end)changeAnimation(&e->modelInstance,ANIMDEATH,true);
		else {e->used=false;return;}
		// if(e->modelInstance.currentAnim!=ANIMDEATH)mmEffect(SFX_ENEMY1DEAD);
	}else{
		if(p->currentRoom==e->r)
		{
			if(!e->task && e->detected)moveEnemyTo(e,p->relativePosition.x,p->relativePosition.z,RUNTO);
			if(e->path.first)e->pathed=true;
			vect3D p1=e->tilePosition, p2=p->relativePosition;
			dist=(p1.x-p2.x)*(p1.x-p2.x)+(p1.z-p2.z)*(p1.z-p2.z)+(p1.y-p2.y)*(p1.y-p2.y);
			if(dist<42 && e->modelInstance.currentAnim!=ANIMPAIN && e->modelInstance.currentAnim!=ANIMATTACK && e->pathed){damagePlayer(NULL);changeAnimation(&e->modelInstance,ANIMATTACK,true);}
			if(dist<120){/*if(!e->detected)mmEffect(SFX_SCREAM);*/e->detected=true;}
			// NOGBA("d %d",dist);
			// NOGBA("RUNNING TO YOU MY LOVE");
			// NOGBA("mem free : %dko (%do)",getMemFree()/1024,getMemFree());
		}
		
		if(e->task==WALKTO || e->task==RUNTO)
		{
			u8 speed=ENEMYWALKINGSPEED;
			if(e->task==WALKTO)changeAnimation(&e->modelInstance,ANIMWALK,false);
			else {changeAnimation(&e->modelInstance,ANIMRUN,false);speed=ENEMYRUNNINGSPEED;}
			if(e->tilePosition.x!=e->nextTilePosition.x || e->tilePosition.z!=e->nextTilePosition.z || e->path.first)
			{
				if(e->tilePosition.x<e->nextTilePosition.x)e->angle=1<<13;
				else if(e->tilePosition.x>e->nextTilePosition.x)e->angle=-(1<<13);
				else if(e->tilePosition.z<e->nextTilePosition.z)e->angle=0;
				else if(e->tilePosition.z>e->nextTilePosition.z)e->angle=(1<<14);
				e->object.position.x=e->tilePosition.x*TILESIZE*2+TILESIZE+(((e->nextTilePosition.x-e->tilePosition.x)*TILESIZE*2)*e->progress)/speed;
				e->object.position.z=e->tilePosition.z*TILESIZE*2+TILESIZE+(((e->nextTilePosition.z-e->tilePosition.z)*TILESIZE*2)*e->progress)/speed;
				if(e->modelInstance.currentAnim!=ANIMPAIN && e->modelInstance.currentAnim!=ANIMATTACK && dist>32)e->progress++;
			}else e->task=0;
			if(e->progress>=speed)
			{
				e->progress=0;
				e->tilePosition=e->nextTilePosition;
				bool r=popPathCell(&e->path, &e->nextTilePosition.x, &e->nextTilePosition.z);
				if(!r)
				{
					e->nextTilePosition=e->tilePosition;
					e->path.first=NULL;
				}
			}
		}else changeAnimation(&e->modelInstance,ANIMIDLE,false);
	}
	
	s16 v=updateSimplePhysicsObjectRoom(e->r,&e->object);
	if(v>=0)e->tilePosition.y=v;
	updateAnimation(&e->modelInstance);
}

void updateEnemies(void)
{
	int i;
	for(i=0;i<NUMENEMIES;i++)
	{
		if(enemy[i].used){updateEnemy(&enemy[i]);}
	}
}

void freeEnemies(void)
{
	freeMd2Model(&enemyModel);
}
