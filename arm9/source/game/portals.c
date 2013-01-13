#include "game/game_main.h"

portal_struct portal1, portal2;
portal_struct* currentPortal;

void preparePortal(void);

void initPortals(void)
{
	initPolygonPool();
	
	initPortal(&portal1, vect(256*4,1024*2,0), vect(inttof32(1),0,0), true);
	initPortal(&portal2, vect(2048,1024+512,0), vect(0,inttof32(1),0), false);
	
	portal1.targetPortal=&portal2;
	portal2.targetPortal=&portal1;
	
	currentPortal=&portal1;
}

void movePortal(portal_struct* p, vect3D pos, vect3D normal, int32 angle, bool send)
{
	if(!p)return;
	p->position=pos;
	p->normal=normal;
	
	p->angle=angle;
	p->oldZ=-1;
	
	computePortalPlane(p);
	
	if(send)updatePortalPI(p==&portal2,p->position,p->normal,p->angle);
	
	freePolygon(&p->unprojectedPolygon);
	p->unprojectedPolygon=createEllipse(vect(0,0,0), vectDivInt(p->plane[0],PORTALFRACTIONX), vectDivInt(p->plane[1],PORTALFRACTIONY), 32);	
	
	NOGBA("PORTAL ! %d %d %d",p->position.x,p->position.y,p->position.z);
}

void initPortal(portal_struct* p, vect3D pos, vect3D normal, bool color)
{
	if(!p)return;
	p->position=pos;
	p->targetPortal=NULL;
	if(color){p->color=RGB15(31,31,0);}
	else {p->color=RGB15(0,31,31);}
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
	vect3D diff=warpVector(p, vectDifference(c->position, p->position));
	
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

const u8 segmentList[4][2]={{0,1},{1,2},{2,3},{3,0}};
const u8 segmentNormal[4]={0,1,0,1};

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
