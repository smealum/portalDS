#include "game/game_main.h"

portal_struct portal1, portal2;
portal_struct* currentPortal;


void initPortals(void)
{
	initPolygonPool();

	initPortal(&portal1, vect(256*4,1024*2,0), vect(inttof32(1),0,0), true);
	initPortal(&portal2, vect(2048,1024+512,0), vect(0,inttof32(1),0), false);

	portal1.targetPortal=&portal2;
	portal2.targetPortal=&portal1;

	currentPortal=&portal1;
}

void freePortals(void)
{
	if(portal1.displayList){free(portal1.displayList);portal1.displayList=NULL;}
	if(portal2.displayList){free(portal2.displayList);portal2.displayList=NULL;}
}

void resetPortals(void)
{
	portal1.used=portal2.used=false;
	resetPortalsPI();
}

extern u32 debugVal; //TEMP

void movePortal(portal_struct* p, vect3D pos, vect3D normal, vect3D plane0, bool actualMove)
{
	if(!p)return;
	p->position=pos;
	p->normal=normal;
	p->plane[0]=plane0;
	p->animCNT=0;

	// p->angle=angle;
	p->oldZ=-1;

	computePortalPlane(p);

	if(actualMove)
	{
		updatePortalPI(p==&portal2,p->position,p->normal,p->plane[0]);
		if(p->displayList)free(p->displayList);
		p->displayList=NULL;
		debugVal=0;
		p->displayList=generateRoomDisplayList(NULL, p->position, p->normal, true);
		// debugVal=getMemFree()/1024; //TEMP
		p->used=true;
	}

	const vect3D v1=vectDivInt(p->plane[0],PORTALFRACTIONX);
	const vect3D v2=vectDivInt(p->plane[1],PORTALFRACTIONY);

	freePolygon(&p->unprojectedPolygon);
	p->unprojectedPolygon=createEllipse(vect(0,0,0), v1, v2, 32);

	freePolygon(&p->unprojectedOutline);
	p->unprojectedOutline=createEllipseOutline(vect(0,0,0), v1, v2, vectMult(v1,inttof32(11)/10), vectMult(v2,inttof32(11)/10), vect(0,0,0), 32);
	freePolygon(&p->outline);
	p->outline=createEllipseOutline(vect(0,0,0), v1, v2, vectMult(v1,inttof32(11)/10), vectMult(v2,inttof32(11)/10), vectDivInt(p->normal,256), 32);

	NOGBA("PORTAL ! %ld %ld %ld",p->position.x,p->position.y,p->position.z);
}

void initPortal(portal_struct* p, vect3D pos, vect3D normal, bool color)
{
	if(!p)return;
	p->position=pos;
	p->targetPortal=NULL;
	if(color){p->color=RGB15(31,31,0);}
	else {p->color=RGB15(0,31,31);}
	initCamera(&p->camera);

	p->oldZ=-1;
	p->normal=normal;
	p->used=false;
	p->normal=vect(0,0,inttof32(1));
	p->plane[0]=vect(0,inttof32(1),0);
	computePortalPlane(p);

	p->displayList=NULL;

	p->polygon=NULL;
	p->unprojectedPolygon=NULL;

	p->outline=NULL;
	p->unprojectedOutline=NULL;
}


void drawPortal(portal_struct* p)
{
	if(!p || !p->used)return;
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

		glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
		glPushMatrix();
			const vect3D v=p->position;
			glTranslate3f32(v.x,v.y,v.z);
			drawPolygonStrip(p->outline,p->innerOutlineColor,p->outlineColor);
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
			drawPolygonStrip(p->unprojectedOutline,p->innerOutlineColor,p->outlineColor);
		glPopMatrix(1);
	}
}

void getInvertedNormal(vect3D* n){if(!n->z)*n=vectMultInt(*n,-1);}
// void getInvertedNormal(vect3D* n){*n=vectMultInt(*n,-1);}

