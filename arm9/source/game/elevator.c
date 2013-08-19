#include "game/game_main.h"

#define ELEVATOR_SPEED (64)

md2Model_struct elevatorModel;
md2Model_struct elevatorFrameModel;

void initElevators(void)
{
	loadMd2Model("models/elevator.md2", "balllauncher.pcx", &elevatorModel);
	generateModelDisplayLists(&elevatorModel, false, true);

	loadMd2Model("models/elevatorframe.md2", "cubedispenser.pcx", &elevatorFrameModel);
	generateModelDisplayLists(&elevatorFrameModel, false, true);
}

void freeElevators(void)
{
	freeMd2Model(&elevatorModel);
	freeMd2Model(&elevatorFrameModel);
}

void initElevator(elevator_struct* ev, room_struct* r, vect3D position, u8 direction, bool up)
{
	if(!ev)return;

	ev->position=convertVect(vect(position.x+r->position.x, position.y-4, position.z+r->position.y));;
	ev->direction=direction|(up<<ELEVATOR_UPDOWNBIT);
	ev->progress=0;
	ev->state=ELEVATOR_OPEN;
	ev->doorSurface=NULL;
	initModelInstance(&ev->modelInstance, &elevatorModel);
				NOGBA("EV %d",up);
				NOGBA("EV %d",ev->direction);
}

void setElevatorArriving(elevator_struct* ev, int32 distance)
{
	if(!ev)return;

	ev->state=ELEVATOR_ARRIVING;
	ev->progress=distance;

	updateElevator(ev);
}

void closeElevator(elevator_struct* ev)
{
	if(!ev || ev->state!=ELEVATOR_OPEN)return;

	changeAnimation(&ev->modelInstance, 0, false);
	changeAnimation(&ev->modelInstance, 3, true);
	ev->state=ELEVATOR_CLOSING;
}

void updateElevator(elevator_struct* ev)
{
	if(!ev)return;

	switch(ev->state)
	{
		case ELEVATOR_ARRIVING:
			changeAnimation(&ev->modelInstance, 0, false);
			if(ev->progress<=0)
			{
				ev->progress=0;
				changeAnimation(&ev->modelInstance, 2, false);
				changeAnimation(&ev->modelInstance, 1, true);
				ev->state=ELEVATOR_OPENING;
			}else ev->progress-=ELEVATOR_SPEED;
			break;
		case ELEVATOR_OPENING:
			ev->progress=0;
			if(ev->modelInstance.currentAnim==2)ev->state=ELEVATOR_OPEN;
			break;
		case ELEVATOR_OPEN:
			ev->progress=0;
			break;
		case ELEVATOR_CLOSING:
			ev->progress=0;
			if(ev->modelInstance.currentAnim==0)ev->state=ELEVATOR_LEAVING;
			break;
		case ELEVATOR_LEAVING:
			changeAnimation(&ev->modelInstance, 0, false);
			ev->progress+=ELEVATOR_SPEED;
			break;
	}

	bool up=(ev->direction&(1<<ELEVATOR_UPDOWNBIT))!=0;
	ev->realPosition=ev->position;
	ev->realPosition.y+=up?(ev->progress):(-ev->progress);

	if(ev->doorSurface)ev->doorSurface->collides=ev->state!=ELEVATOR_OPEN;

	updateAnimation(&ev->modelInstance);
}

void drawElevator(elevator_struct* ev)
{
	if(!ev)return;

	glPushMatrix();
		u32 params=POLY_ALPHA(31)|POLY_CULL_FRONT|POLY_ID(30)|POLY_TOON_HIGHLIGHT;
		setupObjectLighting(NULL, ev->position, &params);

		glTranslate3f32(ev->position.x,ev->position.y,ev->position.z);
		
		switch(ev->direction&(~(1<<ELEVATOR_UPDOWNBIT)))
		{
			case 0:
				glRotateYi(8192);
				break;
			case 1:
				glRotateYi(-8192);
				break;
			case 5:
				glRotateYi(8192*2);
				break;
			default:
				break;
		}
		
		renderModelFrameInterp(0,0,0,&elevatorFrameModel,params,false,NULL,RGB15(31,31,31));

		bool up=(ev->direction&(1<<ELEVATOR_UPDOWNBIT))!=0;
		glTranslate3f32(0,up?(ev->progress):(-ev->progress),0);
		renderModelFrameInterp(ev->modelInstance.currentFrame,ev->modelInstance.nextFrame,ev->modelInstance.interpCounter,ev->modelInstance.model,params,false,ev->modelInstance.palette,RGB15(31,31,31));
	glPopMatrix(1);
}
