#pragma once
#include "MultiCXPSource.h"

// RecordedDataDlg dialog

class RecordedDataDlg : public CDialogEx
{
	DECLARE_DYNAMIC(RecordedDataDlg)

public:
	MultiCXPSource * m_psource;	
	BITMAPINFO* m_bitmapInfoCOL;		// bitmap header color ( display)
	// bitfiled 
	// bit 0 ( play/ stop)
	// bit 1-2 ( play speed, 0 slow , 1 normal, 2 fast , 3 very fast)
	// bit 3 directon ( 0 normal, 1 backwards)
	int m_playType; 
	uint64_t m_count;
	uint64_t m_start;
	uint64_t m_startexport;
	uint64_t m_stopexport;
	uint64_t m_stop;
	uint64_t m_pos;
	uint8_t * m_pbuf;
	ImgNfo m_nfo;
	bool m_reDraw;
	RecordedDataDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~RecordedDataDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_RECORDED };
#endif

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void Play();
	void ReDraw();
	void SetTime(CString& str, uint64_t time);

	// Progressbar showing where we are
	afx_msg void OnBnClickedBtRecplay();
	afx_msg void OnBnClickedBtRecffor();
	afx_msg void OnBnClickedBtRecfback();
	afx_msg void OnBnClickedBtRecpause();
	afx_msg void OnBnClickedBtRecstop();
	afx_msg void OnBnClickedBtRecstepf();
	// Tells where we are in the recorded images
	CSliderCtrl m_sldPosition;
	afx_msg void OnNMReleasedcaptureSldPos(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedBtRecstepb();
	afx_msg void OnPaint();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	CString m_StrCurrentFrame;
	CString m_StrStartExport;
	CString m_StrEndExport;
	afx_msg void OnBnClickedBtSetstartexport();
	afx_msg void OnBnClickedBtSetendexport();
	afx_msg void OnBnClickedBtExportnow();
};
