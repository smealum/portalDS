#ifndef __FILES9_H__
#define __FILES9_H__

#define ROOT "asds"

extern char* basePath;

extern bool saveAvailable;
extern u8 fsMode;

int chdir (const char *path);
bool fileExists(char* filename, char* dir);
bool initFilesystem(int argc, char **argv);
void* bufferizeFile(char* filename, char* dir, u32* size, bool binary);

#endif
