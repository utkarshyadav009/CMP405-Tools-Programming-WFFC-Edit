#include "ToolMain.h"
#include "resource.h"
#include <vector>
#include <sstream>

//
//ToolMain Class
ToolMain::ToolMain()
{

	m_currentChunk = 0;		//default value
	m_sceneGraph.clear();	//clear the vector for the scenegraph
	m_databaseConnection = NULL;

	//zero input commands
	m_toolInputCommands.forward =
		m_toolInputCommands.back =
		m_toolInputCommands.left =
		m_toolInputCommands.right =
		m_toolInputCommands.moveUp =
		m_toolInputCommands.moveDown = false;

	m_toolInputCommands.rotRight =
		m_toolInputCommands.rotLeft =
		m_toolInputCommands.pitchUp =
		m_toolInputCommands.pitchDown = false;

	m_toolInputCommands.mouseX = m_toolInputCommands.mouseY = 0;
	m_toolInputCommands.windowMouseX = m_toolInputCommands.windowMouseY = 0;
	m_toolInputCommands.mouseOriginX = m_toolInputCommands.mouseOriginY = 0;
	m_toolInputCommands.mouseLeftDown = m_toolInputCommands.mouseRightDown = false;
	m_toolInputCommands.ctrlDown = false;

	m_d3dRenderer.m_EditObjectTransform = &m_EditObjectTransform;
}


ToolMain::~ToolMain()
{
	sqlite3_close(m_databaseConnection);		//close the database connection
}


std::vector<unsigned int> ToolMain::getCurrentSelectionID()
{

	return m_selectedObjects;
}

void ToolMain::onActionInitialise(HWND handle, int width, int height)
{
	//window size, handle etc for directX
	m_width		= width;
	m_height	= height;
	
	m_toolHandle = handle;

	m_d3dRenderer.Initialize(m_toolHandle, m_width, m_height);

	//database connection establish
	int rc;
	rc = sqlite3_open_v2("database/test.db",&m_databaseConnection, SQLITE_OPEN_READWRITE, NULL);

	if (rc) 
	{
		TRACE("Can't open database");
		//if the database cant open. Perhaps a more catastrophic error would be better here
	}
	else 
	{
		TRACE("Opened database successfully");
	}

	onActionLoad();
}

