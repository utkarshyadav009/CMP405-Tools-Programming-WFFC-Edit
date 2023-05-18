#include "UndoRedoActions.h"
#include "ToolMain.h"


UndoRedoActions::UndoRedoActions(ToolMain* toolMain)
{
	m_ToolMain = toolMain;
}

void UndoRedoActions::FlagForDeletion(std::vector<unsigned int> objectID)
{
	for (size_t i = 0; i < objectID.size(); i++)
	{
		m_IDsOfObjectsToDelete.push_back(objectID[i]);
		SceneObject* updateData = &m_ToolMain->m_sceneGraph[m_ToolMain->GetIndexFromID(objectID[i])];

		// set object to appear to be deleted 
		updateData->collision = false;
		updateData->render = false;
		updateData->editor_render = false;
	}

	AddPostAction(objectID);
}

void UndoRedoActions::RemoveIDsFromDeletionQueue(std::vector<unsigned int> objectID)
{
	for (size_t i = 0; i < objectID.size(); i++)
	{
		std::vector<int>::iterator pos = std::find(m_IDsOfObjectsToDelete.begin(), m_IDsOfObjectsToDelete.end(), objectID[i]);

		if (pos != m_IDsOfObjectsToDelete.end()) {
			m_IDsOfObjectsToDelete.erase(pos);
		}
	}
}

void UndoRedoActions::SetGameObjectData(ActionDirection direction)
{

	SceneObject updateData;
	std::vector<unsigned int> objectIDs;

	switch (direction)
	{
	case UndoRedoActions::Backward:

		for (size_t i = 0; i < m_PastUserActions[m_ActionStackPos].preChange.size(); i++)
		{

			updateData = m_PastUserActions[m_ActionStackPos].preChange[i];

			if (m_PastUserActions[m_ActionStackPos].typeOfAction == Action::Addition) {
				m_IDsOfObjectsToDelete.push_back(updateData.ID);
				SceneObject* objectToSet = &m_ToolMain->m_sceneGraph[m_ToolMain->GetIndexFromID(updateData.ID)];
				*objectToSet = updateData;
			}
			else if (m_PastUserActions[m_ActionStackPos].typeOfAction == Action::Deletion) {
				objectIDs.push_back(updateData.ID);
				SceneObject* objectToSet = &m_ToolMain->m_sceneGraph[m_ToolMain->GetIndexFromID(updateData.ID)];
				*objectToSet = updateData;
			}
			else if (m_PastUserActions[m_ActionStackPos].typeOfAction == Action::Default)
			{
				SceneObject* objectToSet = &m_ToolMain->m_sceneGraph[m_ToolMain->GetIndexFromID(updateData.ID)];
				*objectToSet = updateData;
			}
		}

		if (m_PastUserActions[m_ActionStackPos].typeOfAction == Action::Deletion) {
			RemoveIDsFromDeletionQueue(objectIDs);
		}

		break;
	case UndoRedoActions::Forward:
		for (size_t i = 0; i < m_PastUserActions[m_ActionStackPos].preChange.size(); i++)
		{
			updateData = m_PastUserActions[m_ActionStackPos].postChange[i];

			if (m_PastUserActions[m_ActionStackPos].typeOfAction == Action::Addition) {
				objectIDs.push_back(updateData.ID);
				SceneObject* objectToSet = &m_ToolMain->m_sceneGraph[m_ToolMain->GetIndexFromID(updateData.ID)];
				*objectToSet = updateData;
			}
			else if (m_PastUserActions[m_ActionStackPos].typeOfAction == Action::Deletion) {
				m_IDsOfObjectsToDelete.push_back(updateData.ID);
				SceneObject* objectToSet = &m_ToolMain->m_sceneGraph[m_ToolMain->GetIndexFromID(updateData.ID)];
				*objectToSet = updateData;
			}
			else if (m_PastUserActions[m_ActionStackPos].typeOfAction == Action::Default)
			{
				SceneObject* objectToSet = &m_ToolMain->m_sceneGraph[m_ToolMain->GetIndexFromID(updateData.ID)];
				*objectToSet = updateData;
			}
		}

		if (m_PastUserActions[m_ActionStackPos].typeOfAction == Action::Addition) {
			RemoveIDsFromDeletionQueue(objectIDs);
		}

		break;
	default:
		break;
	}


	m_ToolMain->RebuildDisplayList();
}


