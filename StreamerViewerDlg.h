
// StreamerViewerDlg.h : header file
//

#pragma once
#include "MultiCXPSource.h"




// CStreamerViewerDlg dialog
class CStreamerViewerDlg : public CDialogEx
{
	MultiCXPSource source;
		
	void MenuGrab(bool enable);
	void MenuView(bool enable);

// Construction
public:
	CStreamerViewerDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_STREAMERVIEWER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCameraDetect();
	afx_msg void OnGrabberStart();
	afx_msg void OnGrabberRecord();
	afx_msg void OnGrabberStop();
};