void ToolMain::onActionLoad()
{
	//load current chunk and objects into lists
	if (!m_sceneGraph.empty())		//is the vector empty
	{
		m_sceneGraph.clear();		//if not, empty it
	}

	//SQL
	int rc;
	char *sqlCommand;
	char *ErrMSG = 0;
	sqlite3_stmt *pResults;								//results of the query
	sqlite3_stmt *pResultsChunk;

	//OBJECTS IN THE WORLD
	//prepare SQL Text
	sqlCommand = "SELECT * from Objects";				//sql command which will return all records from the objects table.
	//Send Command and fill result object
	rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand, -1, &pResults, 0 );
	
	//loop for each row in results until there are no more rows.  ie for every row in the results. We create and object
	while (sqlite3_step(pResults) == SQLITE_ROW)
	{	
		SceneObject newSceneObject;
		newSceneObject.ID = sqlite3_column_int(pResults, 0);
		newSceneObject.chunk_ID = sqlite3_column_int(pResults, 1);
		newSceneObject.model_path		= reinterpret_cast<const char*>(sqlite3_column_text(pResults, 2));
		newSceneObject.tex_diffuse_path = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 3));
		newSceneObject.posX = sqlite3_column_double(pResults, 4);
		newSceneObject.posY = sqlite3_column_double(pResults, 5);
		newSceneObject.posZ = sqlite3_column_double(pResults, 6);
		newSceneObject.rotX = sqlite3_column_double(pResults, 7);
		newSceneObject.rotY = sqlite3_column_double(pResults, 8);
		newSceneObject.rotZ = sqlite3_column_double(pResults, 9);
		newSceneObject.scaX = sqlite3_column_double(pResults, 10);
		newSceneObject.scaY = sqlite3_column_double(pResults, 11);
		newSceneObject.scaZ = sqlite3_column_double(pResults, 12);
		newSceneObject.render = sqlite3_column_int(pResults, 13);
		newSceneObject.collision = sqlite3_column_int(pResults, 14);
		newSceneObject.collision_mesh = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 15));
		newSceneObject.collectable = sqlite3_column_int(pResults, 16);
		newSceneObject.destructable = sqlite3_column_int(pResults, 17);
		newSceneObject.health_amount = sqlite3_column_int(pResults, 18);
		newSceneObject.editor_render = sqlite3_column_int(pResults, 19);
		newSceneObject.editor_texture_vis = sqlite3_column_int(pResults, 20);
		newSceneObject.editor_normals_vis = sqlite3_column_int(pResults, 21);
		newSceneObject.editor_collision_vis = sqlite3_column_int(pResults, 22);
		newSceneObject.editor_pivot_vis = sqlite3_column_int(pResults, 23);
		newSceneObject.pivotX = sqlite3_column_double(pResults, 24);
		newSceneObject.pivotY = sqlite3_column_double(pResults, 25);
		newSceneObject.pivotZ = sqlite3_column_double(pResults, 26);
		newSceneObject.snapToGround = sqlite3_column_int(pResults, 27);
		newSceneObject.AINode = sqlite3_column_int(pResults, 28);
		newSceneObject.audio_path = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 29));
		newSceneObject.volume = sqlite3_column_double(pResults, 30);
		newSceneObject.pitch = sqlite3_column_double(pResults, 31);
		newSceneObject.pan = sqlite3_column_int(pResults, 32);
		newSceneObject.one_shot = sqlite3_column_int(pResults, 33);
		newSceneObject.play_on_init = sqlite3_column_int(pResults, 34);
		newSceneObject.play_in_editor = sqlite3_column_int(pResults, 35);
		newSceneObject.min_dist = sqlite3_column_double(pResults, 36);
		newSceneObject.max_dist = sqlite3_column_double(pResults, 37);
		newSceneObject.camera = sqlite3_column_int(pResults, 38);
		newSceneObject.path_node = sqlite3_column_int(pResults, 39);
		newSceneObject.path_node_start = sqlite3_column_int(pResults, 40);
		newSceneObject.path_node_end = sqlite3_column_int(pResults, 41);
		newSceneObject.parent_id = sqlite3_column_int(pResults, 42);
		newSceneObject.editor_wireframe = sqlite3_column_int(pResults, 43);
		newSceneObject.name = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 44));

		newSceneObject.light_type = sqlite3_column_int(pResults, 45);
		newSceneObject.light_diffuse_r = sqlite3_column_double(pResults, 46);
		newSceneObject.light_diffuse_g = sqlite3_column_double(pResults, 47);
		newSceneObject.light_diffuse_b = sqlite3_column_double(pResults, 48);
		newSceneObject.light_specular_r = sqlite3_column_double(pResults, 49);
		newSceneObject.light_specular_g = sqlite3_column_double(pResults, 50);
		newSceneObject.light_specular_b = sqlite3_column_double(pResults, 51);
		newSceneObject.light_spot_cutoff = sqlite3_column_double(pResults, 52);
		newSceneObject.light_constant = sqlite3_column_double(pResults, 53);
		newSceneObject.light_linear = sqlite3_column_double(pResults, 54);
		newSceneObject.light_quadratic = sqlite3_column_double(pResults, 55);
	

		//send completed object to scenegraph
		m_sceneGraph.push_back(newSceneObject);
	}

	//THE WORLD CHUNK
	//prepare SQL Text
	sqlCommand = "SELECT * from Chunks";				//sql command which will return all records from  chunks table. There is only one tho.
														//Send Command and fill result object
	rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand, -1, &pResultsChunk, 0);


	sqlite3_step(pResultsChunk);
	m_chunk.ID = sqlite3_column_int(pResultsChunk, 0);
	m_chunk.name = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 1));
	m_chunk.chunk_x_size_metres = sqlite3_column_int(pResultsChunk, 2);
	m_chunk.chunk_y_size_metres = sqlite3_column_int(pResultsChunk, 3);
	m_chunk.chunk_base_resolution = sqlite3_column_int(pResultsChunk, 4);
	m_chunk.heightmap_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 5));
	m_chunk.tex_diffuse_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 6));
	m_chunk.tex_splat_alpha_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 7));
	m_chunk.tex_splat_1_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 8));
	m_chunk.tex_splat_2_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 9));
	m_chunk.tex_splat_3_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 10));
	m_chunk.tex_splat_4_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 11));
	m_chunk.render_wireframe = sqlite3_column_int(pResultsChunk, 12);
	m_chunk.render_normals = sqlite3_column_int(pResultsChunk, 13);
	m_chunk.tex_diffuse_tiling = sqlite3_column_int(pResultsChunk, 14);
	m_chunk.tex_splat_1_tiling = sqlite3_column_int(pResultsChunk, 15);
	m_chunk.tex_splat_2_tiling = sqlite3_column_int(pResultsChunk, 16);
	m_chunk.tex_splat_3_tiling = sqlite3_column_int(pResultsChunk, 17);
	m_chunk.tex_splat_4_tiling = sqlite3_column_int(pResultsChunk, 18);


	//Process REsults into renderable
	m_d3dRenderer.BuildDisplayList(&m_sceneGraph);
	//build the renderable chunk 
	m_d3dRenderer.BuildDisplayChunk(&m_chunk);

}

