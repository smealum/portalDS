#include "neweditor/editor.h"

camera_struct editorCamera;
editorRoom_struct editorRoom;
selection_struct editorSelection;

vect3D editorTranslation;
int32 editorScale;


void initSelection(selection_struct* s)
{
	if(!s)s=&editorSelection;
	
	s->firstFace=s->secondFace=NULL;
	s->origin=s->size=vect(0,0,0);
	s->active=false;
}

void initRoomEditor(void)
{
	initBlocks();
	initEditorRoom(&editorRoom);
	initSelection(&editorSelection);
	initCamera(&editorCamera);
	initProjectionMatrixOrtho(&editorCamera, inttof32(-128), inttof32(127),inttof32(-96), inttof32(95), inttof32(-1000), inttof32(1000));
	editorCamera.position=vect(0,0,0);
	editorTranslation=vect(0,0,0);
	editorScale=inttof32(1);
}

void updateSelection(selection_struct* s)
{
	if(!s)s=&editorSelection;
	if(!s->active || !s->firstFace || !s->secondFace)return;
	
	s->planar=(s->firstFace->direction==s->secondFace->direction) && 
			(((s->firstFace->direction==0 || s->firstFace->direction==1) && s->firstFace->x==s->secondFace->x)
			 || ((s->firstFace->direction==2 || s->firstFace->direction==3) && s->firstFace->y==s->secondFace->y)
			 || ((s->firstFace->direction==4 || s->firstFace->direction==5) && s->firstFace->z==s->secondFace->z));
	
	if(s->planar)
	{
		s->origin=vect(s->firstFace->x,s->firstFace->y,s->firstFace->z);
		s->size=vectDifference(vect(s->secondFace->x,s->secondFace->y,s->secondFace->z),s->origin);
	}else{
		s->origin=vect(s->firstFace->x,s->firstFace->y,s->firstFace->z);
		switch(s->firstFace->direction)
		{
			case 0: if(s->secondFace->x>s->origin.x)s->origin.x++; break;
			case 1: if(s->secondFace->x<s->origin.x)s->origin.x--; break;
			case 2: if(s->secondFace->y>s->origin.y)s->origin.y++; break;
			case 3: if(s->secondFace->y<s->origin.y)s->origin.y--; break;
			case 4: if(s->secondFace->z>s->origin.z)s->origin.z++; break;
			case 5: if(s->secondFace->z<s->origin.z)s->origin.z--; break;
		}
		s->size=vectDifference(vect(s->secondFace->x,s->secondFace->y,s->secondFace->z),s->origin);
		switch(s->secondFace->direction)
		{
			case 0: if(s->firstFace->x>s->secondFace->x)s->size.x++; break;
			case 1: if(s->firstFace->x<s->secondFace->x)s->size.x--; break;
			case 2: if(s->firstFace->y>s->secondFace->y)s->size.y++; break;
			case 3: if(s->firstFace->y<s->secondFace->y)s->size.y--; break;
			case 4: if(s->firstFace->z>s->secondFace->z)s->size.z++; break;
			case 5: if(s->firstFace->z<s->secondFace->z)s->size.z--; break;
		}
	}
	if(s->size.x<0){s->origin.x+=s->size.x;s->size.x=-s->size.x;}
	if(s->size.y<0){s->origin.y+=s->size.y;s->size.y=-s->size.y;}
	if(s->size.z<0){s->origin.z+=s->size.z;s->size.z=-s->size.z;}
	s->size.x++;s->size.y++;s->size.z++;
}

void updateEditorCamera(void)
{
	camera_struct* c=&editorCamera;
	
	c->position=vect(0,0,0);
	updateViewMatrix(c);
	updateFrustum(c);
	
	fixMatrix(c->transformationMatrix);
}

void transformRay(vect3D* o, vect3D* v)
{
	int32* m=editorCamera.transformationMatrix;
	// int32 m[9]; transposeMatrix33(editorCamera.transformationMatrix,m);
	*o=evalVectMatrix33(m,vectDifference(divideVect(*o,editorScale),editorTranslation));
	*v=evalVectMatrix33(m,*v);
}

blockFace_struct* getBlockFaceCoordinates(s16 x, s16 y)
{	
	vect3D o=vect(inttof32(x),inttof32(y),inttof32(0));
	vect3D v=vect(0,0,-inttof32(1));
	transformRay(&o, &v);
	
	return collideLineBlockFaceListClosest(editorRoom.blockFaceList, o, v);
}

