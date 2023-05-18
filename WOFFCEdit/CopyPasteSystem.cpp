#include "CopyPasteSystem.h"
#include "ToolMain.h"


CopyPasteSystem::CopyPasteSystem(ToolMain* toolMain)
{
	m_ToolMain = toolMain;
}

void CopyPasteSystem::CopySelectedObjects(std::vector<unsigned int> selectedObjectIDs)
{
	SceneObject gameObjectFromSelection;
	std::vector<SceneObject> selectedObjects;
	SceneObject newCopyedObject;

	for (size_t i = 0; i < selectedObjectIDs.size(); i++)
	{
		// get the game object from its idea
		newCopyedObject = m_ToolMain->m_sceneGraph[m_ToolMain->GetIndexFromID(selectedObjectIDs[i])];

		// add newCopyedObject to the selectedObjects vector
		selectedObjects.push_back(newCopyedObject);
	}

	if (selectedObjects.size() != 0) {
		m_CopyedObjects.clear();
		m_CopyedObjects = selectedObjects;
	}
}

void CopyPasteSystem::PasteCopiedObjects()
{
	for (size_t i = 0; i < m_CopyedObjects.size(); i++)
	{
		// move each of the coppied objects so that if pasted the object dose not just pile ontop of the origoanl object and thus will not be seen
		m_CopyedObjects[i].posX += 2;
		m_CopyedObjects[i].posY += 2;
	}

	// create a new objects using the copyed data
	m_ToolMain->onActionPasteObjects(m_CopyedObjects);
}
