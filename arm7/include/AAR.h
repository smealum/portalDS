#ifndef AAR_H //axis aligned rectangle !
#define AAR_H

#define NUMAARSEGMENTS (4)

#define NODESIZE (4096)

typedef struct
{
	vect3D position, size, normal;
	bool used;
}AAR_struct;
// 4*3 + 4*3 + 4*3 + 1 = 37
// 2*2 + 1 + 3*1 + 1 + 1 = 10

typedef struct
{
	u16* data;
	u8 length;
}node_struct;

typedef struct
{
	node_struct* nodes;
	u16 width, height;
	vect3D m, M;
}grid_struct;

void initAARs(void);
void drawAARs(void);
void toggleAAR(u16 id);
void generateGrid(grid_struct* g);
void updateAAR(u16 id, vect3D position);
void AARsOBBContacts(OBB_struct* o, bool sleep);
AAR_struct* createAAR(u16 id, vect3D position, vect3D size, vect3D normal);
bool AAROBBContacts(AAR_struct* a, OBB_struct* o, vect3D* v, bool port);

#endif