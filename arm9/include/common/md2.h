#ifndef __MD29__
#define __MD29__

/* Vector */
typedef float vec3_t[3];

/* MD2 header */
typedef struct
{
	int ident;
	int version;

	int skinwidth;
	int skinheight;

	int framesize;

	int num_skins;
	int num_vertices;
	int num_st;
	int num_tris;
	int num_glcmds;
	int num_frames;

	int offset_skins;
	int offset_st;
	int offset_tris;
	int offset_frames;
	int offset_glcmds;
	int offset_end;
}md2_header_t;

/* Texture name */
typedef struct
{
	char name[64];
}md2_skin_t;

/* Texture coords */
typedef struct
{
	short s;
	short t;
}md2_texCoord_t;

/* Triangle info */
typedef struct
{
	unsigned short vertex[3];
	unsigned short st[3];
}md2_triangle_t;

/* Compressed vertex */
typedef struct
{
	unsigned char v[3];
	unsigned char normalIndex;
}md2_vertex_t;

/* Model frame */
typedef struct
{
	vect3D scale;
	vect3D translate;
	char name[16];
	md2_vertex_t *verts;
	vect3D *packedVerts;
	u32 *packedv10;
	vect3D* faceNormals;
	vect3D min, max;
	u16 next;
	u32* displayList[4];
}md2_frame_t;

typedef struct
{
	u16 start, end;
}md2Anim_struct;

/* MD2 model structure */
typedef struct
{
	md2_header_t header;
	
	u8 numAnim;
	md2Anim_struct* animations;

	mtlImg_struct* texture;

	md2_skin_t *skins;
	md2_texCoord_t *texcoords;
	u32 *packedTexcoords;
	md2_triangle_t *triangles;
	md2_frame_t *frames;
}md2Model_struct;

typedef struct
{
	u16 currentFrame, nextFrame, interpCounter;
	u8 currentAnim;
	u8 oldAnim;
	bool oneshot;
	u32* palette;
	md2Model_struct* model;
}modelInstance_struct;

int loadMd2Model (const char *filename, char *texname, md2Model_struct *mdl);
void freeMd2Model(md2Model_struct *mdl);
void renderModelFrame (int n, const md2Model_struct *mdl);
void renderModelFrameInterp(int n, int n2, int m, const md2Model_struct *mdl, u32 params, bool center, u32* pal);

void generateModelDisplayLists(md2Model_struct *mdl, bool interp, u8 normals);

void initModelInstance(modelInstance_struct* mi, md2Model_struct* mdl);
void changeAnimation(modelInstance_struct* mi, u16 newAnim, bool oneshot);
void updateAnimation(modelInstance_struct* mi);

#endif