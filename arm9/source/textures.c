#include "common/general.h"

vramBank_struct bank[4];
vramBank_struct palBank;

mtlImg_struct texture[MAX_TEX];

void getGlWL(u16 width, u16 height, u8* w, u8* l)
{
	switch(width)
	{
		case 8:
			*w=TEXTURE_SIZE_8;
		break;
		case 16:
			*w=TEXTURE_SIZE_16;
		break;
		case 32:
			*w=TEXTURE_SIZE_32;
		break;
		case 64:
			*w=TEXTURE_SIZE_64;
		break;
		case 128:
			*w=TEXTURE_SIZE_128;
		break;
		case 256:
			*w=TEXTURE_SIZE_256;
		break;
		case 512:
			*w=TEXTURE_SIZE_512;
		break;
	}
	
	switch(height)
	{
		case 8:
			*l=TEXTURE_SIZE_8;
		break;
		case 16:
			*l=TEXTURE_SIZE_16;
		break;
		case 32:
			*l=TEXTURE_SIZE_32;
		break;
		case 64:
			*l=TEXTURE_SIZE_64;
		break;
		case 128:
			*l=TEXTURE_SIZE_128;
		break;
		case 256:
			*l=TEXTURE_SIZE_256;
		break;
		case 512:
			*l=TEXTURE_SIZE_512;
		break;
	}
}

void adjustDimension(u16* x)
{
	if(*x<=8)*x=8;
	else if(*x<=16)*x=16;
	else if(*x<=32)*x=32;
	else if(*x<=64)*x=64;
	else if(*x<=128)*x=128;
	else if(*x<=256)*x=256;
	else if(*x<=512)*x=512;
}

void initVramBanks(u8 banks)
{
	int i;
	BANKS=banks;
	for(i=0;i<BANKS;i++)
	{
		bank[i].s_total=128*1024;
		bank[i].s_free=128*1024;
		bank[i].s_used=0;
		bank[i].num_t=0;
		bank[i].addr=(void*)(0x6800000+0x0020000*i);
		//iprintf("%d : address %p\n",i,bank[i].addr);
	}
	palBank.s_total=64*1024;
	palBank.s_free=64*1024;
	palBank.s_used=0;
	palBank.num_t=0;
	palBank.addr=(void*)(0x6880000);
}

void initTextures()
{
	int i;
	for(i=0;i<MAX_TEX;i++)
	{
		texture[i].used=false;
		texture[i].ID=i;
	}
}

mtlImg_struct* createTexture(char* filename, char* directory)
{
	int i;
	if(!fileExists(filename, directory))return NULL;
	for(i=0;i<MAX_TEX;i++)
	{
		if(!texture[i].used)
		{
			int j;
			for(j=0;j<MAX_TEX;j++){if(texture[j].used){if(!strcmp(texture[j].name,filename)){NOGBA("STOP !!! (%s)",filename);return &texture[j];}}}
			NOGBA("num %d",i);
			loadTexturePCX(filename, directory, &texture[i]);
			return &texture[i];
		}
	}
	return NULL;
}

mtlImg_struct* createTextureBuffer(u8* buffer, u16* buffer2, u16 x, u16 y)
{
	int i;
	for(i=0;i<MAX_TEX;i++)
	{
		if(!texture[i].used)
		{
			loadTextureBuffer(buffer, buffer2, x, y, &texture[i]);
			return &texture[i];
		}
	}
	return NULL;
}

mtlImg_struct* createTextureBufferA5I3(u8* buffer, u16* buffer2, u16 x, u16 y)
{
	int i;
	for(i=0;i<MAX_TEX;i++)
	{
		if(!texture[i].used)
		{
			loadTextureBufferA5I3(buffer, buffer2, x, y, &texture[i]);
			return &texture[i];
		}
	}
	return NULL;
}

mtlImg_struct* createReservedTextureBufferA5I3(u8* buffer, u16* buffer2, u16 x, u16 y, void* addr)
{
	int i;
	for(i=0;i<MAX_TEX;i++)
	{
		if(!texture[i].used)
		{
			loadReservedTextureBufferA5I3(buffer, buffer2, x, y, &texture[i], addr);
			return &texture[i];
		}
	}
	return NULL;
}

mtlImg_struct* createTextureBuffer16(u16* buffer, u16 x, u16 y, bool cpy)
{
	int i;
	for(i=0;i<MAX_TEX;i++)
	{
		if(!texture[i].used)
		{
			loadTextureBuffer16(buffer, x, y, &texture[i], true, cpy);
			return &texture[i];
		}
	}
	return NULL;
}

