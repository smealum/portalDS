#ifndef OBB_H
#define OBB_H

#define NUMOBJECTS (8)

#define NUMOBBSEGMENTS (12)
#define NUMOBBFACES (6)
#define MAXCONTACTPOINTS (32) // pool system ?
#define PENETRATIONTHRESHOLD (1<<6)
#define MAXPENETRATIONBOX (1<<6)

static const u8 OBBSegments[NUMOBBSEGMENTS][2]={{0,1},{1,2},{3,2},{0,3},
										 {5,4},{5,6},{6,7},{4,7},
										 {3,4},{0,5},{1,6},{2,7}};
static const u8 OBBSegmentsPD[NUMOBBSEGMENTS][2]={{0,0},{1,2},{3,0},{0,2},
										 {5,2},{5,0},{6,2},{4,0},
										 {3,1},{0,1},{1,1},{2,1}};

static const u8 OBBFaces[NUMOBBFACES][4]={{0,1,2,3},{4,5,6,7},{0,5,4,3},{0,1,6,5},{1,2,7,6},{2,3,4,7}};
static const s8 OBBFacesPDDN[NUMOBBFACES][4]={{0,0,2,-2},{5,0,2,2},{0,1,2,-1},{0,0,1,-3},{1,1,2,1},{3,0,1,3}};

typedef enum
{
	BOXCOLLISION,
	PLANECOLLISION,
	TESTPOINT,
	AARCOLLISION
}contactPoint_type;

typedef struct
{
	vect3D point;
	vect3D normal;
	u16 penetration;
	void* target;
	contactPoint_type type;
}contactPoint_struct;

contactPoint_struct contactPoints[MAXCONTACTPOINTS];

typedef struct
{
	int32 mass;
	int32 transformationMatrix[9]; //3x3
	int32 invInertiaMatrix[9]; //3x3
	int32 invWInertiaMatrix[9]; //3x3
	u16 maxPenetration;
	contactPoint_struct* contactPoints; //all point to the same array, temporary
	u8 numContactPoints;
	vect3D size;
	vect3D position;
	vect3D velocity, angularVelocity, forces, moment;
	vect3D angularMomentum;
	vect3D AABBo, AABBs;
	u8 portal[2];
	u8 oldPortal[2];
	s16 groundID;
	bool used;
}OBB_struct;
// 4 + 9*4*3 + 4 + 

extern OBB_struct objects[NUMOBJECTS];

void initOBB(OBB_struct* o, vect3D size, vect3D pos);
void initTransformationMatrix(int32* m);
void getOBBVertices(OBB_struct* o, vect3D* v);
void drawOBB(OBB_struct* o);
void applyOBBImpulses(OBB_struct* o);
void applyOBBForce(OBB_struct* o, vect3D p, vect3D f);
void updateOBB(OBB_struct* o);

void initOBBs(void);
void updateOBBs(void);
void drawOBBs(void);
OBB_struct* createOBB(u8 id, vect3D size, vect3D position);
void collideOBBs(OBB_struct* o1, OBB_struct* o2);

bool clipSegmentOBB(int32* ss, vect3D *uu, vect3D* p1, vect3D* p2, vect3D vv, vect3D* uu1, vect3D* uu2, vect3D vv1, vect3D* n1, vect3D* n2, bool* b1, bool* b2, int32* k1, int32* k2);

#endif
