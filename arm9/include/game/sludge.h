#ifndef SLUDGE_H
#define SLUDGE_H

void initSludge(void);
void addSludgeRectangle(rectangle_struct* rec);
void drawSludge(room_struct* r);

bool collideBoxSludge(OBB_struct* o);

#endif