void updateRoomEditor(void)
{
	touchPosition touchCurrent;
	touchRead(&touchCurrent);
	
	updateEditorCamera();
	
	//TEMP CONTROLS
	if(keysHeld() & KEY_R)editorScale+=inttof32(1)/16;
	if(keysHeld() & KEY_L)editorScale-=inttof32(1)/16;
	
	if(keysHeld() & KEY_UP)editorTranslation.y-=inttof32(1);
	else if(keysHeld() & KEY_DOWN)editorTranslation.y+=inttof32(1);
	if(keysHeld() & KEY_RIGHT)editorTranslation.x-=inttof32(1);
	else if(keysHeld() & KEY_LEFT)editorTranslation.x+=inttof32(1);
	
	if(keysHeld() & KEY_A)rotateMatrixY(editorCamera.transformationMatrix, 64, true);
	if(keysHeld() & KEY_Y)rotateMatrixY(editorCamera.transformationMatrix, -64, true);
	if(keysHeld() & KEY_X)rotateMatrixX(editorCamera.transformationMatrix, 64, false);
	if(keysHeld() & KEY_B)rotateMatrixX(editorCamera.transformationMatrix, -64, false);
	
	if(keysDown() & KEY_TOUCH)
	{
		blockFace_struct* bf=getBlockFaceCoordinates(touchCurrent.px-128, 96-touchCurrent.py);
		if(bf)
		{
			editorSelection.firstFace=editorSelection.secondFace=bf;
			editorSelection.active=true;
		}else{
			editorSelection.active=false;
		}
	}else if(keysHeld() & KEY_TOUCH)
	{
		if(editorSelection.active)
		{
			blockFace_struct* bf=getBlockFaceCoordinates(touchCurrent.px-128, 96-touchCurrent.py);
			if(bf)editorSelection.secondFace=bf;
		}else{
			
		}
	}
	
	updateSelection(NULL);
}

void drawSelection(selection_struct* s)
{
	if(!s)s=&editorSelection;
	if(!s->active || !s->firstFace || !s->secondFace)return;
	
	blockFace_struct* bf=s->firstFace;
	
	if(s->planar)
	{
		u32* vtxPtr=packedVertex[bf->direction];
		vect3D n=faceNormals[bf->direction];
		
		unbindMtl();
		
		glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
		
		glPushMatrix();		
			glScalef32((BLOCKSIZEX),(BLOCKSIZEY),(BLOCKSIZEZ));
			glTranslate3f32(-inttof32(1)/2,-inttof32(1)/2,-inttof32(1)/2);
			glTranslate3f32(inttof32(s->origin.x-ROOMARRAYSIZEX/2),inttof32(s->origin.y-ROOMARRAYSIZEY/2),inttof32(s->origin.z-ROOMARRAYSIZEZ/2));
			glTranslate3f32(n.x/16, n.y/16, n.z/16);
			glScalef32(inttof32(s->size.x),inttof32(s->size.y),inttof32(s->size.z));
			glTranslate3f32(inttof32(1)/2,inttof32(1)/2,inttof32(1)/2);
			
			GFX_COLOR=RGB15(31,0,0);		
			GFX_BEGIN=GL_QUADS;
			
			GFX_VERTEX10=*vtxPtr++;
			GFX_VERTEX10=*vtxPtr++;
			GFX_VERTEX10=*vtxPtr++;
			GFX_VERTEX10=*vtxPtr++;		
		glPopMatrix(1);
	}else{
		u32* vtxPtr=(u32*)packedVertex;
		
		unbindMtl();
		
		glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
		
		glPushMatrix();		
			glScalef32((BLOCKSIZEX),(BLOCKSIZEY),(BLOCKSIZEZ));
			glTranslate3f32(-inttof32(1)/2,-inttof32(1)/2,-inttof32(1)/2);
			glTranslate3f32(inttof32(s->origin.x-ROOMARRAYSIZEX/2),inttof32(s->origin.y-ROOMARRAYSIZEY/2),inttof32(s->origin.z-ROOMARRAYSIZEZ/2));
			glScalef32(inttof32(s->size.x),inttof32(s->size.y),inttof32(s->size.z));
			glTranslate3f32(inttof32(1)/2,inttof32(1)/2,inttof32(1)/2);
			
			GFX_COLOR=RGB15(31,0,0);		
			GFX_BEGIN=GL_QUADS;
			int i;for(i=0;i<6*4;i++)GFX_VERTEX10=*vtxPtr++;	
		glPopMatrix(1);
	}
}

void drawRoomEditor(void)
{
	projectCamera(&editorCamera);
	glLoadIdentity();
	
	glPushMatrix();
		
		glScalef32(editorScale,editorScale,editorScale);
		glTranslate3f32(editorTranslation.x,editorTranslation.y,editorTranslation.z);
		transformCamera(&editorCamera);
		drawEditorRoom(&editorRoom);
		drawSelection(NULL);
		
	glPopMatrix(1);
	
	glFlush(0);
}

void freeRoomEditor(void)
{
	freeEditorRoom(&editorRoom);
}



