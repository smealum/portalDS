#ifndef MENUPAGE_H
#define MENUPAGE_H

typedef struct
{
	const char* string;
	buttonTargetFunction targetFunction;
}menuButton_struct;

extern menuButton_struct startMenuPage[];
extern u8 startMenuPageLength;

extern menuButton_struct mainMenuPage[];
extern u8 mainMenuPageLength;

extern menuButton_struct playMenuPage[];
extern u8 playMenuPageLength;

extern menuButton_struct createMenuPage[];
extern u8 createMenuPageLength;

extern menuButton_struct newLevelMenuPage[];
extern u8 newLevelMenuPageLength;

extern menuButton_struct selectLevelMenuPage[];
extern u8 selectLevelMenuPageLength;

extern menuButton_struct loadLevelMenuPage[];
extern u8 loadLevelMenuPageLength;

void initMenuButtons(void);
void setupMenuPage(menuButton_struct* mp, u8 n);

#endif
