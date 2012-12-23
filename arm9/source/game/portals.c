#include "game/game_main.h"

mtlImg_struct *portalTexture1, *portalTexture2; //retarded but temp
portal_struct portal1, portal2;
portal_struct* currentPortal;

void preparePortal(void);

void initPortals(void)
{
	initPolygonPool();
	
	portalTexture1=createTexture("portal1.pcx", "textures");
	portalTexture2=createTexture("portal2.pcx", "textures");
	
	initPortal(&portal1, vect(256*4,1024*2,0), vect(inttof32(1),0,0), true);
	initPortal(&portal2, vect(2048,1024+512,0), vect(0,inttof32(1),0), false);
	
	portal1.targetPortal=&portal2;
	portal2.targetPortal=&portal1;
	
	currentPortal=&portal1;
}

void movePortal(portal_struct* p, vect3D pos, vect3D normal, int32 angle)
{
	if(!p)return;
	p->position=pos;
	p->normal=normal;
	
	p->angle=angle;
	p->oldZ=-1;
	
	computePortalPlane(p);
	
	updatePortalPI(p==&portal2,pos,normal);
	
	freePolygon(&p->unprojectedPolygon);
	p->unprojectedPolygon=createEllipse(vect(0,0,0), vectDivInt(p->plane[0],12), vectDivInt(p->plane[1],6), 32);	
	
	NOGBA("PORTAL ! %d %d %d",p->position.x,p->position.y,p->position.z);
}

void initPortal(portal_struct* p, vect3D pos, vect3D normal, bool color)
{
	if(!p)return;
	p->position=pos;
	p->targetPortal=NULL;
	if(color){p->color=RGB15(31,31,0);p->texture=portalTexture1;}
	else {p->color=RGB15(0,31,31);p->texture=portalTexture2;}
	initCamera(&p->camera);
	
	p->angle=0;
	p->oldZ=-1;
	p->normal=normal;
	computePortalPlane(p);
	
	p->polygon=NULL;
	p->unprojectedPolygon=NULL;
}

portal_struct portal1, portal2;

void drawPortal(portal_struct* p)
{
	if(!p)return;
	// glPolyFmt(POLY_ALPHA(31) | (1<<14) | POLY_CULL_BACK | POLY_ID(32));
	// glPolyFmt(POLY_ALPHA(31) | POLY_DECAL | POLY_CULL_BACK);
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);
	// glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
	
	player_struct* pl=getPlayer();
	int32 dist=distance(pl->object->position,p->position);
	
	if(dist<inttof32(1)/6)
	{
		glPushMatrix();
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
				glLoadIdentity();
				glOrthof32(0, inttof32(255), inttof32(191), 0, 0, inttof32(1));
				
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				
				unbindMtl();
				GFX_COLOR=p->color;
				
				glScalef32(1<<24, 1<<24, 1<<24);
				
				drawPolygon(p->polygon);
				freePolygon(&p->polygon);
				
				glMatrixMode(GL_PROJECTION);
			glPopMatrix(1);
			glMatrixMode(GL_MODELVIEW);
		glPopMatrix(1);
	}
	else
	{
		glPushMatrix();
			const vect3D v=addVect(p->position,vectDivInt(p->normal,64));
			glTranslate3f32(v.x,v.y,v.z);
			
			unbindMtl();
			GFX_COLOR=p->color;
			drawPolygon(p->unprojectedPolygon);
		glPopMatrix(1);
	}
	
	return;
	
	glPushMatrix();
		glTranslate3f32(p->position.x,p->position.y,p->position.z);
		
		applyMTL(p->texture);
		GFX_COLOR=RGB15(31,31,31);
		const vect3D v1=(vectDivInt(p->plane[0],12)), v2=(vectDivInt(p->plane[1],6)), v3=vectDivInt(p->normal,128);

		glTranslate3f32(v3.x,v3.y,v3.z);

		glBegin(GL_QUAD);
			GFX_TEX_COORD=TEXTURE_PACK(inttot16(127),inttot16(0));
			glVertex3v16(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z);
			GFX_TEX_COORD=TEXTURE_PACK(inttot16(0),inttot16(0));
			glVertex3v16(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z);
			GFX_TEX_COORD=TEXTURE_PACK(inttot16(0),inttot16(127));
			glVertex3v16(-v1.x+v2.x, -v1.y+v2.y, -v1.z+v2.z);
			GFX_TEX_COORD=TEXTURE_PACK(inttot16(127),inttot16(127));
			glVertex3v16(-v1.x-v2.x, -v1.y-v2.y, -v1.z-v2.z);
	glPopMatrix(1);
}

void getInvertedNormal(vect3D* n){if(!n->z)*n=vectMultInt(*n,-1);}
// void getInvertedNormal(vect3D* n){*n=vectMultInt(*n,-1);}

bool isPointInPortal(portal_struct* p, vect3D o, vect3D *v, int32* x, int32* y, int32* z)
{
	*v=vectDifference(o,p->position);
	const vect3D u1=p->plane[0], u2=p->plane[1];
	*x=dotProduct(*v,u1);
	*y=dotProduct(*v,u2);
	*z=dotProduct(*v,p->normal);
	return !(*x<-PORTALSIZEX || *y<-PORTALSIZEY || *x>=PORTALSIZEX || *y>=PORTALSIZEY);
}

extern u16 mainScreen[256*192];

