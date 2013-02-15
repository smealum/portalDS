#include "common/general.h"

#define v1 NORMAL_PACK( 0, 0,0);
#define v2 NORMAL_PACK( 0,64,0);
#define v3 NORMAL_PACK(64,64,0);
#define v4 NORMAL_PACK(64, 0,0);

bool fading;
bool API_Updating;

bool cull;
bool API_DrawingOutline;
guiEntityList_struct API_List;
touchPosition API_Touch;

bool touchOnly=true;

font_struct APIfont;

guiEntity_struct* buttonBlock;

guiFunction API_ToCall;
guiEntity_struct* API_ToCallEntity;
guiEntity_struct* Cursor;
guiEntity_struct* DefaultCursor;

bool collideGuiPrio(guiEntityList_struct *cl, int px, int py, int prio);

guiEntity_struct* createGuiEntity(guiEntityList_struct *cl)
{
	guiListElement_struct *ce=malloc(sizeof(guiListElement_struct));
	ce->entity=NULL;
	ce->entity=malloc(sizeof(guiEntity_struct));
	ce->entity->data=NULL;
	ce->entity->father=NULL;
	ce->entity->up=NULL;
	ce->entity->down=NULL;
	ce->entity->left=NULL;
	ce->entity->right=NULL;
	ce->entity->kidToDate=2;
	ce->entity->bastard=true;
	ce->entity->shadow=false;
	ce->entity->id=cl->count;
	ce->next=NULL;
	if(!cl->count)
	{
		ce->previous=NULL;
		cl->first=ce;
	}else{
		ce->previous=cl->last;
		cl->last->next=ce;
	}
	cl->last=ce;
	cl->count++;
	
	return ce->entity;
}

void toggleShadow(guiEntity_struct* e)
{
	e->shadow=!e->shadow;
}

void drawWindow(void* e)
{
	guiEntity_struct* de=(guiEntity_struct*)e;
	guiWindowData_struct* data=(guiWindowData_struct*)de->data;
	int width, height;
	// NOGBA("lala2");
	
	if(data->background==NULL)
	{
		unbindMtl();
		width=0;
		height=0;
	}else 
	{
		applyMTL(data->background);
		width=data->background->width;
		height=data->background->height;
	}
	
	glBegin(GL_QUADS);
	
		GFX_TEX_COORD = 0;
		GFX_VERTEX10 = v1;
		GFX_TEX_COORD = TEXTURE_PACK(0, 16*height);
		GFX_VERTEX10 = v2;
		GFX_TEX_COORD = TEXTURE_PACK(16*width, 16*height);
		GFX_VERTEX10 = v3;
		GFX_TEX_COORD = TEXTURE_PACK(16*width, 0);
		GFX_VERTEX10 = v4;

	glEnd();
}

void drawLabel(void* e)
{
	guiEntity_struct* de=(guiEntity_struct*)e;
	guiLabelData_struct* data=(guiLabelData_struct*)de->data;
	
	if(f32toint(de->a_Position.x+de->a_Size.x)<-128 || f32toint(de->a_Position.x)>127)return;
	if(cull)
	{
		if(f32toint(de->a_Position.y+de->a_Size.y)<0 || f32toint(de->a_Position.y)>191)return;
	}
	
	glScalev(&de->scaleVs);
	drawString(data->string, data->color, inttof32(1), 0, 0);
}

void updateButton(void* e)
{
	guiEntity_struct* de=(guiEntity_struct*)e;
	guiButtonData_struct* data=(guiButtonData_struct*)de->data;
	int width, height;
	if(!data->activated)return;
	if(API_Updating)
	{
		if(data->clickable && !collideGuiPrio(NULL, API_Touch.px, API_Touch.py, de->prio) && (buttonBlock==NULL || buttonBlock->id==de->id))
		{
			if(((keysUp() & KEY_TOUCH) && data->over) || (!touchOnly&&((keysUp() & KEY_A) && Cursor==e)))
			{
				NOGBA("TOUCH, %p != %p ?",API_ToCall,&keyboardButtonPressed);
				if(!keyboardLock || data->function==(guiFunction)&keyboardButtonPressed)
				{
					API_ToCall=(guiFunction)data->function;
					API_ToCallEntity=de;
					data->over=false;
					data->clicked=true;
					Cursor=NULL;
					// mmEffect(SFX_TOUCHED);
				}
			}else if((keysHeld() & KEY_TOUCH))
			{
				if(!keyboardLock || data->function==(guiFunction)&keyboardButtonPressed)
				{
					if((API_Touch.px >= f32toint(de->a_Position.x)+128 && API_Touch.px < f32toint(de->a_Position.x+de->a_Size.x)+128
					&& API_Touch.py >= f32toint(de->a_Position.y)-2 && API_Touch.py < f32toint(de->a_Position.y+de->a_Size.y)))
					{
						// if(!data->over)mmEffect(SFX_SELECTED);
						data->over=true;
						Cursor=e;
					}else data->over=false;
				}
			}else {data->over=false;}
		}
	}
	
	if(!de->alpha || de->a_Position.x>=inttof32(128) || de->a_Position.x<=inttof32(-128) || de->a_Position.y>=inttof32(192) || de->a_Position.y<=inttof32(-192))return;
	
	if(!data->colorTakeover)
	{
		switch(data->over||(!touchOnly&&Cursor==e))
		{
			case true:
				de->outline_color=RGB15(31,0,0);
				de->color=RGB15(31,20,27);
			break;
			default:
				de->outline_color=RGB15(0,0,0);
				de->color=RGB15(31,31,31);
			break;
		}
	}
	
	if(data->background==NULL)
	{
		unbindMtl();
		width=0;
		height=0;
	}else 
	{
		applyMTL(data->background);
		width=data->background->rwidth;
		height=data->background->rheight;
	}
	
	glBegin(GL_QUADS);
	
		GFX_TEX_COORD = 0;
		GFX_VERTEX10 = v1;
		GFX_TEX_COORD = TEXTURE_PACK(0, 16*height);
		GFX_VERTEX10 = v2;
		GFX_TEX_COORD = TEXTURE_PACK(16*width, 16*height);
		GFX_VERTEX10 = v3;
		GFX_TEX_COORD = TEXTURE_PACK(16*width, 0);
		GFX_VERTEX10 = v4;

	glEnd();
}

