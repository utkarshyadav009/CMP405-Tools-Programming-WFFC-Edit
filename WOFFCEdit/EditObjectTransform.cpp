#include "EditObjectTransform.h"
#include "ToolMain.h"

EditObjectTransform::EditObjectTransform(ToolMain* toolMain)
{
	m_ToolMain = toolMain;
}

void EditObjectTransform::ChangePos(float deltaTime, Directions actionDirection, std::vector<unsigned int> objectID)
{
	for (size_t i = 0; i < objectID.size(); i++)
	{
		int currentObjectID = objectID[i];
		SceneObject* updateData = &m_ToolMain->m_sceneGraph[m_ToolMain->GetIndexFromID(currentObjectID)];

		switch (actionDirection)
		{
		case EditObjectTransform::PositiveX:
			updateData->posX += m_EditSpeed * deltaTime;
			break;
		case EditObjectTransform::NegativeX:
			updateData->posX -= m_EditSpeed * deltaTime;
			break;
		case EditObjectTransform::PositiveZ:
			updateData->posZ += m_EditSpeed * deltaTime;
			break;
		case EditObjectTransform::NegativeZ:
			updateData->posZ -= m_EditSpeed * deltaTime;
			break;
		case EditObjectTransform::PositiveY:
			updateData->posY += m_EditSpeed * deltaTime;
			break;
		case EditObjectTransform::NegativeY:
			updateData->posY -= m_EditSpeed * deltaTime;
			break;
		default:
			break;
		}
	}

	m_ToolMain->RebuildDisplayList();
}

void EditObjectTransform::ChangeRot(float deltaTime, Directions actionDirection, std::vector<unsigned int> objectID)
{
	for (size_t i = 0; i < objectID.size(); i++)
	{
		int currentObjectID = objectID[i];
		SceneObject* updateData = &m_ToolMain->m_sceneGraph[m_ToolMain->GetIndexFromID(currentObjectID)];

		switch (actionDirection)
		{
		case EditObjectTransform::PositiveX:
			updateData->rotX += m_EditRotationSpeed * deltaTime;
			break;
		case EditObjectTransform::NegativeX:
			updateData->rotX -= m_EditRotationSpeed * deltaTime;
			break;
		case EditObjectTransform::PositiveZ:
			updateData->rotZ += m_EditRotationSpeed * deltaTime;
			break;
		case EditObjectTransform::NegativeZ:
			updateData->rotZ -= m_EditRotationSpeed * deltaTime;
			break;
		case EditObjectTransform::PositiveY:
			updateData->rotY += m_EditRotationSpeed * deltaTime;
			break;
		case EditObjectTransform::NegativeY:
			updateData->rotY -= m_EditRotationSpeed * deltaTime;
			break;
		default:
			break;
		}
	}
	m_ToolMain->RebuildDisplayList();
}

void EditObjectTransform::ChangeScale(float deltaTime, Directions actionDirection, std::vector<unsigned int> objectID)
{
	for (size_t i = 0; i < objectID.size(); i++)
	{
		int currentObjectID = objectID[i];
		SceneObject* updateData = &m_ToolMain->m_sceneGraph[m_ToolMain->GetIndexFromID(currentObjectID)];

		switch (actionDirection)
		{
		case EditObjectTransform::PositiveX:
			updateData->scaX += m_EditSpeed * deltaTime;
			break;
		case EditObjectTransform::NegativeX:
			if (updateData->scaX > 0.01) {
				updateData->scaX -= m_EditSpeed * deltaTime;
				if (updateData->scaX < 0.01) {
					updateData->scaX = 0.01;
				}
			}
			break;
		case EditObjectTransform::PositiveZ:
			updateData->scaZ += m_EditSpeed * deltaTime;
			break;
		case EditObjectTransform::NegativeZ:

			if (updateData->scaZ > 0.01) {
				updateData->scaZ -= m_EditSpeed * deltaTime;
				if (updateData->scaZ < 0.01) {
					updateData->scaZ = 0.01;
				}
			}
			break;
		case EditObjectTransform::PositiveY:
			updateData->scaY += m_EditSpeed * deltaTime;
			break;
		case EditObjectTransform::NegativeY:

			if (updateData->scaY > 0.01) {
				updateData->scaY -= m_EditSpeed * deltaTime;
				if (updateData->scaY < 0.01) {
					updateData->scaY = 0.01;
				}
			}
			break;
		default:
			break;
		}
	}

	m_ToolMain->RebuildDisplayList();
}
