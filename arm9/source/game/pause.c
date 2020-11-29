#include "game/game_main.h"

struct gl_texture_t* pauseImage;
struct gl_texture_t* pauseButtonsImage;
extern int mainBG;

vect3D pauseButtonSize=(vect3D){128,29,0};
vect3D pauseButtonPositions[]={(vect3D){59,47,0}, (vect3D){59,85,0}, (vect3D){59,123,0}};

void initPause(void)
{
	pauseImage=(struct gl_texture_t *)ReadPCXFile("pause.pcx","");
	convertPCX16Bit(pauseImage);
	pauseButtonsImage=(struct gl_texture_t *)ReadPCXFile("pause_buttons.pcx","");
	convertPCX16Bit(pauseButtonsImage);
}

//inelegant/inefficient, but wgaf
void drawPixelArea(u16* b1, u16* b2, vect3D o, vect3D s, vect3D v, u16 w)
{
	if(!b1)return;

	int i, j;
	u16* p=bgGetGfxPtr(mainBG);
	if(b2)
	{
		for(i=o.x;i<o.x+s.x;i++)
		{
			for(j=o.y;j<o.y+s.y;j++)
			{
				if(b1[(i-v.x)+(j-v.y)*w]==RGB15(31,0,31))p[i+j*256]=b2[(i-v.x)+(j-v.y)*w];
				else p[i+j*256]=b1[(i-v.x)+(j-v.y)*w];
			}
		}
	}else{
		for(i=o.x;i<o.x+s.x;i++)
		{
			for(j=o.y;j<o.y+s.y;j++)
			{
				p[i+j*256]=b1[(i-v.x)+(j-v.y)*w];
			}
		}
	}
}

void doPause(u16* buffer)
{
	lcdMainOnBottom();
	drawPixelArea(pauseImage->texels16, buffer, vect(0,0,0), vect(256,192,0), vect(0,0,0), 256);
	touchPosition tp;
	pausePI();
	u8 done=0;
	while(!done)
	{
		scanKeys();
		int i; for(i=0;i<3;i++)
		{
			if(tp.px>pauseButtonPositions[i].x && tp.py>pauseButtonPositions[i].y && tp.px<pauseButtonPositions[i].x+pauseButtonSize.x && tp.py<pauseButtonPositions[i].y+pauseButtonSize.y)
			{
				drawPixelArea(pauseButtonsImage->texels16, buffer, pauseButtonPositions[i], pauseButtonSize, addVect(pauseButtonPositions[i],vect(0,-30*i,0)), 128);
				if(!(keysHeld() & KEY_TOUCH))done=i+1;
			}else{
				drawPixelArea(pauseImage->texels16, buffer, pauseButtonPositions[i], pauseButtonSize, vect(0,0,0), 256);
			}
		}

		touchRead(&tp);
		tp.px = (float)(tp.rawx)*256.0/4080.0;
		tp.py = (float)(tp.rawy)*192.0/3072.0;
		swiWaitForVBlank();
	}
	lcdMainOnTop();

	switch(done)
	{
		case 2:
			changeState(&gameState);
			break;
		case 3:
			changeState(&menuState);
			break;
		default:
			startPI();
			break;
	}
}

void freePause(void)
{
	freePCX(pauseImage);
	freePCX(pauseButtonsImage);
}
