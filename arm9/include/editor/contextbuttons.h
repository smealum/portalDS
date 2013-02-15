#ifndef CONTEXTBUTONS_H
#define CONTEXTBUTONS_H

typedef struct
{
	const char* string;
	buttonTargetFunction targetFunction;
}contextButton_struct;

void initContextButtons(void);
void setupContextButtons(contextButton_struct* cb, u8 n);
bool updateContextButtons(touchPosition* tp);
void drawContextButtons(void);
void cleanUpContextButtons(void);

#endif
