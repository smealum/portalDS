#ifndef ROOMEDITOR_H
#define ROOMEDITOR_H

extern editorRoom_struct editorRoom;

void initRoomEdition(void);
void drawRoomEditor(void);
void freeRoomEditor(void);
void updateRoomEditor(void);

void switchScreens(void);

#endif