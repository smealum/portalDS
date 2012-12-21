#ifndef __ENEMY9__
#define __ENEMY9__

#define ANIMIDLE 0
#define ANIMWALK 1
#define ANIMRUN 3
#define ANIMATTACK 5
#define ANIMDEATH 7
#define ANIMPAIN 8

#define NUMENEMIES 256

#define ENEMYWALKINGSPEED 32
#define ENEMYRUNNINGSPEED 8

#define WALKTO 1
#define RUNTO 2

typedef struct
{
	vect3D tilePosition;
	vect3D targetPosition;
	vect3D nextTilePosition;
	physicsObject_struct object;
	entityCollection_struct* ec;
	modelInstance_struct modelInstance;
	path_struct path;
	room_struct* r;
	u16 progress;
	s16 angle;
	u16 task;
	s16 life;
	bool used, detected, pathed;
}enemy_struct;

void initEnemies(void);
void createEn(room_struct* r, vect3D p);
void drawEnemy(enemy_struct* e);
void drawEnemies(void);
void freeEnemies(void);
void updateEnemies(void);
void moveRoomEnemiesTo(room_struct* r, int x, int y, u8 task);
bool collideLineEnemy(enemy_struct* e, vect3D l, vect3D u, int32 d, int32* k, vect3D* ip);
enemy_struct* collideLineRoomEnemies(room_struct* r, vect3D l, vect3D u, int32 d, int32* k, vect3D* ip);

void getClosestLights(entityCollection_struct* ec, enemy_struct* e, entity_struct** ll1, entity_struct** ll2, entity_struct** ll3, int32* dd1, int32* dd2, int32* dd3);

#endif