void warpPlayer(portal_struct* p, player_struct* pl)
{
	camera_struct* c=getPlayerCamera();
	updatePortalCamera(p,c);
	NOGBA("cp %d %d %d",p->camera.position.x,p->camera.position.y,p->camera.position.z);
	pl->object->position=c->position=p->camera.position;
	NOGBA("cs %d %d %d",pl->object->speed.x,pl->object->speed.y,pl->object->speed.z);
	pl->object->speed=warpVector(p,pl->object->speed);
	NOGBA("cs2 %d %d %d",pl->object->speed.x,pl->object->speed.y,pl->object->speed.z);
	memcpy(c->transformationMatrix,p->camera.transformationMatrix,9*sizeof(int32));
	dmaCopy(mainScreen, p->targetPortal->viewPoint, 256*192*2);
}

void updatePortal(portal_struct* p)
{
	if(!p)return;
	player_struct* pl=getPlayer();
	vect3D v;
	int32 x, y, z;
	bool r=isPointInPortal(p, pl->object->position, &v, &x, &y, &z);
	if(r)
	{
		if(z<0 && p->oldZ>=0){currentPortal=p;warpPlayer(p,pl);}
	}
	p->oldZ=z;
	
	{
		vect3D v[4];
		
		v[0]=(addVect(p->position,addVect(vectDivInt(p->plane[0],-12), vectDivInt(p->plane[1],-6))));
		v[1]=(addVect(p->position,addVect(vectDivInt(p->plane[0],12), vectDivInt(p->plane[1],-6))));
		v[2]=(addVect(p->position,addVect(vectDivInt(p->plane[0],12), vectDivInt(p->plane[1],6))));
		v[3]=(addVect(p->position,addVect(vectDivInt(p->plane[0],-12), vectDivInt(p->plane[1],6))));
		
		const vect3D u1=normalize(vectDifference(v[1],v[0]));
		const vect3D u2=normalize(vectDifference(v[3],v[0]));
		const int32 d1=distance(v[0],v[1]);
		const int32 d2=distance(v[0],v[3]);
		
		freePolygon(&p->polygon);
		p->polygon=createEllipse(p->position, vectDivInt(p->plane[0],12), vectDivInt(p->plane[1],6), 32);
		projectPolygon(NULL, &p->polygon,v[0],u1,u2,d1,d2);
	}
}

void updatePortals(void)
{				
	updatePortal(&portal1);
	updatePortal(&portal2);
}

void updatePortalCamera(portal_struct* p, camera_struct* c)
{
	if(!p||!p->targetPortal)return;
	if(!c)c=getPlayerCamera();
	
	portal_struct* p2=p->targetPortal;
	
	// p->camera.position=vectDifference(addVect(p->targetPortal->position,c->position),p->position);
	vect3D diff=warpVector(p, vectDifference(c->position,p->position));
	
	p->camera.position=addVect(p2->position,diff);
	// p->camera.position=p2->position;
	
	// p->camera.angle=c->angle;
	
	p->camera.angle=c->angle;
	p->camera.angle2=vect(0,0,0);
	
	if(p2->normal.z)
	{
		p->camera.angle.z=p->camera.angle.x;
		p->camera.angle.x=0;
	}
	
	memcpy(p->camera.transformationMatrix,c->transformationMatrix,9*sizeof(int32));
	
	// vect3D n1=p->normal;
	// getInvertedNormal(&n1);
	// vect3D plane[2], plane1[2];
	// computePortalPlane(&n1,plane1,0);
	
	// vect3D n=vectMultInt(p2->normal,-1);
	// computePortalPlane(&n,plane,p2->angle-p->angle);
	
	// changeBase(p->camera.transformationMatrix, plane1[0], plane1[1], n1, false);
	// changeBase(p->camera.transformationMatrix, plane[0], plane[1], n, false);
	
	computePortalPlane(p);
	
	vect3D x=vect(p->camera.transformationMatrix[0],p->camera.transformationMatrix[3],p->camera.transformationMatrix[6]);
	vect3D y=vect(p->camera.transformationMatrix[1],p->camera.transformationMatrix[4],p->camera.transformationMatrix[7]);
	vect3D z=vect(p->camera.transformationMatrix[2],p->camera.transformationMatrix[5],p->camera.transformationMatrix[8]);
	
	x=vect(dotProduct(x,p->plane[0]),dotProduct(x,p->plane[1]),dotProduct(x,p->normal));
	y=vect(dotProduct(y,p->plane[0]),dotProduct(y,p->plane[1]),dotProduct(y,p->normal));
	z=vect(dotProduct(z,p->plane[0]),dotProduct(z,p->plane[1]),dotProduct(z,p->normal));
	
	computePortalPlane(p2);
	
	vect3D x2=addVect(vectMult(p2->plane[0],-x.x),addVect(vectMult(p2->plane[1],x.y),vectMult(p2->normal,-x.z)));
	vect3D y2=addVect(vectMult(p2->plane[0],-y.x),addVect(vectMult(p2->plane[1],y.y),vectMult(p2->normal,-y.z)));
	vect3D z2=addVect(vectMult(p2->plane[0],-z.x),addVect(vectMult(p2->plane[1],z.y),vectMult(p2->normal,-z.z)));
	
	p->camera.transformationMatrix[0]=x2.x;p->camera.transformationMatrix[3]=x2.y;p->camera.transformationMatrix[6]=x2.z;
	p->camera.transformationMatrix[1]=y2.x;p->camera.transformationMatrix[4]=y2.y;p->camera.transformationMatrix[7]=y2.z;
	p->camera.transformationMatrix[2]=z2.x;p->camera.transformationMatrix[5]=z2.y;p->camera.transformationMatrix[8]=z2.z;
}
