#ifndef OBB_H
#define OBB_H

#define NUMOBJECTS (8)

#define NUMOBBSEGMENTS (12)
#define NUMOBBFACES (6)
#define MAXCONTACTPOINTS (32) // pool system ?
#define PENETRATIONTHRESHOLD (1<<6)
#define MAXPENETRATIONBOX (1<<6)

#define SLEEPTHRESHOLD (50)
#define SLEEPTIMETHRESHOLD (48)

static const u8 OBBSegments[NUMOBBSEGMENTS][2]={{0,1},{1,2},{3,2},{0,3},
										 {5,4},{5,6},{6,7},{4,7},
										 {3,4},{0,5},{1,6},{2,7}};
static const u8 OBBSegmentsPD[NUMOBBSEGMENTS][2]={{0,0},{1,2},{3,0},{0,2},
										 {5,2},{5,0},{6,2},{4,0},
										 {3,1},{0,1},{1,1},{2,1}};


//static const s8 OBBFacesPDDN[NUMOBBFACES][4]={{0,0,2,-2},{5,0,2,2},{0,1,2,-1},{0,0,1,-3},{1,1,2,1},{3,0,1,3}};

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

extern contactPoint_struct contactPoints[MAXCONTACTPOINTS];

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
	u32 energy;
	u16 counter;
	s16 groundID;
	bool portaled;
	bool sleep;
	bool used;
}OBB_struct;
// 4 + 9*4*3 + 4 +

extern OBB_struct objects[NUMOBJECTS];
extern u32 coll, integ, impul;
extern u8 sleeping;

void initOBB(OBB_struct* o, vect3D size, vect3D pos, int32 mass, s32 cosine, s32 sine);
void initTransformationMatrix(int32* m, s32 cosine, s32 sine);
void getOBBVertices(OBB_struct* o, vect3D* v);
void drawOBB(OBB_struct* o);
void applyOBBImpulses(OBB_struct* o);
void applyOBBForce(OBB_struct* o, vect3D p, vect3D f);
void updateOBB(OBB_struct* o);

void initOBBs(void);
void updateOBBs(void);
void drawOBBs(void);
void wakeOBBs(void);
OBB_struct* createOBB(u8 id, vect3D size, vect3D position, int32 mass, s32 cosine, s32 sine);
void updateOBBPortals(OBB_struct* o, u8 id, bool init);
void getVertices(vect3D s, vect3D p, vect3D u1, vect3D u2, vect3D u3, vect3D* v);
void collideOBBs(OBB_struct* o1, OBB_struct* o2);

bool clipSegmentOBB(int32* ss, vect3D *uu, vect3D* p1, vect3D* p2, vect3D vv, vect3D* uu1, vect3D* uu2, vect3D vv1, vect3D* n1, vect3D* n2, bool* b1, bool* b2, int32* k1, int32* k2);

#endif
