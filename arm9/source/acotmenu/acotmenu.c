#include "acotmenu/acotmenu_main.h"

#define NUMFRAMES 16
#define TOTALFRAMES 130
#define EMAILPERIOD 30
#define BUTTONLENGTH 30

struct gl_texture_t* videoFrames[NUMFRAMES];
struct gl_texture_t* bottom1;
struct gl_texture_t* bottom2;
struct gl_texture_t* message[3];
u16 messagePalette[256];
int emailCounter=0;
int currentFrame=0;
int bg3;
int bg3Sub, bg2Sub;

void copyImage(u16* dest, int h)
{
	int offset=0;
	if(h<256)
	{
		int size=max(min((256-h)*256,192*256),0);
		memcpy(dest,&message[0]->texels[h*256],size);
		offset+=size;
		size=max((h+192-256)*256,0);
		memcpy(&dest[offset/2],message[1]->texels,size);
		offset+=size;
	}else{
		int size=max(min((512-h)*256,192*256),0);
		memcpy(dest,&message[1]->texels[(h-256)*256],size);
		offset+=size;
		size=min(max((h+192-512)*256,0),192*256);
		memcpy(&dest[offset/2],&message[2]->texels[max((h-512)*256,0)],size);
		offset+=size;
	}
}

int messageHeight=0;

int state=0;

void initACOTMenu(void)
{
	NOGBA("mem free : %dko (%do)",getMemFree()/1024,getMemFree());
	int i;
	for(i=0;i<NUMFRAMES;i++)
	{
		char str[255];
		sprintf(str,"video200%d.pcx",22+i*2);
		NOGBA("str : %s",str);
		videoFrames[i]=(struct gl_texture_t *)ReadPCXFile(str,"menu");
	
		NOGBA("free : %dko",getMemFree()/1024);
	}
	bottom1=(struct gl_texture_t *)ReadPCXFile("bottom1.pcx","menu");
	bottom2=(struct gl_texture_t *)ReadPCXFile("bottom2.pcx","menu");
	message[0]=(struct gl_texture_t *)ReadPCXFile("message1.pcx","menu");
	message[1]=(struct gl_texture_t *)ReadPCXFile("message2.pcx","menu");
	message[2]=(struct gl_texture_t *)ReadPCXFile("message3.pcx","menu");
	
	
	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_5_2D);
	
    vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
    vramSetBankC(VRAM_C_SUB_BG);

	bg3 = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0,0);
	bg3Sub = bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 0,0);
	bg2Sub = bgInitSub(2, BgType_Bmp8, BgSize_B8_256x256, 3,0);

	dmaCopy(videoFrames[0]->texels, bgGetGfxPtr(bg3), 256*192);
	dmaCopy(videoFrames[0]->palette, BG_PALETTE, 256*2);
	
	dmaCopy(bottom1->texels, bgGetGfxPtr(bg3Sub), 256*192);
	dmaCopy(bottom1->palette, BG_PALETTE_SUB, 256*2);
	
	currentFrame=0;
	emailCounter=0;
	messageHeight=0;
	state=0;
	NOGBA("START mem free : %dko (%do)",getMemFree()/1024,getMemFree());
	fadeIn();
}

touchPosition oldTouchPos;
void ACOTMenuFrame(void)
{
	touchPosition touchPos;
	touchRead(&touchPos);
	scanKeys();
	
	int realFrame=currentFrame;
	if(currentFrame>=NUMFRAMES)realFrame=13+(currentFrame%2);
	dmaCopy(videoFrames[realFrame]->texels, bgGetGfxPtr(bg3), 256*192);
	dmaCopy(videoFrames[realFrame]->palette, BG_PALETTE, 256*2);
	currentFrame++;
	if(currentFrame>=TOTALFRAMES)currentFrame=0;
	emailCounter++;
	if(emailCounter>=EMAILPERIOD){if(emailCounter>=EMAILPERIOD+BUTTONLENGTH)emailCounter=0;BG_PALETTE_SUB[1]=RGB15(31,22,0);}
	else BG_PALETTE_SUB[1]=RGB15(31,31,31);
	
			if(keysDown()&(KEY_SELECT))changeState(&ACOTMenuState);
	
	switch(state)
	{
		case 0:
			if(oldTouchPos.px>23&&oldTouchPos.py>140&&oldTouchPos.px<240&&oldTouchPos.py<155)
			{
				if((keysHeld()&KEY_TOUCH))
				{
					BG_PALETTE_SUB[1]=RGB15(31,22,0);
					emailCounter=0;
				}else if(keysUp()&KEY_TOUCH)
				{
					emailCounter=0;
					dmaCopy(bottom2->texels, bgGetGfxPtr(bg2Sub), 256*192);
					dmaCopy(bottom2->palette, BG_PALETTE_SUB, 256*2);
					state++;
				}
			}
			break;
		default:
			if(oldTouchPos.px>50&&oldTouchPos.py>144&&oldTouchPos.px<222&&oldTouchPos.py<155)
			{
				if((keysHeld()&KEY_TOUCH))
				{
					BG_PALETTE_SUB[1]=RGB15(31,22,0);
					emailCounter=0;
				}else if(keysUp()&KEY_TOUCH)
				{
					//START GAME HERE
					changeState(&gameState);
				}
			}else if((keysHeld()&KEY_TOUCH)&&!(keysDown()&KEY_TOUCH))messageHeight-=touchPos.py-oldTouchPos.py;
			if(messageHeight<0)messageHeight=0;
			if(messageHeight>=768-192)messageHeight=768-193;
			copyImage(bgGetGfxPtr(bg3Sub), messageHeight);
			break;
	}
	
	swiWaitForVBlank();
	swiWaitForVBlank();
	oldTouchPos=touchPos;
}

void killACOTMenu(void)
{
	fadeOut();
	int i;
	for(i=0;i<NUMFRAMES;i++)
	{
		freePCX(videoFrames[i]);
	}
	freePCX(bottom1);
	freePCX(bottom2);
	freePCX(message[0]);
	freePCX(message[1]);
	freePCX(message[2]);
	NOGBA("END mem free : %dko (%do)",getMemFree()/1024,getMemFree());
}

void ACOTMenuVBL(void)
{

}
