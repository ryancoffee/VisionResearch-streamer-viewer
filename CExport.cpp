// CExport.cpp : implementation file
//

#include "pch.h"
#include "StreamerViewer.h"
#include "CExport.h"
#include "afxdialogex.h"

// worker thread

// Export Thread
void ExportThread(LPVOID Param)
{
	CExport* dlg = (CExport*)Param;

	int imgcount = 0;
	UINT8* buf = nullptr;
	MultiCXPSource* psource = dlg->m_pSource;
	uint64_t time = dlg->m_start;
	uint64_t end = dlg->m_stop;

	int ret = psource->GetRecordImageAt(&buf, time);
	// get each saved image
	while (SUCCESS == ret && time < end) // while there are data and while we are not too fare in the stream
	{

		// MKV
		if (dlg->m_MkvExport)
		{
			dlg->RecordMkvData((void*)buf, time);
		}

		// Tiff
		if (dlg->m_TiffExport)
		{

			dlg->RecordTiffY8((void*)buf, time);
		}
		
		imgcount++;
		uint32_t us = (uint32_t)(time % 1000);
		uint32_t ms = (uint32_t)((time / 1000) % 1000);
		uint32_t s = (uint32_t)(time / 1000000);

		dlg->m_CS_ExportStatus.Format(L"Exporting image %07d at %u:%03u:%03u", imgcount, s, ms, us);
		
		ret = psource->GetRecordedImageNext(&buf, time);
	}
	
	if (nullptr != dlg->m_pMkvVideo)
	{
		dlg->m_pMkvVideo->release();
		delete dlg->m_pMkvVideo;
		dlg->m_pMkvVideo = nullptr;
	}

	dlg->m_CS_Export_Status = L"export finished";
	Sleep(1000);
	dlg->m_IsExport = false;
}



// CExport dialog

IMPLEMENT_DYNAMIC(CExport, CDialogEx)

CExport::CExport(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_EXPORT, pParent)
	, m_CS_RecordPath(_T(""))
	, m_CS_RecordBaseName(_T(""))
	, m_TiffExport(FALSE)
	, m_MkvExport(FALSE)
	, m_CS_ExportStatus(_T(""))
	, m_start(0)
	, m_stop(0)
	, m_pSource(nullptr)
	, m_IsExport(false)
{

}

CExport::~CExport()
{
}

void CExport::RecordMkvData(void* buffer, uint64_t time)
{
	if (nullptr == m_pMkvVideo)
	{
		std::wostringstream savename;
		savename << (LPCWSTR)m_CS_RecordPath << L"\\" << (LPCWSTR)m_CS_RecordBaseName << L".mkv";
		m_HDSave.SetPath(savename.str());

		m_pMkvVideo = new cv::VideoWriter(wstr2str(savename.str()), 0, 1000 / m_Period, cv::Size((int)fh.sizex, (int)fh.sizey), false);
		if (!m_pMkvVideo->isOpened())
		{
			AfxMessageBox(L"Could not create MKV file for writing");
			return;
		}
	}
	// create a Mat around the retrieved buffer
	// NOTICE Mat are created FIRST Height THEN Width
	cv::Mat frame((int)fh.sizey, (int)fh.sizex, CV_8U, data);
	// store it in the mkv file
	m_pMkvVideo->write(frame);
}

void CExport::RecordTiffY8(void* buffer, uint64_t time)
{
	// CXP driver has its own tiff saving feature 
	
	
	
	ge::ImageConvertInput input = { static_cast<int>(fh.sizex), static_cast<int>(fh.sizey),
		buff,
		"Mono8",
		{ &bufsize, 0, 0, 0 },
		{ 0, 0, 0, 0 } };

	m_pgenTL->imageSaveToDisk(input, wstr2str(filename));
}

void CExport::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDT_BROWSE, m_CS_RecordPath);
	DDX_Text(pDX, IDC_EDT_NAME, m_CS_RecordBaseName);
	DDX_Check(pDX, IDC_CHK_TIFF, m_TiffExport);
	DDX_Check(pDX, IDC_CHK_MKV, m_MkvExport);
	DDX_Text(pDX, IDC_EDT_STATUS, m_CS_ExportStatus);
}


BEGIN_MESSAGE_MAP(CExport, CDialogEx)
	ON_BN_CLICKED(IDC_BT_BROWSE, &CExport::OnBnClickedBtBrowse)
	ON_BN_CLICKED(IDC_BT_EXPORT, &CExport::OnBnClickedBtExport)
END_MESSAGE_MAP()


// CExport message handlers


void CExport::OnBnClickedBtBrowse()
{
	CFolderPickerDialog fpdlg;
	UpdateData(TRUE);
	fpdlg.m_ofn.lpstrTitle = L"Select to recording folder";
	fpdlg.m_ofn.lpstrInitialDir = _T("C:\\");
	if (IDOK == fpdlg.DoModal())
	{
		m_CS_RecordPath = fpdlg.GetPathName();
		UpdateData(FALSE);   // To show updated folder in GUI
	}
}


void CExport::OnBnClickedBtExport()
{
	UpdateData();	// read value of GUI
	if (m_TiffExport || m_MkvExport)
	{
		// no export selection
		AfxMessageBox(L"No export format selected");
		return;
	}
	if (m_CS_RecordBaseName.IsEmpty())
	{
		AfxMessageBox(L"No name defined");
		return;
	}
	if (m_CS_RecordPath.IsEmpty())
	{
		AfxMessageBox(L"No path defined");
		return;
	}
	if (GetFileAttributes(m_CS_RecordPath) == INVALID_FILE_ATTRIBUTES)
	{
		AfxMessageBox(L"Can not access defined path, make sure the path exists and have write access rights");
		return;
	}
	if (m_start >= m_stop || nullptr == m_pSource)
	{
		AfxMessageBox(L"No data to export");
		return;
	}
	m_IsExport = true;
}
