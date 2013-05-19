#include "game/game_main.h"

md2Model_struct elevatorModel;
md2Model_struct elevatorFrameModel;

void initElevators(void)
{
	loadMd2Model("models/elevator.md2", "cubedispenser.pcx", &elevatorModel);
	generateModelDisplayLists(&elevatorModel, false, true);

	loadMd2Model("models/elevatorframe.md2", "cubedispenser.pcx", &elevatorFrameModel);
	generateModelDisplayLists(&elevatorFrameModel, false, true);
}

void initElevator(elevator_struct* ev, room_struct* r, vect3D position, u8 direction, bool up)
{
	if(!ev)return;

	ev->position=convertVect(vect(position.x+r->position.x, position.y-4, position.z+r->position.y));;
	ev->direction=direction|(up<<ELEVATOR_UPDOWNBIT);
	ev->progress=0;
	initModelInstance(&ev->modelInstance, &elevatorModel);
}

void updateElevator(elevator_struct* ev)
{
	if(!ev)return;

	changeAnimation(&ev->modelInstance, 1, false);

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
		
		renderModelFrameInterp(ev->modelInstance.currentFrame,ev->modelInstance.nextFrame,ev->modelInstance.interpCounter,ev->modelInstance.model,params,false,ev->modelInstance.palette,RGB15(31,31,31));
	
	glPopMatrix(1);
}
