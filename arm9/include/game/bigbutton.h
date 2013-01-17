#ifndef BIGBUTTON_H
#define BIGBUTTON_H

#define NUMBIGBUTTONS (16)

typedef struct
{
	vect3D position;
	bool used;
}bigButton_struct;

void initBigButtons(room_struct* r);
bigButton_struct* createBigButton(vect3D position);
void drawBigButtons(void);

#endif
