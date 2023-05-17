#pragma once
#include "SceneObject.h"
#include <vector>

class EditObjectTransform
{
public:

	float m_EditSpeed = 3.0;
	float m_EditRotationSpeed = 25.0;

	enum Directions
	{
		PositiveX, NegativeX, PositiveY, NegativeY, PositiveZ, NegativeZ
	};

	EditObjectTransform(class ToolMain* toolMain);

	void ChangePos(float deltaTime, Directions actionDirection, std::vector<unsigned int> objectID);
	void ChangeRot(float deltaTime, Directions actionDirection, std::vector<unsigned int> objectID);
	void ChangeScale(float deltaTime, Directions actionDirection, std::vector<unsigned int> objectID);


private:
	class ToolMain* m_ToolMain;
};

