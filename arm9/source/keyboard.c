#include "common/general.h"

#define KEYBOARDBUTTONS 39

sguiButton_struct* keyboardButton[KEYBOARDBUTTONS];

static const char keyboardButtons[] = { '1','\0','2','\0','3','\0','4','\0','5','\0','6','\0','7','\0','8','\0','9','\0','0','\0',
										'q','\0','w','\0','e','\0','r','\0','t','\0','y','\0','u','\0','i','\0','o','\0','p','\0',
										'a','\0','s','\0','d','\0','f','\0','g','\0','h','\0','j','\0','k','\0','l','\0',
										'z','\0','x','\0','c','\0','v','\0','b','\0','n','\0','m','\0',' ','\0'};
						
static const u8 keyboardRows[] = {10,10,9,7};

char* keyboardString;
int keyboardCursor, keyboardStrlen;
bool keyboardLock;

void keyboardButtonPressed(sguiButton_struct* b)
{
	if(!b || !b->string || !keyboardString)return;
	char c=b->string[0];

	switch(c)
	{
		//backspace
		case '<':
			if(keyboardCursor)
			{
				keyboardString[keyboardCursor-1]='\0';
				keyboardCursor--;
			}
			break;
		//characters
		default:
			if(keyboardCursor<keyboardStrlen){keyboardString[keyboardCursor++]=c;keyboardString[keyboardCursor]='\0';}
			break;
	}
}

void setupKeyboard(char* buffer, u8 size, s16 x, s16 y)
{
	int i, j, k, h;

	i=0;h=5;
	for(k=0;k<4;k++)
	{
		for(j=0;j<keyboardRows[k];j++)
		{
			keyboardButton[i]=createSimpleButton(vect(j*20+5+x, h+y, 0), &keyboardButtons[i*2], (buttonTargetFunction)keyboardButtonPressed);
			i++;
		}
		h+=14;
	}
	keyboardButton[i]=createSimpleButton(vect(48+5+x, h+y, 0), "       ", (buttonTargetFunction)keyboardButtonPressed);
	i++;
	keyboardButton[i]=createSimpleButton(vect(j*20+5+x, h-14+y, 0), "<-", (buttonTargetFunction)keyboardButtonPressed);

	keyboardString=buffer;
	keyboardStrlen=size;
	keyboardCursor=0;
}