void updateCheckBox(void* e)
{
	guiEntity_struct* de=(guiEntity_struct*)e;
	guiCheckBoxData_struct* data=(guiCheckBoxData_struct*)de->data;
	int width, height;
	if(API_Updating)
	{
		if(data->clickable)
		{
			if(keysHeld() & KEY_TOUCH)
			{
				if(API_Touch.px > f32toint(de->a_Position.x)+128 && API_Touch.px < f32toint(de->a_Position.x+de->a_Size.x)+128
				&& API_Touch.py > f32toint(de->a_Position.y-de->a_Size.y) && API_Touch.py < f32toint(de->a_Position.y+de->a_Size.y))
				{
					data->over=true;
				}else data->over=false;
			}else if((keysUp() & KEY_TOUCH) && data->over)
			{
				NOGBA("TOUCH");
				data->checked=!data->checked;
				data->function(de);
				data->over=false;
			}else data->over=false;
		}
	}
	
	switch(data->over)
	{
		case true:
			de->outline_color=RGB15(31,0,0);
		break;
		default:
			de->outline_color=RGB15(0,0,0);
		break;
	}
	
	switch(data->checked)
	{
		case true:
			de->color=RGB15(0,0,0);
			de->outline_color=RGB15(31,31,31);
		break;
		default:
			de->color=RGB15(21,21,21);
		break;
	}
	
	if(data->background==NULL)
	{
		unbindMtl();
		width=0;
		height=0;
	}else 
	{
		applyMTL(data->background);
		width=data->background->width;
		height=data->background->height;
	}
	
	glBegin(GL_QUADS);
	
		GFX_TEX_COORD = 0;
		GFX_VERTEX10 = v1;
		GFX_TEX_COORD = TEXTURE_PACK(0, 16*height);
		GFX_VERTEX10 = v2;
		GFX_TEX_COORD = TEXTURE_PACK(16*width, 16*height);
		GFX_VERTEX10 = v3;
		GFX_TEX_COORD = TEXTURE_PACK(16*width, 0);
		GFX_VERTEX10 = v4;

	glEnd();
}

void updateSlider(void* e)
{
	guiEntity_struct* de=(guiEntity_struct*)e;
	guiSliderData_struct* data=(guiSliderData_struct*)de->data;
	int width, height, tpos;
	
	if(API_Updating)
	{
		if(keysHeld() & KEY_TOUCH)
		{
			if(API_Touch.px > f32toint(de->a_Position.x)+128+data->position-3 && API_Touch.px < f32toint(de->a_Position.x)+128+data->position+6+3
			&& API_Touch.py > f32toint(de->a_Position.y-de->a_Size.y)-2 && API_Touch.py < f32toint(de->a_Position.y+de->a_Size.y)+2)
			{
				data->over=true;
				data->selected=true;
			}else data->over=false;
		}else if((keysUp() & KEY_TOUCH) && data->over)
		{
			NOGBA("TOUCH");
			data->over=false;
		}else{
			data->over=false;
			data->selected=false;
		}
		
		switch(data->selected)
		{
			case true:
				if(!(API_Touch.px==0 &&  API_Touch.py==0))
				{
					tpos=mulf32(divf32(de->a_Size.x,de->Size.x),API_Touch.px)-f32toint(de->a_Position.x)-128;
					if(tpos<f32toint(de->Size.x) && tpos>0 && data->position!=tpos)data->function(de);
					if(tpos>f32toint(de->Size.x))data->position=f32toint(de->Size.x);
					else if(tpos<0)data->position=0;
					else data->position=tpos;
					de->outline_color=RGB15(31,0,0);
				}
			break;
			default:
				de->outline_color=RGB15(0,0,0);
			break;
		}
	}
	
	if(data->background==NULL)
	{
		unbindMtl();
		width=0;
		height=0;
	}else 
	{
		applyMTL(data->background);
		width=data->background->width;
		height=data->background->height;
	}
	
	glBegin(GL_QUADS);
	
		GFX_TEX_COORD = 0;
		GFX_VERTEX10 = v1;
		GFX_TEX_COORD = TEXTURE_PACK(0, 16*height);
		GFX_VERTEX10 = v2;
		GFX_TEX_COORD = TEXTURE_PACK(16*width, 16*height);
		GFX_VERTEX10 = v3;
		GFX_TEX_COORD = TEXTURE_PACK(16*width, 0);
		GFX_VERTEX10 = v4;

	glEnd();
	
	glPushMatrix();
		GLvector scaleV;
		
		if(API_DrawingOutline)
		{
			scaleV.x = divf32(inttof32(1),de->Size.x+inttof32(2*de->outline));
			scaleV.y = divf32(inttof32(1),de->Size.y+inttof32(2*de->outline));
			scaleV.z = inttov16(1);
		}else{		
			scaleV.x = divf32(inttof32(1),de->Size.x);
			scaleV.y = divf32(inttof32(1),de->Size.y);
			scaleV.z = inttov16(1);
		}
		
		glScalev(&scaleV);
		
		glTranslate3f32(inttof32(data->position-3),inttof32(-3),inttof32(5));
		
		if(API_DrawingOutline)
		{
			scaleV.x = inttof32(5+2*de->outline);
			scaleV.y = inttof32(10+2*de->outline);
			scaleV.z = inttov16(1);
		}else{
			scaleV.x = inttof32(5);
			scaleV.y = inttof32(10);
			scaleV.z = inttov16(1);
		}
		
		glScalev(&scaleV);
		
		glBegin(GL_QUADS);
		
			GFX_TEX_COORD = 0;
			GFX_VERTEX10 = v1;
			GFX_TEX_COORD = TEXTURE_PACK(0, 16*height);
			GFX_VERTEX10 = v2;
			GFX_TEX_COORD = TEXTURE_PACK(16*width, 16*height);
			GFX_VERTEX10 = v3;
			GFX_TEX_COORD = TEXTURE_PACK(16*width, 0);
			GFX_VERTEX10 = v4;

		glEnd();
	glPopMatrix(1);
}