bool isPointInPortal(portal_struct* p, vect3D o, vect3D *v, int32* x, int32* y, int32* z)
{
	if(!x || !y || !z || !v || !p)return false;
	*v=vectDifference(o,p->position);
	const vect3D u1=p->plane[0], u2=p->plane[1];
	*x=dotProduct(*v,u1);
	*y=dotProduct(*v,u2);
	*z=dotProduct(*v,p->normal);
	return !(*x<-PORTALSIZEX || *y<-PORTALSIZEY || *x>=PORTALSIZEX || *y>=PORTALSIZEY);
}

u16 getCurrentPortalColor(vect3D o)
{
	u16 col=0;
	u32 dist=inttof32(10);
	int32 x, y, z;
	vect3D v;
	if(isPointInPortal(&portal1,o,&v,&x,&y,&z))
	{
		dist=abs(z);
		col=portal1.color;
	}
	if(isPointInPortal(&portal2,o,&v,&x,&y,&z))
	{
		if(z<dist)
		{
			dist=abs(z);
			col=portal2.color;
		}
	}
	if(dist>512)col=0;
	return col;
}

extern u16 mainScreen[256*192];

void warpPlayer(portal_struct* p, player_struct* pl)
{
	camera_struct* c=getPlayerCamera();
	updatePortalCamera(p,c);

	c->viewPosition=p->camera.viewPosition;
	pl->object->position=c->position=reverseViewPosition(p->camera.viewPosition);

	int32 z=dotProduct(vectDifference(c->position,p->targetPortal->position),p->targetPortal->normal);
	if(z<0)pl->object->position=c->position=addVect(c->position,vectMult(p->targetPortal->normal,-2*z));
	c->viewPosition=getViewPosition(c->position);

	pl->object->speed=warpVector(p,pl->object->speed);
	memcpy(c->transformationMatrix,p->camera.transformationMatrix,9*sizeof(int32));
	updateViewMatrix(c);
	updateFrustum(c);
	dmaCopy(mainScreen, p->targetPortal->viewPoint, 256*192*2);
}

void checkPortalPlayerWarp(portal_struct* p)
{
	if(!p)return;
	player_struct* pl=getPlayer();
	vect3D v;
	int32 x, y, z = 0;
	bool r=isPointInPortal(p, pl->object->position, &v, &x, &y, &z);
	if(r)
	{
		if(z<0 && p->oldZ>=0){currentPortal=p;warpPlayer(p,pl);gravityGunTarget=-1;}
		pl->oldInPortal=pl->inPortal;
		if(abs(z)<PLAYERRADIUS){pl->inPortal=true;}
		else pl->inPortal=false;
	}
	p->oldZ=z;
}

void updatePortal(portal_struct* p)
{
	if(!p || !p->used)return;

	if(portal1.used&&portal2.used)
	{
		player_struct* pl=getPlayer();

		int32 dist=distance(pl->object->position,p->position);
		if(dist<inttof32(1)/6)
		{
			vect3D v[4];

			v[0]=(addVect(p->position,addVect(vectDivInt(p->plane[0],-PORTALFRACTIONX), vectDivInt(p->plane[1],-PORTALFRACTIONY))));
			v[1]=(addVect(p->position,addVect(vectDivInt(p->plane[0],PORTALFRACTIONX), vectDivInt(p->plane[1],-PORTALFRACTIONY))));
			v[2]=(addVect(p->position,addVect(vectDivInt(p->plane[0],PORTALFRACTIONX), vectDivInt(p->plane[1],PORTALFRACTIONY))));
			v[3]=(addVect(p->position,addVect(vectDivInt(p->plane[0],-PORTALFRACTIONX), vectDivInt(p->plane[1],PORTALFRACTIONY))));

			const vect3D u1=normalize(vectDifference(v[1],v[0]));
			const vect3D u2=normalize(vectDifference(v[3],v[0]));
			const int32 d1=distance(v[0],v[1]);
			const int32 d2=distance(v[0],v[3]);

			freePolygon(&p->polygon);
			p->polygon=createEllipse(p->position, vectDivInt(p->plane[0],PORTALFRACTIONX), vectDivInt(p->plane[1],PORTALFRACTIONY), 32);
			projectPolygon(NULL, &p->polygon,v[0],u1,u2,d1,d2);
		}
	}

	const u16 range=10;
	s8 x=abs((((p->animCNT)/2)%(range*2))-range)-range/2;

	p->innerOutlineColor=(p->color==RGB15(31,31,0))?(RGB15(27,27,0)):(RGB15(0,27,27));
	p->outlineColor=(p->color==RGB15(31,31,0))?(RGB15(31,16+x,0)):(RGB15(0,16+x,31));

	p->animCNT++;
}

