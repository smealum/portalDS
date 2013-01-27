#include "game/game_main.h"

extern camera_struct playerCamera;
polygon_struct polygonPoolData[POLYPOOLSIZE];
polygon_struct* polygonPool;
u16 polygonPoolCnt;

void initPolygonPool(void)
{
	int i;
	for(i=0;i<POLYPOOLSIZE-1;i++)
	{
		polygonPoolData[i].next=&polygonPoolData[i+1];
	}
	polygonPoolCnt=POLYPOOLSIZE;
	polygonPoolData[i].next=NULL;
	polygonPool=polygonPoolData;
}

polygon_struct* createPolygon(vect3D v)
{
	if(!polygonPool)return NULL;
	polygon_struct* pp=polygonPool;
	polygonPool=pp->next;
	pp->v=v;
	pp->next=NULL;
	polygonPoolCnt--;
	return pp;
}

void freePolygon(polygon_struct** p)
{
	polygon_struct* pp=*p;
	while(pp)
	{
		polygon_struct* tpp=pp->next;
		pp->next=polygonPool;
		polygonPool=pp;
		polygonPoolCnt++;
		pp=tpp;
	}
	*p=NULL;
}

vect3D intersectSegmentPlane(plane_struct* pl, vect3D o, vect3D v, int32 d)
{
	if(!pl)return o;
	vect3D n=vect(pl->A,pl->B,pl->C);
	int32 p1=dotProduct(v,n);

	vect3D p=pl->point;		
	int32 p2=dotProduct(vectDifference(p,o),n);
	int32 k=divf32(p2,p1);
	return addVect(o,vectMult(v,k));
}

polygon_struct* createQuad(vect3D v1, vect3D v2, vect3D v3, vect3D v4)
{
	polygon_struct *p, *pp;
	p=pp=createPolygon(v1);
	if(!p)return NULL;
	p->next=createPolygon(v2);
	p=p->next;
	if(!p)return NULL;
	p->next=createPolygon(v3);
	p=p->next;
	if(!p)return NULL;
	p->next=createPolygon(v4);
	p=p->next;
	if(!p)return NULL;
	return pp;
}

polygon_struct* createEllipse(vect3D po, vect3D v1, vect3D v2, int n)
{
	int i;
	polygon_struct *p, *pp;
	p=pp=createPolygon(addVect(po,v1));
	if(!p)return NULL;
	for(i=1;i<n;i++)
	{
		const u16 a1=i*32768/n;
		vect3D v=addVect(po,addVect(vectMult(v1,cosLerp(a1)),vectMult(v2,sinLerp(a1))));
		p->next=createPolygon(v);
		p=p->next;
		if(!p)return NULL;
	}
	return pp;
}

polygon_struct* createEllipseOutline(vect3D po, vect3D v1_1, vect3D v2_1, vect3D v1_2, vect3D v2_2, vect3D norm, int n)
{
	int i;
	polygon_struct *p, *pp;
	p=pp=createPolygon(addVect(po,v1_1));
	p->next=createPolygon(addVect(norm,addVect(po,v1_2)));
	p=p->next;
	if(!p)return NULL;
	for(i=1;i<=n;i++)
	{
		const u16 a1=i*32768/n;
		vect3D v=addVect(po,addVect(vectMult(v1_1,cosLerp(a1)),vectMult(v2_1,sinLerp(a1))));
		p->next=createPolygon(v);
		p=p->next;
		if(!p)return NULL;
		v=addVect(po,addVect(vectMult(v1_2,cosLerp(a1)),vectMult(v2_2,sinLerp(a1))));
		p->next=createPolygon(addVect(norm,v));
		p=p->next;
		if(!p)return NULL;
	}
	return pp;
}

void clipSegmentPlane(plane_struct* pl, polygon_struct** o, polygon_struct* pp1, polygon_struct* pp2)
{
	if(!pl || !o)return;
	const vect3D v1=pp1->v;const vect3D v2=pp2->v;
	int32 val1=evaluatePlanePoint(pl,v1);
	int32 val2=evaluatePlanePoint(pl,v2);
	// const int32 val1=pp1->val;
	// const int32 val2=pp2->val=evaluatePlanePoint(pl,v2);
	if(val1>=0)
	{
		polygon_struct* p=createPolygon(v1);
		if(!p)return;
		p->next=*o;
		*o=p;
		if(val2<0)
		{
			vect3D dir=vectDifference(v2,v1);
			dir=vect(dir.x<<8,dir.y<<8,dir.z<<8); //improves precision, but limits polygon size; so be careful not to use polygons that are too big
			int32 dist=magnitude(dir); //not the actual distance between v1 and v2 but intersectSegmentPlane doesn't need it so...
			dir=divideVect(dir,dist);
			
			vect3D v=intersectSegmentPlane(pl,v1,dir,dist);
			
			p=createPolygon(v);
			if(!p)return;
			p->next=*o;
			*o=p;
		}
	}else{
		if(val2>=0)
		{
			vect3D dir=vectDifference(v2,v1);
			dir=vect(dir.x<<8,dir.y<<8,dir.z<<8); //improves precision, but limits polygon size; so be careful not to use polygons that are too big
			int32 dist=magnitude(dir); //not the actual distance between v1 and v2 but intersectSegmentPlane doesn't need it so...
			dir=divideVect(dir,dist);
			
			vect3D v=intersectSegmentPlane(pl,v1,dir,dist);
			
			polygon_struct* p=createPolygon(v);
			if(!p)return;
			p->next=*o;
			*o=p;
		}
	}
}

