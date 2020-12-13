#include "common/general.h"

#define DUAL3D_N_SPRITES 128

static SpriteEntry d3dSprites[DUAL3D_N_SPRITES];
bool d3dScreen;

void initSprites(void)
{
	int i;
	for(i = 0; i < DUAL3D_N_SPRITES; i++)
	{
	   d3dSprites[i].attribute[0] = ATTR0_DISABLED;
	   d3dSprites[i].attribute[1] = 0;
	   d3dSprites[i].attribute[2] = 0;
    }
}

void updateOAM(void)
{
	DC_FlushRange(d3dSprites, DUAL3D_N_SPRITES * sizeof(SpriteEntry));
	memcpy(OAM_SUB, d3dSprites, DUAL3D_N_SPRITES * sizeof(SpriteEntry));
}

void initD3D()
{
	int x,y,i;

	videoSetMode(MODE_3_3D);
	videoSetModeSub(MODE_5_2D | DISPLAY_BG2_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_2D_BMP_256);
    vramSetPrimaryBanks(VRAM_A_TEXTURE,VRAM_B_TEXTURE,VRAM_C_SUB_BG,VRAM_D_SUB_SPRITE);
	vramSetBankH(VRAM_H_LCD);
	vramSetBankI(VRAM_I_LCD);

	REG_BG0CNT = BG_PRIORITY(1);

	REG_BG2CNT_SUB = BG_BMP16_256x256 | BG_BMP_BASE(0) | BG_PRIORITY(1);
        REG_BG2PA_SUB = 1 << 8;
        REG_BG2PB_SUB = 0;
        REG_BG2PC_SUB = 0;
        REG_BG2PD_SUB = 1 << 8;

        REG_BG2X_SUB = 0;
        REG_BG2Y_SUB = 0;

	initSprites();

	pSpriteRotation d3dSpriteRotations = (pSpriteRotation)d3dSprites;
	d3dSpriteRotations[0].hdx=256;
	d3dSpriteRotations[0].hdy=0;
	d3dSpriteRotations[0].vdx=0;
	d3dSpriteRotations[0].vdy=256;

	i=0;

	for (y = 0; y < 3; y++)
	{
		for (x = 0; x < 4; x++) {
			d3dSprites[i].attribute[0] = ATTR0_BMP | ATTR0_SQUARE | (64 * y);
			d3dSprites[i].attribute[1] = ATTR1_SIZE_64 | (64 * x);
			d3dSprites[i].attribute[2] = ATTR2_ALPHA(1) | (8 * 32 * y) | (8 * x);
			i++;
		}
	}

	updateOAM();
	d3dScreen=true;
}

void setRegCapture(bool enable, uint8 srcBlend, uint8 destBlend, uint8 bank, uint8 offset, uint8 size, uint8 source, uint8 srcOffset)
{
	uint32 value=0;

	if(enable)value|=1 << 31; // 31 is enable
	value|=0 << 29; // 29-30 seems to have something to do with the blending
	value|=(srcOffset & 0x3) << 26; // capture source offset is 26-27
	value|=(source & 0x3) << 24; // capture source is 24-25
	value|=(size & 0x3) << 20; // capture data write size is 20-21
	value|=(offset & 0x3) << 18; // write offset is 18-19
	value|=(bank & 0x3) << 16; // vram bank select is 16-17
	value|=(srcBlend & 0xF) << 8; // graphics blend evb is 8..12
	value|=(destBlend & 0xF) << 0; // ram blend EVA is bits 0..4
	REG_DISPCAPCNT=value;
}

void updateD3D()
{
	if (d3dScreen) {
		videoSetModeSub(MODE_5_2D | DISPLAY_BG2_ACTIVE);
		vramSetBankC(VRAM_C_SUB_BG);
		vramSetBankD(VRAM_D_LCD);
		setRegCapture(true, 0, 15, 3, 0, 3, 0, 0);
		d3dScreen=false;
	}else{
		videoSetModeSub(MODE_5_2D | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_2D_BMP_256);
		vramSetBankC(VRAM_C_LCD);
		vramSetBankD(VRAM_D_SUB_SPRITE);
		setRegCapture(true, 0, 15, 2, 0, 3, 0, 0);
		d3dScreen=true;
	}
    lcdSwap();
}

