#ifndef BIGBUTTON_H
#define BIGBUTTON_H

#define NUMBIGBUTTONS (16)

typedef struct
{
	room_struct* room;
	rectangle_struct* surface;
	activator_struct activator;
	vect3D position;
	bool active;
	bool used;
}bigButton_struct;

void initBigButtons(void);
bigButton_struct* createBigButton(room_struct* r, vect3D position);
void drawBigButtons(void);
void updateBigButtons(void);

#endif
