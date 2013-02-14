#include "editor/editor_main.h"

#define TOUCHSPEEDX (-32)
#define TOUCHSPEEDY (32)

camera_struct editorCamera;
editorRoom_struct editorRoom;

vect3D editorTranslation;
int32 editorScale;

vect3D lineOfTouchOrigin, lineOfTouchVector;
vect3D planeOfTouch[2];
touchPosition currentTouch, oldTouch;

bool currentScreen;

void initRoomEdition(void)
{
	initLights();
	initBlocks();
	initEntities();
	initInterface();
	initEditorRoom(&editorRoom);
	initSelection(NULL);
	initCamera(&editorCamera);
	// initProjectionMatrixOrtho(&editorCamera, inttof32(-128), inttof32(127),inttof32(-96), inttof32(95), inttof32(-1000), inttof32(1000));
	initProjectionMatrix(&editorCamera, 70*90, inttof32(4)/3, inttof32(1)/10, inttof32(1000)); //TEMP?
	editorCamera.position=vect(0,0,0);
	editorTranslation=vect(0,0,inttof32(-1));
	editorScale=inttof32(1);
	lineOfTouchOrigin=vect(0,0,0);
	lineOfTouchVector=vect(0,0,0);
	currentScreen=false;

	//initial camera setup
	rotateMatrixY(editorCamera.transformationMatrix, 2048+384, true);
	rotateMatrixX(editorCamera.transformationMatrix, 1024+128, false);
	editorScale=inttof32(8*20);

	//controls stuff
	touchRead(&currentTouch);
	oldTouch=currentTouch;

	//cosmetics
	glSetOutlineColor(0,RGB15(0,0,0));
	glSetOutlineColor(1,RGB15(29,15,3));

	glMaterialf(GL_AMBIENT, RGB15(8,8,8));
	glMaterialf(GL_DIFFUSE, RGB15(24,24,24));
	glMaterialf(GL_SPECULAR, RGB15(0,0,0));
	glMaterialf(GL_EMISSION, RGB15(0,0,0));

	glSetToonTableRange(0, 2, RGB15(8,8,8));
	glSetToonTableRange(3, 31, RGB15(24,24,24));

	glLight(0, RGB15(31,31,31), cosLerp(4096)>>3, 0, sinLerp(4096)>>3);
}

void updateEditorCamera(void)
{
	camera_struct* c=&editorCamera;
	
	updateViewMatrix(c);
	updateFrustum(c);
	
	fixMatrix(c->transformationMatrix);
}

void transformRay(vect3D* o, vect3D* v)
{
	int32* m=editorCamera.transformationMatrix;
	// int32 m[9]; transposeMatrix33(editorCamera.transformationMatrix,m);
	// *o=evalVectMatrix33(m,vectDifference(divideVect(*o,editorScale),editorTranslation));
	*o=addVect(evalVectMatrix33(m,divideVect(*o,editorScale)),editorCamera.position);
	*v=evalVectMatrix33(m,*v);
}

blockFace_struct* getBlockFaceTouch(int32* d)
{
	return collideLineBlockFaceListClosest(editorRoom.blockFaceList, lineOfTouchOrigin, lineOfTouchVector, d);
}

void* getBlockEntityTouch(u8* t)
{
	int32 d1, d2;
	if(t)*t=0;
	entity_struct* e=collideLineEntities(lineOfTouchOrigin, lineOfTouchVector, planeOfTouch[0], planeOfTouch[1], &d1);
	blockFace_struct* bf=getBlockFaceTouch(&d2);
	if(!e || d1>d2)return (void*)bf;
	if(t)*t=1;
	return (void*)e;
}

void updateLineOfTouch(s16 x, s16 y)
{
	vect3D o=vect(inttof32(x),inttof32(y),inttof32(0));
	vect3D v=vect(0,0,-inttof32(1));
	getUnprojectedZLine(&editorCamera, x, y, &o, &v);
	transformRay(&o, &v);
	lineOfTouchOrigin=o;
	lineOfTouchVector=vectMultInt(normalize(v),-1);
	planeOfTouch[0]=evalVectMatrix33(editorCamera.transformationMatrix,vect(inttof32(1),0,0));
	planeOfTouch[1]=evalVectMatrix33(editorCamera.transformationMatrix,vect(0,inttof32(1),0));
}

