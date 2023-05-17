//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include "DisplayObject.h"
#include <string>


using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game()

{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
	m_displayList.clear();
	
	//initial Settings
	//modes
	m_grid = false;

    m_isEditingObjects = false;
}

Game::~Game()
{

#ifdef DXTK_AUDIO
    if (m_audEngine)
    {
        m_audEngine->Suspend();
    }
#endif
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_Window = window;

    m_gamePad = std::make_unique<GamePad>();

    m_keyboard = std::make_unique<Keyboard>();

    m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(window);

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    GetClientRect(m_Window, &m_screenDimensions);

    m_Camera = Camera(12.0f, 150.0f, 1.5f, Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, 0.f));

#ifdef DXTK_AUDIO
    // Create DirectXTK for Audio objects
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif

    m_audEngine = std::make_unique<AudioEngine>(eflags);

    m_audioEvent = 0;
    m_audioTimerAcc = 10.f;
    m_retryDefault = false;

    m_waveBank = std::make_unique<WaveBank>(m_audEngine.get(), L"adpcmdroid.xwb");

    m_soundEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"MusicMono_adpcm.wav");
    m_effect1 = m_soundEffect->CreateInstance();
    m_effect2 = m_waveBank->CreateInstance(10);

    m_effect1->Play(true);
    m_effect2->Play();
#endif
}

