#pragma once

#include "pch.h"
#include "InputCommands.h"

class Camera
{
private:
	RECT m_ViewportDims;

	// camera settings
	float m_MoveSpeed,
		m_CamRotationSpeed,
		m_MouseSensitivity;

	// Cam Info vectors 
	DirectX::SimpleMath::Vector3 m_CamPos,
		m_CamOrientation,
		m_CamLookAt,
		m_CamLookDir,
		m_CamRightVector;

public:
	Camera();
	Camera(float moveSpeed, float camRotationSpeed, float mouseSensitivity, DirectX::SimpleMath::Vector3 camPos, DirectX::SimpleMath::Vector3 camOrientation);

	void HandleInput(float deltaTime, InputCommands input);
	DirectX::SimpleMath::Matrix GetLookAtMatrix();

	inline void UpdateViewport(RECT viewportDims) { m_ViewportDims = viewportDims; } // Inline is faster and compiled first used insted of #define good for small fuctions 

	inline void SetMoveSpeed(float newSpeed) { m_MoveSpeed = newSpeed; }
	inline float GetMoveSpeed() { return m_MoveSpeed; }

	inline void SetRotationSpeed(float newRotationSpeed) { m_CamRotationSpeed = newRotationSpeed; }
	inline float  GetRotationSpeed() { return m_CamRotationSpeed; }

	inline void SetMouseSensitivity(float newSensitivity) { m_MouseSensitivity = newSensitivity; }
	inline float  GetMouseSensitivity() { return m_MouseSensitivity; }

	inline DirectX::SimpleMath::Vector3  GetCamPos() { return m_CamPos; }
	inline DirectX::SimpleMath::Vector3  GetCamOrientation() { return m_CamOrientation; }
};

