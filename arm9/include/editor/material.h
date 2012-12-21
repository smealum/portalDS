#ifndef __MATERIAL9__
#define __MATERIAL9__

#define NUMMATERIALS 256
#define NUMMATERIALSLICES 256

typedef struct
{
	mtlImg_struct* img;
	u16 id;
	s16 factor;
	bool align;
	bool used;
}materialSlice_struct;

typedef struct
{
	materialSlice_struct* top;
	materialSlice_struct* side;
	materialSlice_struct* bottom;
	u16 id;
	bool used;
}material_struct;

static inline u16 getMaterialID(material_struct* m){if(m)return m->id;return 0;}

void initMaterials(void);
material_struct* createMaterial();
materialSlice_struct* createMaterialSlice();
void loadMaterialSlice(materialSlice_struct* ms, char* filename);
void loadMaterialSlices(char* filename);
void loadMaterials(char* filename);

material_struct* getMaterial(u16 i);

char** getMaterialList(int* m, int** cl);
void freeMaterialList(char** l);

#endif
