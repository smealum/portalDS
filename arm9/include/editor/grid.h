#ifndef __GRID9__
#define __GRID9__

void initGrid(void);
void drawGrid(void);
void projectGrid(void);
void transformGrid(void);
void setGridScale(int32 s);
void setGridTranslation(vect3D v);
void translateGrid(vect3D v);
void scaleGrid(int32 s);
void getGridCell(int* x, int* y, int px, int py);

#endif
