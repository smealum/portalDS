#ifndef SLUDGE_H
#define SLUDGE_H

#define SLUDGEMARGIN (256)

void initSludge(void);
void freeSludge(void);
void addSludgeRectangle(rectangle_struct* rec);
void drawSludge(room_struct* r);

bool collideBoxSludge(OBB_struct* o);
bool collideAABBSludge(vect3D p, vect3D s);

#endif
