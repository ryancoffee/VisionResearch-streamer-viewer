
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

void CStreamerViewerDlg::MenuView()
{
	CMenu* pMenu = GetMenu();
	std::vector<UINT> vids; // view menu IDs
	vids.push_back(ID_VIEW_FULLSIZE);
	vids.push_back(ID_VIEW_FITTOSCREEN);
	vids.push_back(ID_VIEW_KEEPASPECTRATIO);
	vids.push_back(ID_ZOOMIN_X2);
	vids.push_back(ID_ZOOMIN_X4);
	vids.push_back(ID_ZOOMIN_X8);
	vids.push_back(ID_ZOOMIN_X16);
	vids.push_back(ID_ZOOMOUT_1);
	vids.push_back(ID_ZOOMOUT_2);
	vids.push_back(ID_ZOOMOUT_3);
	vids.push_back(ID_ZOOMOUT_4);
	
	// uncheck all
	for (UINT id : vids)
	{
		pMenu->CheckMenuItem(id, MF_UNCHECKED);
	}
	// check what is needed
	if (m_viewFullSize)
		pMenu->CheckMenuItem(ID_VIEW_FULLSIZE, MF_CHECKED);
	if (m_viewRatio)
		pMenu->CheckMenuItem(ID_VIEW_KEEPASPECTRATIO, MF_CHECKED);
	if (m_viewZoom != 0)
	{
		UINT id = ID_ZOOMIN_X2;
		switch (m_viewZoom)
		{
		case 1:
			id = ID_VIEW_FITTOSCREEN;
			break;
		case 2:
			id = ID_ZOOMIN_X2;
			break;
		case 4:
			id = ID_ZOOMIN_X4;
			break;
		case 8:
			id = ID_ZOOMIN_X8;
			break;
		case 16:
			id = ID_ZOOMIN_X16;
			break;
		case -2:
			id = ID_ZOOMOUT_1;
			break;
		case -4:
			id = ID_ZOOMOUT_2;
			break;
		case -8:
			id = ID_ZOOMOUT_3;
			break;
		case -16:
			id = ID_ZOOMOUT_4;
			break;
		default:
			break;
		}
		pMenu->CheckMenuItem(id, MF_CHECKED);
	}
}

CStreamerViewerDlg::CStreamerViewerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_STREAMERVIEWER_DIALOG, pParent)

	, m_dFps(0)
	, m_dMbps(0)
	, m_iLostFrame(0)
	, m_viewFullSize(true)
	, m_viewZoom(0)		
	, m_viewRatio(false)
	, m_maxBuf(0)
	, m_isRec(false)

{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bitmapInfoCOL = nullptr;
	m_bitmapInfoBW = nullptr;
	m_pdata = nullptr;
	m_nfo.sizeX = 0;
	m_nfo.sizeY = 0;
	m_nfo.color = false;
	m_reDraw = true;
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

void CStreamerViewerDlg::setMemUsage()
{
	if (!m_isRec)	// if dlg didn't send rec command do nothing
		return;

	
	if (source.IsRecording())		// we are still recording 
	{
		size_t val = source.GetRecCount();
		val = (m_maxBuf > val) ? m_maxBuf - val : 0;
		m_Prog_Mem.SetPos((int)val);
	}
	else
	{
		// finish recording ...
		m_Prog_Mem.SetPos((int)m_maxBuf);
		// stop the current preview
		OnGrabberStop();
		// spawn new window to see the recorded data
		m_RecDlg.ShowWindow(SW_SHOW);
	}

}

// get an image from the source and display if available

void CStreamerViewerDlg::getImgnDisplay()
{
	CString fct("Get new image");
	check(source.GetImageInfo(m_nfo), fct);
	check(source.GetImage(&m_pdata), fct);
	if (m_pdata != nullptr)
	{
		// post message to redraw the screen
		if (m_reDraw)
		{
			m_reDraw = false;
			CWnd* pic = GetDlgItem(IDC_STPICTURE);
			CRect rect;
			pic->GetClientRect(&rect);
			InvalidateRect(&rect,false); 
		}

		GrabStat s;
		source.GetStat(s);
		m_iLostFrame = (uint32_t)s.lostframes;
		m_dFps = floor(s.fps*100)/100.0;
		m_dMbps = floor(s.mbps*100)/100.0;
		UpdateData(FALSE);
	}

	//cimg_library::CImg<unsigned char> img(nfo.sizeX, nfo.sizeY, 1, nfo.color ? 3 : 1, 0);
}

void CStreamerViewerDlg::check(int code, CString fct)
{
	if (SUCCESS == code)
		return;
	CString msg;
	msg.Format(L"Error %s in function %s", (LPCTSTR)errorText(code), (LPCTSTR)fct);
	AfxMessageBox(msg);

}

void CStreamerViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDT_FPS, m_dFps);
	DDX_Text(pDX, IDC_EDT_MPS, m_dMbps);
	DDX_Text(pDX, IDC_EDT_LOSTFRAME, m_iLostFrame);
	DDX_Control(pDX, IDC_PR_RECORD, m_Prog_Mem);
}

