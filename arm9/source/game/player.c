#include "game/game_main.h"

player_struct player;
md2Model_struct gun, playerModel;
mtlImg_struct* crossHair;
struct gl_texture_t *bottomScreen;
struct gl_texture_t *bottomButton;

u8* bottomScreenIMG;
u16* bottomScreenPAL;

touchPosition touchCurrent, touchOld;

SFX_struct *gunSFX1, *gunSFX2;
SFX_struct *portalEnterSFX[2];
SFX_struct *portalExitSFX[2];

s16 gravityGunTarget;

int subBG;

u8 touchCnt;

bool isPortalInRectangle(room_struct* r, rectangle_struct* rec, portal_struct* p, vect3D* o)
{
	vect3D pr=addVect(convertVect(vect(r->position.x,0,r->position.y)),vect(rec->position.x*TILESIZE*2,rec->position.y*HEIGHTUNIT,rec->position.z*TILESIZE*2));
	vect3D sr=vect(rec->size.x*TILESIZE*2,rec->size.y*HEIGHTUNIT,rec->size.z*TILESIZE*2);
	*o=vectDifference(pr,p->position);
	if(!sr.x)return (equals(o->x,0));
	else if(!sr.y)return (equals(o->y,0));
	else return (equals(o->z,0));
}

void collidePortal(room_struct* r, rectangle_struct* rec, portal_struct* p, vect3D* point)
{
	vect3D o;
	if(!isPortalInRectangle(r,rec,p,&o))return;
	vect3D v=vectDifference(*point,p->position);
	const vect3D u1=p->plane[0], u2=p->plane[1];
	int32 xp=dotProduct(v,u1)+PORTALSIZEX;
	int32 yp=dotProduct(v,u2)+PORTALSIZEY;
	// NOGBA("IN PORTAL ? %d %d", xp, yp);
	if(xp<0 || yp<0 || xp>=PORTALSIZEX*2 || yp>=PORTALSIZEY*2)return;
	int32 d1=(xp),d2=(yp),d3=PORTALSIZEX*2-(xp),d4=PORTALSIZEY*2-(yp);
	if(d1<d2 && d1<d3 && d1<d4)
	{
		*point=addVect(*point,vectMult(u1,-d1));
	}else if(d2<d1 && d2<d3 && d2<d4)
	{
		*point=addVect(*point,vectMult(u2,-d2));
	}else if(d3<d1 && d3<d2 && d3<d4)
	{
		*point=addVect(*point,vectMult(u1,d3));
	}else{
		*point=addVect(*point,vectMult(u2,d4));
	}
	// NOGBA("YES %d %d %d %d",d1,d2,d3,d4);
}

void collidePlayer(player_struct* p, room_struct* r)
{
	if(!p)return;
	collideObjectRoom(p->object,r);
}

void setFog(u8 intensity)
{
	glEnable(GL_FOG);
	glFogShift(2);
	glFogColor(31,0,0,31);
	int i; for(i=0;i<32;i++)glFogDensity(i,intensity);
	glFogOffset(0x6500);
}