void updateVSlider(void* e)
{
	guiEntity_struct* de=(guiEntity_struct*)e;
	guiSliderData_struct* data=(guiSliderData_struct*)de->data;
	int width, height, tpos;
	
	if(API_Updating)
	{
		if(keysHeld() & KEY_TOUCH)
		{
			if(API_Touch.py > f32toint(de->a_Position.y)+data->position-3-2 && API_Touch.py < f32toint(de->a_Position.y)+data->position+3
			&& API_Touch.px > f32toint(de->a_Position.x-de->a_Size.x)+128-2 && API_Touch.px < f32toint(de->a_Position.x+de->a_Size.x)+128+2)
			{
				data->over=true;
				data->selected=true;
			}else data->over=false;
		}else if((keysUp() & KEY_TOUCH) && data->over)
		{
			NOGBA("TOUCH");
			data->over=false;
		}else{
			data->over=false;
			data->selected=false;
		}
		
		switch(data->selected)
		{
			case true:
				if(!(API_Touch.px==0 &&  API_Touch.py==0))
				{
					tpos=mulf32(divf32(de->a_Size.y,de->Size.y),API_Touch.py)-f32toint(de->a_Position.y);
					// if(tpos<f32toint(de->Size.y) && tpos>0 && data->position!=tpos)data->function(de);
					if(tpos>f32toint(de->Size.y))data->position=f32toint(de->Size.y);
					else if(tpos<0)data->position=0;
					else data->position=tpos;
					de->outline_color=RGB15(31,0,0);
					if(data->oldpos!=data->position)data->function(de);
					data->oldpos=data->position;
				}
			break;
			default:
				de->outline_color=RGB15(0,0,0);
			break;
		}
	}
	
	if(data->background==NULL)
	{
		unbindMtl();
		width=0;
		height=0;
	}else 
	{
		applyMTL(data->background);
		width=data->background->width;
		height=data->background->height;
	}
	
	glBegin(GL_QUADS);
	
		GFX_TEX_COORD = 0;
		GFX_VERTEX10 = v1;
		GFX_TEX_COORD = TEXTURE_PACK(0, 16*height);
		GFX_VERTEX10 = v2;
		GFX_TEX_COORD = TEXTURE_PACK(16*width, 16*height);
		GFX_VERTEX10 = v3;
		GFX_TEX_COORD = TEXTURE_PACK(16*width, 0);
		GFX_VERTEX10 = v4;

	glEnd();
	
	glPushMatrix();
		GLvector scaleV;
		
		if(API_DrawingOutline)
		{
			scaleV.x = divf32(inttof32(1),de->Size.x+inttof32(2*de->outline));
			scaleV.y = divf32(inttof32(1),de->Size.y+inttof32(2*de->outline));
			scaleV.z = inttov16(1);
		}else{		
			scaleV.x = divf32(inttof32(1),de->Size.x);
			scaleV.y = divf32(inttof32(1),de->Size.y);
			scaleV.z = inttov16(1);
		}
		
		glScalev(&scaleV);
		
		glTranslate3f32(inttof32(-3),inttof32(data->position-3),inttof32(5));
		
		if(API_DrawingOutline)
		{
			scaleV.x = inttof32(10+2*de->outline);
			scaleV.y = inttof32(5+2*de->outline);
			scaleV.z = inttov16(1);
		}else{
			scaleV.x = inttof32(10);
			scaleV.y = inttof32(5);
			scaleV.z = inttov16(1);
		}
		
		glScalev(&scaleV);
		
		glBegin(GL_QUADS);
		
			GFX_TEX_COORD = 0;
			GFX_VERTEX10 = v1;
			GFX_TEX_COORD = TEXTURE_PACK(0, 16*height);
			GFX_VERTEX10 = v2;
			GFX_TEX_COORD = TEXTURE_PACK(16*width, 16*height);
			GFX_VERTEX10 = v3;
			GFX_TEX_COORD = TEXTURE_PACK(16*width, 0);
			GFX_VERTEX10 = v4;

		glEnd();
	glPopMatrix(1);
}

guiEntity_struct* createWindow(s16 x, s16 y, u16 sx, u16 sy, u8 alpha, u16 color, u8 outline, u16 outline_color, char* filename)
{
	guiEntity_struct* e=createGuiEntity(&API_List);
	NOGBA("window 1");
	e->data=malloc(sizeof(guiWindowData_struct));
	e->Draw=(guiDrawFunction)&drawWindow;
	e->Position.x=inttof32(x);
	e->Position.y=inttof32(y);
	e->t_Position.x=inttof32(x);
	e->t_Position.y=inttof32(y);
	e->Size.x=inttof32(sx);
	e->Size.y=inttof32(sy);
	e->b_Size.x=inttof32(sx);
	e->b_Size.y=inttof32(sy);
	e->t_Size.x=inttof32(sx);
	e->t_Size.y=inttof32(sy);
	e->alpha=alpha<<10;
	e->t_alpha=alpha<<10;
	e->type=0;
	e->timep=0;
	e->t_timep=0;
	e->times=0;
	e->t_times=0;
	e->timea=0;
	e->t_timea=0;
	e->prio=400;
	e->outline=outline;
	e->outline_color=outline_color;
	e->color=color;
	((guiWindowData_struct*)e->data)->background=createTexture(filename, "gui");
	return e;
}

