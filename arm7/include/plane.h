#ifndef PLANE_H
#define PLANE_H

typedef struct
{
	int32 A, B, C, D;
	vect3D point;
}plane_struct;

static inline int32 evaluatePlanePoint(plane_struct* p, vect3D v)
{
	if(!p)return 0;
	return mulf32(p->A,v.x)+mulf32(p->B,v.y)+mulf32(p->C,v.z)+p->D;
}

plane_struct testPlane;

void initPlane(plane_struct* pl, int32 A, int32 B, int32 C, int32 D);
vect3D intersectSegmentPlane(plane_struct* pl, vect3D o, vect3D v, int32 d);
void planeOBBContacts(plane_struct* p, OBB_struct* o);


#endif