void ToolMain::onActionSave()
{
	StopEditingObjects();

	//SQL
	int rc;
	char *sqlCommand;
	char *ErrMSG = 0;
	sqlite3_stmt *pResults;								//results of the query
	

	//OBJECTS IN THE WORLD Delete them all
	//prepare SQL Text
	sqlCommand = "DELETE FROM Objects";	 //will delete the whole object table.   Slightly risky but hey.
	rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand, -1, &pResults, 0);
	sqlite3_step(pResults);

	//Populate with our new objects
	std::wstring sqlCommand2;
	int numObjects = m_sceneGraph.size();	//Loop thru the scengraph.

	for (int i = 0; i < numObjects; i++)
	{
		std::stringstream command;
		command << "INSERT INTO Objects " 
			<<"VALUES(" << m_sceneGraph.at(i).ID << ","
			<< m_sceneGraph.at(i).chunk_ID  << ","
			<< "'" << m_sceneGraph.at(i).model_path <<"'" << ","
			<< "'" << m_sceneGraph.at(i).tex_diffuse_path << "'" << ","
			<< m_sceneGraph.at(i).posX << ","
			<< m_sceneGraph.at(i).posY << ","
			<< m_sceneGraph.at(i).posZ << ","
			<< m_sceneGraph.at(i).rotX << ","
			<< m_sceneGraph.at(i).rotY << ","
			<< m_sceneGraph.at(i).rotZ << ","
			<< m_sceneGraph.at(i).scaX << ","
			<< m_sceneGraph.at(i).scaY << ","
			<< m_sceneGraph.at(i).scaZ << ","
			<< m_sceneGraph.at(i).render << ","
			<< m_sceneGraph.at(i).collision << ","
			<< "'" << m_sceneGraph.at(i).collision_mesh << "'" << ","
			<< m_sceneGraph.at(i).collectable << ","
			<< m_sceneGraph.at(i).destructable << ","
			<< m_sceneGraph.at(i).health_amount << ","
			<< m_sceneGraph.at(i).editor_render << ","
			<< m_sceneGraph.at(i).editor_texture_vis << ","
			<< m_sceneGraph.at(i).editor_normals_vis << ","
			<< m_sceneGraph.at(i).editor_collision_vis << ","
			<< m_sceneGraph.at(i).editor_pivot_vis << ","
			<< m_sceneGraph.at(i).pivotX << ","
			<< m_sceneGraph.at(i).pivotY << ","
			<< m_sceneGraph.at(i).pivotZ << ","
			<< m_sceneGraph.at(i).snapToGround << ","
			<< m_sceneGraph.at(i).AINode << ","
			<< "'" << m_sceneGraph.at(i).audio_path << "'" << ","
			<< m_sceneGraph.at(i).volume << ","
			<< m_sceneGraph.at(i).pitch << ","
			<< m_sceneGraph.at(i).pan << ","
			<< m_sceneGraph.at(i).one_shot << ","
			<< m_sceneGraph.at(i).play_on_init << ","
			<< m_sceneGraph.at(i).play_in_editor << ","
			<< m_sceneGraph.at(i).min_dist << ","
			<< m_sceneGraph.at(i).max_dist << ","
			<< m_sceneGraph.at(i).camera << ","
			<< m_sceneGraph.at(i).path_node << ","
			<< m_sceneGraph.at(i).path_node_start << ","
			<< m_sceneGraph.at(i).path_node_end << ","
			<< m_sceneGraph.at(i).parent_id << ","
			<< m_sceneGraph.at(i).editor_wireframe << ","
			<< "'" << m_sceneGraph.at(i).name << "'" << ","

			<< m_sceneGraph.at(i).light_type << ","
			<< m_sceneGraph.at(i).light_diffuse_r << ","
			<< m_sceneGraph.at(i).light_diffuse_g << ","
			<< m_sceneGraph.at(i).light_diffuse_b << ","
			<< m_sceneGraph.at(i).light_specular_r << ","
			<< m_sceneGraph.at(i).light_specular_g << ","
			<< m_sceneGraph.at(i).light_specular_b << ","
			<< m_sceneGraph.at(i).light_spot_cutoff << ","
			<< m_sceneGraph.at(i).light_constant << ","
			<< m_sceneGraph.at(i).light_linear << ","
			<< m_sceneGraph.at(i).light_quadratic

			<< ")";
		std::string sqlCommand2 = command.str();
		rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand2.c_str(), -1, &pResults, 0);
		sqlite3_step(pResults);	
	}
	MessageBox(NULL, L"Objects Saved", L"Notification", MB_OK);
}

