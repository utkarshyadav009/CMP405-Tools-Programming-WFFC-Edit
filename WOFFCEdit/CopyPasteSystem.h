#pragma once
#include "SceneObject.h"
#include <vector>




class CopyPasteSystem
{
public:

	CopyPasteSystem(class ToolMain* toolMain);
	void CopySelectedObjects(std::vector<unsigned int> selectedObjectIDs);
	void PasteCopiedObjects();

private:
	ToolMain* m_ToolMain;
	std::vector<SceneObject> m_CopyedObjects;
};