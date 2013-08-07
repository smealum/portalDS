#ifndef CAMERATRANSITION_H
#define CAMERATRANSITION_H

typedef struct
{
	vect3D position, angle;
}cameraState_struct;

typedef struct
{
	cameraState_struct *start, *finish;
	int progress, length;
}cameraTransition_struct;

extern cameraState_struct cameraStates[];

void applyCameraState(camera_struct* c, cameraState_struct* cs);

cameraTransition_struct startCameraTransition(cameraState_struct* s, cameraState_struct* f, int length);
void updateCameraTransition(camera_struct* c, cameraTransition_struct* ct);

#endif
