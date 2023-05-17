#pragma once
#include "afxdialogex.h"
#include "resource.h"
#include "afxwin.h"
#include "SceneObject.h"
#include <vector>

// SelectDialogue dialog

class SelectDialogue : public CDialogEx
{
	DECLARE_DYNAMIC(SelectDialogue)

public:
	SelectDialogue(CWnd* pParent, std::vector<SceneObject>* SceneGraph);   // modal // takes in out scenegraph in the constructor
	SelectDialogue(CWnd* pParent = NULL);
	virtual ~SelectDialogue();
	void SetObjectData(std::vector<SceneObject>* SceneGraph, std::vector<unsigned int>* Selection);	//passing in pointers to the data the class will operate on.
	
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void End();		//kill the dialogue
	afx_msg void Select();	//Item has been selected

	std::vector<SceneObject> * m_sceneGraph;
	std::vector<unsigned int>* m_currentSelection;


	DECLARE_MESSAGE_MAP()
private:

	bool IsGameObjectSelected(unsigned int objectID);
	void RemoveObjectFromSelection(unsigned int objectID);

public:
	// Control variable for more efficient access of the listbox
	CListBox m_listBox;
	//class UndoRedoActions* m_UndoRedoActions;
	virtual BOOL OnInitDialog() override;
	afx_msg void OnLbnSelchangeList1();
	afx_msg void OnBnClickedOk();
	afx_msg void OnClose();
};


INT_PTR CALLBACK SelectProc( HWND   hwndDlg,UINT   uMsg,WPARAM wParam,LPARAM lParam);