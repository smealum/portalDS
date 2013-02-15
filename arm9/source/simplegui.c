#include "common/general.h"

void initSimpleGui(void)
{
	initText();
}

void drawSimpleLabel(sguiLabel_struct* l)
{
	if(!l || !l->string)return;

	drawString(l->string, RGB15(31,31,31), inttof32(1), l->position.x, l->position.y);
}
