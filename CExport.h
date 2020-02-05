#pragma once
#include "MultiCXPSource.h"

// CExport dialog

class CExport : public CDialogEx
{
	DECLARE_DYNAMIC(CExport)

public:
	CExport(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CExport();
	uint64_t m_start;
	uint64_t m_stop;
	bool m_IsExport;

	MultiCXPSource* m_pSource;
	void RecordMkvData(void* buffer, uint64_t time);
	void RecordTiffY8( void* buffer, uint64_t time);
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_EXPORT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtBrowse();
	CString m_CS_RecordPath;
	afx_msg void OnBnClickedBtExport();
	CString m_CS_RecordBaseName;
	BOOL m_TiffExport;
	BOOL m_MkvExport;
	CString m_CS_ExportStatus;
};
