#include "editor/editor_main.h"

#define MINSCALE (inttof32(1)/2)
#define MAXSCALE (inttof32(3)/2)

#define NUMLINESX 9
#define NUMLINESY 8

vect3D gridTranslation;
int32 gridScale;

void projectGrid(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof32(0, inttof32(255)>>6, inttof32(191)>>6, 0, -1000, 1000);
	
	glMatrixMode(GL_MODELVIEW);
}

void initGrid(void)
{	
	gridScale=inttof32(1);
	gridTranslation.x=0;
	gridTranslation.y=0;
	gridTranslation.z=0;
} 

void transformGrid(void)
{
	glTranslate3f32(inttof32(4)/2,inttof32(3)/2,0);
	glScalef32(gridScale,gridScale,gridScale);
	glTranslate3f32(gridTranslation.x,gridTranslation.y,gridTranslation.z);
}

void setGridScale(int32 s){gridScale=s;if(gridScale<MINSCALE)gridScale=MINSCALE;else if(gridScale>MAXSCALE)gridScale=MAXSCALE;}
void setGridTranslation(vect3D v){gridTranslation=v;}

void scaleGrid(int32 s){setGridScale(gridScale+s);}
void translateGrid(vect3D v){setGridTranslation(vect(gridTranslation.x+v.x,gridTranslation.y+v.y,gridTranslation.z+v.z));}

void getGridCell(int* x, int* y, int px, int py)
{
	*x=(divf32((px-128)*(1<<6),gridScale)-gridTranslation.x)/64;
	if(*x>=0)*x/=8;
	else{*x/=8;(*x)--;}
	
	*y=(divf32((py-96)*(1<<6),gridScale)-gridTranslation.y)/64;
	if(*y>=0)*y/=8;
	else{*y/=8;(*y)--;}
}

void drawGrid(void)
{
	int i;
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);
	
	glPushMatrix();
	
	glTranslate3f32(-(gridTranslation.x-gridTranslation.x%(16<<6)),-(gridTranslation.y-gridTranslation.y%(16<<6)),0);
	
	glBegin(GL_TRIANGLES);
		glColor3b(200,200,200);
		for(i=8-NUMLINESX*2;i<8+NUMLINESX*2;i++)
		{
			int k=(i-8)*16;
			glVertex3v16((k)<<6, -(112<<7), 0);
			glVertex3v16((k)<<6, (112<<7), 0);
			glVertex3v16((k)<<6, (112<<7), 0);
		}
		for(i=6-NUMLINESY*2;i<6+NUMLINESY*2;i++)
		{
			int k=(i-8)*16;
			glVertex3v16(-(144)<<7, (k)<<6, 0);
			glVertex3v16((144)<<7, (k)<<6, 0);
			glVertex3v16((144)<<7, (k)<<6, 0);
		}
		glColor3b(100,100,100);
		for(i=8-NUMLINESX*2;i<8+NUMLINESX*2;i++)
		{
			int k=(i-8)*16+8;
			glVertex3v16((k)<<6, -(112<<7), 0);
			glVertex3v16((k)<<6, (112<<7), 0);
			glVertex3v16((k)<<6, (112<<7), 0);
		}
		for(i=6-NUMLINESY*2;i<6+NUMLINESY*2;i++)
		{
			int k=(i-8)*16+8;
			glVertex3v16(-(144)<<7, (k)<<6, 0);
			glVertex3v16((144)<<7, (k)<<6, 0);
			glVertex3v16((144)<<7, (k)<<6, 0);
		}
	glEnd();
	
	glPopMatrix(1);
}
