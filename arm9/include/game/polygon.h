#ifndef __POLYGON9__
#define __POLYGON9__

#define POLYPOOLSIZE 512

typedef struct polygon_struct
{
	vect3D v, t;
	vect3D dir;
	int32 val, dist;
	struct polygon_struct* next;
}polygon_struct;

void initPolygonPool(void);
void drawPolygon(polygon_struct* p);
void freePolygon(polygon_struct** p);
polygon_struct* createPolygon(vect3D v);
vect3D projectPoint(camera_struct* c, vect3D p);
polygon_struct* createEllipse(vect3D po, vect3D v1, vect3D v2, int n);
polygon_struct* clipPolygonPlane(plane_struct* pl, polygon_struct* p);
polygon_struct* createQuad(vect3D v1, vect3D v2, vect3D v3, vect3D v4);
polygon_struct* clipPolygonFrustum(frustum_struct* f, polygon_struct* p);
vect3D intersectSegmentPlane(plane_struct* pl, vect3D o, vect3D v, int32 d);
void clipSegmentPlane(plane_struct* pl, polygon_struct** o, polygon_struct* pp1, polygon_struct* pp2);
void projectPolygon(camera_struct* c, polygon_struct** p, vect3D o, vect3D u1, vect3D u2, int32 d1, int32 d2);

#endif
