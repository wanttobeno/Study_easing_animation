// Win32_MemDC.cpp : Defines the entry point for the application.
//


#include "stdafx.h"
#include "Win32_MemDC.h"
#include "MemDcUsage.h"

#include <string>
#include <sstream>

#define MAX_LOADSTRING 100
const UINT_PTR  k_animate     = 2011;

/* Global Variables **********************************************************/
HINSTANCE hInst;								          // current instance
TCHAR szTitle[MAX_LOADSTRING];					  // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

bool            g_isDragging      = false;
bool            g_isRightClick    = false;
POINT           g_startPoint      = {0,0};
POINT           g_adjustedPoint   = {0,0};
long            g_adjustmentDiff  = 0;

/* Forward Declarations ******************************************************/
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
  ::LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
  ::LoadString(hInstance, IDC_WIN32_MEMDC, szWindowClass, MAX_LOADSTRING);
  MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

  hAccelTable = ::LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32_MEMDC));

	// Main message loop:
  while (::GetMessage(&msg, NULL, 0, 0))
	{
    if (!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
		}
	}

  // Free demonstration resources.
  article::Term();

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
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
  wcex.hIcon			= ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32_MEMDC));
  wcex.hCursor		= ::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WIN32_MEMDC);
	wcex.lpszClassName	= szWindowClass;
  wcex.hIconSm		= ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

  return ::RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = ::CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   // Initialize state of the demonstration code.
   RECT client;
   ::GetClientRect(hWnd, &client);
   article::Init( client.right - client.left,
                  client.bottom - client.top);

   ::ShowWindow(hWnd, nCmdShow);
   ::UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
  // Create an destroy messages.
  case WM_CREATE:
    {
      // Create a time to run at the specified fps.
      ::SetTimer(hWnd, k_animate, article::GetFrameRateDelay(), NULL);
    }
    break;

  case WM_DESTROY:
    ::PostQuitMessage(0);
    break;

  // Basic Command Handling.
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
      ::DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
      ::DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

  // Handle User input.
  case WM_RBUTTONDOWN:
    ::SetCapture(hWnd);
    g_isRightClick = true;
    break;
  case WM_RBUTTONUP:
    if (g_isRightClick)
    {
      POINT pt = {LOWORD(lParam), HIWORD(lParam)};
      RECT  client;
      ::GetClientRect(hWnd, &client);
      if (::PtInRect(&client, pt))
      {
        ::ReleaseCapture();
        // Toggle the use of the back buffer.
        article::EnableBackBuffer(!article::IsBackBufferEnabled());
      }
    }

    g_isRightClick = false;
    break;
  case WM_LBUTTONDOWN:
    ::SetCapture(hWnd);

    // Mark the starting point of the drag operation.
    g_startPoint.x    = LOWORD(lParam);
    g_startPoint.y    = HIWORD(lParam);

    g_adjustedPoint   = g_startPoint;
    g_adjustmentDiff  = 0;

    g_isDragging      = true;
    break;
  case WM_LBUTTONUP:
    ::ReleaseCapture();
    g_isDragging = false;
    break;
  case WM_MOUSEMOVE:
    if (g_isDragging)
    {
      // Move the position of the Highlight based on the users drag actions.
      int curX   = LOWORD(lParam);
      int offset = curX - (g_adjustedPoint.x - g_adjustmentDiff);

      int appliedOffset = article::AdjustAnimation(hWnd, offset);

      // Adjust the starting point by the applied offset
      // to keep the movements of the mouse relative to the animation.
      g_adjustmentDiff   = offset - appliedOffset;
      g_adjustedPoint.x += appliedOffset;
    }
    break;

  // Handle Window Size changes.
  case WM_SIZE:
  case WM_SIZING:
    // Flush the back buffer, 
    // The size of the image that needs to be painted has changed.
    article::FlushBackBuffer();

    // Allow the default processing to take care of the rest.
    return DefWindowProc(hWnd, message, wParam, lParam);

  case WM_TIMER:
    {
      if (k_animate == wParam)
      {
        // Skip processing if the user is in dragging mode.
        if (!g_isDragging)
        {
          article::StepAnimation(hWnd);
        }
      }
    }
    break;

  case WM_ERASEBKGND:
    // Disable background erase functionality.
    // This will be taken care of in the WM_PAINT message.
    // The PAINTSTRUCT::fErase value will be TRUE by returning a non-zero value.
    return 1;

	case WM_PAINT:
    {
      hdc = ::BeginPaint(hWnd, &ps);

      article::PaintAnimation(hWnd, hdc);

      ::EndPaint(hWnd, &ps);

      break;
    }
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
      ::EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}




