#ifndef LIGHTING_H
#define LIGHTING_H

typedef enum
{
	LIGHTMAP_DATA,
	VERTEXLIGHT_DATA
}lightingData_type;

typedef struct
{
	vect3D lmPos, lmSize;
	bool rot;
}lightMapCoordinates_struct;

typedef struct
{
	vect3D lmSize;
	u8* buffer;
	lightMapCoordinates_struct* coords;
	mtlImg_struct* texture;
}lightMapData_struct;

typedef struct
{
	u8 width, height;
	u8* values;
}vertexLightingData_struct;

typedef struct
{
	lightingData_type type;
	union{
		lightMapData_struct lightMap;
		vertexLightingData_struct* vertexLighting;
	}data;
	u16 size;
}lightingData_struct;

void initLightData(lightingData_struct* ld);
void freeLightData(lightingData_struct* ld);

void initLightDataLM(lightingData_struct* ld, u16 n);
void initLightDataVL(lightingData_struct* ld, u16 n);

#endif