void ToolMain::onActionSaveTerrain()
{
	m_d3dRenderer.SaveDisplayChunk(&m_chunk);
}

void ToolMain::Tick(MSG *msg)
{
	CWnd* myDXFrame = CWnd::FromHandle(m_toolHandle);
	CRect dxViewRect;
	myDXFrame->GetWindowRect(dxViewRect);

	m_d3dRenderer.SetScreenDimensions(dxViewRect);

	//Renderer Update Call
	m_d3dRenderer.Tick(&m_toolInputCommands);

	//do we have a selection
	//do we have a mode
	//are we clicking / dragging /releasing
	//has something changed
		//update Scenegraph
		//add to scenegraph
		//resend scenegraph to Direct X renderer
	if (m_d3dRenderer.ShouldRebuildDisplayList())
		m_d3dRenderer.BuildDisplayList(&m_sceneGraph);

	m_selectedObjects = m_d3dRenderer.GetSelectedObjectID();

}

void ToolMain::UpdateInput(MSG * msg)
{
	if (HasFocus()) 
	{
		switch (msg->message)
		{
			//Global inputs,  mouse position and keys etc
		case WM_KEYDOWN:
			m_keyArray[msg->wParam] = true;
			break;

		case WM_KEYUP:
			m_keyArray[msg->wParam] = false;
			break;

		case WM_MOUSEMOVE:
			// get mouse pos x/y
			m_toolInputCommands.windowMouseX = GET_X_LPARAM(msg->lParam);
			m_toolInputCommands.windowMouseY = GET_Y_LPARAM(msg->lParam);

			POINT mousePos;
			if (GetCursorPos(&mousePos)) {
				m_toolInputCommands.mouseX = mousePos.x;
				m_toolInputCommands.mouseY = mousePos.y;
			}
			break;

		case WM_LBUTTONDOWN:
			m_toolInputCommands.mouseLeftDown = true;
			break;

		case WM_LBUTTONUP:
			m_toolInputCommands.mouseLeftDown = false;
			break;

		case WM_RBUTTONDOWN:
		{
			m_toolInputCommands.mouseRightDown = true;

			CWnd* myDXFrame = CWnd::FromHandle(m_toolHandle);
			CRect dxViewRect;
			myDXFrame->GetWindowRect(dxViewRect);

			bool mouseWithinViewport = (m_toolInputCommands.mouseX > dxViewRect.left && m_toolInputCommands.mouseX < dxViewRect.right)
				&& (m_toolInputCommands.mouseY > dxViewRect.top && m_toolInputCommands.mouseY < dxViewRect.bottom);

			if (mouseWithinViewport && HasFocus()) {
				m_toolInputCommands.mouseControllingCam = true;
				ShowCursor(false);

				POINT mousePos;
				if (GetCursorPos(&mousePos)) {
					m_toolInputCommands.mouseOriginX = mousePos.x;
					m_toolInputCommands.mouseOriginY = mousePos.y;
				}

				SetCursorPos(dxViewRect.CenterPoint().x, dxViewRect.CenterPoint().y);
			}

			break;
		}
		case WM_RBUTTONUP:
			m_toolInputCommands.mouseRightDown = false;
			if (m_toolInputCommands.mouseControllingCam) {

				m_toolInputCommands.mouseControllingCam = false;
				SetCursorPos(m_toolInputCommands.mouseOriginX, m_toolInputCommands.mouseOriginY);
				ShowCursor(true);
			}
			break;
		}

		if (m_keyArray['E'])
			m_toolInputCommands.moveUp = true;
		else
			m_toolInputCommands.moveUp = false;

		if (m_keyArray['Q'])
			m_toolInputCommands.moveDown = true;
		else
			m_toolInputCommands.moveDown = false;

		//WASD camMovment

		if (m_keyArray['W'])
			m_toolInputCommands.forward = true;
		else
			m_toolInputCommands.forward = false;

		if (m_keyArray['S'])
			m_toolInputCommands.back = true;
		else
			m_toolInputCommands.back = false;

		if (m_keyArray['A'])
			m_toolInputCommands.left = true;
		else
			m_toolInputCommands.left = false;

		if (m_keyArray['D'])
			m_toolInputCommands.right = true;
		else
			m_toolInputCommands.right = false;

		//cam rot
		if (!m_d3dRenderer.m_isEditingObjects) {

			if (m_keyArray[VK_UP])
				m_toolInputCommands.pitchUp = true;
			else
				m_toolInputCommands.pitchUp = false;

			if (m_keyArray[VK_DOWN])
				m_toolInputCommands.pitchDown = true;
			else
				m_toolInputCommands.pitchDown = false;

			if (m_keyArray[VK_LEFT])
				m_toolInputCommands.rotLeft = true;
			else
				m_toolInputCommands.rotLeft = false;

			if (m_keyArray[VK_RIGHT])
				m_toolInputCommands.rotRight = true;
			else
				m_toolInputCommands.rotRight = false;
		}
		else {
			if (m_toolInputCommands.pitchUp == true) {
				m_toolInputCommands.pitchUp = false;
			}
			if (m_toolInputCommands.pitchDown == true) {
				m_toolInputCommands.pitchDown = false;
			}
			if (m_toolInputCommands.rotLeft == true) {
				m_toolInputCommands.rotLeft = false;
			}
			if (m_toolInputCommands.rotRight == true) {
				m_toolInputCommands.rotRight = false;
			}
		}



		// Activate multi selection
		if (m_keyArray[VK_CONTROL])
			m_toolInputCommands.ctrlDown = true;
		else
			m_toolInputCommands.ctrlDown = false;


		// Delete Object
		if (m_keyArray[VK_DELETE])
		{
			if (m_selectedObjects.size() > 0 && m_toolInputCommands.deleteKeyDown == false) {

				//m_UndoRedoSystem.AddNewAction(m_selectedObjects, Action::Deletion);
				m_selectedObjects.clear();
				m_toolInputCommands.clearSelectedObjects = true;
				m_d3dRenderer.RebuildDisplayList();
				m_d3dRenderer.m_SelectedObjectIDs.clear();
				m_UnalteredObjects.clear();
				m_UpdatedObjects.clear();
			}

			m_toolInputCommands.deleteKeyDown = true;
		}

		// turn on eddit object pos
		if (m_keyArray[VK_F1]) {
			if (m_toolInputCommands.fOneDown == false && m_d3dRenderer.m_isEditingPos == false) {
				m_d3dRenderer.m_isEditingObjects = true;
				m_d3dRenderer.m_isEditingPos = true;
				m_d3dRenderer.m_isEditingRot = false;
				m_d3dRenderer.m_isEditingScale = false;
				m_d3dRenderer.RebuildDisplayList();
				m_toolInputCommands.fOneDown = true;
			}
			else if (m_toolInputCommands.fOneDown == false && m_d3dRenderer.m_isEditingPos == true) {

				StopEditingObjects();

				m_toolInputCommands.fOneDown = true;
			}

		}
		if (!m_keyArray[VK_F1]) {
			m_toolInputCommands.fOneDown = false;
		}

		// turn on eddit object rot
		if (m_keyArray[VK_F2]) {
			if (m_toolInputCommands.fTwoDown == false && m_d3dRenderer.m_isEditingRot == false) {
				StopEditingObjects();
				m_d3dRenderer.m_isEditingObjects = true;
				m_d3dRenderer.m_isEditingRot = true;
				m_d3dRenderer.m_isEditingScale = false;
				m_d3dRenderer.m_isEditingPos = false;
				m_toolInputCommands.fTwoDown = true;
			}
			else if (m_toolInputCommands.fTwoDown == false && m_d3dRenderer.m_isEditingRot == true) {
				StopEditingObjects();
				m_toolInputCommands.fTwoDown = true;
			}
		}
		if (!m_keyArray[VK_F2]) {
			m_toolInputCommands.fTwoDown = false;
		}

		// turn on eddit object sclae
		if (m_keyArray[VK_F3]) {
			if (m_toolInputCommands.fThreeDown == false && m_d3dRenderer.m_isEditingScale == false) {
				StopEditingObjects();
				m_d3dRenderer.m_isEditingObjects = true;
				m_d3dRenderer.m_isEditingScale = true;
				m_d3dRenderer.m_isEditingPos = false;
				m_d3dRenderer.m_isEditingRot = false;
				m_toolInputCommands.fThreeDown = true;
			}
			else if (m_toolInputCommands.fThreeDown == false && m_d3dRenderer.m_isEditingScale == true) {
				StopEditingObjects();
				m_toolInputCommands.fThreeDown = true;
			}
		}
		if (!m_keyArray[VK_F3]) {
			m_toolInputCommands.fThreeDown = false;
		}

		if (m_d3dRenderer.m_isEditingObjects) {

			m_UpdatedObjectsCaptured = false;
			if (m_numEditingObjects != m_d3dRenderer.m_SelectedObjectIDs.size()) {

				AddObjectsToUnalteredObjects();
			}
			//set m_WasEditingObjectTransforms to true so that if the state changes we can take the aproprite actions the next time we are not
			m_WasEditingObjectTransforms = true;

			if (m_toolInputCommands.slowMove == true) {
				m_toolInputCommands.slowMove = false;
			}

			if (m_keyArray[VK_RIGHT]) {
				m_toolInputCommands.rightDown = true;
				m_d3dRenderer.RebuildDisplayList();

				if (m_NewEditingStarted == false) {
					m_NewEditingStarted = true;
				}
			}
			else {
				m_toolInputCommands.rightDown = false;
				if (m_NewEditingStarted) {
					if (EditingTransformsStoped()) {
						AddObjectsUpdatedObjects();
					}
				}
			}

			if (m_keyArray[VK_LEFT]) {
				m_toolInputCommands.leftDown = true;
				m_d3dRenderer.RebuildDisplayList();

				if (m_NewEditingStarted == false) {
					m_NewEditingStarted = true;
				}
			}
			else {
				m_toolInputCommands.leftDown = false;
				if (m_NewEditingStarted) {
					if (EditingTransformsStoped()) {
						AddObjectsUpdatedObjects();
					}
				}
			}

			if (m_keyArray[VK_UP]) {
				m_toolInputCommands.upDown = true;
				m_d3dRenderer.RebuildDisplayList();

				if (m_NewEditingStarted == false) {
					m_NewEditingStarted = true;
				}
			}
			else {
				m_toolInputCommands.upDown = false;
				if (m_NewEditingStarted) {
					if (EditingTransformsStoped()) {
						AddObjectsUpdatedObjects();
					}
				}
			}

			if (m_keyArray[VK_DOWN]) {
				m_toolInputCommands.downDown = true;
				m_d3dRenderer.RebuildDisplayList();

				if (m_NewEditingStarted == false) {
					m_NewEditingStarted = true;
				}
			}
			else {
				m_toolInputCommands.downDown = false;

				if (m_NewEditingStarted) {
					if (EditingTransformsStoped()) {
						AddObjectsUpdatedObjects();
					}
				}
			}
		}
		else {

			if (m_WasEditingObjectTransforms) {
				m_WasEditingObjectTransforms = false;

				if (m_NewEditingStarted == true && m_UpdatedObjectsCaptured == false) {
					AddObjectsUpdatedObjects();
				}
			}

			if (m_toolInputCommands.rightDown == true) {
				m_toolInputCommands.rightDown = false;
			}
			if (m_toolInputCommands.leftDown == true) {
				m_toolInputCommands.leftDown = false;
			}
			if (m_toolInputCommands.upDown == true) {
				m_toolInputCommands.upDown = false;
			}
			if (m_toolInputCommands.downDown == true) {
				m_toolInputCommands.downDown = false;
			}
			if (m_toolInputCommands.shiftDown == true) {
				m_toolInputCommands.shiftDown = false;
			}

			// Activate slow movement
			if (m_keyArray[VK_SHIFT])
				m_toolInputCommands.slowMove = true;
			else
				m_toolInputCommands.slowMove = false;
		}
	}
	else {
		StopEditingObjects();
		m_toolInputCommands.left = false;
		m_toolInputCommands.right = false;
		m_toolInputCommands.moveUp = false;
		m_toolInputCommands.moveDown = false;
		m_toolInputCommands.forward = false;
		m_toolInputCommands.back = false;
	}
}

