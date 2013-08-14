#include "game/game_main.h"

#define TURRET_SIGHTANGLE (5144)

turret_struct turrets[NUMTURRETS];
md2Model_struct turretModel;
mtlImg_struct* turretShotTexture;

const vect3D laserOrigin=(vect3D){16,76,140};

void initTurrets(void)
{
	int i;
	for(i=0;i<NUMTURRETS;i++)
	{
		turrets[i].used=false;
		turrets[i].OBB=NULL;
	}
	
	loadMd2Model("models/turret.md2","turret.pcx",&turretModel);
	NOGBA("turret1 mem free : %dko (%do)",getMemFree()/1024,getMemFree());
	// generateModelDisplayLists(&turretModel, true, 1);
	generateModelDisplayLists(&turretModel, false, 1);
	NOGBA("turret2 mem free : %dko (%do)",getMemFree()/1024,getMemFree());
	turretShotTexture=createTexture("turret_shot.pcx", "textures");
}

void freeTurrets(void)
{
	freeMd2Model(&turretModel);
}

void initTurret(turret_struct* t, room_struct* r, vect3D position, u8 d)
{
	if(!t)return;

	position=vect(position.x+r->position.x, position.y, position.z+r->position.y);
	position=vectMultInt(convertVect(position),4);

	t->OBB=createBox(position,TURRETMASS,&turretModel, -d*8192);
	if(!t->OBB)return;
	t->used=true;
	t->state=TURRET_CLOSED;
	t->OBB->modelInstance.palette=loadPalettePCX("turret.pcx","textures");

	t->drawShot[0]=t->drawShot[1]=0;
}

turret_struct* createTurret(room_struct* r, vect3D position, u8 d)
{
	if(!r)r=&gameRoom;
	int i;
	for(i=0;i<NUMTURRETS;i++)
	{
		if(!turrets[i].used)
		{
			initTurret(&turrets[i],r,position,d);
			return &turrets[i];
		}
	}
	return NULL;
}

void drawLaser(vect3D orig, vect3D target)
{
	unbindMtl();
	glPolyFmt(POLY_ID(63));
	GFX_COLOR=RGB15(31,0,0);
	glPushMatrix();
		vect3D v=vectDifference(target,orig);
		glBegin(GL_TRIANGLES);
			glTranslatef32(orig.x, orig.y, orig.z);
			glVertex3v16(0, 0, 0);
			glTranslatef32(v.x, v.y, v.z);
			glVertex3v16(0, 0, 0);
			glVertex3v16(0, 0, 0);
	glPopMatrix(1);
}