polygon_struct* clipPolygonPlane(plane_struct* pl, polygon_struct* p)
{
	if(!pl || !p || !p->next)return NULL;
	
	polygon_struct *pp1=p;
	polygon_struct *pp2=p->next;
	polygon_struct *out=NULL;
	
	pp1->val=evaluatePlanePoint(pl,pp1->v);
	
	while(pp2)
	{
		clipSegmentPlane(pl, &out, pp1, pp2);	
		pp1=pp2;
		pp2=pp2->next;
	}
	if(pp1!=p->next)clipSegmentPlane(pl, &out, pp1, p);
	return out;
}

// RIGHT AND BOTTOM PLANE PROBLEM ?
polygon_struct* clipPolygonFrustum(frustum_struct* f, polygon_struct* p)
{
	if(!p)return NULL;
	if(!f)f=&playerCamera.frustum;
	int i;
	
	// for(i=0;i<6;i++)
	for(i=2;i<6;i++)
	{
		polygon_struct* np=clipPolygonPlane(&f->plane[i],p);
		freePolygon(&p);
		p=np;
	}
	
	return p;
}

vect3D projectPoint(camera_struct* c, vect3D p)
{
	if(!c)c=&playerCamera;
	vect3D v;
	int32* m=c->viewMatrix;
	
	v.x=mulf32(p.x,m[0])+mulf32(p.y,m[4])+mulf32(p.z,m[8])+m[12];
	v.y=mulf32(p.x,m[1])+mulf32(p.y,m[5])+mulf32(p.z,m[9])+m[13];
	// v.z=mulf32(p.x,m[2])+mulf32(p.y,m[6])+mulf32(p.z,m[10])+m[14];
	int32 w=mulf32(p.x,m[3])+mulf32(p.y,m[7])+mulf32(p.z,m[11])+m[15];
	
	// GBATEK
	// screen_x = (xx+ww)*viewport_width / (2*ww) + viewport_x1
	// screen_y = (yy+ww)*viewport_height / (2*ww) + viewport_y1
	
	v.x=divf32(v.x+(w),(w)*2)*128/2048;
	v.y=192-divf32(v.y+(w),(w)*2)*96/2048;
	v.z=0;
	// v.z=divf32(v.z+(w),(w)*2);
	
	return v;
}

void projectPolygon(camera_struct* c, polygon_struct** p, vect3D o, vect3D u1, vect3D u2, int32 d1, int32 d2)
{
	if(!p)return;
	if(!c)c=&playerCamera;
	
	*p=clipPolygonFrustum(&c->frustum,*p);
	
	polygon_struct *pp=*p;
	
	while(pp)
	{
		// vect3D vr=vectDifference(pp->v,o);
		// pp->t=vect(dotProduct(vr,u1),dotProduct(vr,u2),0);
		// pp->t.x=(pp->t.x*inttot16(127))/d1;
		// pp->t.y=(pp->t.y*inttot16(127))/d2;
		
		pp->v=projectPoint(c,pp->v);
		pp=pp->next;
	}
}

void drawPolygon(polygon_struct* p)
{
	if(!p || !p->next || !p->next->next)return;	
	polygon_struct* pp1=p->next;
	polygon_struct* pp2=pp1->next;
	
	glBegin(GL_TRIANGLES);
	while(pp2)
	{		
		// GFX_TEX_COORD=TEXTURE_PACK(p->t.x,p->t.y);
		glVertex3v16(p->v.x, p->v.y, p->v.z);
		// GFX_TEX_COORD=TEXTURE_PACK(pp1->t.x,pp1->t.y);
		glVertex3v16(pp1->v.x, pp1->v.y, pp1->v.z);
		// GFX_TEX_COORD=TEXTURE_PACK(pp2->t.x,pp2->t.y);
		glVertex3v16(pp2->v.x, pp2->v.y, pp2->v.z);
		pp1=pp2;
		pp2=pp2->next;
	}
}

void drawPolygonStrip(polygon_struct* p, u16 col1, u16 col2)
{
	if(!p || !p->next || !p->next->next || !p->next->next->next)return;	
	polygon_struct* pp1=p;
	u8 cnt=0;
	
	glBegin(GL_QUAD_STRIP);
	while(pp1)
	{
		// GFX_TEX_COORD=TEXTURE_PACK(pp1->t.x,pp1->t.y);
		GFX_COLOR=cnt?col2:col1;
		glVertex3v16(pp1->v.x, pp1->v.y, pp1->v.z);
		pp1=pp1->next;
		cnt++;
		cnt%=2;
	}
}
