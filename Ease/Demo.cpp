
#include <stdio.h>
#include <Windows.h>
#include "resource.h"
#include "Easing.h"





#include <commctrl.h>
#pragma comment(lib,"comctl32.lib")

HWND g_hwndImg;
HWND g_hwndInfo;

float current_frame = 0.0;

// 数值越小速度越快，
float end_frame = 20;

float start_value = 10.0;
float change_value = 300.0;

const int EASE_COUNT = 22;
int nType = 0;


void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	current_frame++;
	if (current_frame <= end_frame)
	{

		char buf[128] = {0};
		sprintf(buf,"当前动画类型%s ，帧数%d/%d",GetEaseName((EASE_TYPE)nType),(int)current_frame,(int)end_frame);
		
		SetWindowTextA(g_hwndInfo,buf);


		float fValue2 = doEase((EASE_TYPE)nType,current_frame, start_value, end_frame, change_value);
		::SetWindowPos(g_hwndImg, HWND_TOP, (int)fValue2, 10, 0, 0, SWP_NOSIZE);
		::InvalidateRect(hwnd, NULL, true);
		::UpdateWindow(hwnd);
	}
}

LRESULT CALLBACK Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		g_hwndImg = GetDlgItem(hDlg, IDC_IMG);
		g_hwndInfo = GetDlgItem(hDlg,IDC_INFO);
		return TRUE;
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
			case IDCANCEL:
				EndDialog(hDlg, LOWORD(wParam));
				break;
			case IDC_BTN_START:
				{


					KillTimer(hDlg, 1000);
					//nType = rand() % EASE_COUNT;
					nType = (nType + 1) % EASE_COUNT;
					current_frame = 0;
					SetTimer(g_hwndImg, 1000, 20, TimerProc);
					break;
				}
			case  IDC_BTN_STOP:
				{
					KillTimer(hDlg, 1000);
					break;
				}
			case IDC_BTN_REFUSH:
				{
					::InvalidateRect(hDlg, NULL, FALSE);
					::UpdateWindow(hDlg);
					break;
				}
			default:
				break;
			}
			return DefWindowProc(hDlg, message, wParam, lParam);
		}
		break;
	case WM_CLOSE:
		{
			EndDialog(hDlg, wParam);
			return TRUE;
		}
		break;

	}
	return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	InitCommonControls();
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)Proc);
	return 0;
}