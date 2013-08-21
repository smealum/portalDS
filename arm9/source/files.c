#include "common/general.h"
#include <errno.h>

bool saveAvailable;
char* basePath;
int lastSize;
u8 fsMode;

u32 getFileSize(FILE *file) {
    fseek(file, 0, SEEK_END);
    return ftell(file);
}

bool initFilesystem(int argc, char **argv)
{
	#ifndef FATONLY
		if(nitroFSInit(&basePath))
		{
			printf("init : done");
			chdir("nitro:/");
			chdir(ROOT);
			if(!argv){saveAvailable=false;fsMode=1;} // no fat but nitro (gba slot)
			else{
				fsMode=2; // nitro and fat
				saveAvailable=true;
				chdir(basePath);
				int r=mkdir("asds", S_IRWXU|S_IRGRP|S_IXGRP);//!ROOT!
				if(r!=0 && errno!=EEXIST)saveAvailable=false;
				r=mkdir("asds/maps", S_IRWXU|S_IRGRP|S_IXGRP);
				if(r!=0 && errno!=EEXIST)saveAvailable=false;
				r=mkdir("asds/screens", S_IRWXU|S_IRGRP|S_IXGRP);
				if(r!=0 && errno!=EEXIST)saveAvailable=false;
				NOGBA("can save : %d",saveAvailable);
				chdir("nitro:/");
				chdir(ROOT);
			}
			return true;
		}
	#endif
	fsMode=3; // fat only ?
	saveAvailable=false;
	if(!fatInitDefault())return false;
	saveAvailable=true;
	int r=mkdir("asds", S_IRWXU|S_IRGRP|S_IXGRP);//!ROOT!
	if(r!=0 && errno!=EEXIST)saveAvailable=false;
	r=mkdir("asds/maps", S_IRWXU|S_IRGRP|S_IXGRP);
	if(r!=0 && errno!=EEXIST)saveAvailable=false;
	r=mkdir("asds/screens", S_IRWXU|S_IRGRP|S_IXGRP);
	if(r!=0 && errno!=EEXIST)saveAvailable=false;
	// chdir("sd:/");
	basePath=malloc(255);
	getcwd(basePath,255);
	chdir(ROOT);
	return true;
}

void* bufferizeFile(char* filename, char* dir, u32* size, bool binary)
{
	char path[255];getcwd(path,255);
	/*int r=*/chdir(dir);
	//if(strlen(dir)<=0)r=-1;
	FILE* file;
	NOGBA("opening %s...",filename);
	
	if(!binary)file = fopen(filename, "r+");
	else file = fopen(filename, "rb+");
	
	NOGBA("done.");
	
	if(!file){chdir(path);return NULL;}
	
	u8* buffer;
	long lsize;
	fseek (file, 0 , SEEK_END);
	lsize = ftell (file);
	rewind (file);
	buffer=(u8*)malloc(lsize);
	lastSize=lsize;
	if(size)*size=lsize;
	
	if(!buffer){fclose(file);chdir(path);return NULL;}
		
	fread(buffer, 1, lsize, file);
	fclose(file);
	chdir(path);
	return buffer;
}

bool fileExists(char* filename, char* dir)
{
	char path[255];getcwd(path,255);
	NOGBA(path);
	chdir(dir);
	FILE* file;	
	file=fopen(filename, "r+");
	if(!file){chdir(path);return false;}
	chdir(path);
	fclose (file);
	return true;
}
