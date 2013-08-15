#include "menu/menu_main.h"

cameraState_struct cameraStates[]={{(vect3D){156,8000,13000}, (vect3D){0,0,0}},
									{(vect3D){-5184,7424,6272}, (vect3D){-2368,-4928,832}},
									{(vect3D){-14976,6144,-15424}, (vect3D){1664,-12288,576}},
									{(vect3D){-4992,4224,-10496}, (vect3D){-704,2048,-448}},
									{(vect3D){3776,6720,10944}, (vect3D){-3328,2944,384}},
								};

void applyCameraState(camera_struct* c, cameraState_struct* cs)
{
	if(!c || !cs)return;

	c->position=cs->position;
	c->viewPosition=cs->position;

	initTransformationMatrix(c);
	rotateMatrixX(c->transformationMatrix, cs->angle.x, false);
	rotateMatrixY(c->transformationMatrix, cs->angle.y, false);
	rotateMatrixZ(c->transformationMatrix, cs->angle.z, false);

	// NOGBA("p : %d %d %d",cs->position.x,cs->position.y,cs->position.z);
	// NOGBA("a : %d %d %d",cs->angle.x,cs->angle.y,cs->angle.z);
}

cameraTransition_struct startCameraTransition(cameraState_struct* s, cameraState_struct* f, int length)
{
	cameraTransition_struct ct;

	ct.start=s;
	ct.finish=f;

	ct.progress=0;
	ct.length=length;

	return ct;
}

void updateCameraTransition(camera_struct* c, cameraTransition_struct* ct)
{
	if(!c || !ct || !ct->start || !ct->finish || ct->progress>=ct->length)return;

	cameraState_struct cs;

	//TEMP TEST
	cs.position=vect(ct->start->position.x+((ct->finish->position.x-ct->start->position.x)*ct->progress)/ct->length,
				ct->start->position.y+((ct->finish->position.y-ct->start->position.y)*ct->progress)/ct->length,
				ct->start->position.z+((ct->finish->position.z-ct->start->position.z)*ct->progress)/ct->length);

	cs.angle=vect(ct->start->angle.x+((ct->finish->angle.x-ct->start->angle.x)*ct->progress)/ct->length,
				ct->start->angle.y+((ct->finish->angle.y-ct->start->angle.y)*ct->progress)/ct->length,
				ct->start->angle.z+((ct->finish->angle.z-ct->start->angle.z)*ct->progress)/ct->length);

	applyCameraState(c, &cs);

	ct->progress++;
}