int ToolMain::GetIndexFromID(unsigned int objectID)
{
	for (size_t i = 0; i < m_sceneGraph.size(); i++)
	{
		if (m_sceneGraph[i].ID == objectID)
			return i;
	}

	return -1;
}

bool ToolMain::HasFocus()
{
	return (GetFocus() == GetParent(m_toolHandle));
}

void ToolMain::RebuildDisplayList()
{
	m_d3dRenderer.RebuildDisplayList();
}

void ToolMain::StopEditingObjects()
{
	m_d3dRenderer.m_isEditingObjects = false;
	m_d3dRenderer.m_isEditingPos = false;
	m_d3dRenderer.m_isEditingRot = false;
	m_d3dRenderer.m_isEditingScale = false;

	m_UpdatedObjectsCaptured = true;
	m_NewEditingStarted = false;
	m_UnalteredObjects.clear();
	m_UpdatedObjects.clear();
	m_d3dRenderer.RebuildDisplayList();
}

BOOL ToolMain::DetectChangeInObject()
{
	if (m_OriginalObject.name != m_ObjectUpdate.name) {
		return TRUE;
	}

	if (m_OriginalObject.parent_id != m_ObjectUpdate.parent_id) {
		return TRUE;
	}

	if (m_OriginalObject.model_path != m_ObjectUpdate.model_path) {
		return TRUE;
	}

	if (m_OriginalObject.tex_diffuse_path != m_ObjectUpdate.tex_diffuse_path) {
		return TRUE;
	}

	if (m_OriginalObject.posX != m_ObjectUpdate.posX) {
		return TRUE;
	}

	if (m_OriginalObject.posY != m_ObjectUpdate.posY) {
		return TRUE;
	}

	if (m_OriginalObject.posZ != m_ObjectUpdate.posZ) {
		return TRUE;
	}

	if (m_OriginalObject.rotX != m_ObjectUpdate.rotX) {
		return TRUE;
	}

	if (m_OriginalObject.rotY != m_ObjectUpdate.rotY) {
		return TRUE;
	}

	if (m_OriginalObject.rotZ != m_ObjectUpdate.rotZ) {
		return TRUE;
	}

	if (m_OriginalObject.scaX != m_ObjectUpdate.scaX) {
		return TRUE;
	}

	if (m_OriginalObject.scaY != m_ObjectUpdate.scaY) {
		return TRUE;
	}

	if (m_OriginalObject.scaZ != m_ObjectUpdate.scaZ) {
		return TRUE;
	}

	if (m_OriginalObject.pivotX != m_ObjectUpdate.pivotX) {
		return TRUE;
	}

	if (m_OriginalObject.pivotY != m_ObjectUpdate.pivotY) {
		return TRUE;
	}

	if (m_OriginalObject.pivotZ != m_ObjectUpdate.pivotZ) {
		return TRUE;
	}

	if (m_OriginalObject.snapToGround != m_ObjectUpdate.snapToGround) {
		return TRUE;
	}

	if (m_OriginalObject.collision_mesh != m_ObjectUpdate.collision_mesh) {
		return TRUE;
	}

	if (m_OriginalObject.collision != m_ObjectUpdate.collision) {
		return TRUE;
	}

	if (m_OriginalObject.collectable != m_ObjectUpdate.collectable) {
		return TRUE;
	}

	if (m_OriginalObject.destructable != m_ObjectUpdate.destructable) {
		return TRUE;
	}

	if (m_OriginalObject.health_amount != m_ObjectUpdate.health_amount) {
		return TRUE;
	}

	if (m_OriginalObject.light_type != m_ObjectUpdate.light_type) {
		return TRUE;
	}

	if (m_OriginalObject.light_diffuse_r != m_ObjectUpdate.light_diffuse_r) {
		return TRUE;
	}

	if (m_OriginalObject.light_diffuse_g != m_ObjectUpdate.light_diffuse_g) {
		return TRUE;
	}

	if (m_OriginalObject.light_diffuse_b != m_ObjectUpdate.light_diffuse_b) {
		return TRUE;
	}

	if (m_OriginalObject.light_specular_r != m_ObjectUpdate.light_specular_r) {
		return TRUE;
	}

	if (m_OriginalObject.light_specular_g != m_ObjectUpdate.light_specular_g) {
		return TRUE;
	}

	if (m_OriginalObject.light_specular_b != m_ObjectUpdate.light_specular_b) {
		return TRUE;
	}

	if (m_OriginalObject.light_spot_cutoff != m_ObjectUpdate.light_spot_cutoff) {
		return TRUE;
	}

	if (m_OriginalObject.light_constant != m_ObjectUpdate.light_constant) {
		return TRUE;
	}

	if (m_OriginalObject.light_linear != m_ObjectUpdate.light_linear) {
		return TRUE;
	}

	if (m_OriginalObject.light_quadratic != m_ObjectUpdate.light_quadratic) {
		return TRUE;
	}

	if (m_OriginalObject.render != m_ObjectUpdate.render) {
		return TRUE;
	}

	if (m_OriginalObject.editor_render != m_ObjectUpdate.editor_render) {
		return TRUE;
	}

	if (m_OriginalObject.camera != m_ObjectUpdate.camera) {
		return TRUE;
	}

	if (m_OriginalObject.editor_wireframe != m_ObjectUpdate.editor_wireframe) {
		return TRUE;
	}

	if (m_OriginalObject.editor_texture_vis != m_ObjectUpdate.editor_texture_vis) {
		return TRUE;
	}

	if (m_OriginalObject.editor_normals_vis != m_ObjectUpdate.editor_normals_vis) {
		return TRUE;
	}

	if (m_OriginalObject.editor_collision_vis != m_ObjectUpdate.editor_collision_vis) {
		return TRUE;
	}

	if (m_OriginalObject.editor_pivot_vis != m_ObjectUpdate.editor_pivot_vis) {
		return TRUE;
	}

	if (m_OriginalObject.audio_path != m_ObjectUpdate.audio_path) {
		return TRUE;
	}

	if (m_OriginalObject.volume != m_ObjectUpdate.volume) {
		return TRUE;
	}

	if (m_OriginalObject.pan != m_ObjectUpdate.pan) {
		return TRUE;
	}

	if (m_OriginalObject.pitch != m_ObjectUpdate.pitch) {
		return TRUE;
	}

	if (m_OriginalObject.one_shot != m_ObjectUpdate.one_shot) {
		return TRUE;
	}

	if (m_OriginalObject.play_on_init != m_ObjectUpdate.play_on_init) {
		return TRUE;
	}

	if (m_OriginalObject.play_in_editor != m_ObjectUpdate.play_in_editor) {
		return TRUE;
	}

	if (m_OriginalObject.min_dist != m_ObjectUpdate.min_dist) {
		return TRUE;
	}

	if (m_OriginalObject.max_dist != m_ObjectUpdate.max_dist) {
		return TRUE;
	}

	if (m_OriginalObject.AINode != m_ObjectUpdate.AINode) {
		return TRUE;
	}

	if (m_OriginalObject.path_node != m_ObjectUpdate.path_node) {
		return TRUE;
	}

	if (m_OriginalObject.path_node_start != m_ObjectUpdate.path_node_start) {
		return TRUE;
	}

	if (m_OriginalObject.path_node_end != m_ObjectUpdate.path_node_end) {
		return TRUE;
	}

	return FALSE;
}

