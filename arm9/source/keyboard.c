#include "common/general.h"

extern guiEntityList_struct API_List;
extern guiEntity_struct* Cursor;

void initKeyboard(u8 t)
{
	int i, j, k, h;
	char string[2];
	
	if(t==0 || t>1){keyboardWindow=createWindow(150, 50, 170, 80, 31, RGB15(31,31,31), 1, RGB15(0,0,0), "");keyboardWindow->prio=800;}	
	
	if(t>0)
	{
		i=0;h=5;
		for(k=0;k<4;k++)
		{
			for(j=0;j<keyboardRows[k];j++)
			{
				sprintf(string,"%c",keyboardButtons[i]);
				keyboardButton[i]=createButtonFather(j*16+5, h, RGB15(31,31,31), &keyboardButtonPressed, keyboardWindow, string, "arsenal_buttons.pcx", false);
				i++;
			}
			h+=16;
		}
		keyboardButton[i]=createButtonFather(32+5, h, RGB15(31,31,31), &keyboardButtonPressed, keyboardWindow, "       ", "arsenal_buttons.pcx", false);
		i++;
		keyboardButton[i]=createButtonFather(j*16+5, h-16, RGB15(31,31,31), &keyboardButtonPressed, keyboardWindow, "<-", "arsenal_buttons.pcx", false);
		i++;
		keyboardButton[i]=createButtonFather((j+1)*16+14, h-30, RGB15(31,31,31), &keyboardButtonPressed, keyboardWindow, "", "enter.pcx", false);
		setSize(keyboardButton[i],22,30);
		computeDirections(&API_List, true);
		setAlphaSons(keyboardWindow,0);
	}
	keyboardLock=false;
}

void keyboardButtonPressed(guiEntity_struct *e)
{
	int i;
	Cursor=e;
	for(i=0;i<KEYBOARDBUTTONS;i++)
	{
		if(((guiButtonData_struct*)keyboardButton[i]->data)->clicked)
		{
			NOGBA("%d",i);
			if(i<KEYBOARDBUTTONS-2)
			{
				if(keyboardCursor<keyboardStrlen-1)
				{
					(keyboardString)[keyboardCursor]=keyboardButtons[i];
					keyboardCursor++;
					(keyboardString)[keyboardCursor]='\0';
				}
			}else if(i==KEYBOARDBUTTONS-2){
				if(keyboardCursor>0)
				{
					keyboardCursor--;
					(keyboardString)[keyboardCursor]='\0';
				}
			}else if(i==KEYBOARDBUTTONS-1){
				if(keyboardCursor>0)
				{
					keyboardReturn(e);
				}
			}
			break;
		}
	}
	for(i=0;i<KEYBOARDBUTTONS;i++)((guiButtonData_struct*)keyboardButton[i]->data)->clicked=false;
	NOGBA("%s",(keyboardString));
}

void setupKeyboard(char* string, u8 stringlen, guiDrawFunction r)
{
	keyboardString=string;
	keyboardStrlen=stringlen;
	keyboardCursor=0;
	keyboardReturn=r;
}

void lockKeyboard()
{
	keyboardLock=true;
}

void showKeyboard()
{
	setPosition(keyboardWindow,-85,50);
	setAlpha(keyboardWindow,15);
	setAlphaSons(keyboardWindow,31);
	Cursor=keyboardButton[38];
}

void hideKeyboard()
{
	setPosition(keyboardWindow,150,50);
	setAlpha(keyboardWindow,15);
	setAlphaSons(keyboardWindow,0);
	keyboardLock=false;
	Cursor=NULL;
}

void appearKeyboard(u8 t)
{
	moveGuiEntity(keyboardWindow,-85,50,t);
	fadeGuiEntity(keyboardWindow,15,t);
	fadeSons(keyboardWindow,31,t);
	Cursor=keyboardButton[38];
}

void disappearKeyboard(u8 t)
{
	moveGuiEntity(keyboardWindow,150,50,t);
	fadeGuiEntity(keyboardWindow,0,t);
	fadeSons(keyboardWindow,0,t);
	keyboardLock=false;
	Cursor=NULL;
}
