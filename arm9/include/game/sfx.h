#ifndef SFX_H
#define SFX_H

#define NUMSFX (32)

typedef struct
{
	u8* data;
	u32 size;
	SoundFormat format;
	bool used;
}SFX_struct;

void initSound(void);
void freeSound(void);
void initSFX(SFX_struct* s);
void loadSFX(SFX_struct* s, char* filename, SoundFormat format);
SFX_struct* createSFX(char* filename, SoundFormat format);
void playSFX(SFX_struct* s);

#endif