guiEntity_struct* createLabel(s16 x, s16 y, u16 color, char* text)
{
	x+=CHARSIZE/4;
	y+=CHARSIZE/2;

	guiEntity_struct* e=createGuiEntity(&API_List);
	e->data=malloc(sizeof(guiLabelData_struct));
	((guiLabelData_struct*)e->data)->string=alloc(strlen(text)+3,NULL);
	// ((guiLabelData_struct*)e->data)->string=malloc(strlen(text)+3);
	strcpy(((guiLabelData_struct*)e->data)->string,text);
	((guiLabelData_struct*)e->data)->color=RGB15(31,31,31);
	e->Draw=(guiDrawFunction)&drawLabel;
	e->Position.x=inttof32(x);
	e->Position.y=inttof32(y);
	e->t_Position.x=inttof32(x);
	e->t_Position.y=inttof32(y);
	e->alpha=31<<10;
	e->t_alpha=31<<10;
	e->type=2;
	e->timep=0;
	e->t_timep=0;
	e->times=0;
	e->t_times=0;
	e->timea=0;
	e->t_timea=0;
	e->prio=400;
	e->outline=0;
	e->outline_color=0;
	e->color=color;
	
	/*int width=strlen(text)*8;
	((guiLabelData_struct*)e->data)->width=width;
	if(width<=8)width=8;
	else if(width<=16)width=16;
	else if(width<=32)width=32;
	else if(width<=64)width=64;
	else if(width<=128)width=128;
	else if(width<=256)width=256;
	else if(width<=512)width=512;
	else return e;
	
	NOGBA("%s, %d, %d\n",text,strlen(text)*8,width);
	
	u8 *buffer=malloc(width*8);
	u16 *buffer2=malloc(256*8*2);
	NOGBA("%s, %p, %p\n",text,buffer,buffer2);
	buffer2[0]=RGB15(31,0,31);
	buffer2[255]=RGB15(31,31,31);
	int i;
	for(i=0;i<width*8;i++)buffer[i]=0;
	API_Print(buffer, width, text);
	
	((guiLabelData_struct*)e->data)->background=createTextureBuffer(buffer, buffer2, width, 8);
	free(buffer);
	free(buffer2);
	NOGBA("%p\n",((guiLabelData_struct*)e->data)->background);*/
	
	e->Size.x=inttof32(((guiLabelData_struct*)e->data)->width);
	e->Size.y=inttof32(8);
	e->b_Size.x=inttof32(((guiLabelData_struct*)e->data)->width);
	e->b_Size.y=inttof32(8);
	e->t_Size.x=inttof32(((guiLabelData_struct*)e->data)->width);
	e->t_Size.y=inttof32(8);
	e->Size.x=inttof32(1);
	e->Size.y=inttof32(1);
	e->b_Size.x=inttof32(1);
	e->b_Size.y=inttof32(1);
	e->t_Size.x=inttof32(1);
	e->t_Size.y=inttof32(1);
	return e;
}

guiEntity_struct* createLabelFather(s16 x, s16 y, u16 color, guiEntity_struct* father, char* text, bool halpha)
{
	guiEntity_struct* e=createLabel(x, y, color, text);
	if(e==NULL || father==NULL)return e;
	e->halpha=halpha;
	e->father=father;
	e->bastard=false;
	return e;
}

void updateLabelText(guiEntity_struct* e, char* text)
{
	if(!e || strlen(text)<=0)return;
	guiLabelData_struct* ld=e->data;
	if(!ld)return;
	ld->string=reAlloc(ld->string,strlen(text)+3,NULL);
	strcpy(ld->string,text);
}

void updateButtonText(guiEntity_struct* e, char* text)
{
	if(!e || strlen(text)<=0)return;
	guiButtonData_struct* d=e->data;
	if(!d || !d->label)return;
	updateLabelText(d->label,text);
	int width=strlen(text)*8;
	e->Size.x=inttof32(width+8);
	e->b_Size.x=inttof32(width+8);
	e->t_Size.x=inttof32(width+8);
}

guiEntity_struct* createButton(s16 x, s16 y, u16 color, guiFunction function, char* text, char* filename)
{
	guiEntity_struct* e=createGuiEntity(&API_List);
	e->data=malloc(sizeof(guiButtonData_struct));
	e->Draw=(guiDrawFunction)&updateButton;
	e->Position.x=inttof32(x);
	e->Position.y=inttof32(y);
	e->t_Position.x=inttof32(x);
	e->t_Position.y=inttof32(y);
	e->alpha=31<<10;
	e->t_alpha=31<<10;
	e->type=1;
	e->timep=0;
	e->t_timep=0;
	e->times=0;
	e->t_times=0;
	e->timea=0;
	e->t_timea=0;
	e->prio=400;
	// e->outline=1;
	e->outline=0;
	((guiButtonData_struct*)e->data)->clickable=1;
	((guiButtonData_struct*)e->data)->clicked=0;
	((guiButtonData_struct*)e->data)->over=0;
	((guiButtonData_struct*)e->data)->activated=true;
	e->color=color;
	e->outline_color=0;
	
	int width=strlen(text)*8;
	e->Size.x=inttof32(width+8);
	e->Size.y=inttof32(16);
	e->b_Size.x=inttof32(width+8);
	e->b_Size.y=inttof32(16);
	e->t_Size.x=inttof32(width+8);
	e->t_Size.y=inttof32(16);
	
	NOGBA("filename : %s, %d", filename, strlen(text));

	((guiButtonData_struct*)e->data)->background=createTexture(filename, "gui");
	((guiButtonData_struct*)e->data)->function=function;
	((guiButtonData_struct*)e->data)->colorTakeover=false;
	if(strlen(text)>0)((guiButtonData_struct*)e->data)->label=createLabelFather(4, 4, RGB15(31,31,31)-color, e, text, true);
	else
	{
		e->t_Size.x=e->b_Size.x=e->Size.x=inttof32(((guiButtonData_struct*)e->data)->background->rwidth);
		e->t_Size.y=e->b_Size.y=e->Size.y=inttof32(((guiButtonData_struct*)e->data)->background->rheight);
	}
	return e;
}

guiEntity_struct* createButtonFather(s16 x, s16 y, u16 color, guiFunction function, guiEntity_struct* father, char* text, char* filename, bool halpha)
{
	guiEntity_struct* e=createButton(x, y, color, function, text, filename);
	
	if(e==NULL || father==NULL)return e;
	e->halpha=halpha;
	e->father=father;
	e->bastard=false;
	return e;
}

