// RecordedDataDlg.cpp : implementation file
//

#include "pch.h"
#include "StreamerViewer.h"
#include "RecordedDataDlg.h"
#include "afxdialogex.h"


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
	, m_pos(0)
	, m_pbuf(nullptr)
{

}

RecordedDataDlg::~RecordedDataDlg()
{
}

void RecordedDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PRG_PLAY, m_PrgPlay);
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
	if (nullptr == m_psource)
		return TRUE;
	m_psource->GetRecordedRange(m_count, m_start, m_stop);
	m_pos = m_start;
	m_psource->GetRecordImageAt(&m_pbuf, m_pos);
	m_PrgPlay.SetRange32(0, m_stop - m_start);
	m_PrgPlay.SetPos(0);

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

	m_playType = 0b0111;
}
void RecordedDataDlg::OnBnClickedBtRecfback()
{
	if (!(m_playType & 0x01))
		SetTimer(1, 40, NULL);			// system is stopped

	m_playType = 0b01111;
}

void RecordedDataDlg::OnBnClickedBtRecpause()
{
	if (!(m_playType & 0x01))
		KillTimer(1);
	m_playType = 0;
}
void RecordedDataDlg::OnBnClickedBtRecstop()
{
	if (!(m_playType & 0x01))
		KillTimer(1);
	m_pos = m_start;
	m_psource->GetRecordImageAt(&m_pbuf, m_pos);
	m_PrgPlay.SetPos(0);
	m_playType = 0;
	// todo draw image
}
void RecordedDataDlg::OnBnClickedBtRecstepf()
{
	if (!(m_playType & 0x01))
		KillTimer(1);
	m_playType = 0;
	if (m_psource->GetRecordedImageNext(&m_pbuf, m_pos))
	{
		m_PrgPlay.SetPos(m_pos - m_start);
		// todo draw image
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
	int step;
	switch (speed)
	{
	case 0: step = 10;
		break;
	case 1: step = 100;
		break;
	case 2: step = 400;
		break;
	case 3: step = 1000;
		break;
	}
	if (m_playType & 0b01000) // reverse
		speed = (-1) * speed;

	if (m_psource->GetRecordedImageNextEx(&m_pbuf, speed, m_pos))
	{
		m_PrgPlay.SetPos(m_pos - m_start);
		// todo draw image
	}
	else
	{
		//we reach the end
		KillTimer(1);
		m_playType = 0;
	}
}










