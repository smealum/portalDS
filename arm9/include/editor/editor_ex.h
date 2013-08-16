#ifndef __EDITOREX9__
#define __EDITOREX9__

void initEditor(void);
void editorFrame(void);
void killEditor(void);
void editorVBL(void);

void setEditorMapFilePath(char* str);
char* getEditorMapFilePath(void);

#endif