bool collideLinePlane(vect3D p, vect3D n, vect3D o, vect3D v, vect3D* ip)
{
	int32 p1=dotProduct(v,n);
	if(!equals(p1,0))
	{		
		int32 p2=dotProduct(vectDifference(p,o),n);
		int32 k=divf32(p2,p1);
		if(ip)*ip=addVect(o,vectMult(v,k));
		return true;
	}
	return false;
}

vect3D getDragPosition(blockFace_struct* bf, vect3D o, vect3D v, vect3D los)
{
	if(!bf)return vect(0,0,0);
	vect3D n, ip;
	n=los;
	switch(bf->direction)
	{
		case 0: case 1: n.x=0; break;
		case 2: case 3: n.y=0; break;
		case 4: case 5: n.z=0; break;
	}
	n=normalize(n);
	if(!collideLinePlane(addVect(faceOrigin[bf->direction],getBlockPosition(bf->x,bf->y,bf->z)), n, o, v, &ip))return faceOrigin[bf->direction];
	return ip;
}

void switchScreens(void)
{
	if(currentScreen)
	{
		pauseEditorInterface();
	}else{
		editorSelection.entity=NULL;
		editorSelection.active=false;
		editorSelection.selecting=false;
	}
	currentScreen^=1;
	lcdSwap();
}

void roomEditorCursor(selection_struct* sel)
{
	if(!sel)sel=&editorSelection;
	if((keysDown() & KEY_TOUCH) && !(sel->entity && !sel->entity->placed))
	{
		u8 type;
		void* ptr=getBlockEntityTouch(&type);
		if(type)
		{
			//entity
			entity_struct* e=(entity_struct*)ptr;

			sel->active=true;
			sel->selecting=false;
			sel->entity=e;
		}else{
			//blockface
			blockFace_struct* bf=(blockFace_struct*)ptr;

			if(bf)
			{
				if(sel->active && isFaceInSelection(bf, sel))
				{
					sel->currentFace=bf;
					sel->currentPosition=vect(bf->x,bf->y,bf->z);
					sel->selecting=false;
				}else{
					sel->firstFace=sel->secondFace=bf;
					sel->active=true;
					sel->selecting=true;
				}
				sel->entity=NULL;
			}else{
				sel->active=false;
			}
		}
	}else if(keysHeld() & KEY_TOUCH)
	{
		if(sel->active)
		{
			if(sel->entity)
			{
				blockFace_struct* bf=getBlockFaceTouch(NULL);
				if(bf)
				{
					sel->error=!moveEntityToBlockFace(sel->entity, bf);
				}
			}else{
				if(sel->selecting)
				{
					blockFace_struct* bf=getBlockFaceTouch(NULL);
					if(bf)sel->secondFace=bf;
				}else{
					blockFace_struct* bf=sel->currentFace;
					bool fill=true;
					vect3D p=getDragPosition(bf, lineOfTouchOrigin, lineOfTouchVector, lineOfTouchVector);
					p=vect((p.x+(ROOMARRAYSIZEX*BLOCKSIZEX+BLOCKSIZEX)/2)/BLOCKSIZEX,(p.y+(ROOMARRAYSIZEY*BLOCKSIZEY+BLOCKSIZEY)/2)/BLOCKSIZEY,(p.z+(ROOMARRAYSIZEZ*BLOCKSIZEZ+BLOCKSIZEZ)/2)/BLOCKSIZEZ);
					
					switch(bf->direction)
					{
						case 0: p.y=bf->y; p.z=bf->z; fill=p.x>sel->currentPosition.x; break;
						case 1: p.y=bf->y; p.z=bf->z; fill=p.x<sel->currentPosition.x; break;
						case 2: p.x=bf->x; p.z=bf->z; fill=p.y>sel->currentPosition.y; break;
						case 3: p.x=bf->x; p.z=bf->z; fill=p.y<sel->currentPosition.y; break;
						case 4: p.x=bf->x; p.y=bf->y; fill=p.z>sel->currentPosition.z; break;
						default:p.x=bf->x; p.y=bf->y; fill=p.z<sel->currentPosition.z; break;
					}

					if(p.x!=sel->currentPosition.x || p.y!=sel->currentPosition.y || p.z!=sel->currentPosition.z)
					{
						blockFace_struct oldFirstFace=*sel->firstFace;
						blockFace_struct oldSecondFace=*sel->secondFace;
						blockFace_struct oldCurrentFace=*sel->currentFace;

						vect3D v=vectDifference(p, sel->currentPosition);
						vect3D o=(bf->direction%2)?(sel->origin):(adjustVectForNormal(bf->direction, sel->origin)); vect3D s=v;
						fixOriginSize(&o, &s); s=adjustVectForNormal((bf->direction%2)?(bf->direction):(oppositeDirection[bf->direction]), addVect(s,sel->size));
						vect3D oe=adjustVectForNormal(bf->direction, sel->origin); vect3D se=sel->size;

						moveEntitiesRange(oe, se, v);
						if(fill)fillBlockArrayRange(editorRoom.blockArray, &editorRoom.blockFaceList, o, s);
						else emptyBlockArrayRange(editorRoom.blockArray, &editorRoom.blockFaceList, o, s);
						getEntityBlockFacesRange(editorRoom.blockFaceList, addVect(oe,v), se, true);

						adjustSelection(&editorRoom, sel, oldFirstFace, oldSecondFace, oldCurrentFace, v);
						sel->currentPosition=p;
					}
				}
			}
		}else{
			if(abs(oldTouch.px-currentTouch.px)>2)rotateMatrixY(editorCamera.transformationMatrix, -TOUCHSPEEDX*(oldTouch.px-currentTouch.px), true);
			if(abs(oldTouch.py-currentTouch.py)>2)rotateMatrixX(editorCamera.transformationMatrix, -TOUCHSPEEDY*(oldTouch.py-currentTouch.py), false);
		}
	}
}