void drawTurretStuff(turret_struct* t)
{
	if(!t || !t->used)return;
	
	drawLaser(t->laserOrigin,t->laserDestination);
	if(t->laserThroughPortal)drawLaser(t->laserOrigin2,t->laserDestination2);

	//TEMP TEST BILLBOARD
		vect3D u1=vect(t->OBB->transformationMatrix[0],t->OBB->transformationMatrix[3],t->OBB->transformationMatrix[6]);
		vect3D u2=vect(t->OBB->transformationMatrix[1],t->OBB->transformationMatrix[4],t->OBB->transformationMatrix[7]);
		vect3D u3=vect(t->OBB->transformationMatrix[2],t->OBB->transformationMatrix[5],t->OBB->transformationMatrix[8]);

		glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_ID(63));
		GFX_COLOR=RGB15(31,31,31);
		applyMTL(turretShotTexture);

		if(t->drawShot[0])
		{
			glPushMatrix();
				glTranslatef32(t->laserOrigin.x, t->laserOrigin.y, t->laserOrigin.z);
				glTranslatef32(u3.x/128, u3.y/128, u3.z/128);
				glTranslatef32(u1.x/32, u1.y/32, u1.z/32);
				glScalef32(inttof32(1)/16,inttof32(1)/16,inttof32(1)/16);
				glRotatef32i(t->shotAngle[0],u3.x,u3.y,u3.z);
				glBegin(GL_QUADS);
					GFX_TEX_COORD = TEXTURE_PACK(inttot16(0), inttot16(0));
					glVertex3v16(-u1.x/2-u2.x, -u1.y/2-u2.y, -u1.z/2-u2.z);
					GFX_TEX_COORD = TEXTURE_PACK(inttot16(32), inttot16(0));
					glVertex3v16(+u1.x/2-u2.x, +u1.y/2-u2.y, +u1.z/2-u2.z);
					GFX_TEX_COORD = TEXTURE_PACK(inttot16(32), inttot16(64));
					glVertex3v16(+u1.x/2+u2.x, +u1.y/2+u2.y, +u1.z/2+u2.z);
					GFX_TEX_COORD = TEXTURE_PACK(inttot16(0), inttot16(64));
					glVertex3v16(-u1.x/2+u2.x, -u1.y/2+u2.y, -u1.z/2+u2.z);
			glPopMatrix(1);
		}

		if(t->drawShot[1])
		{
			glPushMatrix();
				glTranslatef32(t->laserOrigin.x, t->laserOrigin.y, t->laserOrigin.z);
				glTranslatef32(u3.x/128, u3.y/128, u3.z/128);
				glTranslatef32(-u1.x/32, -u1.y/32, -u1.z/32);
				glScalef32(inttof32(1)/16,inttof32(1)/16,inttof32(1)/16);
				glRotatef32i(t->shotAngle[1],u3.x,u3.y,u3.z);
				glBegin(GL_QUADS);
					GFX_TEX_COORD = TEXTURE_PACK(inttot16(0), inttot16(0));
					glVertex3v16(-u1.x/2-u2.x, -u1.y/2-u2.y, -u1.z/2-u2.z);
					GFX_TEX_COORD = TEXTURE_PACK(inttot16(32), inttot16(0));
					glVertex3v16(+u1.x/2-u2.x, +u1.y/2-u2.y, +u1.z/2-u2.z);
					GFX_TEX_COORD = TEXTURE_PACK(inttot16(32), inttot16(64));
					glVertex3v16(+u1.x/2+u2.x, +u1.y/2+u2.y, +u1.z/2+u2.z);
					GFX_TEX_COORD = TEXTURE_PACK(inttot16(0), inttot16(64));
					glVertex3v16(-u1.x/2+u2.x, -u1.y/2+u2.y, -u1.z/2+u2.z);
			glPopMatrix(1);
		}
}

void drawTurretsStuff(void)
{
	int i;
	for(i=0;i<NUMTURRETS;i++)
	{
		if(turrets[i].used)
		{
			drawTurretStuff(&turrets[i]);
		}
	}
}

void laserProgression(room_struct* r, vect3D* origin, vect3D* destination, vect3D dir)
{
	if(!r || !origin || !destination)return;
	vect3D ip;
	vect3D l=*origin;
	vect3D v=vectDifference(addVect(l,vectMult(dir,32)),convertSize(vect(r->position.x,0,r->position.y)));
	gridCell_struct* gc=getCurrentCell(r,l);
	while(gc && !collideGridCell(gc, NULL, v, dir, inttof32(100), &ip, NULL))
	{
		l=addVect(l,vectMult(dir,CELLSIZE*TILESIZE*2));
		gc=getCurrentCell(r,l);
	}
	if(gc)*destination=addVect(ip,convertSize(vect(r->position.x,0,r->position.y)));
}

bool pointInTurretSight(turret_struct* t, vect3D p, int32* angle, int32* d2)
{
	if(!t)return false;
	const int32* m=t->OBB->transformationMatrix;

	const vect3D u=vectDifference(p,vectDivInt(t->OBB->position,4));
	const int32 d=magnitude(u);
	const int32 v=dotProduct(u,vect(m[2],m[5],m[8]));
	if(angle)*angle=dotProduct(u,vect(m[0],m[3],m[6]));

	if(d2)*d2=d;

	return v>mulf32(cosLerp(TURRET_SIGHTANGLE),d);
}