void UndoRedoActions::AddNewAction(std::vector<unsigned int> changedObjectIDs, Action::ActionTypes typeOfNewAction)
{
	// if the user is lower on the top of the action stack when they add a new action we remove all actions ahead of the new one
	while ((int)m_PastUserActions.size() - 1 > m_ActionStackPos) {
		m_PastUserActions.pop_back();
	}
	// limits the amout of actions that can be added to the stack to prevent potental memory problems
	if ((int)m_PastUserActions.size() >= 20) {
		m_PastUserActions.erase(m_PastUserActions.begin());
		m_ActionStackPos--;
	}

	Action newAction;
	newAction.actionCaptureComplete = false;
	newAction.typeOfAction = typeOfNewAction;
	for (size_t i = 0; i < changedObjectIDs.size(); i++)
	{
		SceneObject updateData = m_ToolMain->m_sceneGraph[m_ToolMain->GetIndexFromID(changedObjectIDs[i])];
		newAction.preChange.push_back(updateData);
	}

	m_PastUserActions.push_back(newAction);
	m_ActionStackPos++;

	switch (typeOfNewAction)
	{
	case Action::Deletion:
		// if the change type is deletion then we call FlagForDeletion
		FlagForDeletion(changedObjectIDs);
		break;
	case Action::Addition:
	{
		FlagForDeletion(changedObjectIDs);

		for (size_t i = 0; i < changedObjectIDs.size(); i++)
		{
			// When a new object is added to the scene we to create and undo redo instance where the object did not exist
			// to do this we flag the object for deletion 

			// after this the opjects post change will be the instance where it did not exist so this needs to be the pre change
			SceneObject tempPostChange = m_PastUserActions[m_ActionStackPos].postChange[i];
			// to do this we store the change in a temp veriable as seen above then switch the pre and post changes around
			m_PastUserActions[m_ActionStackPos].postChange[i] = (m_PastUserActions[m_ActionStackPos].preChange[i]);
			m_PastUserActions[m_ActionStackPos].preChange[i] = tempPostChange;
			//after this is done the current game object in the scene graph will have the incorrect settings as they are changed in 
			// flag for deletion so the last step is to set the data for the scenegraph object to be the same as the postchange data
			m_ToolMain->m_sceneGraph[m_ToolMain->GetIndexFromID(changedObjectIDs[i])] = m_PastUserActions[m_ActionStackPos].postChange[i];
		}
		// it then needs to be removed from the deletion list
		RemoveIDsFromDeletionQueue(changedObjectIDs);
		break;
	}
	default:
		break;
	}
}

void UndoRedoActions::AddPostAction(std::vector<unsigned int> objectID)
{
	if (m_PastUserActions[m_ActionStackPos].actionCaptureComplete == false) {

		for (size_t i = 0; i < objectID.size(); i++) {

			SceneObject updateData = m_ToolMain->m_sceneGraph[m_ToolMain->GetIndexFromID(objectID[i])];
			m_PastUserActions[m_ActionStackPos].postChange.push_back(updateData);
		}
		m_PastUserActions[m_ActionStackPos].actionCaptureComplete = true;
	}
}


void UndoRedoActions::Undo()
{
	if (m_ActionStackPos < 0) {
		return;
	}

	SetGameObjectData(ActionDirection::Backward);
	m_ActionStackPos--;
	m_ToolMain->RebuildDisplayList();
}

void UndoRedoActions::Redo()
{
	if (m_ActionStackPos >= (int)m_PastUserActions.size() - 1) {
		return;
	}

	if (m_PastUserActions.size() == 0) {
		return;
	}

	m_ActionStackPos++;
	SetGameObjectData(ActionDirection::Forward);
	m_ToolMain->RebuildDisplayList();
}

bool UndoRedoActions::IsObjectFlaggedForDeletion(int objectID)
{
	std::vector<int>::iterator pos = std::find(m_IDsOfObjectsToDelete.begin(), m_IDsOfObjectsToDelete.end(), objectID);

	if (pos != m_IDsOfObjectsToDelete.end())
		return true;

	return false;
}

void UndoRedoActions::DeleteAllFlaggedObjects()
{
	std::vector<unsigned int> objectIDs;

	for (size_t i = 0; i < m_ToolMain->m_sceneGraph.size(); i++)
	{
		int objectID = m_ToolMain->m_sceneGraph[i].ID;

		if (IsObjectFlaggedForDeletion(objectID)) {
			if (objectID >= 0) {

				m_ToolMain->m_sceneGraph.erase(m_ToolMain->m_sceneGraph.begin() + i);
				// remove from list
				objectIDs.push_back(objectID);
				// After deleting an object set the ittorator back one so that objects are not skipt and we dont risk overflow
				i--;
			}
		}
	}

	RemoveIDsFromDeletionQueue(objectIDs);

	m_ToolMain->RebuildDisplayList();

	// reset all undo/redo variables 
	m_ActionStackPos = -1;
	m_PastUserActions.clear();
	m_IDsOfObjectsToDelete.clear();
}

void UndoRedoActions::SignalToRebuildDisplayList()
{
	m_ToolMain->m_d3dRenderer.RebuildDisplayList();
}

void UndoRedoActions::PassSelectionToToolMain(std::vector<unsigned int> newSelctionIDs)
{
	m_ToolMain->m_d3dRenderer.AddChosenSelectionMenuIDs(newSelctionIDs);
}

void UndoRedoActions::ClearActionLists()
{
	m_IDsOfObjectsToDelete.clear();
	m_PastUserActions.clear();
	m_ActionStackPos = -1;
}