void updatePortals(void)
{
	if(portal1.used&&portal2.used)
	{
		checkPortalPlayerWarp(&portal1);
		checkPortalPlayerWarp(&portal2);
	}
	updatePortal(&portal1);
	updatePortal(&portal2);
}

void updatePortalCamera(portal_struct* p, camera_struct* c)
{
	if(!p||!p->targetPortal)return;
	if(!c)c=getPlayerCamera();

	portal_struct* p2=p->targetPortal;

	p->camera.position=addVect(p2->position,warpVector(p, vectDifference(c->position, p->position)));
	p->camera.viewPosition=addVect(p2->position,warpVector(p, vectDifference(c->viewPosition, p->position)));

	memcpy(p->camera.transformationMatrix,c->transformationMatrix,9*sizeof(int32));

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

void drawPortalRoom(portal_struct* p)
{
	room_struct* r=getPlayer()->currentRoom;
	if(!r || !p)return;

	glPushMatrix();
		glTranslate3f32(TILESIZE*2*r->position.x, 0, TILESIZE*2*r->position.y);
		glTranslate3f32(-TILESIZE,0,-TILESIZE);
		glScalef32((TILESIZE*2)<<7,(HEIGHTUNIT)<<7,(TILESIZE*2)<<7);
		if(currentPortal->targetPortal && currentPortal->targetPortal->displayList)glCallList(currentPortal->targetPortal->displayList);
	glPopMatrix(1);
}

static const u8 segmentList[4][2]={{0,1},{1,2},{2,3},{3,0}};
static const u8 segmentNormal[4]={0,1,0,1};

bool doSegmentsIntersect(vect3D s1, vect3D s2, vect3D sn, vect3D p1, vect3D p2, vect3D pn, bool *vv1, bool *vv2)
{
	bool v1=dotProduct(vectDifference(p1,s1),pn)>0;
	bool v2=dotProduct(vectDifference(p1,s2),pn)>0;
	bool v3=dotProduct(vectDifference(s1,p1),sn)>0;
	bool v4=dotProduct(vectDifference(s1,p2),sn)>0;
	*vv1=v1;*vv2=v2;
	return (v1!=v2)&&(v3!=v4);
}

bool portalRectangleIntersection(room_struct* r, portal_struct* p, rectangle_struct* rec, vect3D origpos, bool fix)
{
	if(!p || !rec)return false;
	if((rec->normal.x&&p->normal.x)||(rec->normal.z&&p->normal.z)||(rec->normal.y&&p->normal.y))return false;
	vect3D pr=addVect(convertVect(vect(r->position.x,0,r->position.y)),vect(rec->position.x*TILESIZE*2,rec->position.y*HEIGHTUNIT,rec->position.z*TILESIZE*2));
	vect3D s=convertSize(rec->size);

	//only need to do this once for all rectangles (so optimize it out)
	const vect3D v[]={(vectDivInt(p->plane[0],PORTALFRACTIONX)), (vectDivInt(p->plane[1],PORTALFRACTIONY))};
	const vect3D points[]={addVect(addVect(p->position,v[0]),v[1]),vectDifference(addVect(p->position,v[0]),v[1]),vectDifference(vectDifference(p->position,v[0]),v[1]),addVect(vectDifference(p->position,v[0]),v[1])};

	//projection = more elegant, less efficient ? (rectangles are axis aligned biatch)
	if(p->normal.x)
	{
		if(!((pr.x-PORTALMARGIN<=p->position.x&&pr.x+s.x+PORTALMARGIN>=p->position.x)||(pr.x+PORTALMARGIN>=p->position.x&&pr.x+s.x-PORTALMARGIN<=p->position.x)))return false;
	}else if(p->normal.y)
	{
		if(!((pr.y-PORTALMARGIN<=p->position.y&&pr.y+s.y+PORTALMARGIN>=p->position.y)||(pr.y+PORTALMARGIN>=p->position.y&&pr.y+s.y-PORTALMARGIN<=p->position.y)))return false;
	}else{
		if(!((pr.z-PORTALMARGIN<=p->position.z&&pr.z+s.z+PORTALMARGIN>=p->position.z)||(pr.z+PORTALMARGIN>=p->position.z&&pr.z+s.z-PORTALMARGIN<=p->position.z)))return false;
	}

	vect3D p1=pr, p2=addVect(pr,s);
	vect3D pn=rec->normal;

	int i;
	for(i=0;i<4;i++)
	{
		const vect3D s1=points[segmentList[i][0]], s2=points[segmentList[i][1]]; //segment
		bool v1, v2;
		if(doSegmentsIntersect(s1,s2,v[segmentNormal[i]],p1,p2,pn,&v1,&v2))
		{
			if(fix)
			{
				bool v3=dotProduct(vectDifference(p1,origpos),pn)>0;
				if(v3==v1)
				{
					int32 ln=dotProduct(pn,vectDifference(p1,s2));
					ln+=(ln>0)?(PORTALMARGIN):(-PORTALMARGIN);
					p->position=addVect(p->position,vectMult(pn,ln));
				}else{
					int32 ln=dotProduct(pn,vectDifference(p1,s1));
					ln+=(ln>0)?(PORTALMARGIN):(-PORTALMARGIN);
					p->position=addVect(p->position,vectMult(pn,ln));
				}
			//if put back, add updating of points
			// }else{
				// return true;
			}
			return true;
		}
	}
	p1=vectDifference(p1,p->position);
	p1=vect(dotProduct(p1,p->plane[0]),dotProduct(p1,p->plane[1]),0);
	if(p1.x<PORTALSIZEX||p1.x>PORTALSIZEX||p1.y<PORTALSIZEY||p1.y>PORTALSIZEY)return false;
	p2=vectDifference(p2,p->position);
	p2=vect(dotProduct(p2,p->plane[0]),dotProduct(p2,p->plane[1]),0);
	if(p2.x<PORTALSIZEX||p2.x>PORTALSIZEX||p2.y<PORTALSIZEY||p2.y>PORTALSIZEY)return false;

	return true;
}

bool portalToPortalIntersection(const portal_struct* p, const portal_struct* p2)
{
	if(!p || !p2)return false;


	vect3D vect_portals = vectProduct(p->normal,p2->normal);
	if (equals(vect_portals.x,0) && equals(vect_portals.y,0) && equals(vect_portals.z,0))
	{
		//works as we are on a grid, no inclination
		// y is "up"
		if (p->normal.x==p2->normal.x)
		{
			return abs(p->position.y-p2->position.y)> 782*2-100

			||  abs(p->position.z-p2->position.z)>374*2;
		}
		if (p->normal.z==p2->normal.z)
		{
			return abs(p->position.y-p2->position.y)> 782*2-100 ||  abs(p->position.x-p2->position.x)>374*2;
		}
		if (p->position.y==p2->position.y)
		{
			// TODO : complete this case
			return true;
		}
	}

	return true;
}


bool isPortalOnWall(room_struct* r, portal_struct* p, bool fix)
{
	listCell_struct *lc=r->rectangles.first;
	vect3D origpos=p->position;
	while(lc)
	{
		if(portalRectangleIntersection(r,p,&lc->data,origpos,fix)&&!fix)return false;
		lc=lc->next;
	}
	return true;
}
