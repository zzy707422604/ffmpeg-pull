
// ffmpegDlg.cpp : implementation file
//

#define __STDC_CONSTANT_MACROS
#include "stdafx.h"
#include "ffmpeg.h"
#include "ffmpegDlg.h"
#include "afxdialogex.h"
#include "vld.h"
//#include <string>
#ifdef _WIN32
//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
#include "SDL/SDL.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
//#include "SDL/SDL_main.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <SDL/SDL.h>
#include <SDL/SDL_main.h>
#ifdef __cplusplus
};
#endif
#endif
#pragma comment(lib ,"SDL2.lib")
//#pragma comment(lib ,"SDL2main.lib")
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CffmpegDlg dialog



CffmpegDlg::CffmpegDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CffmpegDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CffmpegDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CffmpegDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CffmpegDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CffmpegDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CffmpegDlg message handlers

BOOL CffmpegDlg::OnInitDialog()
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CffmpegDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CffmpegDlg::OnPaint()
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
HCURSOR CffmpegDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CffmpegDlg::OnBnClickedOk()
{
	AVFormatContext    *pFormatCtx;
	int                i, videoindex;
	AVCodecContext    *pCodecCtx;
	AVCodec            *pCodec;
	//char filepath[]="F:\\Work\\ffmpegdemo\\Debug\\Wildlife.wmv";
	char rtspUrl[] = "rtsp://192.168.1.110:554/2420725677251317_video.sdp+123456";
	av_register_all();//注册组件
	avformat_network_init();//支持网络流
	pFormatCtx = avformat_alloc_context();//初始化AVFormatContext
	if (avformat_open_input(&pFormatCtx,/*filepath*/rtspUrl, NULL, NULL) != 0){//打开文件或网络流
		printf("无法打开文件\n");
		return;
	}
	if (avformat_find_stream_info(pFormatCtx, NULL)<0)//查找流信息
	{
		printf("Couldn't find stream information.\n");
		return;
	}
	videoindex = -1;
	for (i = 0; i<pFormatCtx->nb_streams; i++) //获取视频流ID
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}
	if (videoindex == -1)
	{
		printf("Didn't find a video stream.\n");
		return;
	}
	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);//查找解码器
	if (pCodec == NULL)
	{
		printf("Codec not found.\n");
		return;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL)<0)//打开解码器
	{
		printf("Could not open codec.\n");
		return;
	}
	AVFrame *pFrame, *pFrameYUV;
	pFrame = av_frame_alloc();//存储解码后AVFrame
	pFrameYUV = av_frame_alloc();//存储转换后AVFrame
	uint8_t *out_buffer;
	out_buffer = new uint8_t[avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height)];//分配AVFrame所需内存
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);//填充AVFrame

	//------------SDL初始化--------

	CRect CRect;//获取当前的对话框大小
	GetWindowRect(&CRect);
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		return;
	}
	SDL_Window *screen = SDL_CreateWindowFrom(m_hWnd);
	if (!screen) {
		printf("SDL: could not set video mode - exiting\n");
		return;
	}
	SDL_Renderer* sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
	SDL_Texture* sdlTexture = SDL_CreateTexture(
		sdlRenderer,
		SDL_PIXELFORMAT_IYUV,
		SDL_TEXTUREACCESS_STREAMING,
		pCodecCtx->width,
		pCodecCtx->height);

	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = CRect.Width();
	rect.h = CRect.Height();
	//-----------------------------
	int ret, got_picture;
	static struct SwsContext *img_convert_ctx;
	int y_size = pCodecCtx->width * pCodecCtx->height;

	SDL_Event event;
	AVPacket *packet = (AVPacket *)malloc(sizeof(AVPacket));//存储解码前数据包AVPacket
	av_new_packet(packet, y_size);
	//输出一下信息-----------------------------
	printf("文件信息-----------------------------------------\n");
	//av_dump_format(pFormatCtx,0,filepath,0);
	printf("-------------------------------------------------\n");
	//------------------------------
	while (av_read_frame(pFormatCtx, packet) >= 0)//循环获取压缩数据包AVPacket
	{
		if (packet->stream_index == videoindex)
		{
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);//解码。输入为AVPacket，输出为AVFrame
			if (ret < 0)
			{
				printf("解码错误\n");
				return;
			}
			if (got_picture)
			{
				//像素格式转换。pFrame转换为pFrameYUV。
				img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
				sws_freeContext(img_convert_ctx);
				//------------SDL显示--------
				/*rect.x = 0;
				rect.y = 0;
				rect.w = pCodecCtx->width;
				rect.h = pCodecCtx->height;*/

				SDL_UpdateTexture(sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
				SDL_RenderClear(sdlRenderer);
				SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
				SDL_RenderPresent(sdlRenderer);
				//延时20ms
				SDL_Delay(20);
				//------------SDL-----------
			}
		}
		av_free_packet(packet);
		SDL_PollEvent(&event);
		switch (event.type) {
		case SDL_QUIT:
			SDL_Quit();
			exit(0);
			break;
		default:
			break;
		}
	}

	SDL_DestroyTexture(sdlTexture);
	delete[] out_buffer;
	av_free(pFrameYUV);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	CDialogEx::OnOK();
}

	


void CffmpegDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}
