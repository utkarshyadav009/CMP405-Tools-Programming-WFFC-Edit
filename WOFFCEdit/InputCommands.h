#pragma once

struct InputCommands
{
	//camflight
	bool moveUp;
	bool moveDown;
	bool forward;
	bool back;
	bool left;
	bool right;
	bool slowMove;

	//cam rot
	bool rotLeft;
	bool rotRight;
	bool pitchUp;
	bool pitchDown;

	// cam mouse
	int mouseX, mouseY;
	int windowMouseX, windowMouseY;
	int mouseOriginX, mouseOriginY;

	bool mouseLeftDown, mouseRightDown, mouseControllingCam;

	// multi object selection 
	bool clearSelectedObjects;
	bool ctrlDown;

	bool deleteKeyDown;
	// undo redo
	bool undoDown;
	bool redoDown;
	//copy paste
	bool copyDown;
	bool pasteDown;

	// object transform editing
	bool fOneDown;
	bool fTwoDown;
	bool fThreeDown;

	bool rightDown;
	bool leftDown;
	bool upDown;
	bool downDown;

	bool shiftDown;
};