void initPlayer(player_struct* p)
{
	if(!p)p=&player;
	p->object=&getPlayerCamera()->object;
	//p->object->position=vect(0,0,0);
	p->object->radius=PLAYERRADIUS;
	p->object->sqRadius=SQPLAYERRADIUS;
	p->currentRoom=NULL;
	touchRead(&touchCurrent);
	touchOld=touchCurrent;
	p->walkCnt=0;
	p->life=127;
	p->tempAngle=vect(0,0,0);
	loadMd2Model("models/portalgun.md2","portalgun.pcx",&gun);
	loadMd2Model("models/ratman.md2","ratman.pcx",&playerModel);
	generateModelDisplayLists(&gun, true, 1);
	generateModelDisplayLists(&playerModel, false, 1);
	initModelInstance(&p->modelInstance,&gun);
	initModelInstance(&p->playerModelInstance,&playerModel);

	// bottomScreen=(struct gl_texture_t *)ReadPCXFile("bottom_screen.pcx","bottom");
	bottomButton=(struct gl_texture_t *)ReadPCXFile("bottom_button.pcx","bottom");

	bottomScreenIMG=bufferizeFile("bottom_screen.img.bin", "bottom", NULL, true);
	bottomScreenPAL=bufferizeFile("bottom_screen.pal.bin", "bottom", NULL, true);

	#ifndef DEBUG_GAME
		subBG=bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
		dmaCopy(bottomScreenIMG, bgGetGfxPtr(subBG), 256*192);
		dmaCopy(bottomScreenPAL, BG_PALETTE_SUB, 256*2);
	#endif

	//TEMP INIT VALUES
	p->object->position=vect(0,32*HEIGHTUNIT*4*2,0);

	//SFX
	gunSFX1=createSFX("portalgun_orange.raw", SoundFormat_16Bit);
	gunSFX2=createSFX("portalgun_blue.raw", SoundFormat_16Bit);

	portalEnterSFX[0]=createSFX("portal_enter1.raw", SoundFormat_16Bit);
	portalEnterSFX[1]=createSFX("portal_enter2.raw", SoundFormat_16Bit);

	portalExitSFX[0]=createSFX("portal_exit1.raw", SoundFormat_16Bit);
	portalExitSFX[1]=createSFX("portal_exit2.raw", SoundFormat_16Bit);

	//GRAVITY GUN
	gravityGunTarget=-1;

	touchCnt=0;

	//TEST TEMP
	setFog(0);
}

void drawPlayer(player_struct* p)
{
	if(!p)p=&player;
	
	glPushMatrix();
		u32 params=POLY_ALPHA(31)|POLY_CULL_FRONT|POLY_ID(2)|POLY_TOON_HIGHLIGHT|POLY_FOG;
		setupObjectLighting(NULL, p->object->position, &params);
		
		camera_struct* c=getPlayerCamera();
		glTranslatef32(c->position.x,c->position.y-600,c->position.z);
		int32 m[9];transposeMatrix33(c->transformationMatrix,m);
		m[0]=-m[0];m[3]=-m[3];m[6]=-m[6];
		m[1]=0;m[4]=inttof32(1);m[7]=0;
		m[2]=-m[2];m[5]=-m[5];m[8]=-m[8];
		fixMatrix(m);
		multMatrixGfx33(m);
		// renderModelFrameInterp(p->playerModelInstance.currentFrame,p->playerModelInstance.nextFrame,p->playerModelInstance.interpCounter,p->playerModelInstance.model,params,false,p->playerModelInstance.palette,RGB15(31,31,31));
		renderModelFrameInterp(p->playerModelInstance.currentFrame,p->playerModelInstance.nextFrame,0,p->playerModelInstance.model,params,false,p->playerModelInstance.palette,RGB15(31,31,31));
	glPopMatrix(1);
}

void drawCrosshair(void)
{
	return;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 255, 191, 0, -1, 1);	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	// applyMTL(crossHair);
	
	GFX_COLOR=RGB15(31,31,31);
	
	glPushMatrix();
	
	glTranslate3f32(inttof32(128),inttof32(96),0);
	glScalef32(inttof32(16),inttof32(16),inttof32(16));
	
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
	glBegin(GL_QUADS);
	
		GFX_TEX_COORD = TEXTURE_PACK(32*16, 0);
		GFX_VERTEX10 = NORMAL_PACK(-32,-32,0);
		GFX_TEX_COORD = TEXTURE_PACK(32*16, 32*16);
		GFX_VERTEX10 = NORMAL_PACK(-32,32,0);
		GFX_TEX_COORD = TEXTURE_PACK(0, 32*16);
		GFX_VERTEX10 = NORMAL_PACK(32,32,0);
		GFX_TEX_COORD = 0;
		GFX_VERTEX10 = NORMAL_PACK(32,-32,0);

	glEnd();
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);
	
	glPopMatrix(1);
}

