#ifndef SELECTION_H
#define SELECTION_H

typedef struct
{
	blockFace_struct *firstFace, *secondFace, *currentFace;
	entity_struct* entity;
	vect3D origin, size, currentPosition;
	bool active, selecting, selectingTarget, planar, error;
}selection_struct;

extern selection_struct editorSelection;
extern contextButton_struct targetSelectionButtonArray[];
extern contextButton_struct planarSelectionButtonArray[];
extern contextButton_struct groundSelectionButtonArray[];
extern contextButton_struct nonplanarSelectionButtonArray[];

void initSelection(selection_struct* s);
void drawSelection(selection_struct* s);
void updateSelection(selection_struct* s);
void undoSelection(selection_struct* s);

bool isFaceInSelection(blockFace_struct* bf, selection_struct* s);
void adjustSelection(editorRoom_struct* er, selection_struct* s, blockFace_struct of, blockFace_struct os, blockFace_struct oc, vect3D v);

#endif