#ifndef __ROOMEDIT9__
#define __ROOMEDIT9__

#define NUMROOMEDITS 8
#define OUTLINEWIDTH (3<<6)
#define SELECTIONOUTLINEWIDTH (1<<7)

#define ROOMCREATION 0
#define ROOMSIZING 1
#define ROOMSELECTMOVE 2
#define ROOMMOVING 3
#define ROOMSELECTRESIZE 4
#define ROOMSELECTION 5
#define TILESELECTION 6
#define ENTITYSELECTION 7
#define LIGHTCREATION 8
#define ENTITYMOVING 9
#define ENEMYCREATION 10
#define DOORCREATION 11
#define DOORMOVING 12

typedef struct
{
	entityCollection_struct entityCollection;
	room_struct data;
	vect3D position;
	vect3D origin;
	vect3D size;
	bool selected;
	bool floor;
	bool quadsUpToDate;
	bool lightUpToDate;
	bool used;
	s16 id;
}roomEdit_struct;

static inline void drawTile(u16 x, u16 y, u16 color)
{
	GFX_COLOR=color;
	glPushMatrix();
		glTranslate3f32(x*(1<<9),y*(1<<9),16);
		glScalef32(1*(1<<9),1*(1<<9),inttof32(1));
		glBegin(GL_QUADS);		
			glVertex3v16(0, inttof32(1), 0);
			glVertex3v16(inttof32(1), inttof32(1), 0);
			glVertex3v16(inttof32(1), 0, 0);
			glVertex3v16(0, 0, 0);
	glPopMatrix(1);
}

void drawRoomEdits(void);
// void initRoomEditor(void);
void drawRoomsGame(u8 mode, u16 color);
void drawRoomsPreview(void);
void setRoomEditorMode(u8 mode);
// void updateRoomEditor(int px, int py);
void initRoomEdit(roomEdit_struct* re);
void drawRoomEdit(roomEdit_struct* re);
void deleteRoomEdit(roomEdit_struct* re);
void moveData(s8 v, roomEdit_struct* re, vect3D* so, vect3D* ss);
void applyMaterial(material_struct* mat, roomEdit_struct* re, vect3D* so, vect3D* ss);
void makeWall(roomEdit_struct* re, vect3D* so, vect3D* ss);
void swapData(roomEdit_struct* re);
rectangle_struct* addRoomRectangle(room_struct* r, entityCollection_struct* ec, rectangle_struct rec, material_struct* mat, bool portalable);
void generateLightmaps(roomEdit_struct* re, room_struct* r, entityCollection_struct* ec);
void optimizeRoom(roomEdit_struct* re);
void changeRoom(roomEdit_struct* re, bool both);
void wipeMapEdit(void);
void writeMap(char* filename);
void readMap(char* filename, bool game);
void getRoomDoorWays(void);
entityCollection_struct* getEntityCollection(room_struct* r);

#endif
