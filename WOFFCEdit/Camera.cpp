#include "Camera.h"

Camera::Camera()
{
    //cam floats
    m_MoveSpeed = 15.f;
    m_CamRotationSpeed = 200.f;
    m_MouseSensitivity = 2.f;

    //cam vectors
    m_CamPos.x = 0.f;
    m_CamPos.y = 3.7f;
    m_CamPos.z = -3.5f;

    m_CamOrientation.x = 0.f;
    m_CamOrientation.y = 0.f;
    m_CamOrientation.z = 0.f;

    m_CamLookAt.x = 0.f;
    m_CamLookAt.y = 0.f;
    m_CamLookAt.z = 0.f;

    m_CamLookDir.x = 0.f;
    m_CamLookDir.y = 0.f;
    m_CamLookDir.z = 0.f;

    m_CamRightVector.x = 0.f;
    m_CamRightVector.y = 0.f;
    m_CamRightVector.z = 0.f;

    HandleInput(NULL, InputCommands());
}

Camera::Camera(float moveSpeed, float camRotationSpeed, float mouseSensitivity, DirectX::SimpleMath::Vector3 camPos, DirectX::SimpleMath::Vector3 camOrientation)
{
    //cam floats
    m_MoveSpeed = moveSpeed;
    m_CamRotationSpeed = camRotationSpeed;
    m_MouseSensitivity = mouseSensitivity;

    //cam vectors
    m_CamPos = camPos;
    m_CamOrientation = camOrientation;

    m_CamLookAt.x = 0.f;
    m_CamLookAt.y = 0.f;
    m_CamLookAt.z = 0.f;

    m_CamLookDir.x = 0.f;
    m_CamLookDir.y = 0.f;
    m_CamLookDir.z = 0.f;

    m_CamRightVector.x = 0.f;
    m_CamRightVector.y = 0.f;
    m_CamRightVector.z = 0.f;

    HandleInput(NULL, InputCommands());
}

void Camera::HandleInput(float deltaTime, InputCommands input)
{
    // arrow key rotation controls 
    if (!input.mouseControllingCam) {

        // set cam rotation right
        if (input.rotRight)
        {
            m_CamOrientation.y += m_CamRotationSpeed * deltaTime;
            if (m_CamOrientation.y > 180.f)
                m_CamOrientation.y = -180.f;
        }// set cam rotation Left
        else if (input.rotLeft)
        {
            m_CamOrientation.y -= m_CamRotationSpeed * deltaTime;
            if (m_CamOrientation.y < -180.f)
                m_CamOrientation.y = 180.f;
        }

        // set cam pitch up
        if (input.pitchUp)
        {
            m_CamOrientation.x += m_CamRotationSpeed * deltaTime;
            if (m_CamOrientation.x > 89.f)
                m_CamOrientation.x = 89.f;
        }
        else if (input.pitchDown)
        {// set cam pitch down
            m_CamOrientation.x -= m_CamRotationSpeed * deltaTime;
            if (m_CamOrientation.x < -89.f)
                m_CamOrientation.x = -89.f;
        }

        // the above avoides overflow with wrap around and capping
    }
    else {
        //mouse rotation controls 
        int viewMiddleX = (m_ViewportDims.right + m_ViewportDims.left) / 2,
            viewMiddleY = (m_ViewportDims.bottom + m_ViewportDims.top) / 2;

        int middleToMouseX = input.mouseX - viewMiddleX,
            middleToMouseY = input.mouseY - viewMiddleY;

        float middleToMouseMagnitude = sqrtf(pow(middleToMouseX, 2) + pow(middleToMouseY, 2));

        // If we are not deviding by 0
        if (middleToMouseMagnitude) {
            float middleToMouseDirX = middleToMouseX / middleToMouseMagnitude,
                middleToMouseDirY = middleToMouseY / middleToMouseMagnitude;

            if (middleToMouseDirX) {
                // cam oriantation Y = yaw therefor use screen X
                m_CamOrientation.y += middleToMouseDirX * m_CamRotationSpeed * m_MouseSensitivity * deltaTime;
                if (m_CamOrientation.y > 180.f)
                    m_CamOrientation.y = -180.f;
                else if (m_CamOrientation.y < -180.f)
                    m_CamOrientation.y = 180.f;
            }

            if (middleToMouseDirY) {
                // cam oriantation X = pitch therefor use screen Y
                m_CamOrientation.x -= middleToMouseDirY * m_CamRotationSpeed * m_MouseSensitivity * deltaTime;
                if (m_CamOrientation.x > 89.f)
                    m_CamOrientation.x = 89.f;
                else if (m_CamOrientation.x < -89.f)
                    m_CamOrientation.x = -89.f;
            }

            //set cursor position to middle of viewport
            SetCursorPos(viewMiddleX, viewMiddleY);
        }
    }

    //create look direction from Euler angles in m_camOrientation
    m_CamLookDir.x = cos((m_CamOrientation.y) * 3.1415f / 180.0f) * cos((m_CamOrientation.x) * 3.1415f / 180.0f);
    m_CamLookDir.y = sin((m_CamOrientation.x) * 3.1415f / 180.0f);
    m_CamLookDir.z = sin((m_CamOrientation.y) * 3.1415f / 180.0f) * cos((m_CamOrientation.x) * 3.1415f / 180.0f);

    m_CamLookDir.Normalize();

    //create right vector from look Direction
    m_CamLookDir.Cross(DirectX::SimpleMath::Vector3::UnitY, m_CamRightVector);

    // flight control input
        // move camera forward 
    if (input.forward && input.slowMove)
        m_CamPos += m_CamLookDir * m_MoveSpeed * .5f * deltaTime;
    else if (input.forward)
        m_CamPos += m_CamLookDir * m_MoveSpeed * deltaTime;
    // move camera backward 
    if (input.back && input.slowMove)
        m_CamPos -= m_CamLookDir * m_MoveSpeed * .5f * deltaTime;
    else if (input.back)
        m_CamPos -= m_CamLookDir * m_MoveSpeed * deltaTime;
    // move camera right 
    if (input.right && input.slowMove)
        m_CamPos += m_CamRightVector * m_MoveSpeed * .5f * deltaTime;
    else if (input.right)
        m_CamPos += m_CamRightVector * m_MoveSpeed * deltaTime;
    // move camera left 
    if (input.left && input.slowMove)
        m_CamPos -= m_CamRightVector * m_MoveSpeed * .5f * deltaTime;
    else if (input.left)
        m_CamPos -= m_CamRightVector * m_MoveSpeed * deltaTime;
    // move camera up 
    if (input.moveUp && input.slowMove)
        m_CamPos.y += m_MoveSpeed * .5f * deltaTime;
    else if (input.moveUp)
        m_CamPos.y += m_MoveSpeed * deltaTime;
    // move camera down 
    if (input.moveDown && input.slowMove)
        m_CamPos.y -= m_MoveSpeed * .5f * deltaTime;
    else if (input.moveDown)
        m_CamPos.y -= m_MoveSpeed * deltaTime;

    //update lookat point
    m_CamLookAt = m_CamPos + m_CamLookDir;
}

DirectX::SimpleMath::Matrix Camera::GetLookAtMatrix()
{
    return DirectX::SimpleMath::Matrix::CreateLookAt(m_CamPos, m_CamLookAt, DirectX::SimpleMath::Vector3::UnitY);
}