void getVramStatus()
{
	int i;
	for(i=0;i<BANKS;i++)
	{
		NOGBA("%d : %d/%dko (%dko used)\n",i,bank[i].s_free/1024,bank[i].s_total/1024,bank[i].s_used/1024);
	}
}

void loadToBank(mtlImg_struct *mtl, u8* data)
{
	u32 vramTemp = vramSetPrimaryBanks(VRAM_A_LCD,VRAM_B_LCD,VRAM_C_LCD,VRAM_D_LCD);
	NOGBA("loading to %p",mtl->addr);
		memcpy(mtl->addr, data, mtl->size);
	vramRestorePrimaryBanks(vramTemp);
}

void loadPartToBank(mtlImg_struct *mtl, u8* data, u16 w, u16 h, u16 x, u16 y, bool rot)
{
	u32 vramTemp = vramSetPrimaryBanks(VRAM_A_LCD,VRAM_B_LCD,VRAM_C_LCD,VRAM_D_LCD);
	if(!rot)
	{
		int j;
		for(j=0;j<h;j++)
		{
			// memcpy(&(((u8*)(mtl->addr))[x+(y+j)*mtl->width]), &data[j*w], w);
			int i;
			for(i=0;i<w/2;i++){(((u16*)(mtl->addr))[x/2+i+(y+j)*mtl->width/2])=data[i*2+j*w]|(data[i*2+1+j*w]<<8);}//NOGBA("lala %d",data[i+j*w]);}
		}
	}else{
		int j;
		for(j=0;j<h;j++)
		{
			int i;
			for(i=0;i<w/2;i++){(((u16*)(mtl->addr))[x/2+j+(y+i)*mtl->width/2])=data[i*2+j*w]|(data[i*2+1+j*w]<<8);}
		}
	}
	vramRestorePrimaryBanks(vramTemp);
}

void addToBank(mtlImg_struct *mtl, u8* data, int b)
{
	NOGBA("h : %d, %d, %p",bank[b].s_used,mtl->size,mtl->addr);
	bank[b].s_used+=mtl->size;
	bank[b].s_free=bank[b].s_total-bank[b].s_used;
	bank[b].num_t++;
	
	loadToBank(mtl, data);
}

void reserveInBank(mtlImg_struct *mtl, u8* data, vramBank_struct* b, int pal)
{
	NOGBA("h : %d %d, %d, %p",mtl->bank,b->s_used,mtl->size,mtl->addr);
	int s=mtl->size;
	if(pal)s=pal;
	b->s_used+=s;
	b->s_free=b->s_total-b->s_used;
	b->num_t++;
}

//function from libnds
void setTextureParameter(mtlImg_struct *mtl, uint8 sizeX, uint8 sizeY, const uint32* addr, GL_TEXTURE_TYPE_ENUM mode, uint32 param)
{
	mtl->param = param | (sizeX << 20) | (sizeY << 23) | (((uint32)addr >> 3) & 0xFFFF) | (mode << 26);
}

void* getTextureAddress(mtlImg_struct *mtl)
{
	int i;
	for(i=0;i<BANKS;i++)
	{
		if(bank[i].s_free>=mtl->size)
		{
			mtl->bank=i;
			mtl->addr=(void*)(bank[i].addr+0x1*bank[i].s_used);
			return mtl->addr;
		}
	}
	return 0x0;
}

void loadPaletteToBank(mtlImg_struct *mtl, u16* data, size_t size)
{
	if(data)
	{
		vramSetBankE(VRAM_E_LCD);
			memcpy(mtl->pal, data, size);
		vramSetBankE(VRAM_E_TEX_PALETTE);
	}
}

void addPaletteToBank(mtlImg_struct *mtl, u16* data, size_t size)
{
	mtl->pal=(void*)(palBank.addr+0x1*palBank.s_used);
	palBank.s_used+=size;
	palBank.s_free=palBank.s_total-palBank.s_used;
	palBank.num_t++;
	NOGBA("palettes : %d (%p)\n",palBank.num_t,mtl->pal);
	loadPaletteToBank(mtl, data, size);
}

void editPalette(u16* addr, u8 index, u16 color)
{
	if(!addr)return;
	vramSetBankE(VRAM_E_LCD);
		addr[index]=color;
	vramSetBankE(VRAM_E_TEX_PALETTE);
}

void bindPalette(mtlImg_struct *mtl)
{
	GFX_PAL_FORMAT = ((uint32)mtl->pal)>>(4);
}

