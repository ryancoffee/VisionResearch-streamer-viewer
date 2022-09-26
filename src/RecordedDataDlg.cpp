// RecordedDataDlg.cpp : implementation file
//

#include "pch.h"
#include "StreamerViewer.h"
#include "RecordedDataDlg.h"
#include "afxdialogex.h"
#include "CExport.h"

#ifdef _DEBUG
#define new DEBUG_NEW ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

// RecordedDataDlg dialog

IMPLEMENT_DYNAMIC(RecordedDataDlg, CDialogEx)

RecordedDataDlg::RecordedDataDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_RECORDED, pParent)
	, m_psource(nullptr)
	, m_bitmapInfoCOL(nullptr)
	, m_playType(0)
	, m_count(0)
	, m_start(0)
	, m_stop(0)
	, m_startexport(0)
	, m_stopexport(0)
	, m_pos(0)
	, m_pbuf(nullptr)
	, m_reDraw(true)
	, m_StrCurrentFrame(_T(""))
	, m_StrEndExport(_T(""))
	, m_StrStartExport(_T(""))
{
	m_nfo.sizeX = 0;
	m_nfo.sizeY = 0;
}

RecordedDataDlg::~RecordedDataDlg()
{
	if (m_bitmapInfoCOL != nullptr)
	{
		free(m_bitmapInfoCOL);
		m_bitmapInfoCOL = nullptr;
	}
}

void RecordedDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLD_POS, m_sldPosition);
	DDX_Text(pDX, IDC_EDT_CURRENT, m_StrCurrentFrame);
	DDX_Text(pDX, IDC_EDT_STARTEXPORT, m_StrStartExport);
	DDX_Text(pDX, IDC_EDT_ENDEXPORT, m_StrEndExport);
}


BEGIN_MESSAGE_MAP(RecordedDataDlg, CDialogEx)
	ON_BN_CLICKED(IDCANCEL, &RecordedDataDlg::OnBnClickedCancel)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BT_RECPLAY, &RecordedDataDlg::OnBnClickedBtRecplay)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BT_RECFFOR, &RecordedDataDlg::OnBnClickedBtRecffor)
	ON_BN_CLICKED(IDC_BT_RECFBACK, &RecordedDataDlg::OnBnClickedBtRecfback)
	ON_BN_CLICKED(IDC_BT_RECPAUSE, &RecordedDataDlg::OnBnClickedBtRecpause)
	ON_BN_CLICKED(IDC_BT_RECSTOP, &RecordedDataDlg::OnBnClickedBtRecstop)
	ON_BN_CLICKED(IDC_BT_RECSTEPF, &RecordedDataDlg::OnBnClickedBtRecstepf)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLD_POS, &RecordedDataDlg::OnNMReleasedcaptureSldPos)
	ON_BN_CLICKED(IDC_BT_RECSTEPB, &RecordedDataDlg::OnBnClickedBtRecstepb)
	ON_WM_PAINT()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BT_SETSTARTEXPORT, &RecordedDataDlg::OnBnClickedBtSetstartexport)
	ON_BN_CLICKED(IDC_BT_SETENDEXPORT, &RecordedDataDlg::OnBnClickedBtSetendexport)
	ON_BN_CLICKED(IDC_BT_EXPORTNOW, &RecordedDataDlg::OnBnClickedBtExportnow)
END_MESSAGE_MAP()


// RecordedDataDlg message handlers


void RecordedDataDlg::OnBnClickedCancel()
{
	// we want to recylce this window
	ShowWindow(SW_HIDE);
}


void RecordedDataDlg::OnClose()
{
	// we want to recylce this window
	ShowWindow(SW_HIDE);
}


BOOL RecordedDataDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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
	

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void RecordedDataDlg::OnBnClickedBtRecplay()
{
	if (!(m_playType & 0x01))
		SetTimer(1, 40, NULL);			// system is stopped

	m_playType = 1;
}

void RecordedDataDlg::OnBnClickedBtRecffor()
{
	if (!(m_playType & 0x01))
		SetTimer(1, 40, NULL);			// system is stopped
	
	if (m_playType & 0b01000)
		m_playType = 1;				// if backward => fowards
	else
	{
		int speed = (m_playType >> 1) & 0x03;  // else go faster
		speed++;
		if (speed > 3)
			speed = 3;

		m_playType = (speed << 1) + 1;
	}
}
void RecordedDataDlg::OnBnClickedBtRecfback()
{
	if (!(m_playType & 0x01))
		SetTimer(1, 40, NULL);			// system is stopped

	if (!(m_playType & 0b01000))		// if we are not backwards => go backwards
		m_playType = 0b01001;
	else
	{
		int speed = (m_playType >> 1) & 0x03;  // else go faster
		speed++;
		if (speed > 3)
			speed = 3;
		m_playType = (speed << 1) + 0b01001;	// play backwards
	}

}

void RecordedDataDlg::OnBnClickedBtRecpause()
{
	if (m_playType & 0x01)
		KillTimer(1);
	m_playType = 0;
}
void RecordedDataDlg::OnBnClickedBtRecstop()
{
	if (m_playType & 0x01)
		KillTimer(1);
	m_pos = m_start;
	m_psource->GetRecordImageAt(&m_pbuf, m_pos);
	m_sldPosition.SetPos(0);
	m_playType = 0;
	ReDraw();
}
void RecordedDataDlg::OnBnClickedBtRecstepf()
{
	if (m_playType & 0x01)
		KillTimer(1);
	m_playType = 0;
	if (!m_psource->GetRecordedImageNext(&m_pbuf, m_pos))
	{
		m_sldPosition.SetPos((int)((m_pos - m_start)/1000));
		ReDraw();
	}

}
void RecordedDataDlg::OnBnClickedBtRecstepb()
{
	if (m_playType & 0x01)
		KillTimer(1);
	m_playType = 0;
	if (!m_psource->GetRecordedImageNextEx(&m_pbuf, -1, m_pos))
	{
		m_sldPosition.SetPos((int)((m_pos - m_start) / 1000));
		ReDraw();
	}
}

void RecordedDataDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case 1:
		Play();
		break;
	default:
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}

void RecordedDataDlg::Play()
{
	int speed = ( m_playType >>1) & 0x03;
	int step = 1;
	switch (speed)
	{
	case 0: step = 1;
		break;
	case 1: step = 8;
		break;
	case 2: step = 40;
		break;
	case 3: step = 100;
		break;
	}
	if (m_playType & 0b01000) // reverse
		step = (-1) * step;

	if (!m_psource->GetRecordedImageNextEx(&m_pbuf, step, m_pos))
	{
		m_sldPosition.SetPos((int)((m_pos - m_start)/1000));
		ReDraw();
	}
	else
	{
		//we reach the end
		KillTimer(1);
		m_playType = 0;
	}
}

void RecordedDataDlg::ReDraw()
{
	if (m_reDraw)
	{
		m_reDraw = false;
		CWnd* pic = GetDlgItem(IDC_STRECORD);
		CRect rect;
		pic->GetClientRect(&rect);
		SetTime(m_StrCurrentFrame, m_pos - m_start);
		InvalidateRect(&rect, false);
		UpdateData(false);
	}
}

void RecordedDataDlg::SetTime(CString& str, uint64_t time)
{
	uint32_t us = (uint32_t)(time % 1000);
	uint32_t ms = (uint32_t)((time / 1000) % 1000);
	uint32_t s = (uint32_t)(time / 1000000);
	str.Format(L"%u:%03u:%03u", s, ms, us);
}



void RecordedDataDlg::OnNMReleasedcaptureSldPos(NMHDR* pNMHDR, LRESULT* pResult)
{
	// find current slider position and compute relative time
	size_t pos = m_sldPosition.GetPos();
	pos *= 1000;
	m_pos = m_start + pos;
	// get related image
	if (!m_psource->GetRecordImageAt(&m_pbuf, m_pos))
	{
		m_sldPosition.SetPos((int)((m_pos - m_start) / 1000));
		ReDraw();
	}

	*pResult = 0;
}





void RecordedDataDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	if (m_pbuf != nullptr && m_nfo.sizeX > 0)
	{

		CWnd* pic = GetDlgItem(IDC_STRECORD);
		CDC* pdc = pic->GetWindowDC();

		// color image ?
		BITMAPINFO* bmpNfo;
		bmpNfo = m_bitmapInfoCOL;

		bmpNfo->bmiHeader.biWidth = (LONG)m_nfo.sizeX;
		bmpNfo->bmiHeader.biHeight = -((int)m_nfo.sizeY); // negative because buffer is from top to down and windows expect from bottom to up

														  // DRAW
		//if (m_viewFullSize)
		{
			// draw the image
			SetDIBitsToDevice(pdc->GetSafeHdc(), 0, 0, (DWORD)m_nfo.sizeX, (DWORD)m_nfo.sizeY,
				0, 0, 0, (UINT)m_nfo.sizeY,
				(void*)m_pbuf, bmpNfo, DIB_RGB_COLORS);
		}
		/*else
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
				(void*)m_pdata, bmpNfo, DIB_RGB_COLORS, SRCCOPY); // strech image
				//m_imagePointer[i], m_bitmapInfoCOL, DIB_RGB_COLORS, SRCCOPY); // strech image

		}*/
		pic->ReleaseDC(pdc);
	}

	// accept a new Paint message
	m_reDraw = true;
}


void RecordedDataDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	if (nullptr == m_psource)
		return ;

	m_psource->GetRecordedRange(m_count, m_start, m_stop);
	m_pos = m_start;
	m_startexport = 0;
	m_stopexport = m_stop - m_start;
	m_psource->GetImageInfo(m_nfo);
	m_psource->GetRecordImageAt(&m_pbuf, m_pos);
	int range = (int)((m_stop - m_start) / 1000);
	m_sldPosition.SetRange(0, range); // in miliseconds
	m_sldPosition.SetPos(0);
	SetTime(m_StrStartExport, m_startexport);
	SetTime(m_StrEndExport, m_stopexport);
	UpdateData(false);	
}


void RecordedDataDlg::OnBnClickedBtSetstartexport()
{
	uint64_t pos = m_pos - m_start;
	if (pos < m_stopexport)
	{
		m_startexport = pos;
		SetTime(m_StrStartExport, m_startexport);
		UpdateData(false);
	}
}


void RecordedDataDlg::OnBnClickedBtSetendexport()
{
	uint64_t pos = m_pos - m_start;
	if (pos > m_startexport)
	{
		m_stopexport = pos;
		SetTime(m_StrEndExport, m_stopexport);
		UpdateData(false);
	}
}


void RecordedDataDlg::OnBnClickedBtExportnow()
{
	UpdateData(); // read from controls

	CExport dlg;
	dlg.m_pSource = m_psource;
	dlg.m_start = m_startexport;
	dlg.m_stop = m_stopexport;
	dlg.DoModal();
}