s16 depth=-92;
s16 height=-63;
s16 X=-46;

void renderGun(player_struct* p)
{
	if(!p)p=&player;
	glPushMatrix();
		u32 params=POLY_ALPHA(31) | POLY_CULL_FRONT | POLY_ID(1) | POLY_TOON_HIGHLIGHT | POLY_FOG;
		setupObjectLighting(NULL, p->object->position, &params);
		
		glTranslate3f32((sinLerp(p->walkCnt>>1)>>11),(sinLerp(p->walkCnt)>>11),0);
		glTranslate3f32(0,height,depth);
		glRotateYi(-(1<<13));
		glRotateYi(-p->tempAngle.y);
		glRotateZi(p->tempAngle.x/2);
		glMaterialf(GL_AMBIENT, RGB15(31,31,31));
		glTranslate3f32(0,0,X);
		glScalef32(inttof32(1)>>4,inttof32(1)>>4,inttof32(1)>>4);
		room_struct* r=getPlayer()->currentRoom;
		// vect3D v=reverseConvertVect(vectDifference(p->object->position,convertVect(vect(r->position.x,0,r->position.y))));
		// NOGBA("%d %d %d",v.x,v.y,v.z);
		renderModelFrameInterp(p->modelInstance.currentFrame, p->modelInstance.nextFrame, p->modelInstance.interpCounter, &gun, params, false, p->modelInstance.palette, RGB15(31,31,31));
	glPopMatrix(1);
}

void shootPlayerGun(player_struct* p, bool R)
{
	if(!p)p=&player;
	if(!p->currentRoom)return;
	camera_struct* c=getPlayerCamera();
	
	p->currentPortal=R;
	
	int32 k=inttof32(300);
	vect3D u=getUnitVector(NULL);
	vect3D l=vectDifference(p->object->position,convertVect(vect(p->currentRoom->position.x,0,p->currentRoom->position.y)));
	vect3D ip=vect(0,0,0);
	{
		l.x-=TILESIZE;
		l.z-=TILESIZE;
		int32 lk=0;
		rectangle_struct* r=collideLineMapClosest(p->currentRoom, NULL, l, u, k-128, &ip, &lk);
		OBB_struct* o=collideRayBoxes(p->object->position, u, min(k,2048));
		timedButton_struct* tb=collideRayTimedButtons(p->object->position, u, min(k,1024));
		if(tb)
		{
			activateTimedButton(tb);
		}else if(o)
		{
			gravityGunTarget=o->id;
		}else if(r&&r->portalable&&!collideLineEmancipationGrids(l,u,lk))
		{
			// ip.x+=TILESIZE*2;
			// ip.z+=TILESIZE*2;
			ip.x+=TILESIZE;
			ip.z+=TILESIZE;
			
			vect3D pos=addVect(convertVect(vect(p->currentRoom->position.x,0,p->currentRoom->position.y)),ip);
			NOGBA("SHOT WALL ! GOOD GOING %d %d %d",r->normal.z,r->normal.x, r->AARid);
			
			// particleExplosion(pos,64,R?(RGB15(31,31,0)):(RGB15(0,31,31)));
			
			// r->hide^=1;

			vect3D v=vectDifference(pos,p->object->position);

			vect3D plane0=vect(c->transformationMatrix[0],c->transformationMatrix[3],c->transformationMatrix[6]);
			plane0=normalize(vectDifference(plane0,vectMult(r->normal,dotProduct(r->normal,plane0))));
			
			portal_struct* por=R?(&portal1):(&portal2);
			
			vect3D oldp=por->position;vect3D oldn=por->normal;vect3D oldp0=por->plane[0];
			movePortal(por, pos, vectMultInt(r->normal,-1), plane0, false);
			
			isPortalOnWall(p->currentRoom,por,true);
			
			if(isPortalOnWall(p->currentRoom,por,false))
			{
				pos=por->position;
				movePortal(por, oldp, oldn, oldp0, false); //terribly inelegant, please forgive me	
				ejectPortalOBBs(por);
				
				movePortal(por, pos, vectMultInt(r->normal,-1), plane0, true);
			}else{
				movePortal(por, oldp, oldn, oldp0, false);
			}
		}
	}
}

