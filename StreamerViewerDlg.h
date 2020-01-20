
// StreamerViewerDlg.h : header file
//

#pragma once
#include "MultiCXPSource.h"
#include "RecordedDataDlg.h"


// CStreamerViewerDlg dialog
class CStreamerViewerDlg : public CDialogEx
{
	MultiCXPSource source;
		
	void MenuGrab(bool enable);
// menu View
	void	MenuView();		// set the view menu according to the current setting
	void	Zoom(int z);	// adjust flag for give zoom factor
	bool	m_viewFullSize;	// need to stretch ?
	int		m_viewZoom;		// 0 not zoom , 2,4,8,16 ( zoom in) , -2,-4,-8,-16 zoom out, 1 = fit to screen
	bool	m_viewRatio;	// keep aspect ratio
	int		m_maxBuf;		// number of buffer that count be allocated
	bool	m_isRec;		// did the dlg set rec command	

// Construction
public:
	CStreamerViewerDlg(CWnd* pParent = nullptr);	// standard constructor
	~CStreamerViewerDlg();



// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_STREAMERVIEWER_DIALOG };
#endif
private: 	
	void setMemUsage();
	void getImgnDisplay();
	void check(int code, CString fct);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	BITMAPINFO *	m_bitmapInfoCOL;		// bitmap header color ( display)
	ImgNfo			m_nfo;					// info on the incoming image		
	uint8_t*		m_pdata;				// pointer to the incoming image
	bool			m_reDraw;				// true if the Onpaint can be called again
	RecordedDataDlg m_RecDlg;				// show recorded images

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

	double m_dFps;
	double m_dMbps;
	uint32_t m_iLostFrame;
	CProgressCtrl m_Prog_Mem;		// Memory usage progress bar
	afx_msg void OnViewFullsize();
	afx_msg void OnViewFittoscreen();
	afx_msg void OnViewKeepaspectratio();
	afx_msg void OnZoominX2();
	afx_msg void OnZoominX4();
	afx_msg void OnZoominX8();
	afx_msg void OnZoominX16();
	afx_msg void OnZoomout1();
	afx_msg void OnZoomout2();
	afx_msg void OnZoomout3();
	afx_msg void OnZoomout4();

	afx_msg void OnDestroy();
};