void roomEditorControls(void)
{
	//TEMP CONTROLS
	// if(keysHeld() & KEY_R)editorScale+=inttof32(2);
	// if(keysHeld() & KEY_L)editorScale-=inttof32(2);
	if(keysHeld() & KEY_R)moveCameraImmediate(&editorCamera, vect(0,0,inttof32(1)/64));
	if(keysHeld() & KEY_L)moveCameraImmediate(&editorCamera, vect(0,0,-inttof32(1)/64));
	
	if(keysHeld() & KEY_UP)moveCameraImmediate(&editorCamera, vect(0,inttof32(1)/64,0));
	else if(keysHeld() & KEY_DOWN)moveCameraImmediate(&editorCamera, vect(0,-inttof32(1)/64,0));
	if(keysHeld() & KEY_RIGHT)moveCameraImmediate(&editorCamera, vect(inttof32(1)/64,0,0));
	else if(keysHeld() & KEY_LEFT)moveCameraImmediate(&editorCamera, vect(-inttof32(1)/64,0,0));
	
	if(keysHeld() & KEY_A)rotateMatrixY(editorCamera.transformationMatrix, 64, true);
	if(keysHeld() & KEY_Y)rotateMatrixY(editorCamera.transformationMatrix, -64, true);
	if(keysHeld() & KEY_X)rotateMatrixX(editorCamera.transformationMatrix, 64, false);
	if(keysHeld() & KEY_B)rotateMatrixX(editorCamera.transformationMatrix, -64, false);
	
	if(keysHeld() & KEY_START){writeMapEditor(&editorRoom, "fat:/test.map");}
	if(keysDown() & KEY_SELECT){switchScreens();}
}

void updateRoomEditor(void)
{
	touchRead(&currentTouch);
	
	if(!currentScreen)
	{
		updateLineOfTouch(currentTouch.px-128, currentTouch.py-96);
		updateEditorCamera();
		roomEditorCursor(NULL);
		updateSelection(NULL);
	}else{
		updateInterfaceButtons(oldTouch.px,oldTouch.py); //TEMP
	}

	roomEditorControls();
	
	oldTouch=currentTouch;
}

void drawRoomEditor(void)
{
	projectCamera(&editorCamera);
	glLoadIdentity();
	
	glPushMatrix();		
		glScalef32(editorScale,editorScale,editorScale);
		transformCamera(&editorCamera);
		// glTranslate3f32(editorTranslation.x,editorTranslation.y,editorTranslation.z);

		drawEditorRoom(&editorRoom);
		drawEntities();
		drawSelection(NULL);
	glPopMatrix(1);
	
	glFlush(0);
}

void freeRoomEditor(void)
{
	freeEditorRoom(&editorRoom);
	freeBlockFacePool();
}