extern OBB_struct objects[NUMOBJECTS];

void playerControls(player_struct* p)
{
	if(!p)p=&player;
	
	if(p->life<=0)changeState(&gameState);
	
	touchRead(&touchCurrent);
	
	if(keysDown() & KEY_TOUCH)
	{
		touchOld=touchCurrent;
		if(!touchCnt)touchCnt=16;
		else {touchCnt=0; p->object->speed=addVect(p->object->speed,vectMult(normGravityVector,-(inttof32(1)>>5)));}
	}

	if(touchCnt)touchCnt--;
	
	if(keysHeld() & KEY_TOUCH)
	{		
		int16 dx = touchCurrent.px - touchOld.px;
		int16 dy = touchCurrent.py - touchOld.py;
		
		vect3D angle=vect(0,0,0);
		
		if (dx<20 && dx>-20 && dy<20 && dy>-20)
		{
			// if(dx>-2&&dx<2)dx=0;
			// if(dy>-2&&dy<2)dy=0;

			angle.x -= degreesToAngle(dy);
			angle.y -= degreesToAngle(dx);
		}
		p->tempAngle=addVect(p->tempAngle,angle);
		rotateCamera(NULL, angle);
	}
	
	// if(keysHeld()&(KEY_A))rotateCamera(NULL, vect(0,0,-(1<<8)));
	// if(keysHeld()&(KEY_Y))rotateCamera(NULL, vect(0,0,1<<8));
	if(p->object->contact)
	{
		bool idle=true;
		if((keysHeld()&(KEY_RIGHT))/*||(keysHeld()&(KEY_A))*/){moveCamera(NULL, vect(PLAYERGROUNDSPEED,0,0));p->walkCnt+=2500;changeAnimation(&p->playerModelInstance,4,false);idle=false;}
		else if((keysHeld()&(KEY_LEFT))/*||(keysHeld()&(KEY_Y))*/){moveCamera(NULL, vect(-(PLAYERGROUNDSPEED),0,0));p->walkCnt+=2500;changeAnimation(&p->playerModelInstance,4,false);idle=false;}
		if((keysHeld()&(KEY_DOWN))/*||(keysHeld()&(KEY_B))*/){moveCamera(NULL, vect(0,0,PLAYERGROUNDSPEED));p->walkCnt+=2500;changeAnimation(&p->playerModelInstance,3,false);idle=false;}
		else if((keysHeld()&(KEY_UP))/*||(keysHeld()&(KEY_X))*/){moveCamera(NULL, vect(0,0,-(PLAYERGROUNDSPEED)));p->walkCnt+=2500;changeAnimation(&p->playerModelInstance,3,false);idle=false;}
		if(idle)changeAnimation(&p->playerModelInstance,0,false);
	}else{
		if((keysHeld()&(KEY_RIGHT))/*||(keysHeld()&(KEY_A))*/)moveCamera(NULL, vect(PLAYERAIRSPEED,0,0));
		if((keysHeld()&(KEY_LEFT))/*||(keysHeld()&(KEY_Y))*/)moveCamera(NULL, vect(-(PLAYERAIRSPEED),0,0));
		if((keysHeld()&(KEY_DOWN))/*||(keysHeld()&(KEY_B))*/)moveCamera(NULL, vect(0,0,PLAYERAIRSPEED));
		if((keysHeld()&(KEY_UP))/*||(keysHeld()&(KEY_X))*/)moveCamera(NULL, vect(0,0,-(PLAYERAIRSPEED)));
	}
	
	// if(keysDown()&(KEY_START))p->object->speed=addVect(p->object->speed,vectMult(normGravityVector,-(inttof32(1)>>4)));
	// if(keysDown()&(KEY_START))changeState(&menuState);
	if(keysDown()&(KEY_START))doPause(NULL);

	if(!p->modelInstance.oneshot && ((keysDown()&(KEY_R))||(keysDown()&(KEY_L)))){playSFX(keysDown()&(KEY_R)?gunSFX1:gunSFX2);shootPlayerGun(p,keysDown()&(KEY_R));changeAnimation(&p->modelInstance,1,true);}
	else if(gravityGunTarget>=0 && gravityGunTarget<NUMOBJECTS && objects[gravityGunTarget].used && ((keysHeld() & KEY_R) || (keysHeld() & KEY_L)))
	{
		camera_struct* c=getPlayerCamera();
		setVelocity(gravityGunTarget, vectMultInt(/*normalize*/(vectDifference(vectMultInt(addVect(getPlayer()->object->position,vectDivInt(vect(-c->transformationMatrix[2],-c->transformationMatrix[5],-c->transformationMatrix[8]),4)),4),objects[gravityGunTarget].position)),4));
		changeAnimation(&getPlayer()->modelInstance,2,false);
	}else if(gravityGunTarget>=0)
	{
		gravityGunTarget=-1;
		changeAnimation(&getPlayer()->modelInstance,0,false);
		changeAnimation(&getPlayer()->modelInstance,1,true);
	}

	// camera_struct* c=getPlayerCamera();
	// if(keysDown()&(KEY_SELECT))changeGravity(vect(-normGravityVector.z,normGravityVector.x,normGravityVector.y),16);
	if(keysDown()&(KEY_SELECT))p->life=-5;
	
	touchOld=touchCurrent;
}