BEGIN_MESSAGE_MAP(CStreamerViewerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_CAMERA_DETECT, &CStreamerViewerDlg::OnCameraDetect)
	ON_COMMAND(ID_GRABBER_START, &CStreamerViewerDlg::OnGrabberStart)
	ON_COMMAND(ID_GRABBER_RECORD, &CStreamerViewerDlg::OnGrabberRecord)
	ON_COMMAND(ID_GRABBER_STOP, &CStreamerViewerDlg::OnGrabberStop)
	ON_WM_TIMER()
	ON_COMMAND(ID_VIEW_FULLSIZE, &CStreamerViewerDlg::OnViewFullsize)
	ON_COMMAND(ID_VIEW_FITTOSCREEN, &CStreamerViewerDlg::OnViewFittoscreen)
	ON_COMMAND(ID_VIEW_KEEPASPECTRATIO, &CStreamerViewerDlg::OnViewKeepaspectratio)
	ON_COMMAND(ID_ZOOMIN_X2, &CStreamerViewerDlg::OnZoominX2)
	ON_COMMAND(ID_ZOOMIN_X4, &CStreamerViewerDlg::OnZoominX4)
	ON_COMMAND(ID_ZOOMIN_X8, &CStreamerViewerDlg::OnZoominX8)
	ON_COMMAND(ID_ZOOMIN_X16, &CStreamerViewerDlg::OnZoominX16)
	ON_COMMAND(ID_ZOOMOUT_1, &CStreamerViewerDlg::OnZoomout1)
	ON_COMMAND(ID_ZOOMOUT_2, &CStreamerViewerDlg::OnZoomout2)
	ON_COMMAND(ID_ZOOMOUT_3, &CStreamerViewerDlg::OnZoomout3)
	ON_COMMAND(ID_ZOOMOUT_4, &CStreamerViewerDlg::OnZoomout4)
	ON_WM_DESTROY()
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
	MenuView();


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
	m_RecDlg.Create(IDD_DLG_RECORDED, this);
	m_RecDlg.ShowWindow(SW_HIDE);
	m_RecDlg.m_psource = &source;

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

		if (m_pdata != nullptr && m_nfo.sizeX > 0)
		{

			CWnd *pic= GetDlgItem(IDC_STPICTURE);
			CDC* pdc = pic->GetWindowDC();

			// color image ?
			BITMAPINFO* bmpNfo;
			if (m_nfo.color)
				bmpNfo = m_bitmapInfoCOL;
			else
				bmpNfo = m_bitmapInfoBW;

			bmpNfo->bmiHeader.biWidth = (LONG)m_nfo.sizeX;
			bmpNfo->bmiHeader.biHeight = -((int)m_nfo.sizeY); // negative because buffer is from top to down and windows expect from bottom to up
			
															  // DRAW
			if (m_viewFullSize)
			{
				// draw the image
				SetDIBitsToDevice(pdc->GetSafeHdc(), 0, 0, (DWORD)m_nfo.sizeX, (DWORD)m_nfo.sizeY,
					0, 0, 0, (UINT)m_nfo.sizeY,
					(void*)m_pdata, bmpNfo, DIB_RGB_COLORS);
			}
			else
			{
				size_t oriw = m_nfo.sizeX;
				size_t orih = m_nfo.sizeY;

				CRect rec;
				pic->GetClientRect(rec);
				size_t desw = rec.Width();
				size_t desh = rec.Height();
				if (m_viewRatio)
				{
					// recompute ration
					float rx = (float)oriw / (float)desw;
					float ry = (float)orih / (float)desh;
					if (rx > ry)
					{
						desh = (size_t)((float)orih / rx);
					}
					else
					{
						desw = (size_t)((float)oriw / ry);
					}
				}

				// build in zoom factor
				if (m_viewZoom > 1)
				{
					desw = oriw * m_viewZoom;
					desh = orih * m_viewZoom;
				}
				if (m_viewZoom < -1)
				{
					desw = oriw / (-1 * m_viewZoom);
					desh = orih / (-1 * m_viewZoom);

				}

				// strech image
				SetStretchBltMode(pdc->GetSafeHdc(), HALFTONE);		// best result but slow
				POINT pt;
				SetBrushOrgEx(pdc->GetSafeHdc(), 0, 0, &pt);		// realign brush for proper streching
				StretchDIBits(pdc->GetSafeHdc(), 1, 1, (int)desw - 1, (int)desh - 1, 0, 0, (int)oriw, (int)orih,
					(void*) m_pdata, bmpNfo, DIB_RGB_COLORS, SRCCOPY); // strech image
					//m_imagePointer[i], m_bitmapInfoCOL, DIB_RGB_COLORS, SRCCOPY); // strech image

			}

			pic->ReleaseDC(pdc);


		}


	}

	// accept a new Paint message
	m_reDraw = true;
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
	// Start image acquisition
	source.SetFps(1000.0);
	source.Start();
	
	// Start the timer to pull images
	SetTimer(1, 30, NULL);

	// enable stop and record and View
	CMenu* pMenu = GetMenu();
	pMenu->EnableMenuItem(ID_GRABBER_START, MF_GRAYED);
	pMenu->EnableMenuItem(ID_GRABBER_STOP, MF_ENABLED);
	pMenu->EnableMenuItem(ID_GRABBER_RECORD, MF_ENABLED);
	//MenuView(true);

	m_RecDlg.ShowWindow(SW_HIDE);
}


