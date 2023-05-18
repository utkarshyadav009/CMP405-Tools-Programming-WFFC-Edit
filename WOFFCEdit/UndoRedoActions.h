#pragma once
#include "SceneObject.h"
#include <vector>
struct Action
{
	enum ActionTypes
	{
		Default, Deletion, Addition
	};

	ActionTypes typeOfAction = ActionTypes::Default;
	std::vector<SceneObject> preChange;
	std::vector<SceneObject> postChange;

	bool actionCaptureComplete;
};

class UndoRedoActions
{
public:

	UndoRedoActions(class ToolMain* toolMain);

	void AddNewAction(std::vector<unsigned int> objectID, Action::ActionTypes typeOfAction);
	void AddPostAction(std::vector<unsigned int> objectID);
	void Undo();
	void Redo();
	bool IsObjectFlaggedForDeletion(int objectID);
	void DeleteAllFlaggedObjects();

	void SignalToRebuildDisplayList();
	void PassSelectionToToolMain(std::vector<unsigned int> newSelctionIDs);
	void ClearActionLists();
private:

	enum ActionDirection
	{
		Default, Backward, Forward
	};

	class ToolMain* m_ToolMain;
	std::vector<int> m_IDsOfObjectsToDelete;
	std::vector<Action> m_PastUserActions;
	int m_ActionStackPos = -1;

	void FlagForDeletion(std::vector<unsigned int> objectID);
	void RemoveIDsFromDeletionQueue(std::vector<unsigned int> objectID);
	void SetGameObjectData(ActionDirection direction);
};