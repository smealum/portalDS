#include  "game/game_main.h"

SFX_struct SFX[NUMSFX];

void initSound()
{
	int i;
	for(i=0;i<NUMSFX;i++)
	{
		SFX[i].used=false;
	}

	soundEnable();
}

void initSFX(SFX_struct* s)
{
	if(!s)return;

	s->data=NULL;
	s->size=0;
	s->used=true;
}

void loadSFX(SFX_struct* s, char* filename, SoundFormat format)
{
	if(!s)return;

	initSFX(s);

	s->data=bufferizeFile(filename,"sfx",&s->size,true);
	s->format=format;
}

SFX_struct* createSFX(char* filename, SoundFormat format)
{
	int i;
	for(i=0;i<NUMSFX;i++)
	{
		if(!SFX[i].used)
		{
			loadSFX(&SFX[i],filename,format);

			if(!SFX[i].data)return NULL;
			return &SFX[i];
		}
	}
	return NULL;
}

void playSFX(SFX_struct* s)
{
	if(!s || !s->used || !s->data)return;

	soundPlaySample(s->data, s->format, s->size, 22050, 127, 64, false, 0);
}
