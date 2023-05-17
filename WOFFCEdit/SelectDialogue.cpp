// SelectDialogue.cpp : implementation file
//

#include "stdafx.h"
#include "SelectDialogue.h"

// SelectDialogue dialog

IMPLEMENT_DYNAMIC(SelectDialogue, CDialogEx)

//Message map.  Just like MFCMAIN.cpp.  This is where we catch button presses etc and point them to a handy dandy method.
BEGIN_MESSAGE_MAP(SelectDialogue, CDialogEx)
	ON_COMMAND(IDOK, &SelectDialogue::End)					//ok button
	ON_BN_CLICKED(IDOK, &SelectDialogue::OnBnClickedOk)		
	ON_LBN_SELCHANGE(IDC_LIST1, &SelectDialogue::Select)	//listbox
END_MESSAGE_MAP()


SelectDialogue::SelectDialogue(CWnd* pParent, std::vector<SceneObject>* SceneGraph)		//constructor used in modal
	: CDialogEx(IDD_DIALOG1, pParent)
{
	m_sceneGraph = SceneGraph;
}

SelectDialogue::SelectDialogue(CWnd * pParent)			//constructor used in modeless
	: CDialogEx(IDD_DIALOG1, pParent)
{
}

SelectDialogue::~SelectDialogue()
{
}

///pass through pointers to the data in the tool we want to manipulate
void SelectDialogue::SetObjectData(std::vector<SceneObject>* SceneGraph, std::vector<unsigned int>* selection)
{
	m_sceneGraph = SceneGraph;
	m_currentSelection = selection;

	//roll through all the objects in the scene graph and put an entry for each in the listbox
	int numSceneObjects = m_sceneGraph->size();
	for (int i = 0; i < numSceneObjects; i++)
	{
		int currentObjectID = m_sceneGraph->at(i).ID;
		// double check to make sure that the object is not meant to have been deleted 
		//if (!m_UndoRedoActions->IsObjectFlaggedForDeletion(currentObjectID)) {
		//	//easily possible to make the data string presented more complex. showing other columns.
		//	std::wstring listBoxEntry = std::to_wstring(currentObjectID);
		//	m_listBox.AddString(listBoxEntry.c_str());
		//}
	}
}


void SelectDialogue::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_listBox);
}

void SelectDialogue::End()
{
	DestroyWindow();	//destory the window properly.  INcluding the links and pointers created.  THis is so the dialogue can start again. 
}

void SelectDialogue::Select()
{
	int index = m_listBox.GetCurSel();

	CString currentSelectionValue;
	m_listBox.GetText(index, currentSelectionValue);

	if (IsGameObjectSelected(_ttoi(currentSelectionValue))) {

		RemoveObjectFromSelection(_ttoi(currentSelectionValue));
		//m_UndoRedoActions->PassSelectionToToolMain(*m_currentSelection);
		//m_UndoRedoActions->SignalToRebuildDisplayList();
	}
	else {
		m_currentSelection->push_back(_ttoi(currentSelectionValue));
		//m_UndoRedoActions->PassSelectionToToolMain(*m_currentSelection);
		//m_UndoRedoActions->SignalToRebuildDisplayList();
	}

}

bool SelectDialogue::IsGameObjectSelected(unsigned int objectID)
{
	for (size_t i = 0; i < m_currentSelection->size(); i++)
	{
		if (objectID == m_currentSelection->at(i)) {
			return true;
		}
	}

	return false;
}

void SelectDialogue::RemoveObjectFromSelection(unsigned int objectID)
{
	std::vector<unsigned int>::iterator pos = std::find(m_currentSelection->begin(), m_currentSelection->end(), objectID);

	if (pos != m_currentSelection->end())
		m_currentSelection->erase(pos);
}

BOOL SelectDialogue::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	return TRUE; 
}


void SelectDialogue::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}

void SelectDialogue::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	CDialogEx::OnClose();
	DestroyWindow();
}

