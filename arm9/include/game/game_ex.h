#ifndef __GAMEEX9__
#define __GAMEEX9__

void initGame(void);
void gameFrame(void);
void killGame(void);
void gameVBL(void);

void setMapFilePath(char* path);
void setNextMapFilePath(char* path);
void endGame(void);

extern bool isNextRoom;

#endif
