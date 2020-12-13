#include "common/general.h"
#include <errno.h>

static void *mallocList[MAX_MALLOC];

void initMalloc()
{
	int i;

	for(i=0;i<MAX_MALLOC;i++)
	{
		mallocList[i]=NULL;
	}
}

void* alloc(size_t size, state_struct* s)
{
	int i;
	if(!s)s=getCurrentState();
	for(i=0;i<MAX_MALLOC;i++)
	{
		if(mallocList[i]==NULL)
		{
			mallocList[i]=malloc(size);
			if(mallocList[i]==NULL)NOGBA("MALLOC ERROR ! DUCK FOR COVER : %d, %s (%d,%d)",errno,strerror(errno),DS_UsedMem()/1024,DS_FreeMem()/1024);
			s->mc_id++;
			return mallocList[i];
		}
	}
	NOGBA("malloc error !");
	return NULL;
}

void* reAlloc(void* p, size_t size, state_struct* s)
{
	int i;
	if(!s)s=getCurrentState();
	for(i=0;i<MAX_MALLOC;i++)
	{
		if(mallocList[i]==p)
		{
			free(p);
			mallocList[i]=malloc(size);
			if(mallocList[i]==NULL)NOGBA("MALLOC ERROR ! DUCK FOR COVER : %d, %s (%d,%d)",errno,strerror(errno),DS_UsedMem()/1024,DS_FreeMem()/1024);
			return mallocList[i];
		}
	}
	NOGBA("reAlloc error !");
	return NULL;
}

void freeState(state_struct* s)
{
	if(!s)s=getCurrentState();
	if(s->mc_id>0)
	{
		int i;
		for(i=MAX_MALLOC-1;i>=0;i--)
		{
			if(mallocList[i]!=NULL)
			{
				free(mallocList[i]);
				mallocList[i]=NULL;
				//NOGBA("%d, %p\n",i,GetStackPointer());
			}
		}
		s->mc_id=0;
	}
}