void CStreamerViewerDlg::OnGrabberRecord()
{
	m_maxBuf = source.Record();
	CMenu* pMenu = GetMenu();
	pMenu->EnableMenuItem(ID_GRABBER_RECORD, MF_GRAYED);
	// set the memory usage slider
	m_Prog_Mem.SetRange32(0, m_maxBuf);
	m_Prog_Mem.SetPos(m_maxBuf);
	m_isRec = true;
}


void CStreamerViewerDlg::OnGrabberStop()
{
	// Stop pulling timer
	KillTimer(1);

	// Stop image acquisition
	source.Stop();
	
	CMenu* pMenu = GetMenu();
	pMenu->EnableMenuItem(ID_GRABBER_RECORD, MF_GRAYED);
	pMenu->EnableMenuItem(ID_GRABBER_START, MF_ENABLED);
	pMenu->EnableMenuItem(ID_GRABBER_STOP, MF_GRAYED);
}


void CStreamerViewerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	switch (nIDEvent)
	{
	case 1: // grab image
		getImgnDisplay();
		setMemUsage();
		break;
	default:
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CStreamerViewerDlg::OnViewFullsize()
{
	if (m_viewFullSize)
		return; // noting to do
	
	m_viewFullSize = true;
	m_viewRatio = false;
	m_viewZoom = 0;
	// update menu items
	MenuView();

	// when the displied image is small and we go back to 1:1 there might be some clutter, clean it
	InvalidateRect(NULL);
}


void CStreamerViewerDlg::OnViewFittoscreen()
{
	if (m_viewZoom == 1)
		return; //nothing to do
	m_viewFullSize = false;
	m_viewZoom = 1;
	// update menu items
	MenuView();
}


void CStreamerViewerDlg::OnViewKeepaspectratio()
{
	m_viewRatio = !(m_viewRatio);
	if (m_viewRatio)	// if we active the aspect ratio fit to scree need to be on
		m_viewZoom = 1;
	MenuView();
	// when aspect ratio is activated there might be some clutter on the display, just clean everything
	if (m_viewRatio)
		InvalidateRect(NULL);
}

void CStreamerViewerDlg::Zoom(int z)
{
	if (m_viewZoom == z)
		return;
	m_viewFullSize = false;
	m_viewZoom = z;
	m_viewRatio = false;
	MenuView();
	InvalidateRect(NULL);
}

void CStreamerViewerDlg::OnZoominX2()
{
	Zoom(2);
}


void CStreamerViewerDlg::OnZoominX4()
{
	Zoom(4);
}


void CStreamerViewerDlg::OnZoominX8()
{
	Zoom(8);
}


void CStreamerViewerDlg::OnZoominX16()
{
	Zoom(16);
}


void CStreamerViewerDlg::OnZoomout1()
{
	Zoom(-2);
}


void CStreamerViewerDlg::OnZoomout2()
{
	Zoom(-4);
}


void CStreamerViewerDlg::OnZoomout3()
{
	Zoom(-8);
}


void CStreamerViewerDlg::OnZoomout4()
{
	Zoom(-16);
}





void CStreamerViewerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	m_RecDlg.DestroyWindow();
}