void bindPaletteAddr(u32* addr)
{
	GFX_PAL_FORMAT = ((uint32)addr)>>(4);
}

void bindPalette4(mtlImg_struct *mtl)
{
	GFX_PAL_FORMAT = ((uint32)mtl->pal)>>(4-1);
}

void bindTexture(mtlImg_struct *mtl)
{
	if(mtl)GFX_TEX_FORMAT = mtl->param;
	else unbindMtl();
}

void unbindMtl()
{
		GFX_TEX_FORMAT = 0; 
}

void loadTextureBuffer(u8* buffer, u16* buffer2, u16 x, u16 y, mtlImg_struct *mtl)
{
	int param;
	uint8 texX=0, texy=0;
	
	mtl->used=true;
	
	mtl->width=x;
	mtl->height=y;

	adjustDimension(&mtl->width);
	mtl->size=mtl->width*mtl->height;
	adjustDimension(&mtl->height);
		
	if(buffer2)addPaletteToBank(mtl, buffer2, 256*2);
	else addPaletteToBank(mtl, NULL, 256*2);
	
	getTextureAddress(mtl);
	
	if(buffer)addToBank(mtl, buffer, mtl->bank);
	else reserveInBank(mtl, (u8*)buffer, &bank[mtl->bank], 0);
	
	// adjustDimension(&mtl->height);
	
	if(buffer2 && buffer2[0]==RGB15(31,0,31))param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | (1<<29);
	else param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T;	
	
	getGlWL(mtl->width, mtl->height, &texX, &texy);
	
	setTextureParameter(mtl, texX, texy, mtl->addr, GL_RGB256, param);

}

void loadTextureBufferA5I3(u8* buffer, u16* buffer2, u16 x, u16 y, mtlImg_struct *mtl)
{
	int param;
	uint8 texX=0, texy=0;
	
	mtl->used=true;
	
	mtl->width=x;
	mtl->height=y;

	adjustDimension(&mtl->width);
	mtl->size=mtl->width*mtl->height;
	adjustDimension(&mtl->height);
		
	if(buffer2)addPaletteToBank(mtl, buffer2, 8*2);
	else addPaletteToBank(mtl, NULL, 8*2);
	
	getTextureAddress(mtl);
	
	if(buffer)addToBank(mtl, buffer, mtl->bank);
	else reserveInBank(mtl, (u8*)buffer, &bank[mtl->bank], 0);
	
	// adjustDimension(&mtl->height);
	
	if(buffer2 && buffer2[0]==RGB15(31,0,31))param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | (1<<29);
	else param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T;	
	
	getGlWL(mtl->width, mtl->height, &texX, &texy);
	
	setTextureParameter(mtl, texX, texy, mtl->addr, GL_RGB8_A5, param);
}

void changeTextureSizeA5I3(mtlImg_struct *mtl, u16 x, u16 y)
{
	uint8 texX=0, texY=0;
	mtl->width=mtl->rwidth=x;
	mtl->height=mtl->rheight=y;
	mtl->size=mtl->width*mtl->height;
	getGlWL(mtl->width, mtl->height, &texX, &texY);
	int32 param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T;
	setTextureParameter(mtl, texX, texY, mtl->addr, GL_RGB8_A5, param);
	// mtl->param&=(1<<31)|(((1<<31)-1)^(((1<<7)-1)<<20));
	// mtl->param|=(texX << 20) | (texY << 23);
}

void loadReservedTextureBufferA5I3(u8* buffer, u16* buffer2, u16 x, u16 y, mtlImg_struct *mtl, void* addr)
{
	int param;
	uint8 texX=0, texy=0;
	
	mtl->used=true;
	
	mtl->width=x;
	mtl->height=y;

	adjustDimension(&mtl->width);
	mtl->size=mtl->width*mtl->height;
	adjustDimension(&mtl->height);
		
	if(buffer2)addPaletteToBank(mtl, buffer2, 8*2);
	else addPaletteToBank(mtl, NULL, 8*2);
	
	mtl->addr=addr;
	// getTextureAddress(mtl);
	
	// if(buffer)addToBank(mtl, buffer, mtl->bank);
	// else reserveInBank(mtl, (u8*)buffer, &bank[mtl->bank], 0);
	
	// adjustDimension(&mtl->height);
	
	if(buffer2 && buffer2[0]==RGB15(31,0,31))param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | (1<<29);
	else param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T;	
	
	getGlWL(mtl->width, mtl->height, &texX, &texy);
	
	setTextureParameter(mtl, texX, texy, mtl->addr, GL_RGB8_A5, param);
}