void updateTurret(turret_struct* t)
{
	if(!t || !t->used)return;
	
	if(!t->OBB || !t->OBB->used){t->OBB=NULL;t->used=false;return;}
	
	t->counter+=2;t->counter%=63; //TEMP
	editPalette((u16*)t->OBB->modelInstance.palette,0,RGB15(abs(31-t->counter),0,0)); //TEMP
	
	int32* m=t->OBB->transformationMatrix;
	room_struct* r=getPlayer()->currentRoom;
	if(!r)return;

	int32 angle, d;
	bool b=pointInTurretSight(t, getPlayer()->object->position, &angle, &d);
	if(b)
	{
		vect3D u=vectDifference(getPlayer()->object->position,t->laserOrigin);
		int32 d=magnitude(u);
		u=divideVect(u,d);
		if(collideLineMap(&gameRoom, NULL, t->laserOrigin, u, d, NULL, NULL))b=false;
	}

	switch(t->state)
	{
		case TURRET_CLOSED:
			changeAnimation(&t->OBB->modelInstance, 0, false);
			t->drawShot[0]=t->drawShot[1]=0;

			if(b)t->state=TURRET_OPENING;
			break;
		case TURRET_OPENING:
			if(t->OBB->modelInstance.currentAnim==2)
			{
				t->state=TURRET_OPEN;
			}else if(t->OBB->modelInstance.currentAnim!=1)
			{
				changeAnimation(&t->OBB->modelInstance, 2, false);
				changeAnimation(&t->OBB->modelInstance, 1, true);
			}
			t->drawShot[0]=t->drawShot[1]=0;
			break;
		case TURRET_OPEN:
			{
				if(angle>mulf32(sinLerp(TURRET_SIGHTANGLE/3),d))changeAnimation(&t->OBB->modelInstance, 4, false);
				else if(angle<-mulf32(sinLerp(TURRET_SIGHTANGLE/3),d))changeAnimation(&t->OBB->modelInstance, 5, false);
				else changeAnimation(&t->OBB->modelInstance, 2, false);

				int i;
				for(i=0;i<2;i++)
				{
					if(!t->drawShot[i] && !(rand()%3))
					{
						t->drawShot[i]=rand()%8;
						t->shotAngle[i]=rand();

						shootPlayer(NULL, normalize(vectDifference(t->laserDestination,t->laserOrigin)), 4);
					}

					if(t->drawShot[i])t->drawShot[i]--;
				}

				if(!b)t->state=TURRET_CLOSING;
			}
			break;
		case TURRET_CLOSING:
			if(t->OBB->modelInstance.currentAnim==0)
			{
				t->state=TURRET_CLOSED;
			}else if(t->OBB->modelInstance.currentAnim!=3)
			{
				changeAnimation(&t->OBB->modelInstance, 0, false);
				changeAnimation(&t->OBB->modelInstance, 3, true);
			}
			t->drawShot[0]=t->drawShot[1]=0;
			break;
	}

	t->laserOrigin=addVect(vectDivInt(t->OBB->position,4),evalVectMatrix33(m,laserOrigin));
	t->laserDestination=addVect(t->laserOrigin,vect(m[2],m[5],m[8]));

	if(b) t->laserDestination=getPlayer()->object->position;

	vect3D dir=normalize(vectDifference(t->laserDestination,t->laserOrigin));
	t->laserThroughPortal=false;

	laserProgression(r, &t->laserOrigin, &t->laserDestination, dir);
	
	int32 x, y, z;
	vect3D v;
	portal_struct* portal=NULL;
	if(isPointInPortal(&portal1, t->laserDestination, &v, &x, &y, &z))portal=&portal1;
	if(abs(z)>=32)portal=NULL;
	if(!portal)
	{
		if(isPointInPortal(&portal2, t->laserDestination, &v, &x, &y, &z))portal=&portal2;
		if(abs(z)>=32)portal=NULL;
	}
	if(portal)
	{
		t->laserDestination=addVect(t->laserDestination,vectMult(dir,TILESIZE));


		t->laserThroughPortal=true;
		dir=warpVector(portal,dir);
		t->laserOrigin2=addVect(portal->targetPortal->position, warpVector(portal, vectDifference(t->laserDestination, portal->position)));
		t->laserDestination2=addVect(t->laserOrigin2,dir);
		
		laserProgression(r, &t->laserOrigin2, &t->laserDestination2, dir);
		

		t->laserOrigin2=addVect(t->laserOrigin2,vectMult(dir,-TILESIZE));
	}
	
	updateAnimation(&t->OBB->modelInstance);
}

void updateTurrets(void)
{
	int i;
	for(i=0;i<NUMTURRETS;i++)
	{
		if(turrets[i].used)
		{
			updateTurret(&turrets[i]);
		}
	}
}
