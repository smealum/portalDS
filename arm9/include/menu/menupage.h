#ifndef MENUPAGE_H
#define MENUPAGE_H

typedef struct
{
	const char* string;
	buttonTargetFunction targetFunction;
}menuButton_struct;

extern menuButton_struct mainMenuPage[];
extern u8 mainMenuPageLength;

extern menuButton_struct playMenuPage[];
extern u8 playMenuPageLength;

void initMenuButtons(void);
void setupMenuPage(menuButton_struct* mp, u8 n);

#endif
