#include <windows.h>
#include "resource.h"
#include "Handleball.h"


#define COL_BLACK (RGB(0,0,0))


#define MAX_LOADSTRING 100
#define MIN_WIDTH	550
#define MIN_HEIGHT  400

#define TIMER_ID  100			// 计时器ID号
#define BALL_R	  10			// 球的半径
#define MOVE_LEN  5				// 球移动距离

#define TIMER_INTERVAL	20		// 计时器时间间隔
#define TRANS_TINTERVAL 10		// 玻璃效果计数器时间间隔

// 各timerID
#define ID_BALL_MOVE	100		// 球运动计数器ID
#define ID_TRANS_UP		101		// 淡入
#define ID_TRANS_DOWN	102		// 淡出
#define ID_AUTORUN		103		// 自动运行

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名
TCHAR szEvent[MAX_LOADSTRING];					// 命名事件对象名称，用于检测程序运行的其他实例
TCHAR szActiveExist[MAX_LOADSTRING];			// 自定义消息名称
TCHAR szBgMusic[MAX_LOADSTRING];				// 背景音乐文件名

UINT UM_ACTIVEEXIST=NULL;						// 自定义消息，当检测到有此程序的其他实例在运行时，向其窗口发送此消息

HBITMAP g_hbmpBg=NULL;							// 背景位图句柄
HBITMAP g_hbmpDream=NULL;							// 
BITMAP  g_BgbmpInfo;								// 背景位图信息
BITMAP	g_DreamBmpInfo;		
HMENU g_hMenu=NULL;								// 菜单句柄


bool g_isInit=true;							    // 是否还正处于初始化中，Init函数将其设置为true，而其他的函数则将它设置为false，OnPaint根据此画图
bool g_isStart=false;								// 为true表明执行了start
bool g_isStay=false;								// 为true表明执行了stay
bool g_isFullScreen=false;						// 是否全屏

bool g_needContinue=false;						// 当最小化窗口时设置，恢复时根据此重新启动
bool g_canShowBall=false;							// 是否可以在屏幕显示小球	
bool g_isTimerExist=false;						// TIMER存在与否
HandleBall g_HandleBallInst;						// 对象实例

int g_TransUp=0;								// 窗体透明度
int g_TransDown=255;							


// 此代码模块中包含的函数的前向声明:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void SetMenuItem(HWND hWnd);
void Init(HWND hWnd,HandleBall *pHandleBall);			// 初始化函数
void StartOrStop(HWND hWnd,HandleBall *pHandleBall);	// 开始与停止
void StayOrMove(HWND hWnd,HandleBall *pHandleBall);		// 暂停与继续		
BOOL OnSizing(HWND hWnd ,WPARAM wParam ,LPARAM lParam,LONG MinHeight,LONG MinWidth);
bool FullScreen(HWND hWnd ,RECT *pRect,LONG *pStyle,bool isEnterFull);	
BOOL OnSize(HWND hWnd,WPARAM wParam,LPARAM lParam);

//	相应timer函数，本窗体多timer此函数一一处理
void OnTimer(HWND hWnd,WPARAM wParam ,LPARAM lParam);

// 保证程序运行实例唯一性的函数声明
DWORD WINAPI WaitForExit(LPVOID lpParameter);
bool isExist();

// 加载背景音乐并写入文件
bool LoadAndWriteMIDFile(HMODULE hModule,LPWSTR lpFileName);