guiEntity_struct* createCheckBox(s16 x, s16 y, guiFunction function, char* text)
{
	guiEntity_struct* e=createGuiEntity(&API_List);
	e->data=malloc(sizeof(guiCheckBoxData_struct));
	e->Draw=(guiDrawFunction)&updateCheckBox;
	e->Position.x=inttof32(x);
	e->Position.y=inttof32(y+3);
	e->t_Position.x=inttof32(x);
	e->t_Position.y=inttof32(y+3);
	e->alpha=31<<10;
	e->t_alpha=31<<10;
	e->type=3;
	e->timep=0;
	e->t_timep=0;
	e->times=0;
	e->t_times=0;
	e->timea=0;
	e->t_timea=0;
	e->prio=400;
	e->outline=1;
	e->color=RGB15(21,21,21);
	e->outline_color=0;
	
	//int width=strlen(text)*8;
	e->Size.x=inttof32(8);
	e->Size.y=inttof32(8);
	e->b_Size.x=inttof32(8);
	e->b_Size.y=inttof32(8);
	e->t_Size.x=inttof32(8);
	e->t_Size.y=inttof32(8);

	((guiCheckBoxData_struct*)e->data)->checked=false;
	((guiCheckBoxData_struct*)e->data)->clickable=true;
	((guiCheckBoxData_struct*)e->data)->background=NULL;
	((guiCheckBoxData_struct*)e->data)->function=function;
	if(strlen(text)>0)((guiCheckBoxData_struct*)e->data)->label=createLabelFather(13, 0, RGB15(0,0,0), e, text, true);
	return e;
}

guiEntity_struct* createCheckBoxFather(s16 x, s16 y, guiFunction function, guiEntity_struct* father, char* text, bool halpha)
{
	guiEntity_struct* e=createCheckBox(x, y, function, text);
	if(e==NULL || father==NULL)return e;
	e->halpha=halpha;
	e->father=father;
	e->bastard=false;
	return e;
}

guiEntity_struct* createSlider(s16 x, s16 y, u8 size, guiFunction function, char* text)
{
	guiEntity_struct* e=createGuiEntity(&API_List);
	e->data=malloc(sizeof(guiSliderData_struct));
	e->Draw=(guiDrawFunction)&updateSlider;
	e->Position.x=inttof32(x);
	e->Position.y=inttof32(y+3);
	e->t_Position.x=inttof32(x);
	e->t_Position.y=inttof32(y+3);
	e->alpha=31<<10;
	e->t_alpha=31<<10;
	e->type=4;
	e->timep=0;
	e->t_timep=0;
	e->times=0;
	e->t_times=0;
	e->timea=0;
	e->t_timea=0;
	e->prio=400;
	e->outline=1;
	e->color=RGB15(21,21,21);
	e->outline_color=0;
	
	//int width=strlen(text)*8;
	e->Size.x=inttof32(size);
	e->Size.y=inttof32(4);
	e->b_Size.x=inttof32(size);
	e->b_Size.y=inttof32(4);
	e->t_Size.x=inttof32(size);
	e->t_Size.y=inttof32(4);

	((guiSliderData_struct*)e->data)->selected=false;
	((guiSliderData_struct*)e->data)->background=NULL;
	((guiSliderData_struct*)e->data)->function=function;
	((guiSliderData_struct*)e->data)->position=0;
	//if(strlen(text)>0)((guiCheckBoxData_struct*)e->data)->label=createLabelFather(13, 0, RGB15(0,0,0), e, text);
	return e;
}

guiEntity_struct* createSliderFather(s16 x, s16 y, u8 size, guiFunction function, guiEntity_struct* father, char* text, bool halpha)
{
	guiEntity_struct* e=createSlider(x, y, size, function, text);
	if(e==NULL || father==NULL)return e;
	e->halpha=halpha;
	e->father=father;
	e->bastard=false;
	return e;
}

guiEntity_struct* createVSlider(s16 x, s16 y, u8 size, guiFunction function, char* text)
{
	guiEntity_struct* e=createGuiEntity(&API_List);
	e->data=malloc(sizeof(guiSliderData_struct));
	e->Draw=(guiDrawFunction)&updateVSlider;
	e->Position.x=inttof32(x);
	e->Position.y=inttof32(y+3);
	e->t_Position.x=inttof32(x);
	e->t_Position.y=inttof32(y+3);
	e->alpha=31<<10;
	e->t_alpha=31<<10;
	e->type=4;
	e->timep=0;
	e->t_timep=0;
	e->times=0;
	e->t_times=0;
	e->timea=0;
	e->t_timea=0;
	e->prio=400;
	e->outline=1;
	e->color=RGB15(21,21,21);
	e->outline_color=0;
	
	//int width=strlen(text)*8;
	e->Size.x=inttof32(4);
	e->Size.y=inttof32(size);
	e->b_Size.x=inttof32(4);
	e->b_Size.y=inttof32(size);
	e->t_Size.x=inttof32(4);
	e->t_Size.y=inttof32(size);

	((guiSliderData_struct*)e->data)->selected=false;
	((guiSliderData_struct*)e->data)->background=NULL;
	((guiSliderData_struct*)e->data)->function=function;
	((guiSliderData_struct*)e->data)->position=0;
	((guiSliderData_struct*)e->data)->size=size;
	//if(strlen(text)>0)((guiCheckBoxData_struct*)e->data)->label=createLabelFather(13, 0, RGB15(0,0,0), e, text);
	return e;
}

guiEntity_struct* createVSliderFather(s16 x, s16 y, u8 size, guiFunction function, guiEntity_struct* father, char* text, bool halpha)
{
	guiEntity_struct* e=createVSlider(x, y, size, function, text);
	if(e==NULL || father==NULL)return e;
	e->halpha=halpha;
	e->father=father;
	e->bastard=false;
	return e;
}

void moveGuiEntity(guiEntity_struct* e, s16 x, s16 y, u8 time)
{
	e->t_Position.x=inttof32(x);
	e->t_Position.y=inttof32(y);
	e->timep=0;
	e->t_timep=time;
}

