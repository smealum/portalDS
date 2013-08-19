#include "common/general.h"

#define t1 NORMAL_PACK(-32,-32,0)
#define t2 NORMAL_PACK(-32,32,0)
#define t3 NORMAL_PACK(32,32,0)
#define t4 NORMAL_PACK(32,-32,0)

font_struct hudFont;
font_struct* currentFont;

void initText(void)
{
	loadFont(&hudFont,16,8);
}

void setFont(font_struct* f)
{
	currentFont=f;
}

void loadFont(font_struct* f, u8 charsize, u8 rendersize)
{
	int i, j;
	int param;
	uint8 texX, texy;
	u8 buffer[512*64/4];
	sImage pcx; 
	u8 *buffer2;
	u16 palette[4];
	
	buffer2=bufferizeFile("HUD.pcx", "font", NULL, true);
	if(!buffer2)return;

	loadPCX((u8*)buffer2, &pcx);
	
	palette[0]=RGB15(31,0,31);
	palette[1]=RGB15(31,31,31);
	palette[2]=RGB15(0,0,0);
	palette[3]=RGB15(0,0,0);
	
	for(j=0;j<64;j++)
	{
		for(i=0;i<512/4;i++)
		{
			buffer[i+j*512/4]=(pcx.image.data8[i*4+j*512])|((pcx.image.data8[i*4+1+j*512])<<2)|((pcx.image.data8[i*4+2+j*512])<<4)|((pcx.image.data8[i*4+3+j*512])<<6);
		}	
	}
	
	imageDestroy(&pcx);
	free(buffer2);
	
	f->tex.used=true;

	f->tex.width=512;
	f->tex.height=64;	
	f->tex.size=f->tex.width*f->tex.height/4;
		
	addPaletteToBank(&f->tex, palette, 8*2);
	param = TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | (1<<29);
	
	getTextureAddress(&f->tex);
	getGlWL(f->tex.width, f->tex.height, &texX, &texy);
	
	setTextureParameter(&f->tex, texX, texy, f->tex.addr, GL_RGB4, param);

	addToBank(&f->tex, buffer, f->tex.bank);
	
	f->charsize=charsize;
	f->rendersize=rendersize;
	f->charsizef32=inttof32(charsize);
	
	setFont(f);
}

void drawCharRelative(char c)
{
	c-=32;
	
	int tx=(c*16)%512;
	int ty=16*(c*16-tx)/512;	
		
	glBegin(GL_QUADS);
		GFX_TEX_COORD = TEXTURE_PACK(inttot16(tx), inttot16(ty));
		GFX_VERTEX10 = t1;
		GFX_TEX_COORD = TEXTURE_PACK(inttot16(tx), inttot16(ty+16));
		GFX_VERTEX10 = t2;
		GFX_TEX_COORD = TEXTURE_PACK(inttot16(tx+16), inttot16(ty+16));
		GFX_VERTEX10 = t3;
		GFX_TEX_COORD = TEXTURE_PACK(inttot16(tx+16), inttot16(ty));
		GFX_VERTEX10 = t4;

	glTranslatef32(inttof32(1)/2, 0, 0);
}

void drawChar(char c, u16 color, int32 x, int32 y)
{
	c-=32;
	
	int tx=(c*16)%512;
	int ty=16*(c*16-tx)/512;

	glPushMatrix();

	glColor(color);

	bindTexture(&currentFont->tex);
	bindPalette4(&currentFont->tex);
	
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_ID(63));
	
	glTranslatef32(x, y, 0);
	
	glScalef32((currentFont->charsizef32),(currentFont->charsizef32),inttof32(1));
	
	glBegin(GL_QUADS);
	
		GFX_TEX_COORD = TEXTURE_PACK(inttot16(tx), inttot16(ty));
		GFX_VERTEX10 = t1;
		GFX_TEX_COORD = TEXTURE_PACK(inttot16(tx), inttot16(ty+16));
		GFX_VERTEX10 = t2;
		GFX_TEX_COORD = TEXTURE_PACK(inttot16(tx+16), inttot16(ty+16));
		GFX_VERTEX10 = t3;
		GFX_TEX_COORD = TEXTURE_PACK(inttot16(tx+16), inttot16(ty));
		GFX_VERTEX10 = t4;

	glEnd();
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);
	
	glPopMatrix(1);
}

void drawString(char* s, u16 color, int32 size, int32 x, int32 y)
{
	int n=strlen(s);
	int i;
	
	glColor(color);
	bindTexture(&currentFont->tex);
	bindPalette4(&currentFont->tex);
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_ID(48));

	glPushMatrix();
		glTranslatef32(x, y, 0);
		glScalef32((currentFont->charsizef32),(currentFont->charsizef32),inttof32(1));
		glScalef32(size, size, inttof32(1));
		glTranslatef32(inttof32(1)/4, inttof32(1)/2, 0);
		for(i=0;i<n;i++)
		{
			drawCharRelative(s[i]);
		}
	glPopMatrix(1);
	unbindMtl();
}