void loadTextureBuffer16(u16* buffer, u16 x, u16 y, mtlImg_struct *mtl, bool genaddr, bool cpy)
{
	int param;
	uint8 texX=0, texy=0;
	
	mtl->used=true;

	mtl->width=x;
	mtl->height=y;	
	mtl->size=mtl->width*mtl->height*2;
	
	param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T;	
	
	if(genaddr)getTextureAddress(mtl);
	getGlWL(mtl->width, mtl->height, &texX, &texy);
	
	setTextureParameter(mtl, texX, texy, mtl->addr, GL_RGBA, param);

	if(cpy)addToBank(mtl, (u8*)buffer, mtl->bank);
	else reserveInBank(mtl, (u8*)buffer, &bank[mtl->bank], 0);
}

void loadTexturePCX(char* filename, char* directory, mtlImg_struct* mtl)
{
	int param;
	uint8 texX=0, texy=0;
	u8* texels=NULL;
	
	mtl->used=true;
	mtl->name=alloc(strlen(filename)+1,NULL);
	strcpy(mtl->name,filename);
	
	struct gl_texture_t *pcxt=(struct gl_texture_t *)ReadPCXFile(filename,directory);
	
	mtl->rwidth=mtl->width=pcxt->width;
	mtl->rheight=mtl->height=pcxt->height;
	
	adjustDimension(&mtl->width);
	adjustDimension(&mtl->height);
	
	if(pcxt->palette[0]==RGB15(31,0,31))param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | (1<<29);
	else param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T;	
	
	NOGBA("format : %d",pcxt->format);
	switch(pcxt->format)
	{
		case 4:
			addPaletteToBank(mtl, pcxt->palette, 16*2);
			mtl->size=(mtl->width*mtl->height)/2;
			getTextureAddress(mtl);
			getGlWL(mtl->width, mtl->height, &texX, &texy);
			setTextureParameter(mtl, texX, texy, mtl->addr, GL_RGB16, param);
			if(mtl->width!=pcxt->width || mtl->height!=pcxt->height) //only for even widths
			{
				texels=malloc(mtl->size);
				int j;
				for(j=0;j<pcxt->height;j++)
				{
					int i;
					memcpy(&texels[j*mtl->width/2],&pcxt->texels[j*pcxt->width/2],pcxt->width/2);
					for(i=pcxt->width/2;i<mtl->width/2;i++)texels[i+j*mtl->width/2]=0;
				}
				for(j=pcxt->height*mtl->width/2;j<mtl->height*mtl->width/2;j++)texels[j]=0;
			}
			break;
		case 8:
			addPaletteToBank(mtl, pcxt->palette, 256*2);
			mtl->size=mtl->width*mtl->height;
			getTextureAddress(mtl);
			getGlWL(mtl->width, mtl->height, &texX, &texy);
			setTextureParameter(mtl, texX, texy, mtl->addr, GL_RGB256, param);
			if(mtl->width!=pcxt->width || mtl->height!=pcxt->height){
				texels=malloc(mtl->size);
				int j;
				for(j=0;j<pcxt->height;j++)
				{
					int i;
					memcpy(&texels[j*mtl->width],&pcxt->texels[j*pcxt->width],pcxt->width);
					for(i=pcxt->width;i<mtl->width;i++)texels[i+j*mtl->width]=0;
				}
				for(j=pcxt->height*mtl->width;j<mtl->height*mtl->width;j++)texels[j]=0;
			}
			break;
	}

	if(mtl->width!=pcxt->width || mtl->height!=pcxt->height){if(texels){addToBank(mtl, texels, mtl->bank);free(texels);}}
	else addToBank(mtl, pcxt->texels, mtl->bank);
	
	freePCX(pcxt);
	
	NOGBA("%s loaded.\n",filename);
}

u32* loadPalettePCX(char* filename, char* directory)
{	
	struct gl_texture_t *pcxt=(struct gl_texture_t *)ReadPCXFile(filename,directory);
	mtlImg_struct mtl;
	
	NOGBA("format : %d",pcxt->format);
	switch(pcxt->format)
	{
		case 4:
			addPaletteToBank(&mtl, pcxt->palette, 16*2);
			break;
		case 8:
			addPaletteToBank(&mtl, pcxt->palette, 256*2);
			break;
	}
	
	freePCX(pcxt);
	
	return mtl.pal;
}

void applyMTL(mtlImg_struct* mtl)
{
	bindPalette(mtl);
	bindTexture(mtl);
}