void resizeGuiEntity(guiEntity_struct* e, u16 x, u16 y, u8 time)
{
	e->t_Size.x=inttof32(x);
	e->t_Size.y=inttof32(y);
	e->times=0;
	e->t_times=time;
}

void setPosition(guiEntity_struct* e, s16 x, s16 y)
{
	e->t_Position.x=inttof32(x);
	e->t_Position.y=inttof32(y);
	e->Position.x=inttof32(x);
	e->Position.y=inttof32(y);
	e->timep=0;
	e->t_timep=0;
	e->kidToDate=1;
}

void setPositionX(guiEntity_struct* e, s16 x)
{
	e->t_Position.x=inttof32(x);
	e->Position.x=inttof32(x);
	e->timep=0;
	e->t_timep=0;
	e->kidToDate=1;
}

void setSizeA(guiEntity_struct* e, u16 x, u16 y)
{
	e->Size.x=inttof32(x);
	e->Size.y=inttof32(y);
	e->b_Size=e->t_Size=e->o_Size=e->a_Size=e->Size;
	e->times=0;
	e->t_times=0;
}

void setSize(guiEntity_struct* e, u16 x, u16 y)
{
	e->t_Size.x=inttof32(x);
	e->t_Size.y=inttof32(y);
	e->Size.x=inttof32(x);
	e->Size.y=inttof32(y);
	e->times=0;
	e->t_times=0;
}

void fadeGuiEntity(guiEntity_struct* e, u8 alpha, u8 time)
{
	e->t_alpha=alpha<<10;
	e->timea=0;
	e->t_timea=time;
}

void setAlpha(guiEntity_struct* e, u8 alpha)
{
	e->t_alpha=alpha<<10;
	e->alpha=alpha<<10;
	e->timea=0;
	e->t_timea=0;
}

void fadeSons(guiEntity_struct* f, u8 alpha, u8 time)
{
	guiEntity_struct* e;
	guiEntityList_struct *cl=&API_List;
	guiListElement_struct *ce=cl->first;
	do{
		e=ce->entity;
		if(e!=NULL)
		{
			if(!e->bastard)
			{
				if(e->father->id==f->id)
				{
					fadeGuiEntity(e, alpha, time);
					fadeSons(e, alpha, time);
				}
			}
		}
		ce=ce->next;
	}while(ce!=NULL);
}

void setAlphaSons(guiEntity_struct* f, u8 alpha)
{
	guiEntity_struct* e;
	guiEntityList_struct *cl=&API_List;
	guiListElement_struct *ce=cl->first;
	do{
		e=ce->entity;
		if(e!=NULL)
		{
			if(!e->bastard)
			{
				if(e->father->id==f->id)
				{
					setAlpha(e, alpha);
					setAlphaSons(e, alpha);
				}
			}
		}
		ce=ce->next;
	}while(ce!=NULL);
}

bool collideGui(guiEntityList_struct *cl, int px, int py)
{
	if(!cl)cl=&API_List;
	guiListElement_struct *cn, *ce=cl->last;
	guiEntity_struct *e;
	px=inttof32(px-128);
	py=inttof32(py);
	do{
		cn=ce->previous;
		e=ce->entity;
		
		if(e->Draw==drawWindow && !e->father)
		{
			if(px>=e->Position.x && px<e->Position.x+e->Size.x && py>=e->Position.y && py<e->Position.y+e->Size.y)return true;
		}
		
		ce=cn;
	}while(ce!=NULL);
	return false;
}

bool collideGuiPrio(guiEntityList_struct *cl, int px, int py, int prio)
{
	if(!cl)cl=&API_List;
	guiListElement_struct *cn, *ce=cl->last;
	guiEntity_struct *e;
	px=inttof32(px-128);
	py=inttof32(py);
	do{
		cn=ce->previous;
		e=ce->entity;
		
		if(e->Draw==drawWindow && !e->father)
		{
			if(px>=e->Position.x && px<e->Position.x+e->Size.x && py>=e->Position.y && py<e->Position.y+e->Size.y && e->prio>prio)return true;
		}
		
		ce=cn;
	}while(ce!=NULL);
	return false;
}

