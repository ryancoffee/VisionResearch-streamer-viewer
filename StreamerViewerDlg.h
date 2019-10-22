
// StreamerViewerDlg.h : header file
//

#pragma once
#include "MultiCXPSource.h"
#include "CImg.h"



// CStreamerViewerDlg dialog
class CStreamerViewerDlg : public CDialogEx
{
	MultiCXPSource source;
		
	void MenuGrab(bool enable);
	void MenuView(bool enable);

// Construction
public:
	CStreamerViewerDlg(CWnd* pParent = nullptr);	// standard constructor
	~CStreamerViewerDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_STREAMERVIEWER_DIALOG };
#endif
private: 
	void getImgnDisplay();
	void check(int code, CString fct);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	BITMAPINFO *	m_bitmapInfoCOL;		// bitmap header color ( display)
	BITMAPINFO *	m_bitmapInfoBW;			// bitmap header BW ( display)
	ImgNfo			m_nfo;					// info on the incoming image		
	uint8_t*		m_pdata;				// pointer to the incoming image
	bool			m_reDraw;				// true if the Onpaint can be called again

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
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