void Game::SetGridState(bool state)
{
	m_grid = state;
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick(InputCommands *Input)
{
	//copy over the input commands so we have a local version to use elsewhere.
	m_InputCommands = *Input;
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

#ifdef DXTK_AUDIO
    // Only update audio engine once per frame
    if (!m_audEngine->IsCriticalError() && m_audEngine->Update())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
#endif

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{

    m_DeltaTime = float(timer.GetElapsedSeconds());


    if (m_SelectedObjectIDs.size() == 0) {
        m_isEditingObjects = false;

    }

    if (m_InputCommands.clearSelectedObjects) {
        m_SelectedObjectIDs.clear();
        m_InputCommands.clearSelectedObjects = false;
    }


    // UpdateCamera
    m_Camera.UpdateViewport(m_screenDimensions);
    m_Camera.HandleInput(m_DeltaTime, m_InputCommands);
    m_view = m_Camera.GetLookAtMatrix();

    m_batchEffect->SetView(m_view);
    m_batchEffect->SetWorld(Matrix::Identity);
	m_displayChunk.m_terrainEffect->SetView(m_view);
	m_displayChunk.m_terrainEffect->SetWorld(Matrix::Identity);

    if (m_InputCommands.mouseLeftDown) {
        if (GetFocus() == GetParent(m_Window)) {
            bool mouseWithinViewport = (m_InputCommands.mouseX > m_screenDimensions.left && m_InputCommands.mouseX < m_screenDimensions.right)
                && (m_InputCommands.mouseY > m_screenDimensions.top && m_InputCommands.mouseY < m_screenDimensions.bottom);

            if (mouseWithinViewport) {

                if (m_InputCommands.ctrlDown == false)
                    m_SelectedObjectIDs.clear();

                unsigned int hitID = ObjectSelection();
                if (std::find(m_SelectedObjectIDs.begin(), m_SelectedObjectIDs.end(), hitID) == m_SelectedObjectIDs.end() && hitID != -1) {

                    m_SelectedObjectIDs.push_back(hitID);
                }
            }
        }
    }

#ifdef DXTK_AUDIO
    m_audioTimerAcc -= (float)timer.GetElapsedSeconds();
    if (m_audioTimerAcc < 0)
    {
        if (m_retryDefault)
        {
            m_retryDefault = false;
            if (m_audEngine->Reset())
            {
                // Restart looping audio
                m_effect1->Play(true);
            }
        }
        else
        {
            m_audioTimerAcc = 4.f;

            m_waveBank->Play(m_audioEvent++);

            if (m_audioEvent >= 11)
                m_audioEvent = 0;
        }
    }
#endif

   
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

	if (m_grid)
	{
		// Draw procedurally generated dynamic grid
		const XMVECTORF32 xaxis = { 512.f, 0.f, 0.f };
		const XMVECTORF32 yaxis = { 0.f, 0.f, 512.f };
		DrawGrid(xaxis, yaxis, g_XMZero, 512, 512, Colors::Gray);
	}


	//RENDER OBJECTS FROM SCENEGRAPH
	int numRenderObjects = m_displayList.size();
	for (int i = 0; i < numRenderObjects; i++)
	{
		m_deviceResources->PIXBeginEvent(L"Draw model");
		const XMVECTORF32 scale = { m_displayList[i].m_scale.x, m_displayList[i].m_scale.y, m_displayList[i].m_scale.z };
		const XMVECTORF32 translate = { m_displayList[i].m_position.x, m_displayList[i].m_position.y, m_displayList[i].m_position.z };

		//convert degrees into radians for rotation matrix
		XMVECTOR rotate = Quaternion::CreateFromYawPitchRoll(m_displayList[i].m_orientation.y *3.1415 / 180,
															m_displayList[i].m_orientation.x *3.1415 / 180,
															m_displayList[i].m_orientation.z *3.1415 / 180);

		XMMATRIX local = m_world * XMMatrixTransformation(g_XMZero, Quaternion::Identity, scale, g_XMZero, rotate, translate);

		m_displayList[i].m_model->Draw(context, *m_states, local, m_view, m_projection, false);	//last variable in draw,  make TRUE for wireframe

		m_deviceResources->PIXEndEvent();
	}
    m_deviceResources->PIXEndEvent();

	//RENDER TERRAIN
	context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthDefault(),0);
	context->RSSetState(m_states->CullNone());
//	context->RSSetState(m_states->Wireframe());		//uncomment for wireframe

	//Render the batch,  This is handled in the Display chunk becuase it has the potential to get complex
	m_displayChunk.RenderBatch(m_deviceResources);

    //CAMERA POSITION ON HUD
    m_sprites->Begin();
    // WCHAR   Buffer[256];

    Vector3 camPos = m_Camera.GetCamPos(),
        camOrientation = m_Camera.GetCamOrientation();

    std::wstring varFPS = L"FPS: " + std::to_wstring(m_timer.GetFramesPerSecond());
    std::wstring varPos = L"Cam X: " + std::to_wstring(camPos.x) + L" Cam Y: " + std::to_wstring(camPos.y) + L" Cam Z: " + std::to_wstring(camPos.z);
    std::wstring varRot = L"Cam pitch: " + std::to_wstring(camOrientation.x) + L" Cam yaw: " + std::to_wstring(camOrientation.y) + L" Cam roll: " + std::to_wstring(camOrientation.z);

    m_font->DrawString(m_sprites.get(), varFPS.c_str(), XMFLOAT2((m_screenDimensions.right - m_screenDimensions.left) - 90, 10), Colors::Red, 0.0f, XMFLOAT2(0.0f, 0.0f), 0.80f);
    m_font->DrawString(m_sprites.get(), varPos.c_str(), XMFLOAT2(10, 8), Colors::DarkRed, 0.0f, XMFLOAT2(0.0f, 0.0f), 0.80f);
    m_font->DrawString(m_sprites.get(), varRot.c_str(), XMFLOAT2(10, 28), Colors::DarkRed, 0.0f, XMFLOAT2(0.0f, 0.0f), 0.80f);
    m_sprites->End();

    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetBackBufferRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}

void XM_CALLCONV Game::DrawGrid(FXMVECTOR xAxis, FXMVECTOR yAxis, FXMVECTOR origin, size_t xdivs, size_t ydivs, GXMVECTOR color)
{
    m_deviceResources->PIXBeginEvent(L"Draw grid");

    auto context = m_deviceResources->GetD3DDeviceContext();
    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthNone(), 0);
    context->RSSetState(m_states->CullCounterClockwise());

    m_batchEffect->Apply(context);

    context->IASetInputLayout(m_batchInputLayout.Get());

    m_batch->Begin();

    xdivs = std::max<size_t>(1, xdivs);
    ydivs = std::max<size_t>(1, ydivs);

    for (size_t i = 0; i <= xdivs; ++i)
    {
        float fPercent = float(i) / float(xdivs);
        fPercent = (fPercent * 2.0f) - 1.0f;
        XMVECTOR vScale = XMVectorScale(xAxis, fPercent);
        vScale = XMVectorAdd(vScale, origin);

        VertexPositionColor v1(XMVectorSubtract(vScale, yAxis), color);
        VertexPositionColor v2(XMVectorAdd(vScale, yAxis), color);
        m_batch->DrawLine(v1, v2);
    }

    for (size_t i = 0; i <= ydivs; i++)
    {
        float fPercent = float(i) / float(ydivs);
        fPercent = (fPercent * 2.0f) - 1.0f;
        XMVECTOR vScale = XMVectorScale(yAxis, fPercent);
        vScale = XMVectorAdd(vScale, origin);

        VertexPositionColor v1(XMVectorSubtract(vScale, xAxis), color);
        VertexPositionColor v2(XMVectorAdd(vScale, xAxis), color);
        m_batch->DrawLine(v1, v2);
    }

    m_batch->End();

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
#ifdef DXTK_AUDIO
    m_audEngine->Suspend();
#endif
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

#ifdef DXTK_AUDIO
    m_audEngine->Resume();
#endif
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

void Game::BuildDisplayList(std::vector<SceneObject> * SceneGraph)
{
	auto device = m_deviceResources->GetD3DDevice();
	auto devicecontext = m_deviceResources->GetD3DDeviceContext();

	if (!m_displayList.empty())		//is the vector empty
	{
		m_displayList.clear();		//if not, empty it
	}

	//for every item in the scenegraph
	int numObjects = SceneGraph->size();
    for (int i = 0; i < numObjects; i++)
    {
        if (!SceneGraph->at(i).editor_render)
            continue;
        //create a temp display object that we will populate then append to the display list.
        DisplayObject newDisplayObject;

        //load model
        std::wstring modelwstr = StringToWCHART(SceneGraph->at(i).model_path);							//convect string to Wchar
        newDisplayObject.m_model = Model::CreateFromCMO(device, modelwstr.c_str(), *m_fxFactory, true);	//get DXSDK to load model "False" for LH coordinate system (maya)

        //Load Texture
        std::wstring texturewstr = StringToWCHART(SceneGraph->at(i).tex_diffuse_path);								//convect string to Wchar
        HRESULT rs;
        rs = CreateDDSTextureFromFile(device, texturewstr.c_str(), nullptr, &newDisplayObject.m_texture_diffuse);	//load tex into Shader resource

        //if texture fails.  load error default
        if (rs)
        {
            CreateDDSTextureFromFile(device, L"database/data/Error.dds", nullptr, &newDisplayObject.m_texture_diffuse);	//load tex into Shader resource
        }

        //apply new texture to models effect
        newDisplayObject.m_model->UpdateEffects([&](IEffect* effect) //This uses a Lambda function,  if you dont understand it: Look it up.
            {
                auto lights = dynamic_cast<BasicEffect*>(effect);
                if (lights)
                {
                    lights->SetTexture(newDisplayObject.m_texture_diffuse);
                }
            });

        // USE ID FOR SELECTION, INDEX IS USELESS
        newDisplayObject.m_ID = SceneGraph->at(i).ID;

        //set position
        newDisplayObject.m_position.x = SceneGraph->at(i).posX;
        newDisplayObject.m_position.y = SceneGraph->at(i).posY;
        newDisplayObject.m_position.z = SceneGraph->at(i).posZ;

        //setorientation
        newDisplayObject.m_orientation.x = SceneGraph->at(i).rotX;
        newDisplayObject.m_orientation.y = SceneGraph->at(i).rotY;
        newDisplayObject.m_orientation.z = SceneGraph->at(i).rotZ;

        //set scale
        newDisplayObject.m_scale.x = SceneGraph->at(i).scaX;
        newDisplayObject.m_scale.y = SceneGraph->at(i).scaY;
        newDisplayObject.m_scale.z = SceneGraph->at(i).scaZ;

        //set wireframe / render flags
        newDisplayObject.m_render = SceneGraph->at(i).editor_render;
        newDisplayObject.m_wireframe = SceneGraph->at(i).editor_wireframe;

        newDisplayObject.m_light_type = SceneGraph->at(i).light_type;
        newDisplayObject.m_light_diffuse_r = SceneGraph->at(i).light_diffuse_r;
        newDisplayObject.m_light_diffuse_g = SceneGraph->at(i).light_diffuse_g;
        newDisplayObject.m_light_diffuse_b = SceneGraph->at(i).light_diffuse_b;
        newDisplayObject.m_light_specular_r = SceneGraph->at(i).light_specular_r;
        newDisplayObject.m_light_specular_g = SceneGraph->at(i).light_specular_g;
        newDisplayObject.m_light_specular_b = SceneGraph->at(i).light_specular_b;
        newDisplayObject.m_light_spot_cutoff = SceneGraph->at(i).light_spot_cutoff;
        newDisplayObject.m_light_constant = SceneGraph->at(i).light_constant;
        newDisplayObject.m_light_linear = SceneGraph->at(i).light_linear;
        newDisplayObject.m_light_quadratic = SceneGraph->at(i).light_quadratic;

        m_displayList.push_back(newDisplayObject);

        if (std::find(m_SelectedObjectIDs.begin(), m_SelectedObjectIDs.end(), SceneGraph->at(i).ID) != m_SelectedObjectIDs.end())
        {
            DisplayObject objectHighlight = newDisplayObject;

            objectHighlight.m_ID = -1;
            objectHighlight.m_wireframe = true;

            objectHighlight.m_model->UpdateEffects([&](IEffect* effect)
                {
                    auto fog = dynamic_cast<IEffectFog*>(effect);

                    if (fog) {
                        fog->SetFogEnabled(true);
                        fog->SetFogStart(0);
                        fog->SetFogEnd(0); // 0,0 for one object distance irrelevant as incressed math will slow down the program;

                        if (!m_isEditingObjects) {
                            fog->SetFogColor(Colors::Goldenrod);
                        }
                        else {

                            //if (m_isEditingPos) {
                            //    fog->SetFogColor(Colors::DarkGreen);
                            //}
                            //if (m_isEditingRot) {
                            //    fog->SetFogColor(Colors::DarkRed);
                            //}
                            //if (m_isEditingScale) {
                            //    fog->SetFogColor(Colors::DarkBlue);
                            //}
                        }
                    }
                });

            m_displayList.push_back(objectHighlight);

        }
    }
    m_RebuildDisplayList = false;
		
}

void Game::BuildDisplayChunk(ChunkObject * SceneChunk)
{
	//populate our local DISPLAYCHUNK with all the chunk info we need from the object stored in toolmain
	//which, to be honest, is almost all of it. Its mostly rendering related info so...
	m_displayChunk.PopulateChunkData(SceneChunk);		//migrate chunk data
	m_displayChunk.LoadHeightMap(m_deviceResources);
	m_displayChunk.m_terrainEffect->SetProjection(m_projection);
	m_displayChunk.InitialiseBatch();
}

void Game::SaveDisplayChunk(ChunkObject * SceneChunk)
{
	m_displayChunk.SaveHeightMap();			//save heightmap to file.
}

int Game::ObjectSelection()
{
    RECT screenDims;
    GetClientRect(m_Window, &screenDims);

    int selection = -1;
    float distinceFromCam = INFINITY, shortestDistance = INFINITY;

    //  near and far planes of THE frustrum w / mouse
    const XMVECTOR nearPlane = XMVectorSet(m_InputCommands.windowMouseX, m_InputCommands.windowMouseY, 0.0f, 1.0f);
    const XMVECTOR farPlane = XMVectorSet(m_InputCommands.windowMouseX, m_InputCommands.windowMouseY, 1.0f, 1.0f);

    for (size_t i = 0; i < m_displayList.size(); i++)
    {
        // get scale / translation 
        const XMVECTORF32 scale = { m_displayList[i].m_scale.x, m_displayList[i].m_scale.y, m_displayList[i].m_scale.z };
        const XMVECTORF32 pos = { m_displayList[i].m_position.x, m_displayList[i].m_position.y, m_displayList[i].m_position.z };

        // convert rotation to quaternion for rotation - in radians
        XMVECTOR rot = Quaternion::CreateFromYawPitchRoll(
            m_displayList[i].m_orientation.y * 3.1415f / 180.0f,
            m_displayList[i].m_orientation.x * 3.1415f / 180.0f,
            m_displayList[i].m_orientation.z * 3.1415f / 180.0f);

        // local transfromation matrix
        XMMATRIX localTransfrom = m_world * XMMatrixTransformation(
            g_XMZero, Quaternion::Identity, scale,
            g_XMZero, rot,
            pos);

        //de projecting the rays vectors / converting from screen space to world space
        XMVECTOR nearPoint = XMVector3Unproject(
            nearPlane,
            0.0f, 0.0f,
            screenDims.right, screenDims.bottom,
            m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth,
            m_projection, m_view,
            localTransfrom);


        XMVECTOR farPoint = XMVector3Unproject(
            farPlane,
            0.0f, 0.0f,
            screenDims.right, screenDims.bottom,
            m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth,
            m_projection, m_view,
            localTransfrom);

        // create ray trace from nearpoint to farpoint
        XMVECTOR ray = farPoint - nearPoint;
        ray = XMVector3Normalize(ray);

        for (size_t num = 0; num < m_displayList[i].m_model.get()->meshes.size(); num++)
        {
            if (m_displayList[i].m_model.get()->meshes[num]->boundingBox.Intersects(nearPoint, ray, distinceFromCam))
                if (distinceFromCam < shortestDistance) {
                    selection = m_displayList[i].m_ID;
                    shortestDistance = distinceFromCam;
                }
        }
    }

    RebuildDisplayList();
    return selection;
}

void Game::AddChosenSelectionMenuIDs(std::vector<unsigned int> newSelectedObject)
{
    m_SelectedObjectIDs = newSelectedObject;
}

void Game::SetCameraValues(float moveSpeed, float camRotationSpeed, float mouseSensitivity)
{
	m_Camera.SetMoveSpeed(moveSpeed);
	m_Camera.SetRotationSpeed(camRotationSpeed);
	m_Camera.SetMouseSensitivity(mouseSensitivity);
}

#ifdef DXTK_AUDIO
void Game::NewAudioDevice()
{
    if (m_audEngine && !m_audEngine->IsAudioDevicePresent())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
}
#endif


#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto device = m_deviceResources->GetD3DDevice();

    m_states = std::make_unique<CommonStates>(device);

    m_fxFactory = std::make_unique<EffectFactory>(device);
	m_fxFactory->SetDirectory(L"database/data/"); //fx Factory will look in the database directory
	m_fxFactory->SetSharing(false);	//we must set this to false otherwise it will share effects based on the initial tex loaded (When the model loads) rather than what we will change them to.

    m_sprites = std::make_unique<SpriteBatch>(context);

    m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

    m_batchEffect = std::make_unique<BasicEffect>(device);
    m_batchEffect->SetVertexColorEnabled(true);

    {
        void const* shaderByteCode;
        size_t byteCodeLength;

        m_batchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

        DX::ThrowIfFailed(
            device->CreateInputLayout(VertexPositionColor::InputElements,
                VertexPositionColor::InputElementCount,
                shaderByteCode, byteCodeLength,
                m_batchInputLayout.ReleaseAndGetAddressOf())
        );
    }

    m_font = std::make_unique<SpriteFont>(device, L"SegoeUI_18.spritefont");

//    m_shape = GeometricPrimitive::CreateTeapot(context, 4.f, 8);

    // SDKMESH has to use clockwise winding with right-handed coordinates, so textures are flipped in U
    m_model = Model::CreateFromSDKMESH(device, L"tiny.sdkmesh", *m_fxFactory);
	

    // Load textures
    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(device, L"seafloor.dds", nullptr, m_texture1.ReleaseAndGetAddressOf())
    );

    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(device, L"windowslogo.dds", nullptr, m_texture2.ReleaseAndGetAddressOf())
    );

}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();
    float aspectRatio = float(size.right) / float(size.bottom);
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    // This is a simple example of change that can be made when the app is in
    // portrait or snapped view.
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    // This sample makes use of a right-handed coordinate system using row-major matrices.
    m_projection = Matrix::CreatePerspectiveFieldOfView(
        fovAngleY,
        aspectRatio,
        0.01f,
        1000.0f
    );

    m_batchEffect->SetProjection(m_projection);
	
}

void Game::OnDeviceLost()
{
    m_states.reset();
    m_fxFactory.reset();
    m_sprites.reset();
    m_batch.reset();
    m_batchEffect.reset();
    m_font.reset();
    m_shape.reset();
    m_model.reset();
    m_texture1.Reset();
    m_texture2.Reset();
    m_batchInputLayout.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion

std::wstring StringToWCHART(std::string s)
{

	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}
