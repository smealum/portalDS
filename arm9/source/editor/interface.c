#include "editor/editor_main.h"

#define NUMINTERFACEBUTTONS (11)

interfaceButton_struct interfaceButtons[]={ (interfaceButton_struct){37,38,"storagecube2_ui.pcx",NULL,6,false},
											(interfaceButton_struct){72,38,"pressurebttn2_ui.pcx",NULL,3,false},
											(interfaceButton_struct){107,38,"platform2_ui.pcx",NULL,9,false},
											(interfaceButton_struct){142,38,"grid2_ui.pcx",NULL,8,false},
											(interfaceButton_struct){177,38,"door2_ui.pcx",NULL,10,false},
											(interfaceButton_struct){212,38,"light2_ui.pcx",NULL,11,false},
											(interfaceButton_struct){37,81,"companion2_ui.pcx",NULL,5,false},
											(interfaceButton_struct){72,81,"button2_ui.pcx",NULL,2,false},
											(interfaceButton_struct){107,81,"balllauncher2_ui.pcx",NULL,1,false},
											(interfaceButton_struct){142,81,"ballcatcher2_ui.pcx",NULL,0,false},
											(interfaceButton_struct){177,81,"turret2_ui.pcx",NULL,4,false}};

struct gl_texture_t *interfaceBackground;
int bgSub;

void initInterfaceButton(interfaceButton_struct* ib)
{
	if(!ib)return;
	ib->imageData=(struct gl_texture_t *)ReadPCXFile(ib->imageName,"editor");
	convertPCX16Bit(ib->imageData);
	ib->down=false;
}

void initInterfaceButtons(void)
{
	int i;
	for(i=0;i<NUMINTERFACEBUTTONS;i++)
	{
		initInterfaceButton(&interfaceButtons[i]);
	}
}

void initInterface(void)
{
	interfaceBackground=(struct gl_texture_t *)ReadPCXFile("interface.pcx","editor");
	convertPCX16Bit(interfaceBackground);
	bgSub=bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	if(interfaceBackground->texels16)dmaCopy(interfaceBackground->texels16,bgGetGfxPtr(bgSub),256*192*2);

	initInterfaceButtons();
}

void drawInterfaceButton(interfaceButton_struct* ib)
{
	if(!ib || !ib->imageData || !ib->imageData->texels16)return;

	int j;
	for(j=0;j<ib->imageData->height;j++)
	{
		dmaCopy(&ib->imageData->texels16[j*ib->imageData->width],&bgGetGfxPtr(bgSub)[ib->x+(ib->y+j)*256],ib->imageData->width*2);
	}
}

void eraseInterfaceButton(interfaceButton_struct* ib)
{
	if(!ib || !ib->imageData || !interfaceBackground)return;

	int j;
	for(j=0;j<ib->imageData->height;j++)
	{
		dmaCopy(&interfaceBackground->texels16[ib->x+(ib->y+j)*256],&bgGetGfxPtr(bgSub)[ib->x+(ib->y+j)*256],ib->imageData->width*2);
	}
}

void activateInterfaceButton(interfaceButton_struct* ib)
{
	if(!ib)return;

	if(ib->argument>=0 && ib->argument<12)
	{
		entity_struct* e=NULL;
		if(!(editorSelection.entity && !editorSelection.entity->placed))e=createEntity(vect(32,32,32), ib->argument, false);
		else{e=editorSelection.entity;changeEntityType(e,ib->argument);}
		editorSelection.active=true;
		editorSelection.entity=e;
		switchScreens();
	}
}

void updateInterfaceButton(interfaceButton_struct* ib, u8 x, u8 y)
{
	if(!ib || !ib->imageData)return;

	if(x>=ib->x && y>=ib->y && x<ib->x+ib->imageData->width && y<ib->y+ib->imageData->height)
	{
		if(keysHeld() & KEY_TOUCH)
		{
			if(!ib->down)
			{
				drawInterfaceButton(ib);
				ib->down=true;
			}
		}else if(keysUp() & KEY_TOUCH)
		{
			activateInterfaceButton(ib);
			eraseInterfaceButton(ib);
			ib->down=false;
		}
	}else{
		if(ib->down)
		{
			eraseInterfaceButton(ib);
			ib->down=false;
		}
	}
}

void updateInterfaceButtons(u8 x, u8 y)
{
	int i;
	for(i=0;i<NUMINTERFACEBUTTONS;i++)
	{
		updateInterfaceButton(&interfaceButtons[i], x, y);
	}
}

void pauseEditorInterface(void)
{
	int i;
	for(i=0;i<NUMINTERFACEBUTTONS;i++)
	{
		updateInterfaceButton(&interfaceButtons[i], 0, 0);
	}
}

void freeInterfaceButton(interfaceButton_struct* ib)
{
	if(!ib || !ib->imageData)return;
	freePCX(ib->imageData);
	ib->imageData=NULL;
}

void freeInterfaceButtons(void)
{
	int i;
	for(i=0;i<NUMINTERFACEBUTTONS;i++)
	{
		freeInterfaceButton(&interfaceButtons[i]);
	}
}

void freeInterface(void)
{
	freePCX(interfaceBackground);
	freeInterfaceButtons();
}