//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
//  注释:
//
//    仅当希望
//    此代码与添加到 Windows 95 中的“RegisterClassEx”
//    函数之前的 Win32 系统兼容时，才需要此函数及其用法。调用此函数十分重要，
//    这样应用程序就可以获得关联的
//    “格式正确的”小图标。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MOVEBALL));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= CreateSolidBrush(COL_BLACK);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDR_MENU1);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // 将实例句柄存储在全局变量中

	hWnd = CreateWindowEx(WS_EX_LAYERED,szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}
	// 
	SetLayeredWindowAttributes(hWnd,RGB(255,0,0),0,LWA_COLORKEY | LWA_ALPHA);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	HDC hCompatibleDC;			// 兼容DC
	RECT WndRect,ClientRect;
	HMENU hSubMenu;

	static LONG s_WndStyle;		// 保存窗体样式
	static RECT s_WndRt;			// 保存窗体矩形区域

	if(message==UM_ACTIVEEXIST)
	{
		SetLayeredWindowAttributes(hWnd,RGB(255,0,0),0,LWA_COLORKEY | LWA_ALPHA);
		SetTimer(hWnd,ID_TRANS_UP,TRANS_TINTERVAL,NULL);
		ShowWindow(hWnd,SW_SHOWNORMAL);
		return 1;
	}
	switch (message)
	{
	case WM_CREATE:

		SetTimer(hWnd,ID_TRANS_UP,TRANS_TINTERVAL,NULL);
		// 操作菜单
		g_hMenu=GetMenu(hWnd);
		hSubMenu=GetSubMenu(g_hMenu,0);
		for(int i=1;i<9;i++)
			EnableMenuItem(hSubMenu,i,MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		SetMenuDefaultItem(hSubMenu,0,TRUE);

		// 调整窗体位置
		GetWindowRect(hWnd,&WndRect);
		SetWindowPos(hWnd,0,GetSystemMetrics(SM_CXSCREEN)/2-(WndRect.right-WndRect.left)/2,GetSystemMetrics(SM_CYSCREEN)/2-(WndRect.bottom-WndRect.top)/2,0,0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

		// 初始化对象
		Init(hWnd,&g_HandleBallInst);

		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
		case IDM_EXIT:
			SetTimer(hWnd,ID_TRANS_DOWN,TRANS_TINTERVAL,NULL);
			break;
		case ID_INIT:	// 初始化
			Init(hWnd,&g_HandleBallInst);
			break;
		case ID_START:	// 启动
			StartOrStop(hWnd,&g_HandleBallInst);
			break;
		case ID_STOP:	// 停止
			StartOrStop(hWnd,&g_HandleBallInst);
			break;
		case ID_PAUSE:	// 暂停
			StayOrMove(hWnd,&g_HandleBallInst);	
			break;
		case ID_CONTINUE: // 继续
			StayOrMove(hWnd,&g_HandleBallInst);	
			break;
		case ID_FULLSCREEN:			// 全屏显示
			ModifyMenu(GetSubMenu(g_hMenu,0),ID_FULLSCREEN,MF_BYCOMMAND | MF_STRING ,ID_NORMALSCREEN,L"退出全屏(&X)\tF5");
			// 隐藏菜单
			SetMenu(hWnd,NULL);
			// 调用函数进入全屏
			FullScreen(hWnd,&s_WndRt,&s_WndStyle,true);
			g_isFullScreen=true;
			break;
		case ID_NORMALSCREEN:		// 退出全屏			
			ModifyMenu(GetSubMenu(g_hMenu,0),ID_NORMALSCREEN,MF_BYCOMMAND | MF_STRING ,ID_FULLSCREEN,L"全屏显示(&F)\tF5");
			// 调用函数退出全屏
			FullScreen(hWnd,&s_WndRt,&s_WndStyle,false);
			// 显示菜单
			SetMenu(hWnd,g_hMenu);

			g_isFullScreen=false;
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此添加任意绘图代码...
		if(g_isInit && (hCompatibleDC=CreateCompatibleDC(hdc)))
		{
			// 处于初始化之中则显示背景图片
			GetClientRect(hWnd,&ClientRect);
			SelectObject(hCompatibleDC,g_hbmpBg);
			BitBlt(hdc,(ClientRect.right+ClientRect.left)/2-g_BgbmpInfo.bmWidth/2,(ClientRect.bottom+ClientRect.top)/2-g_BgbmpInfo.bmHeight/2,g_BgbmpInfo.bmWidth,g_BgbmpInfo.bmHeight,hCompatibleDC,0,0,SRCCOPY);
			DeleteDC(hCompatibleDC);
		}
		if(g_canShowBall)
			g_HandleBallInst.Show(0,false);
		if(!g_isInit && !g_isStart && (hCompatibleDC=CreateCompatibleDC(hdc)))
		{
			// 非初始化之中且执行了停止动作
			GetClientRect(hWnd,&ClientRect);
			SelectObject(hCompatibleDC,g_hbmpDream);
			BitBlt(hdc,(ClientRect.right+ClientRect.left)/2-g_DreamBmpInfo.bmWidth/2,(ClientRect.bottom+ClientRect.top)/2-g_DreamBmpInfo.bmHeight/2,g_DreamBmpInfo.bmWidth,g_DreamBmpInfo.bmHeight,hCompatibleDC,0,0,SRCCOPY);
			DeleteDC(hCompatibleDC);
		}
		EndPaint(hWnd, &ps);
		break;
	case WM_RBUTTONDOWN:
		// 创建快捷菜单
		POINT pt;
		pt.x=MAKEPOINTS(lParam).x;
		pt.y=MAKEPOINTS(lParam).y;
		ClientToScreen(hWnd,&pt);
		TrackPopupMenu(GetSubMenu(g_hMenu,0),
			TPM_TOPALIGN | TPM_LEFTALIGN,									// 标记位
			pt.x,pt.y,0,hWnd,0);
		break;
	case WM_TIMER:
		// 交给timer函数统一处理
		OnTimer(hWnd,wParam,lParam);
		break;
	case WM_CHAR:
		switch(wParam)
		{
		case 0x20:	// 空格
			StayOrMove(hWnd,&g_HandleBallInst);	
			break;
		case 0x0D:  // 回车
			StartOrStop(hWnd,&g_HandleBallInst);
			break;
		case 0x1B:
			Init(hWnd,&g_HandleBallInst);
			break;
		}
		break;
	case WM_KEYDOWN:	
		// 响应F5功能键
		if(wParam==VK_F5)
		{
			if(g_isFullScreen)
			{
				// 当前处于全屏，应退出全屏
				SendMessage(hWnd,WM_COMMAND,(WPARAM)ID_NORMALSCREEN,(LPARAM)0);
				return 0;
			}
			else 
			{
				// 当前不是全屏，应进入全屏
				SendMessage(hWnd,WM_COMMAND,(WPARAM)ID_FULLSCREEN,(LPARAM)0);
				return 0;
			}
		}
		break;
	case WM_SIZE:
		OnSize(hWnd,wParam,lParam);
		break;
	case WM_SIZING:
		return OnSizing(hWnd,wParam,lParam,MIN_HEIGHT ,MIN_WIDTH);
		break;
	case WM_CLOSE:
		SetTimer(hWnd,ID_TRANS_DOWN,TRANS_TINTERVAL,NULL);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void Init(HWND hWnd,HandleBall *pHandleBall)
{
	if(g_isTimerExist)
		KillTimer(hWnd,ID_BALL_MOVE);

	pHandleBall->init(hWnd,BALL_R,MOVE_LEN);
	InvalidateRect(hWnd,NULL,TRUE);
	g_isInit=true;
	g_canShowBall=false;
	g_isTimerExist=false;
	SetMenuItem(hWnd);

	SetTimer(hWnd,ID_AUTORUN,3000,NULL);
}

void StartOrStop(HWND hWnd,HandleBall *pHandleBall)
{
	if(g_isInit || (!g_isInit && !g_isStart))		// 执行了初始化，或者初始化被抵消但没有执行Start
	{	
		// 开始
		// 设置timer并将canShowBall设置为true
		//		InvalidateRect(hWnd,NULL,TRUE);
		pHandleBall->init(hWnd,BALL_R,MOVE_LEN);

		SetTimer(hWnd,ID_BALL_MOVE,TIMER_INTERVAL,NULL);
		g_canShowBall=false;
		g_isStart=true;
		g_isTimerExist=true;
		g_isInit=false;
		g_isStay=false;
	}
	else if(!g_isInit && g_isStart)			// 非初始化且执行了Start
	{
		// 停止
		KillTimer(hWnd,ID_BALL_MOVE);
		InvalidateRect(hWnd,NULL,TRUE);

		g_canShowBall=false;
		g_isStart=false;
		g_isTimerExist=false;
		g_isInit=false;
		g_isStay=false;
	}
	SetMenuItem(hWnd);
}

void StayOrMove(HWND hWnd,HandleBall *pHandleBall)
{
	if(!g_isInit && g_isStart && !g_isStay)		// 非初始化且执行了Start且没有执行Stay
	{
		KillTimer(hWnd,ID_BALL_MOVE);
		g_canShowBall=true;
		g_isStay=true;
		g_isTimerExist=false;
		g_isInit=false;
	}
	else if(!g_isInit && g_isStart && g_isStay)
	{
		SetTimer(hWnd,ID_BALL_MOVE,TIMER_INTERVAL,NULL);
		g_canShowBall=false;
		g_isStay=false;
		g_isTimerExist=true;
		g_isInit=false;
	}
	SetMenuItem(hWnd);
}

BOOL OnSizing(HWND hWnd ,WPARAM wParam ,LPARAM lParam,LONG MinHeight,LONG MinWidth)
{
	LONG NewWidth,NewHeight;
	RECT* pRect;
	// lParam is a Pointer point a RECT structure;
	// So we can get the Width and Height as follow
	if(wParam!=WMSZ_BOTTOM 
		&& wParam!=WMSZ_BOTTOMLEFT
		&& wParam!=WMSZ_BOTTOMRIGHT
		&& wParam!=WMSZ_LEFT
		&& wParam!=WMSZ_RIGHT
		&& wParam!=WMSZ_TOP
		&& wParam!=WMSZ_TOPLEFT
		&& wParam!=WMSZ_TOPRIGHT)
		return FALSE;

	NewWidth=((RECT*)lParam)->right-((RECT*)lParam)->left;
	NewHeight=((RECT*)lParam)->bottom-((RECT*)lParam)->top;

	pRect=(RECT*)lParam;

	switch(wParam)
	{
	case WMSZ_BOTTOM:
		if(NewHeight<MinHeight)
		{
			pRect->bottom=pRect->top+MinHeight;
			return TRUE;
		}
		break;
	case WMSZ_BOTTOMLEFT:
		if(NewWidth<MinWidth || NewHeight<MinHeight)
		{
			if(NewWidth<MinWidth)
				pRect->left=pRect->right-MinWidth;
			if(NewHeight<MinHeight)
				pRect->bottom=pRect->top+MinHeight;
			return TRUE;
		}
		break;
	case WMSZ_BOTTOMRIGHT:
		if(NewWidth<MinWidth || NewHeight < MinHeight)
		{
			if(NewWidth<MinWidth)
				pRect->right=pRect->left+MinWidth;
			if(NewHeight < MinHeight)
				pRect->bottom=pRect->top+MinHeight;
			return TRUE;
		}
		break;
	case WMSZ_LEFT:
		if(NewWidth < MinWidth)
		{
			pRect->left=pRect->right-MinWidth;
			return TRUE;
		}
		break;
	case WMSZ_RIGHT:
		if(NewWidth < MinWidth)
		{
			pRect->right=pRect->left+MinWidth;
			return TRUE;
		}
		break;
	case WMSZ_TOP:
		if(NewHeight < MinHeight)
		{
			pRect->top=pRect->bottom-MinHeight;
			return TRUE;
		}
		break;
	case WMSZ_TOPLEFT:
		if(NewWidth < MinWidth || NewHeight < MinHeight)
		{
			if(NewWidth < MinWidth)
				pRect->left=pRect->right-MinWidth;
			if(NewHeight < MinHeight)
				pRect->top=pRect->bottom-MinHeight;	
			return TRUE;
		}
		break;
	case WMSZ_TOPRIGHT:
		if(NewWidth < MinWidth || NewHeight < MinHeight)
		{
			if(NewWidth < MinWidth)
				pRect->right=pRect->left+MinWidth;
			if(NewHeight < MinHeight)
				pRect->top=pRect->bottom-MinHeight;
			return TRUE;
		}
		break;
	default:
		return FALSE;
	}
	return FALSE;
}

BOOL OnSize(HWND hWnd,WPARAM wParam,LPARAM lParam)
{
	switch(wParam)
	{
	case SIZE_RESTORED:		// 从最小化恢复
		SetLayeredWindowAttributes(hWnd,RGB(255,0,0),0,LWA_COLORKEY | LWA_ALPHA);
		SetTimer(hWnd,ID_TRANS_UP,TRANS_TINTERVAL,NULL);
		if(g_needContinue)
		{
			SendMessage(hWnd,WM_COMMAND,(WPARAM)ID_CONTINUE,(LPARAM)0);
			g_needContinue=false;
		}
		return true;
	case SIZE_MINIMIZED:
		if(!g_isInit && g_isStart && !g_isStay)
		{
			//正在运行中，则将其暂停，且设置标准needContinue
			SendMessage(hWnd,WM_COMMAND,(WPARAM)ID_PAUSE,(LPARAM)0);
			g_needContinue=true;
		}
		return true;
	case SIZE_MAXIMIZED:
		SetLayeredWindowAttributes(hWnd,RGB(255,0,0),0,LWA_COLORKEY | LWA_ALPHA);
		SetTimer(hWnd,ID_TRANS_UP,TRANS_TINTERVAL,NULL);
		return true;

	}
	return false;
}

void SetMenuItem(HWND hWnd)
{
	HMENU hSubMenu;
	hSubMenu=GetSubMenu(g_hMenu,0);

	if(g_isInit)
	{
		// 处于初始化状态
		EnableMenuItem(hSubMenu,ID_INIT,MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);		// 初始化-disabled

		EnableMenuItem(hSubMenu,ID_START,MF_BYCOMMAND | MF_ENABLED);					// 启动-enabled
		EnableMenuItem(hSubMenu,ID_STOP,MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);		// 停止-disabled

		EnableMenuItem(hSubMenu,ID_PAUSE,MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);		// 暂停-disabled
		EnableMenuItem(hSubMenu,ID_CONTINUE,MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);	// 继续-enabled
	}
	else if(!g_isInit && g_isStart && !g_isStay)
	{
		// 正处于启动运行状态
		EnableMenuItem(hSubMenu,ID_INIT,MF_BYCOMMAND | MF_ENABLED);					

		EnableMenuItem(hSubMenu,ID_START,MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);		
		EnableMenuItem(hSubMenu,ID_STOP,MF_BYCOMMAND | MF_ENABLED);						

		EnableMenuItem(hSubMenu,ID_PAUSE,MF_BYCOMMAND | MF_ENABLED);		
		EnableMenuItem(hSubMenu,ID_CONTINUE,MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);	
	}
	else if(!g_isInit && g_isStart && g_isStay)
	{
		// 正处于启动暂停状态
		EnableMenuItem(hSubMenu,ID_INIT,MF_BYCOMMAND | MF_ENABLED);					

		EnableMenuItem(hSubMenu,ID_START,MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);		
		EnableMenuItem(hSubMenu,ID_STOP,MF_BYCOMMAND | MF_ENABLED);						

		EnableMenuItem(hSubMenu,ID_PAUSE,MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);		
		EnableMenuItem(hSubMenu,ID_CONTINUE,MF_BYCOMMAND | MF_ENABLED);	
	}
	else if(!g_isInit && !g_isStart)
	{
		// 处于停止状态
		EnableMenuItem(hSubMenu,ID_INIT,MF_BYCOMMAND | MF_ENABLED);					// 初始化-disabled

		EnableMenuItem(hSubMenu,ID_START,MF_BYCOMMAND | MF_ENABLED);					// 启动-enabled
		EnableMenuItem(hSubMenu,ID_STOP,MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);		// 停止-disabled

		EnableMenuItem(hSubMenu,ID_PAUSE,MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);		// 暂停-disabled
		EnableMenuItem(hSubMenu,ID_CONTINUE,MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);	// 继续-
	}
}

/*
对hWnd操作，根据isEnterFull将窗体设置为全屏或者退出全屏
isEnterFull=true;		// 设为全屏
isEnterFull=false;		// 退出全屏

当isEnterFull为真时，pRect返回正常窗口大小，pStyle返回正常的窗体样式。（为返回正常窗体，调用者必须保存此参数）
当isEnterFull为假时，pRect用于传入正常窗口大小，pStyle传入正常的窗体样式。
*/
bool FullScreen(HWND hWnd ,RECT *pRect,LONG *pStyle,bool isEnterFull)
{
	LONG tmpStyle;
	int sx,sy;						// 全屏宽和高


	if(isEnterFull)					// 进入全屏
	{
		// 保存数据
		GetWindowRect(hWnd,pRect);
		*pStyle=GetWindowLong(hWnd,GWL_STYLE);

		tmpStyle=*pStyle;
		tmpStyle&=~WS_BORDER;		// 边框
		tmpStyle&=~WS_CAPTION;		// 标题栏
		tmpStyle&=~WS_SIZEBOX;		// 不可推拉

		sx=GetSystemMetrics(SM_CXSCREEN);
		sy=GetSystemMetrics(SM_CYSCREEN);

		SetWindowLong(hWnd,GWL_STYLE,tmpStyle);
		SetWindowPos(hWnd,HWND_TOPMOST,0,0,sx,sy,SWP_SHOWWINDOW);

		return true;
	}
	else if(!isEnterFull)			// 退出全屏
	{
		SetWindowLong(hWnd,GWL_STYLE,*pStyle);
		SetWindowPos(hWnd,HWND_NOTOPMOST,
			pRect->left,	
			pRect->top,
			pRect->right-pRect->left,
			pRect->bottom-pRect->top,
			SWP_SHOWWINDOW);
	}
	return false;
}
// 
void OnTimer(HWND hWnd,WPARAM wParam ,LPARAM lParam)
{
	static bool isFirst=true;
	if(isFirst)
	{
		g_TransUp=0;
		g_TransDown=255;
		isFirst=false;
	}
	switch(wParam)
	{
	case ID_BALL_MOVE:			// 球运动
		g_HandleBallInst.Show(COL_BLACK,true);
		g_HandleBallInst.Move(10,false);
		g_HandleBallInst.Show(0,false);
		break;
	case ID_TRANS_UP:			// 淡入
		if(g_TransUp<=255)
		{
			SetLayeredWindowAttributes(hWnd,RGB(255,0,0),g_TransUp,LWA_COLORKEY | LWA_ALPHA);
			g_TransUp+=3;
		}
		else 
		{
			KillTimer(hWnd,ID_TRANS_UP);
			isFirst=true;
		}

		break;	
	case ID_TRANS_DOWN:			// 淡出
		if(g_TransDown>=0)
		{
			SetLayeredWindowAttributes(hWnd,RGB(255,0,0),g_TransDown,LWA_COLORKEY | LWA_ALPHA);
			g_TransDown-=3;
		}
		else
		{
			KillTimer(hWnd,ID_TRANS_DOWN);
			PostQuitMessage(0);
		}
		break;
	case ID_AUTORUN:		// 自动运行
		// 发送自动运行消息并销毁时间控件
		SendMessage(hWnd,WM_COMMAND,(WPARAM)ID_START,(LPARAM)0);	
		KillTimer(hWnd,ID_AUTORUN);
		break;
	}
}

/*
是否有此程序的实例正在运行，
存在
且能够得到其窗体句柄则发送消息将其激活
否则将其关闭
不存在
则返回false
*/ 
bool isExist()
{
	HANDLE hEvent;
	HWND hWnd;
	if((hEvent=CreateEvent(NULL,TRUE,FALSE,szEvent))		// 创建一个人工设置 ， 初始为无信号的事件
		&& GetLastError()==ERROR_ALREADY_EXISTS)
	{
		// 如果此程序有实例，则试图得到其窗口句柄
		if(hWnd=FindWindow(szWindowClass,szTitle))
		{
			SendNotifyMessage(hWnd,UM_ACTIVEEXIST,(WPARAM)0,(LPARAM)0);
			return true;
		}
		else 
		{
			// 得到窗体句柄失败，则将其关闭
			SetEvent(hEvent);
			return false;
		}
	}
	CreateThread(NULL,0,WaitForExit,(LPVOID)hEvent,0,NULL);
	return false;
}

/*
此线程只是一直等待
直到lpParameter所标示的事件有信号
则将当前进程关闭
*/
DWORD WINAPI WaitForExit(LPVOID lpParameter)
{
	WaitForSingleObject((HANDLE)lpParameter,INFINITE);
	TerminateProcess(GetCurrentProcess(),0);
	return 0;
}

bool LoadAndWriteMIDFile(HMODULE hModule,LPWSTR lpFileName)
{
	HRSRC hRsrc;
	HGLOBAL hGlobal;
	LPVOID lpMidAddr=0;			// 保存mid资源在内存位置
	HANDLE hFile;
	DWORD dwRet;

	// 加载资源并得到内存地址
	hRsrc=FindResource(hModule,MAKEINTRESOURCE(IDR_MID),L"MID");
	if(!hRsrc)
		return false;
	ULONG FileLen = ::SizeofResource(NULL,hRsrc);  // Data size/length  
	hGlobal=LoadResource(hModule,hRsrc);
	if(!hGlobal)
		return false;

	lpMidAddr=LockResource(hGlobal);
	if(!lpMidAddr)
		return false;

	hFile=CreateFile(lpFileName,GENERIC_READ | GENERIC_WRITE ,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(!hFile)
		return false;
	bool bRet = false;
	WriteFile(hFile,lpMidAddr,FileLen,&dwRet,NULL);
	if( dwRet == FileLen)
		bRet = true;
	CloseHandle(hFile);
	return bRet;
}


int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd) 
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MOVEBALL, szWindowClass, MAX_LOADSTRING);
	LoadString(hInstance, IDS_EVENT, szEvent,MAX_LOADSTRING);
	LoadString(hInstance, IDS_ACTIVEEXIST,szActiveExist,MAX_LOADSTRING);
	LoadString(hInstance, IDS_BGMUSIC,szBgMusic,MAX_LOADSTRING);

	// 注册自定义消息
	UM_ACTIVEEXIST=RegisterWindowMessage(szActiveExist);

	// 检测是否有其他实例正在运行
	if(isExist())
		return 0;	// 如果存在则返回

	// 准备背景音乐
	LoadAndWriteMIDFile(hInstance,szBgMusic);

	MyRegisterClass(hInstance);

	// 加载位图
	if(g_hbmpBg=LoadBitmap(hInstance,MAKEINTRESOURCEW(IDB_BG)))
	{
		GetObject(g_hbmpBg,sizeof(g_BgbmpInfo),&g_BgbmpInfo);
	}
	if(g_hbmpDream=LoadBitmap(hInstance,MAKEINTRESOURCEW(IDB_DREAM)))
	{
		GetObject(g_hbmpDream,sizeof(g_DreamBmpInfo),&g_DreamBmpInfo);
	}


	// 执行应用程序初始化:
	if (!InitInstance (hInstance, nShowCmd))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MOVEBALL));

	// 主消息循环:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	if(g_hbmpBg && DeleteObject(g_hbmpBg))
		g_hbmpBg=NULL;
	if(g_hbmpDream && DeleteObject(g_hbmpDream))
		g_hbmpDream=NULL;

	return (int) msg.wParam;
}


