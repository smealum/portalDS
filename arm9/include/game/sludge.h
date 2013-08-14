#ifndef SLUDGE_H
#define SLUDGE_H

#define SLUDGEMARGIN (256)

void initSludge(void);
void freeSludge(void);
void addSludgeRectangle(rectangle_struct* rec);
void drawSludge(room_struct* r);

bool collideBoxSludge(OBB_struct* o);

#endif