void updateGui(guiEntityList_struct *cl, touchPosition* tp)
{
	if(!cl)cl=&API_List;
	// guiListElement_struct *cn, *ce=cl->first;
		// NOGBA("lala???");
	guiListElement_struct *cn, *ce=cl->last;
	guiEntity_struct *e, *f;
	GLvector scaleV, scaleVf, /*scaleVs, scaleVsh, */scaleVo;
	if(!tp)touchRead(&API_Touch);
	else API_Touch=*tp;
	
	// scaleVsh.x = inttov16(101)/100;
	// scaleVsh.y = inttov16(101)/100;
	// scaleVsh.z = inttov16(101)/100;
	
	// if(Cursor)NOGBA("cursor : %s",((guiLabelData_struct*)((guiButtonData_struct*)Cursor->data)->label->data)->string);
	// if(keysHeld() & KEY_Y)TESTLABEL->prio--;
	// if(keysHeld() & KEY_X)TESTLABEL->prio++;
	// NOGBA("PRIO ! %d",TESTLABEL->prio);
	if(Cursor)
	{
		if((keysDown() & KEY_DOWN) && Cursor->down){Cursor=Cursor->down;}
		else if((keysDown() & KEY_RIGHT) && Cursor->right){Cursor=Cursor->right;}
		else if((keysDown() & KEY_LEFT) && Cursor->left){Cursor=Cursor->left;}
		else if((keysDown() & KEY_UP) && Cursor->up){Cursor=Cursor->up;}
	}else if((keysDown() & KEY_DOWN) || (keysDown() & KEY_RIGHT) || (keysDown() & KEY_LEFT) || (keysDown() & KEY_UP))Cursor=DefaultCursor;
		// NOGBA("lala??");
	if(ce==NULL)return;
	do{
		// cn=ce->next;
		// NOGBA("lala?");
		cn=ce->previous;
		e=ce->entity;
		
		if(e->kidToDate)e->kidToDate--;
		
		if(!e->bastard)
		{
			e->prio=e->father->prio+2;
			if(e->halpha)
			{
				e->alpha=e->father->alpha;
				e->t_alpha=e->alpha;
			}
		}
		
		if(e->timep < e->t_timep)
		{
			e->Position.x+=(e->t_Position.x-e->Position.x)/(e->t_timep-e->timep);
			e->Position.y+=(e->t_Position.y-e->Position.y)/(e->t_timep-e->timep);
			e->kidToDate=1;
			e->timep++;
		}else{
			e->Position.x=e->t_Position.x;
			e->Position.y=e->t_Position.y;
		}
		
		if(e->times < e->t_times)
		{
			e->Size.x+=(e->t_Size.x-e->Size.x)/(e->t_times-e->times);
			e->Size.y+=(e->t_Size.y-e->Size.y)/(e->t_times-e->times);
			e->kidToDate=1;
			e->times++;
		}else{
			e->Size.x=e->t_Size.x;
			e->Size.y=e->t_Size.y;
		}
		
		if(e->timea < e->t_timea)
		{
			e->alpha+=(e->t_alpha-e->alpha)/(e->t_timea-e->timea);
			e->timea++;
		}else{
			e->alpha=e->t_alpha;
		}
		
		
		e->Ratio.x=divf32(e->Size.x,e->b_Size.x);
		e->Ratio.y=divf32(e->Size.y,e->b_Size.y);
		
		scaleV.x = (ce->entity->Size.x);
		scaleV.y = (ce->entity->Size.y);
		scaleV.z = inttov16(1);
		
		scaleVo.x = (ce->entity->Size.x+inttov16(2*e->outline));
		scaleVo.y = (ce->entity->Size.y+inttov16(2*e->outline));
		scaleVo.z = inttov16(1);
		
		glPushMatrix();			
		
			if(!e->bastard)
			{
				if(e->father->kidToDate || e->o_Position.x!=e->Position.x || e->o_Position.y!=e->Position.y)
				// if((keysHeld() & KEY_SELECT))
				{
					e->kidToDate=1;
					e->a_Position.x=0;
					e->a_Position.y=0;
					e->scaleVs.x = inttov16(1);
					e->scaleVs.y = inttov16(1);
					e->scaleVs.z = inttov16(1);
					if(e->halpha)e->alpha=e->father->alpha;
					f=e;
					f->temp_son=NULL;
					do{
						f->father->temp_son=f;
						f=f->father;
					}while(!f->bastard);
					do{
						// glTranslate3f32((f->Position.x),(f->Position.y),inttof32(0));
						e->a_Position.x+=mulf32(f->Position.x,e->scaleVs.x);
						e->a_Position.y+=mulf32(f->Position.y,e->scaleVs.y);
						scaleVf.x = (f->Ratio.x);
						scaleVf.y = (f->Ratio.y);
						scaleVf.z = inttov16(1);
						e->scaleVs.x = mulf32(e->scaleVs.x,scaleVf.x);
						e->scaleVs.y = mulf32(e->scaleVs.y,scaleVf.y);
						e->scaleVs.z = inttov16(1);
						// glScalev(&scaleVf);
						f=f->temp_son;
					}while(f->temp_son!=NULL);
					e->a_Position.x+=mulf32(e->Position.x,e->scaleVs.x);
					e->a_Position.y+=mulf32(e->Position.y,e->scaleVs.y);
					// e->scaleVs.x = mulf32(e->scaleVs.x,scaleV.x);
					// e->scaleVs.y = mulf32(e->scaleVs.y,scaleV.y);
					e->a_Size.x=mulf32(e->scaleVs.x,e->Size.x);
					e->a_Size.y=mulf32(e->scaleVs.y,e->Size.y);
				}
			}else{
				e->a_Position.x=e->Position.x;
				e->a_Position.y=e->Position.y;
				e->a_Size.x=e->scaleVs.x=e->Size.x;
				e->a_Size.y=e->scaleVs.x=e->Size.y;
			}
			glTranslate3f32((e->a_Position.x),(e->a_Position.y),inttof32(ce->entity->prio));
			// glScalev(&e->scaleVs);
			// glTranslate3f32((ce->entity->Position.x),(ce->entity->Position.y),inttof32(ce->entity->prio));
			/*Game_glRotateXi(e->angleX);
			Game_glRotateYi(e->angleY);
			Game_glRotateZi(e->angleZ);*/
			if((ce->entity->alpha>>10) > 0)
			{
				glPolyFmt(POLY_ALPHA(ce->entity->alpha>>10) | POLY_CULL_BACK | POLY_ID(ce->entity->id));
				// glPolyFmt(POLY_ALPHA(ce->entity->alpha>>10) | POLY_CULL_NONE | POLY_ID(ce->entity->id));
				API_DrawingOutline=false;
				API_Updating=true;
				glPushMatrix();
					glScalev(&scaleV);
					glColor(ce->entity->color);
					// NOGBA("lala");
					ce->entity->Draw(ce->entity);
				glPopMatrix(1);
				API_Updating=false;
				if(e->outline)
				{
					API_DrawingOutline=true;
					glPushMatrix();
						glTranslate3f32(-inttof32(e->outline),-inttof32(e->outline),inttof32(-1));
						glScalev(&scaleVo);
						glColor(e->outline_color);
						ce->entity->Draw(ce->entity);
					glPopMatrix(1);
				}
				/*if(e->shadow)
				{
					glPolyFmt(POLY_ALPHA((12*(ce->entity->alpha>>10)/31)+1) | POLY_CULL_BACK | POLY_ID(63));
					glPushMatrix();
						if(f32toint(e->Size.x)<128 || f32toint(e->Size.y)<64)glTranslate3f32(inttof32(8*f32toint(e->Size.x))/160,inttof32(12*f32toint(e->Size.y))/64,inttof32(-1));
						else glTranslate3f32(inttof32(12),inttof32(12),inttof32(-1));
						glScalev(&scaleV);
						glScalev(&scaleVsh);
						glColor(RGB15(0,0,0));
						ce->entity->Draw(ce->entity);
					glPopMatrix(1);
				}*/
				glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);
			}
		glPopMatrix(1);
		e->o_Position=e->Position;
		e->o_Size=e->a_Size;
		ce=cn;
	}while(ce!=NULL);
}

