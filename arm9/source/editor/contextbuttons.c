#include "editor/editor_main.h"

#define CONTEXTMARGINX (4)
#define CONTEXTMARGINY (4)

#define CONTEXTSTEPY (SIMPLEBUTTONSIZEY+2)

void initContextButtons(void)
{
	initSimpleGui();
}

bool updateContextButtons(touchPosition* tp)
{
	return updateSimpleGui(tp->px,tp->py);
}

void setupContextButtons(contextButton_struct* cb, u8 n)
{
	if(!cb || !n)return;

	int i;
	for(i=0;i<n;i++)
	{
		createSimpleButton(vect(CONTEXTMARGINX,CONTEXTMARGINY+CONTEXTSTEPY*i,0), cb[i].string, cb[i].targetFunction);
	}
}

void drawContextButtons(void)
{
	drawSimpleGui();
}

void cleanUpContextButtons(void)
{
	cleanUpSimpleButtons();
}