void ToolMain::AddObjectsToUnalteredObjects()
{
	for (size_t i = 0; i < m_d3dRenderer.m_SelectedObjectIDs.size(); i++)
	{
		bool objectInList = false;

		for (size_t j = 0; j < m_UnalteredObjects.size(); j++)
		{
			if (m_d3dRenderer.m_SelectedObjectIDs[i] == m_UnalteredObjects[j].ID) {
				objectInList = true;
				break;
			}
		}

		if (!objectInList) {
			m_UnalteredObjects.push_back(m_sceneGraph[GetIndexFromID(m_d3dRenderer.m_SelectedObjectIDs[i])]);
		}
	}

	m_UpdatedObjectsCaptured = false;
	m_UpdatedObjects.clear();
}

void ToolMain::AddObjectsUpdatedObjects()
{
	m_UpdatedObjectsCaptured = true;
	m_NewEditingStarted = false;

	for (size_t i = 0; i < m_UnalteredObjects.size(); i++)
	{
		m_OriginalObject = m_UnalteredObjects[i];
		m_ObjectUpdate = m_sceneGraph[GetIndexFromID(m_UnalteredObjects[i].ID)];

		// if the object was not changed remove it from the list
		if (DetectChangeInObject() == false) {
			m_UnalteredObjects.erase(m_UnalteredObjects.begin() + i);
			i--;
		}
	}

	///if m_UnalteredObjects is now empty no changes where made so we return
	if (m_UnalteredObjects.size() == 0) {
		return;
	}

	for (size_t i = 0; i < m_UnalteredObjects.size(); i++)
	{
		m_UpdatedObjects.push_back(m_sceneGraph[GetIndexFromID(m_UnalteredObjects[i].ID)]);
	}

	if (m_UpdatedObjects.size() == m_UnalteredObjects.size()) {

		std::vector<unsigned int> objectIDs;
		// tempereraly reset the objects to the state they where in before the change
		for (size_t i = 0; i < m_UnalteredObjects.size(); i++)
		{
			m_sceneGraph[GetIndexFromID(m_UnalteredObjects[i].ID)] = m_UnalteredObjects[i];
			objectIDs.push_back(m_UnalteredObjects[i].ID);
		}

		// add the new action to the undo redo system
		//m_UndoRedoSystem.AddNewAction(objectIDs, Action::Default);

		// restore the objects to the state they where in after the change
		for (size_t i = 0; i < m_UpdatedObjects.size(); i++)
		{
			m_sceneGraph[GetIndexFromID(m_UnalteredObjects[i].ID)] = m_UpdatedObjects[i];
		}

		// finaly we add the end resut the undo redo system
		//m_UndoRedoSystem.AddPostAction(objectIDs);

		m_UnalteredObjects.clear();
		m_UpdatedObjects.clear();
	}
}

BOOL ToolMain::EditingTransformsStoped()
{
	if (m_toolInputCommands.rightDown == true) {
		return FALSE;
	}
	if (m_toolInputCommands.leftDown == true) {
		return FALSE;
	}
	if (m_toolInputCommands.upDown == true) {
		return FALSE;
	}
	if (m_toolInputCommands.downDown == true) {
		return FALSE;
	}

	return TRUE;
}
