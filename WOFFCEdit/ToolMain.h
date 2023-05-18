#pragma once

#include <afxext.h>
#include "pch.h"
#include "Game.h"
#include "sqlite3.h"
#include "SceneObject.h"
#include "InputCommands.h"
#include "EditObjectTransform.h"
#include "CopyPasteSystem.h"
#include "UndoRedoActions.h"
#include <vector>


class ToolMain
{
public: //methods
	ToolMain();
	~ToolMain();

	//onAction - These are the interface to MFC
	std::vector<unsigned int> 	getCurrentSelectionID();										//returns the selection number of currently selected object so that It can be displayed.
	void	onActionInitialise(HWND handle, int width, int height);			//Passes through handle and hieght and width and initialises DirectX renderer and SQL LITE
	void	onActionFocusCamera();
	void	onActionLoad();													//load the current chunk
	
	void    onActionInsertNewObject(CString modelPath, CString texturePath, float positionX, float positionY, float positionZ,
		float rotationX, float rotationY, float rotationZ, float scaleX, float scaleY, float scaleZ);

	void    onActionPasteObjects(std::vector<SceneObject> m_CopyedObjects);

	void	onAcionUpdateObjectByID(std::vector<unsigned int> updateData, std::vector<SceneObject> newObjects);
	
	afx_msg	void	onActionSave();											//save the current chunk
	afx_msg void	onActionSaveTerrain();									//save chunk geometry

	void	Tick(MSG *msg);
	void	UpdateInput(MSG *msg);

	// Utilities
	int GetIndexFromID(unsigned int objectID);
	bool HasFocus();
	void RebuildDisplayList();

	// Camera Functions
	void UpdateCamSettings(float moveSpeed, float camRotationSpeed, float mouseSensitivity);
	inline float GetMoveSpeed() { return m_d3dRenderer.GetMoveSpeed(); }
	inline float GetRotationSpeed() { return m_d3dRenderer.GetRotationSpeed(); }
	inline float GetMouseSensitivity() { return m_d3dRenderer.GetMouseSensitivity(); }

public:	//variables
	std::vector<SceneObject>    m_sceneGraph;	//our scenegraph storing all the objects in the current chunk
	ChunkObject					m_chunk;		//our landscape chunk
	std::vector<unsigned int> m_selectedObjects;					//ID of current Selection
	EditObjectTransform m_EditObjectTransform{ this };
	CopyPasteSystem m_CopyPasteSystem{ this };
	UndoRedoActions m_UndoRedoSystem{ this };

	Game	m_d3dRenderer;		//Instance of D3D rendering system for our tool

private:	//methods
	void	onContentAdded();
	void StopEditingObjects();


		
private:	//variables
	HWND	m_toolHandle;		//Handle to the  window
	InputCommands m_toolInputCommands;		//input commands that we want to use and possibly pass over to the renderer
	CRect	WindowRECT;		//Window area rectangle. 
	char	m_keyArray[256];
	sqlite3 *m_databaseConnection;	//sqldatabase handle

	int m_width;		//dimensions passed to directX
	int m_height;
	int m_currentChunk;			//the current chunk of thedatabase that we are operating on.  Dictates loading and saving. 
	
	SceneObject m_OriginalObject;
	SceneObject m_ObjectUpdate;
	BOOL DetectChangeInObject();
	bool m_WasEditingObjectTransforms;
	int m_numEditingObjects;

	std::vector<SceneObject> m_UnalteredObjects;
	std::vector<SceneObject> m_UpdatedObjects;
	void AddObjectsToUnalteredObjects();
	void AddObjectsUpdatedObjects();
	bool m_NewEditingStarted;
	bool m_UpdatedObjectsCaptured;
	BOOL EditingTransformsStoped();
	
};
