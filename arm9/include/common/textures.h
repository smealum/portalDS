#ifndef __TEXTURES9__
#define __TEXTURES9__

#define MAX_TEX 128
#define BANKS vramBanks

u8 vramBanks;

typedef struct
{
	size_t s_total, s_used, s_free;
	void* addr;
	int num_t;
}vramBank_struct;

typedef struct
{
	u16 width, height;
	u16 rwidth, rheight;
	int ID;
	int bank;
	size_t size;
	u32 param;
	char* name;
	void *addr, *pal;
	u32 palbind;
	bool used;
}mtlImg_struct;

static inline void Game_FastBind(mtlImg_struct *mtl)
{
	GFX_PAL_FORMAT = mtl->palbind;
	GFX_TEX_FORMAT = mtl->param;
}

void initTextures();
mtlImg_struct* createTexture(char* filename, char* directory);
mtlImg_struct* createTextureBuffer16(u16* buffer, u16 x, u16 y, bool cpy);
mtlImg_struct* createTextureBuffer(u8* buffer, u16* buffer2, u16 x, u16 y);
mtlImg_struct* createTextureBufferA5I3(u8* buffer, u16* buffer2, u16 x, u16 y);
mtlImg_struct* createReservedTextureBufferA5I3(u8* buffer, u16* buffer2, u16 x, u16 y, void* addr);
void loadToBank(mtlImg_struct *mtl, u8* data);
void loadPaletteToBank(mtlImg_struct *mtl, u16* data, size_t size);
void loadTextureBuffer16(u16* buffer, u16 x, u16 y, mtlImg_struct *mtl, bool genaddr, bool cpy);
void loadTextureBuffer(u8* buffer, u16* buffer2, u16 x, u16 y, mtlImg_struct *mtl);
void loadTextureBufferA5I3(u8* buffer, u16* buffer2, u16 x, u16 y, mtlImg_struct *mtl);
void loadReservedTextureBufferA5I3(u8* buffer, u16* buffer2, u16 x, u16 y, mtlImg_struct *mtl, void* addr);
void loadPartToBank(mtlImg_struct *mtl, u8* data, u16 w, u16 h, u16 x, u16 y, bool rot);
void loadTexturePCX(char* filename, char* directory, mtlImg_struct* mtl);
u32* loadPalettePCX(char* filename, char* directory);
void editPalette(u16* addr, u8 index, u16 color);
void applyMTL(mtlImg_struct *mtl);
void initVramBanks(u8 banks);
void getVramStatus();
void unbindMtl();
void addToBank(mtlImg_struct *mtl, u8* data, int b);
void setTextureParameter(mtlImg_struct *mtl, uint8 sizeX, uint8 sizeY, const uint32* addr, GL_TEXTURE_TYPE_ENUM mode, uint32 param);
void* getTextureAddress(mtlImg_struct *mtl);
void addPaletteToBank(mtlImg_struct *mtl, u16* data, size_t size);
void bindPalette(mtlImg_struct *mtl);
void bindPalette4(mtlImg_struct *mtl);
void bindTexture(mtlImg_struct *mtl);
void bindPaletteAddr(u32* addr);
void getGlWL(u16 width, u16 height, u8* w, u8* l);

void changeTextureSizeA5I3(mtlImg_struct *mtl, u16 x, u16 y);

#endif