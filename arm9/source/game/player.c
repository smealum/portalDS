#include "game/game_main.h"

#define PLAYERRADIUS (256)
#define ERRORMARGIN (2)
#define SQPLAYERRADIUS ((PLAYERRADIUS*PLAYERRADIUS)>>12)

player_struct player;
md2Model_struct gun;
mtlImg_struct* crossHair;
struct gl_texture_t *bottomScreen;

touchPosition touchCurrent, touchOld;

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

void initPlayer(player_struct* p)
{
	if(!p)p=&player;
	p->object=&getPlayerCamera()->object;
	p->object->position.x=-3*(TILESIZE*2);
	// p->object->position.z=6*(TILESIZE*2);
	p->object->radius=PLAYERRADIUS;
	p->object->sqRadius=SQPLAYERRADIUS;
	p->currentRoom=NULL;
	touchRead(&touchCurrent);
	touchOld=touchCurrent;
	p->life=4;
	loadMd2Model("pistol.md2","pistol.pcx",&gun);
	initModelInstance(&p->modelInstance,&gun);
	crossHair=createTexture("crshair.pcx","textures");
	bottomScreen=(struct gl_texture_t *)ReadPCXFile("bottom.pcx","bottom");
	// bgSub=bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	// dmaCopy(bottomScreen->palette, BG_PALETTE_SUB, 256*2);
	// copyBottomScreen(0);
}

void damagePlayer(player_struct* p)
{
	if(!p)p=&player;
	NOGBA("DAMAGE");
	// mmEffect(SFX_PLAYERHIT);
	p->life--;
}

void drawCrosshair(void)
{
	return;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 255, 191, 0, -1, 1);	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	applyMTL(crossHair);
	
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

s16 depth=-96;
s16 height=-54;
s16 X=-37;

void renderGun(player_struct* p)
{
	if(!p)p=&player;
	glPushMatrix();
		glTranslate3f32(0,height,depth);
		glRotateYi(-(1<<13));
		// glTranslate3f32(0,0,depth);
		// glRotateYi((1<<10));
		glMaterialf(GL_AMBIENT, RGB15(31,31,31));
		glTranslate3f32(0,0,X);
		glScalef32(inttof32(1)>>4,inttof32(1)>>4,inttof32(1)>>4);
		renderModelFrameInterp(p->modelInstance.currentFrame, p->modelInstance.nextFrame, p->modelInstance.interpCounter, &gun, POLY_ALPHA(31) | POLY_CULL_FRONT | POLY_FORMAT_LIGHT0);
	glPopMatrix(1);
}

void shootPlayerGun(player_struct* p, bool R)
{
	if(!p)p=&player;
	if(!p->currentRoom)return;
	
	// mmEffect(&gunShot);
	// mmEffect(SFX_GUNSHOT);
	
	int32 k=inttof32(300);
	vect3D u=getUnitVector(NULL);
	vect3D l=vectDifference(p->object->position,convertVect(vect(p->currentRoom->position.x,0,p->currentRoom->position.y)));
	// l.x+=u.x/32;
	// l.y+=u.y/32;
	// l.z+=u.z/32;
	vect3D ip=vect(0,0,0);
	{
		l.x-=TILESIZE;
		l.z-=TILESIZE;
		rectangle_struct* r=collideLineMapClosest(p->currentRoom, NULL, l, u, k-128, &ip);
		if(r)
		{
			// ip.x+=TILESIZE*2;
			// ip.z+=TILESIZE*2;
			ip.x+=TILESIZE;
			ip.z+=TILESIZE;
			
			vect3D pos=addVect(convertVect(vect(p->currentRoom->position.x,0,p->currentRoom->position.y)),ip);
			NOGBA("SHOT WALL ! GOOD GOING %d %d",k,pos.z);
			
			particleExplosion(pos,64,R?(RGB15(31,31,0)):(RGB15(0,31,31)));
			
			// r->hide^=1;
			int32 angle=0;
			vect3D v=vectDifference(pos,p->object->position);
			
			if(r->normal.y<0)angle=getAngle(0,0,-v.z,-v.x)<<6;
			else if(r->normal.y>0)angle=getAngle(0,0,v.z,v.x)<<6;
			
			portal_struct* por=R?(&portal1):(&portal2);
			
			vect3D oldp=por->position;vect3D oldn=por->normal;int32 olda=angle;
			movePortal(por, pos, vectMultInt(r->normal,-1), angle, false);
			
			isPortalOnWall(p->currentRoom,por,true);
			
			if(isPortalOnWall(p->currentRoom,por,false))
			{
				movePortal(por, por->position, vectMultInt(r->normal,-1), angle, true);
				NOGBA("lala");
			}else{
				movePortal(por, oldp, oldn, olda, false);
			}
		}
	}
}

