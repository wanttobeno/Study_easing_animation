/* MemDcUsage.h ***************************************************************
Author     Paul Watt
Date:      7/3/2011 11:54:11 AM
Purpose:   Demonstration code for the usage or Win32 Memory DCs.
Copyright 2011 Paul Watt
******************************************************************************/
#ifndef MEMDCUSAGE_H_INCLUDED
#define MEMDCUSAGE_H_INCLUDED
/* Includes ******************************************************************/

namespace article
{

/* Function Declarations *****************************************************/
void Init(const UINT width, const UINT height);
void Term();

UINT GetFrameRateDelay();
void EnableBackBuffer(bool isEnable);
bool IsBackBufferEnabled();
void FlushBackBuffer();
int  AdjustAnimation(HWND hWnd, int offset);
void StepAnimation(HWND hWnd);
void PaintAnimation(HWND hWnd, HDC hDc);

}

#endif // MEMDCUSAGE_H_INCLUDED