void computeGuiDirectionsEntity(guiEntityList_struct *cl, guiEntity_struct *e, u8 force)
{
	guiListElement_struct *cn, *ce=cl->last;
	guiEntity_struct *e2;//, *f;
	int dl,dr,du,dd;
	if(ce==NULL)return;
	coord2Ds pos=e->Position;
	if(force==3)pos=e->a_Position;
	pos.x=f32toint(pos.x+e->Size.x/2);
	pos.y=f32toint(pos.y+e->Size.y/2);
	dl=256*256+384*384;
	dr=256*256+384*384;
	du=256*256+384*384;
	dd=256*256+384*384;
	e->left=NULL;
	e->right=NULL;
	e->up=NULL;
	e->down=NULL;
	do{
		cn=ce->previous;
		e2=ce->entity;
		
		if(!e2->bastard && e2->type==1)
		{
			coord2Ds pos2=e2->Position;
			if(force==3)pos2=e2->a_Position;
			if((e->father==e2->father || force==2) && (e2->alpha>0 || force==1) && pos2.x<inttof32(256) && pos2.x>0 && pos2.y>-inttof32(192) && pos2.y<inttof32(192))
			{
				pos2.x=f32toint(pos2.x+e2->Size.x/2);
				pos2.y=f32toint(pos2.y+e2->Size.y/2);
				int d=(pos.x-pos2.x)*(pos.x-pos2.x)+(pos.y-pos2.y)*(pos.y-pos2.y);
				if(pos2.x>pos.x && dr>d){dr=d;e->right=e2;}
				if(pos2.x<pos.x && dl>d){dl=d;e->left=e2;}
				if(pos2.y>pos.y && dd>d){dd=d;e->down=e2;}
				if(pos2.y<pos.y && du>d){du=d;e->up=e2;}
			}
		}
		
		ce=cn;
	}while(ce!=NULL);
}

void computeDirections(guiEntityList_struct *cl, u8 force)
{
	guiListElement_struct *cn, *ce=cl->last;
	guiEntity_struct *e;//, *f;
	if(ce==NULL)return;
	do{
		cn=ce->previous;
		e=ce->entity;
		if(e->type==1)
		{
			// NOGBA("COMPUTING !");
			computeGuiDirectionsEntity(cl, e, force);
		}		
		ce=cn;
	}while(ce!=NULL);
}

void deleteGuiEntitySons(guiEntityList_struct *cl, guiEntity_struct* father)
{
	guiListElement_struct *cn, *ce=cl->first;
	do{
		cn=ce->next;
		if(ce->entity->father==father)
		{
			//NOGBA("kill %p, %p",ce->entity, father);
			deleteGuiEntitySons(cl, ce->entity);
			cn=ce->next;
		}
		ce=cn;
	}while(ce!=NULL);
	ce=cl->first;
	do{
		cn=ce->next;
		if(ce->entity->father==father)
		{
			//NOGBA("kill2 %p, %p",ce->entity, father);
			if(ce->previous!=NULL)ce->previous->next=cn;
			else cl->first=cn;
			if(ce->next!=NULL)ce->next->previous=ce->previous;
			else cl->last=ce->previous;
			if(ce->entity->data!=NULL)free(ce->entity->data);
			if(ce->entity!=NULL)free(ce->entity);
			if(ce!=NULL)free(ce);
			cl->count--;
			//break;
		}
		ce=cn;
	}while(ce!=NULL);
}

void deleteGuiEntity(guiEntityList_struct *cl, u16 id)
{
	guiListElement_struct *cn, *ce=cl->first;
	do{
		cn=ce->next;
		if(ce->entity->id==id)
		{
			if(ce->previous!=NULL)ce->previous->next=cn;
			else cl->first=cn;
			if(ce->next!=NULL)ce->next->previous=ce->previous;
			else cl->last=ce->previous;
			if(ce->entity->data!=NULL)free(ce->entity->data);
			if(ce->entity!=NULL)free(ce->entity);
			if(ce!=NULL)free(ce);
			cl->count--;
			break;
		}
		ce=cn;
	}while(ce!=NULL);
}

void initGui()
{
	guiEntityList_struct *cl=&API_List;
	cl->count=0;
	//glSetOutlineColor(0, RGB15(31,0,0));
	API_ToCall=NULL;
	API_ToCallEntity=NULL;
	buttonBlock=NULL;
	Cursor=NULL;
	DefaultCursor=NULL;
	loadFont(&APIfont, CHARSIZE, CHARSIZE/2);
	cull=true;
	keyboardLock=false;
	touchOnly=true;
}

void projectGui(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glOrtho(-128, 127, 191, 0, -1000, 1000);
	
	glMatrixMode(GL_MODELVIEW);
}

void guiCall()
{
	if(keyboardLock && API_ToCall!=(guiFunction)&keyboardButtonPressed)API_ToCall=NULL;
	if(API_ToCall!=NULL)API_ToCall(API_ToCallEntity);
	API_ToCall=NULL;
	API_ToCallEntity=NULL;
}

void cleanUpGui()
{
	guiEntityList_struct *cl=&API_List;
	guiListElement_struct *cn, *ce=cl->first;
	NOGBA("Cleaning up...");
	do{
		if(ce!=NULL)
		{
			cn=ce->next;
			if(ce->entity->data!=NULL)free(ce->entity->data);
			if(ce->entity!=NULL)free(ce->entity);
			if(ce!=NULL)free(ce);
			ce=cn;
		}
	}while(ce!=NULL);
	cl->count=0;
	cl->last=NULL;
	cl->first=NULL;
	NOGBA("All cleaned up !");
}