room_struct* getCurrentRoom(void){return player.currentRoom;}
player_struct* getPlayer(void){return &player;}

void updatePlayer(player_struct* p)
{
	if(!p)p=&player;
	// updatePhysicsObjectRoom(NULL,p->object,true);
	room_struct* r=&gameRoom;
	if(r)p->currentRoom=r;
	if(p->currentRoom)
	{
		p->relativePosition=convertCoord(r, p->object->position);
		p->relativePositionReal=vectDifference(p->object->position,convertVect(vect(r->position.x,0,r->position.y)));
	}

	// createParticles(p->object->position,vect(0,0,0),120);
	// particleExplosion(p->object->position,32);

	if(p->inPortal && !p->oldInPortal)playSFX(portalEnterSFX[rand()%2]);
	else if(!p->inPortal && p->oldInPortal)playSFX(portalExitSFX[rand()%2]);
	
	editPalette((u16*)p->modelInstance.model->texture->pal,0,p->currentPortal?(RGB15(31,16,0)):(RGB15(0,12,31))); //TEMP?
	editPalette((u16*)p->playerModelInstance.model->texture->pal,0,p->currentPortal?(RGB15(31,16,0)):(RGB15(0,12,31))); //TEMP?
	
	collidePlayer(p,p->currentRoom);
	if(collideAABBSludge(p->object->position, vect(PLAYERRADIUS,PLAYERRADIUS,PLAYERRADIUS)))p->life=-5;
	
	updateCamera(NULL);

	//regeneration
	p->life+=1;
	if(p->life>127)p->life=127;

	setFog((127-p->life)/2);
	
	p->tempAngle.x/=2;
	p->tempAngle.y/=2;
	
	updateAnimation(&p->playerModelInstance);
	
	updateAnimation(&p->modelInstance);
	updateAnimation(&p->modelInstance); //TEMP?
}

void shootPlayer(player_struct* p, vect3D v, u8 damage)
{
	if(!p)p=&player;

	p->object->speed=addVect(p->object->speed,vectDivInt(v,64));
	p->life-=damage;
}

void freePlayer(void)
{
	freeMd2Model(&gun);
	freeMd2Model(&playerModel);
	free(bottomScreenIMG); free(bottomScreenPAL);
	bottomScreenIMG=bottomScreenPAL=NULL;
	if(bottomButton){freePCX(bottomButton);bottomButton=NULL;}
}
