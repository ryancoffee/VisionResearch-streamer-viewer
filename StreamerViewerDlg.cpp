
// StreamerViewerDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"

#include "StreamerViewer.h"
#include "StreamerViewerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CStreamerViewerDlg dialog


// helpers 
void CStreamerViewerDlg::MenuGrab(bool enable)
{
	CMenu* pMenu = GetMenu();
	UINT val = enable ? MF_ENABLED : MF_GRAYED;
	pMenu->EnableMenuItem(ID_GRABBER_RECORD, val);
	pMenu->EnableMenuItem(ID_GRABBER_START, val);
	pMenu->EnableMenuItem(ID_GRABBER_STOP, val);


}

void CStreamerViewerDlg::MenuView(bool enable)
{
	CMenu* pMenu = GetMenu();
	UINT val = enable ? MF_ENABLED : MF_GRAYED;
	pMenu->EnableMenuItem(ID_VIEW_FULLSIZE, val);
	pMenu->EnableMenuItem(ID_VIEW_ZOOMIN, val);
	pMenu->EnableMenuItem(ID_VIEW_ZOOMOUT, val);

}

CStreamerViewerDlg::CStreamerViewerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_STREAMERVIEWER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bitmapInfoCOL = nullptr;
	m_bitmapInfoBW = nullptr;
}

CStreamerViewerDlg::~CStreamerViewerDlg()
{
	if (m_bitmapInfoCOL != nullptr)
	{
		free(m_bitmapInfoCOL);
		m_bitmapInfoCOL = nullptr;
	}

	if (m_bitmapInfoBW != nullptr)
	{
		free(m_bitmapInfoBW);
		m_bitmapInfoBW = nullptr;
	}
}

void CStreamerViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CStreamerViewerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_CAMERA_DETECT, &CStreamerViewerDlg::OnCameraDetect)
	ON_COMMAND(ID_GRABBER_START, &CStreamerViewerDlg::OnGrabberStart)
	ON_COMMAND(ID_GRABBER_RECORD, &CStreamerViewerDlg::OnGrabberRecord)
	ON_COMMAND(ID_GRABBER_STOP, &CStreamerViewerDlg::OnGrabberStop)
END_MESSAGE_MAP()


// CStreamerViewerDlg message handlers

BOOL CStreamerViewerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	/*CMenu* pSysMenu = GetSystemMenu(FALSE);
	int syscount = GetMenuItemCount(pSysMenu->GetSafeHmenu());*/
	
	// disable anything els then detect camera
	CMenu* pMenu = GetMenu();
	pMenu->EnableMenuItem(ID_CAMERA_PARAMETERS, MF_GRAYED);
	MenuGrab(false);
	MenuView(false);
	SetWindowText(L"Streamer Viewer");

	/*if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}*/



	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// color image info header

	m_bitmapInfoCOL = (LPBITMAPINFO)malloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD));
	if (m_bitmapInfoCOL) // prevent C6011 warning
	{
		m_bitmapInfoCOL->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		m_bitmapInfoCOL->bmiHeader.biPlanes = 1;
		m_bitmapInfoCOL->bmiHeader.biBitCount = 24;
		m_bitmapInfoCOL->bmiHeader.biCompression = BI_RGB;
		m_bitmapInfoCOL->bmiHeader.biSizeImage = 0;
		m_bitmapInfoCOL->bmiHeader.biXPelsPerMeter = 0;
		m_bitmapInfoCOL->bmiHeader.biYPelsPerMeter = 0;
		m_bitmapInfoCOL->bmiHeader.biClrUsed = 0;
		m_bitmapInfoCOL->bmiHeader.biClrImportant = 0;

	}
	// bw image info header
	m_bitmapInfoBW = (BITMAPINFO*) malloc(sizeof(BITMAPINFO) + 255 * sizeof(RGBQUAD));
	if (m_bitmapInfoBW)
	{
		m_bitmapInfoBW->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		m_bitmapInfoBW->bmiHeader.biPlanes = 1;
		m_bitmapInfoBW->bmiHeader.biBitCount = 8;
		m_bitmapInfoBW->bmiHeader.biCompression = BI_RGB;
		m_bitmapInfoBW->bmiHeader.biSizeImage = 0;
		m_bitmapInfoBW->bmiHeader.biXPelsPerMeter = 0;
		m_bitmapInfoBW->bmiHeader.biYPelsPerMeter = 0;
		m_bitmapInfoBW->bmiHeader.biClrUsed = 0;
		m_bitmapInfoBW->bmiHeader.biClrImportant = 0;
		for (int i = 0; i < 256; i++)
		{
			m_bitmapInfoBW->bmiColors[i].rgbBlue = (BYTE)i;
			m_bitmapInfoBW->bmiColors[i].rgbGreen = (BYTE)i;
			m_bitmapInfoBW->bmiColors[i].rgbRed = (BYTE)i;
			m_bitmapInfoBW->bmiColors[i].rgbReserved = 0;
		}
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CStreamerViewerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CStreamerViewerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CStreamerViewerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CStreamerViewerDlg::OnCameraDetect()
{
	// find camera
	CamNfo nfo;
	if (source.Init(nfo) < 0)
		return;
	CString title;
	title.Format(L"Streamer Viewer for %s on %u bank(s)", (LPCWSTR) nfo.name , nfo.lnkCount);
	// display the name of the camera
	SetWindowText(title);

	// if success enable parameters and View
	CMenu* pMenu = GetMenu();

	pMenu->EnableMenuItem(ID_CAMERA_DETECT, MF_GRAYED);
	pMenu->EnableMenuItem(ID_CAMERA_PARAMETERS, MF_ENABLED);

	pMenu->EnableMenuItem(ID_GRABBER_START, MF_ENABLED);
}


void CStreamerViewerDlg::OnGrabberStart()
{
	source.Start();
	// Start the timer to pull images

	// enable stop and record and View
	CMenu* pMenu = GetMenu();
	pMenu->EnableMenuItem(ID_GRABBER_START, MF_GRAYED);
	pMenu->EnableMenuItem(ID_GRABBER_STOP, MF_ENABLED);
	pMenu->EnableMenuItem(ID_GRABBER_RECORD, MF_ENABLED);
	MenuView(true);

}


void CStreamerViewerDlg::OnGrabberRecord()
{
	source.Record();
	CMenu* pMenu = GetMenu();
	pMenu->EnableMenuItem(ID_GRABBER_RECORD, MF_GRAYED);

}


void CStreamerViewerDlg::OnGrabberStop()
{
	source.Stop();
	CMenu* pMenu = GetMenu();
	pMenu->EnableMenuItem(ID_GRABBER_RECORD, MF_ENABLED);
	pMenu->EnableMenuItem(ID_GRABBER_START, MF_ENABLED);
	pMenu->EnableMenuItem(ID_GRABBER_STOP, MF_GRAYED);
}
