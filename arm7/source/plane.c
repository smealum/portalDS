#include "stdafx.h"

void initPlane(plane_struct* pl, int32 A, int32 B, int32 C, int32 D)
{
	if(!pl)return;
	pl->A=A;pl->B=B;pl->C=C;pl->D=D;
	int32 r=magnitude(vect(A,B,C));
	pl->A=divv16(pl->A,r);
	pl->B=divv16(pl->B,r);
	pl->C=divv16(pl->C,r);
	pl->D=divv16(pl->D,r);
	pl->point.x=-mulf32(pl->D,pl->A);
	pl->point.y=-mulf32(pl->D,pl->B);
	pl->point.z=-mulf32(pl->D,pl->C);
}

vect3D intersectSegmentPlane(plane_struct* pl, vect3D o, vect3D v, int32 d)
{
	if(!pl)return o;
	vect3D n=vect(pl->A,pl->B,pl->C);
	int32 p1=dotProduct(v,n);

	vect3D p=pl->point;		
	int32 p2=dotProduct(vectDifference(p,o),n);
	int32 k=divv16(p2,p1);
	return addVect(o,vectMult(v,k));
}

void planeOBBContacts(plane_struct* p, OBB_struct* o)
{
	if(!p || !o)return;
	int i;
	vect3D v[8];
	getOBBVertices(o,v);
	o->maxPenetration=0;
	o->numContactPoints=0;
	for(i=0;i<8;i++)
	{
		int32 val1=evaluatePlanePoint(p,v[i]);
		if(val1<=PENETRATIONTHRESHOLD)
		{
			o->contactPoints[o->numContactPoints].point=v[i];
			o->contactPoints[o->numContactPoints].normal=vect(p->A,p->B,p->C);
			o->contactPoints[o->numContactPoints].penetration=abs(val1);
			o->contactPoints[o->numContactPoints].target=p;
			o->contactPoints[o->numContactPoints].type=PLANECOLLISION;
			o->maxPenetration=max(o->maxPenetration,-(min(val1,0)));
			o->numContactPoints++;
		}
	}
}