void playerControls(player_struct* p)
{
	if(!p)p=&player;
	
	if(p->life<=0)changeState(&gameState);
	
	touchRead(&touchCurrent);
	
	if(keysDown() & KEY_TOUCH)touchOld=touchCurrent;
	
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
		rotateCamera(NULL, angle);
	}
	
	// if(keysHeld()&(KEY_A))rotateCamera(NULL, vect(0,0,-(1<<8)));
	// if(keysHeld()&(KEY_Y))rotateCamera(NULL, vect(0,0,1<<8));
	if(p->object->contact)
	{
		if((keysHeld()&(KEY_RIGHT))/*||(keysHeld()&(KEY_A))*/)moveCamera(NULL, vect(PLAYERGROUNDSPEED,0,0));
		if((keysHeld()&(KEY_LEFT))/*||(keysHeld()&(KEY_Y))*/)moveCamera(NULL, vect(-(PLAYERGROUNDSPEED),0,0));
		if((keysHeld()&(KEY_DOWN))/*||(keysHeld()&(KEY_B))*/)moveCamera(NULL, vect(0,0,PLAYERGROUNDSPEED));
		if((keysHeld()&(KEY_UP))/*||(keysHeld()&(KEY_X))*/)moveCamera(NULL, vect(0,0,-(PLAYERGROUNDSPEED)));
	}else{
		if((keysHeld()&(KEY_RIGHT))/*||(keysHeld()&(KEY_A))*/)moveCamera(NULL, vect(PLAYERAIRSPEED,0,0));
		if((keysHeld()&(KEY_LEFT))/*||(keysHeld()&(KEY_Y))*/)moveCamera(NULL, vect(-(PLAYERAIRSPEED),0,0));
		if((keysHeld()&(KEY_DOWN))/*||(keysHeld()&(KEY_B))*/)moveCamera(NULL, vect(0,0,PLAYERAIRSPEED));
		if((keysHeld()&(KEY_UP))/*||(keysHeld()&(KEY_X))*/)moveCamera(NULL, vect(0,0,-(PLAYERAIRSPEED)));
	}
	// if(keysHeld()&(KEY_SELECT))moveCamera(NULL, vect(0,-(inttof32(1)>>6),0));
	// if(keysHeld()&(KEY_START))moveCamera(NULL, vect(0,inttof32(1)>>6,0));
	if(keysDown()&(KEY_START))p->object->speed.y=(inttof32(1)>>4);
	if(keysDown()&(KEY_SELECT)){camera_struct* c=getPlayerCamera();p->object->position=c->position=portal1.camera.position;memcpy(c->transformationMatrix,portal1.camera.transformationMatrix,9*sizeof(int32));}
	if(!p->modelInstance.oneshot && ((keysDown()&(KEY_R))||(keysDown()&(KEY_L)))){shootPlayerGun(p,keysDown()&(KEY_R));changeAnimation(&p->modelInstance,2,true);}
	
	//DEBUG
	// if(keysHeld()&(KEY_R)){height++;}
	// if(keysHeld()&(KEY_L)){height--;}
	// NOGBA("height : %d",height);
	// if(keysHeld()&(KEY_A)){X++;}
	// if(keysHeld()&(KEY_B)){X--;}
	// NOGBA("X : %d",X);
	// if(keysHeld()&(KEY_X)){depth++;}
	// if(keysHeld()&(KEY_Y)){depth--;}
	// NOGBA("depth : %d",depth);
	
	/*if(keysDown()&(KEY_SELECT))
	{
		room_struct* r=getRoomPoint(p->object->position);
		if(r)
		{
			vect3D p2=convertCoord(r, p->object->position);
			int dist=0;
			door_struct* d=getClosestDoorRoom(NULL,r,vect2(p2.x,p2.z),&dist);
			if(d)
			{
				NOGBA("DOOOOOOR %d %p",dist,d);
				if((!d->open||dist>0) && dist<=4)
				{
					closeRoomDoors(NULL, r, d);
					if(toggleDoor(r,d))
					{
						// mmEffect(SFX_DOOR);
						if(d->open)
						{
							// room_struct* r2=d->secondaryRoom;
							// if(r==r2)r2=d->primaryRoom;
							// unloadLightMaps(r,r2);
						}else{
							unloadLightMaps(r,NULL);
							room_struct* r2=d->secondaryRoom;
							if(r==r2)r2=d->primaryRoom;
							loadLightMap(r2);
						}
					}
				}
				// moveRoomEnemiesTo(r,p2.x,p2.z,RUNTO);
			}
		}
	}*/
	touchOld=touchCurrent;
}

room_struct* getCurrentRoom(void){return player.currentRoom;}
player_struct* getPlayer(void){return &player;}

void updatePlayer(player_struct* p)
{
	if(!p)p=&player;
	// updatePhysicsObjectRoom(NULL,p->object,true);
	room_struct* r=getRoomPoint(p->object->position);
	if(r)p->currentRoom=r;
	if(p->currentRoom)
	{
		p->relativePosition=convertCoord(r, p->object->position);
		p->relativePositionReal=vectDifference(p->object->position,convertVect(vect(r->position.x,0,r->position.y)));
	}
	if(r && !r->lmSlot)loadLightMap(r);
	
	// createParticles(p->object->position,vect(0,0,0),120);
	// particleExplosion(p->object->position,32);
	
	collidePlayer(p,p->currentRoom);
	
	// updatePhysicsObject(p->object);
	updateCamera(NULL);
	updateAnimation(&p->modelInstance);
}

void freePlayer(void)
{
	freeMd2Model(&gun);
	freePCX(bottomScreen);
}
