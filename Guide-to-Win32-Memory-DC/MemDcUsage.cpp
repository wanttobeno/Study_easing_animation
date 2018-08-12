/* MemDcUsage.cpp *************************************************************
Author      Paul Watt
Date:       7/3/2011 11:52:48 AM
Purpose:    Demonstration code for the usage or Win32 Memory DCs.
            Create an animated gradient sequence that is 1/4 of the screen width
            and scroll across the screen in 5 seconds at ~30 fps.
            
Copyright 2011 Paul Watt
******************************************************************************/
/* Includes ******************************************************************/
#include <stdafx.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <list>

#include "MemDcUsage.h"

/* Linker Directives *********************************************************/
#pragma comment( lib, "Msimg32" )   // Lib required for Gradient Fill

namespace // anonymous
{

/* Declarations **************************************************************/
struct RectInfo
{
  double x;
  double y;
  double cx;
  double cy;
  HBRUSH hBrush;
};

typedef std::list<RectInfo> RectList;
typedef RectList::iterator  RectListIter;

/* Forward Declarations ******************************************************/
void CreateAnimationClipRgns(const RECT&, const RECT&, HRGN&, HRGN&);
void DrawWiper(HDC hDC, const RECT &rc, COLORREF baseColor, COLORREF hiColor);
void DrawShapes(HDC, HRGN, double, double, RectList&, bool isEllipse);

void UpdateFpsDisplay();
void GetRectangleGroup(const UINT , const UINT , const UINT , RectList&);
void GetRandomRectangle(const UINT width, const UINT height, RectInfo &randomRect);
HBITMAP GetWiperAlphaBlendMask(HDC hDC, const RECT &rc, COLORREF, COLORREF);

/* Constants *****************************************************************/
const double   k_percentWidth       = 0.33;
const double   k_aniToWindowRatio   = 1.0 / (1 + (2 * k_percentWidth));
const int      k_rectCount          = 10;

const COLORREF k_white                  = RGB(0xFF, 0xFF, 0xFF);
const COLORREF k_gray                   = RGB(0xED, 0xED, 0xED);
const COLORREF k_black                  = RGB(0x00, 0x00, 0x00);
const COLORREF k_codeProjectDarkOrange  = RGB(0xFF, 0x99, 0x00);
const COLORREF k_codeProjectLightOrange = RGB(0xFF, 0xE2, 0xA8);
const COLORREF k_codeProjectDarkGreen   = RGB(0x48, 0x8E, 0x00);
const COLORREF k_codeProjectLightGreen  = RGB(0x76, 0xAB, 0x40);

const char     k_instructions[]     = " -  Hold Left Mouse Button down on window to pause screen updates.\n"
                                      " -  Drag with Left Mouse Button to move the Wiper right and left.\n"
                                      " -  Right Click to toggle painting mode.  Wiper color indicates mode:\n"
                                      "        Green:  Paint indirectly using a double buffer\n"
                                      "        Orange: Paint directly to the display";

/* Global Variable ***********************************************************/
bool        g_useBackBuffer = true;   // Indicates if the back buffer is used or not.
HDC         g_hMemDc      = NULL;     // Memory DC for the back buffer.
HBITMAP     g_hBackBuffer = NULL;     // Back buffer bitmap.

int         g_cur         = 0;        // Current offset from the beginning of the
                                      // animation.

ULONGLONG   g_lastTime    = 0;        // Time marker for use in FPS calculations.
UINT        g_frameCount  = 0;        // Current frame count for this second.
UINT        g_desiredRate = 75;       // Track the desired frame-rate.
UINT        g_desiredLen  = 5;        // The desired length in seconds of the animation.
UINT        g_steps       = g_desiredRate * g_desiredLen; // count of steps in the sequence

double      g_currentFps  = 0.0;      // Cache the last calculated FPS until
                                      // the next rate is calculated.

RectList    g_group1;                 // list of random rectangles 
RectList    g_group2;                 // to make the display a bit more complicated.
bool        g_isGroup1Active = true;  // Status for which group paints as ellipses.

HPEN        g_hPenBlack      = NULL;
HBRUSH      g_hBrGray        = NULL;
HBRUSH      g_hBrLightOrange = NULL;
HBRUSH      g_hBrDarkOrange  = NULL;
HBRUSH      g_hBrLightGreen  = NULL;
HBRUSH      g_hBrDarkGreen   = NULL;

HBITMAP     g_hWiperMask     = NULL;

}

// Namespace to contain functions implemented for the demonstration.
namespace article
{

/******************************************************************************
Date:       7/4/2011
Purpose:    Initialize any data required for the demonstration.
Parameters: width[in]: Every generated rectangle will be smaller and fit 
              within the boundary width.  
            height[in]: Every generated rectangle will be smaller and fit 
              within the boundary height.  
*******************************************************************************/
void Init(const UINT width, const UINT height)
{
  ::srand(::GetTickCount());

  g_hPenBlack       = ::CreatePen(PS_SOLID, 3, k_black);
  g_hBrGray         = ::CreateSolidBrush(k_gray);
  g_hBrLightOrange  = ::CreateSolidBrush(k_codeProjectLightOrange);
  g_hBrDarkOrange   = ::CreateSolidBrush(k_codeProjectDarkOrange);
  g_hBrLightGreen   = ::CreateSolidBrush(k_codeProjectLightGreen);
  g_hBrDarkOrange   = ::CreateSolidBrush(k_codeProjectDarkGreen);

  GetRectangleGroup(width, height, k_rectCount, g_group1);
  GetRectangleGroup(width, height, k_rectCount, g_group2);
}

/******************************************************************************
Date:       7/4/2011
Purpose:    Free resources from the demo app.
*******************************************************************************/
void Term()
{
  // Free STL Resources
  g_group1.clear();
  g_group2.clear();

  // Free GDI Resources.
  ::DeleteObject(g_hPenBlack);
  ::DeleteObject(g_hBrGray);
  ::DeleteObject(g_hBrLightOrange);
  ::DeleteObject(g_hBrDarkOrange);
  ::DeleteObject(g_hBrLightGreen);
  ::DeleteObject(g_hBrDarkGreen);

  g_hPenBlack      = NULL;
  g_hBrGray        = NULL;
  g_hBrLightOrange = NULL;
  g_hBrDarkOrange  = NULL;
  g_hBrLightGreen  = NULL;
  g_hBrDarkGreen   = NULL;

  FlushBackBuffer();
}

/******************************************************************************
Date:       7/4/2011
Purpose:    Calculates and returns the delay in between update frames.
*******************************************************************************/
UINT  GetFrameRateDelay()
{
  return static_cast<UINT>(1000.0 / g_desiredRate);
}

/******************************************************************************
Date:       7/3/2011
Purpose:    Sets a flag that enables back-buffer painting with the memory DC.
Parameters: isEnable[in]: true will enable back buffer painting
                          false will disable back buffer painting and paint
                            directly to the DC passed in by the user.
*******************************************************************************/
void EnableBackBuffer(bool isEnable)
{
  if (g_useBackBuffer != isEnable)
  {
    g_useBackBuffer = isEnable;
    FlushBackBuffer();
  }
  
}

/******************************************************************************
Date:       7/3/2011
Purpose:    Reports if the back buffer is being used.
*******************************************************************************/
bool IsBackBufferEnabled()
{
  return g_useBackBuffer;
}

/******************************************************************************
Date:       7/3/2011
Purpose:    Clears any state stored in the back buffer, 
            including releasing allocated resources.
*******************************************************************************/
void FlushBackBuffer()
{
  if (g_hMemDc)
  {
    ::DeleteDC(g_hMemDc);
    g_hMemDc = NULL;
  }

  if (g_hBackBuffer)
  {
    ::DeleteObject(g_hBackBuffer);
    g_hBackBuffer = NULL;
  }

  if (g_hWiperMask)
  {
    ::DeleteObject(g_hWiperMask);
    g_hWiperMask = NULL;
  }
}

/******************************************************************************
Date:       7/3/2011
Purpose:    Insures the target windows back buffer is initialized, and returns
            the handle to the caller.
Parameters: hWnd[in]: Handle to the window which back buffer painting will be used.
Return:     Initialized back buffer memory DC.
            This is a cached resource.
            A call to Term or FlushBackBuffer will release this resource.
******************************************************************************/
HDC GetBackBuffer(HWND hWnd)
{
  // !!! Create and return a back-buffer for double-buffer painting.
  // If the memory DC is already initialized, simply return.
  if (g_hMemDc)
  {
    return g_hMemDc;
  }
  
  // Initialize the memory DC and a back buffer bitmap to write on.
  RECT client;
  ::GetClientRect(hWnd, &client);

  int width = client.right - client.left;
  int height= client.bottom - client.top;

  // Get a DC from the target paint location to be sure a compatible 
  // drawing context is created.
  HDC hDc  = ::GetDC(hWnd);

  // Create the Memory DC to be compatible with the target window.
  g_hMemDc = ::CreateCompatibleDC(hDc);

  // It is important the BITMAP be created to be compatible as well.
  // This will insure the correct bit-color depth and other parameters
  // match the target DC.
  g_hBackBuffer = ::CreateCompatibleBitmap(hDc, width, height);

  // Select the bitmap into the memory DC.
  // Otherwise there will be no buffer for the the operations to write into.
  ::SelectObject(g_hMemDc, g_hBackBuffer);

  // Release handles to target window 
  ::ReleaseDC(hWnd, hDc);

  return g_hMemDc;
}

/******************************************************************************
Date:       7/4/2011
Purpose:    Adjusts the offset of the current position of the highlight animation.
Parameters: hWnd[in]: The handle to the window with the animation.
            offset[in]: Offset in pixels.  Value can be positive or negative.
              Any values that are beyond the width of the window will be ignored.
Return:     The amount of offset that is accepted and applied to the animation.
*******************************************************************************/
int AdjustAnimation(HWND hWnd, int offset)
{
  // Determine the number of pixels in each animation step in window coordinates.
  RECT client;
  ::GetClientRect(hWnd, &client);

  int width       = client.right - client.left;
  int hiWidth     = static_cast<int>(width * k_percentWidth);
  int aniRange    = width + hiWidth;
  int stepWidth   = aniRange / g_steps;

  // Calculate the number of steps to offset.
  int offsetCount = offset / stepWidth;

  g_cur += offsetCount;

  // Force an update of the display.
  ::InvalidateRect(hWnd, NULL, FALSE);
 
  return offsetCount * stepWidth;
}

/******************************************************************************
Date:       7/3/2011
Purpose:    Update the animation to the next frame.
*******************************************************************************/
void StepAnimation(HWND hWnd)
{
  // Advance to the next step, 
  // when the animation reaches the end, start over.
  g_cur = ((g_cur + 1) % g_steps);

  // If the Wiper is starting back on the left side of the screen, swap
  // rectangle groups, and regenerate the second group.
  if (0 == g_cur)
  {
    std::swap(g_group1, g_group2);
    g_group2.clear();

    RECT client;
    ::GetClientRect(hWnd, &client);
    UINT width = client.right - client.left;
    UINT height= client.bottom - client.top;

    GetRectangleGroup(width, height, k_rectCount, g_group2);

    g_isGroup1Active = !g_isGroup1Active;
  }

  // Force an update of the display.
  ::InvalidateRect(hWnd, NULL, FALSE);
}

/******************************************************************************
Date:       7/3/2011
Purpose:    Primary Demonstration function for Memory DC usage.
Parameters: hWnd
            hDC
*******************************************************************************/
void PaintAnimation(HWND hWnd, HDC hDC)
{
  RECT client;
  ::GetClientRect(hWnd, &client);

  // The total width of the animation will be: 
  //    100% of the window width + (2*animation width)  -> 150%
  // Calculate an appropriate offset 
  // based on the current position of the animation.
  double width    = client.right - client.left;
  double height   = client.bottom - client.top;
  double hiWidth  = width * k_percentWidth;
  double aniWidth = width + (2 * hiWidth); 

  // The range of the offset for the wiper is (-aniWidth Width to 1.0 Width)
  //    -: 
  //    H: Highlight
  //    X: Offscreen
  // 
  //    Start, No Visible Highlight:      H---X
  //    Highlight Visible, Ani 33%:       XH--X
  //    Highlight Visible, Ani 66%:       X-H-X
  //    Highlight Visible, Ani 100%:      X--HX
  //    End, Highlight no longer Visible: X---H
  //
  double  offsetBase  = -hiWidth;
  double  offsetRange = aniWidth - hiWidth;
  double  stepWidth   = offsetRange / g_steps;
  int     offsetX     = static_cast<int>((stepWidth * g_cur) + offsetBase); 

  RECT rc =
  {
    offsetX,
    client.top,
    static_cast<LONG>(offsetX + hiWidth),
    client.bottom
  };

  HDC hBufferDC = hDC;

  COLORREF foreColor = k_codeProjectDarkOrange;
  COLORREF aftColor  = k_codeProjectDarkGreen;
  if (g_useBackBuffer)
  {
    hBufferDC = GetBackBuffer(hWnd);
    std::swap(foreColor, aftColor);
  }

  // Clear the canvas back to solid white.
  ::FillRect(hBufferDC, &client, (HBRUSH)::GetStockObject(WHITE_BRUSH));

  // Create two clipping regions divided by the wiper, 
  // and paint each set of shapes on one side of the wiper.
  HRGN hLeftRgn  = NULL;
  HRGN hRightRgn = NULL;
  CreateAnimationClipRgns(client, rc, hLeftRgn, hRightRgn);

  //!!! Save DC / Restore DC Sample
  int ctx = ::SaveDC(hDC);

  // Draw the set of rectangles.
  ::SelectObject(hBufferDC, g_hPenBlack);
  ::SelectObject(hBufferDC, ::GetStockObject(WHITE_BRUSH));

  // Draw Right Side:
  DrawShapes(hBufferDC, hRightRgn, width, height, g_group1, g_isGroup1Active);
  // Draw Left Side:
  DrawShapes(hBufferDC, hLeftRgn, width, height, g_group2, !g_isGroup1Active);

  ::RestoreDC(hDC, ctx);

  ::DeleteObject(hRightRgn);
  ::DeleteObject(hLeftRgn);

  // Draw the highlight gradient.
  DrawWiper(hBufferDC, rc, foreColor, aftColor);

  // Perform status output.
  UpdateFpsDisplay();

  // Set up to draw text with a transparent background.
  ::SetBkMode(hBufferDC, TRANSPARENT);

  // Display input instructions.
  ::DrawTextA(hBufferDC, k_instructions, -1, &client, DT_LEFT | DT_TOP);

  // Display the frame rate.
  std::stringstream fpsText;
  fpsText.flags(std::ios_base::fixed);
  fpsText.precision(3);
  fpsText << "FrameRate: " << g_currentFps << " FPS";
  ::DrawTextA(hBufferDC, fpsText.str().c_str(), -1, &client, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE | DT_EXPANDTABS);

  if (g_useBackBuffer)
  {
    //!!! This is the point where the Back-Buffer is drawn to the window canvas.
    // Blt to the actual window now.
    // BitBlt: Bit Block Transfer
    ::BitBlt( hDC,          // Destination DC
              client.left,  // Starting X offset on destination to paint
              client.top,   // Starting Y offset on destination to paint
              (int)width,   // Width of image to paint onto destination
              (int)height,  // Height of image to paint onto destination
              hBufferDC,    // Source DC
              0,            // Starting X off on the source to paint from
              0,            // Starting Y off on the source to paint from
              SRCCOPY);     // Raster Operation (ROP) code, Simple Copy
  }
}

} // namespace article

/* Anonymous Namespace Implementation ****************************************/
namespace // anonymous
{

/******************************************************************************
Date:       7/4/2011
Purpose:    Calculate the Frame rate that is currently displaying.
*******************************************************************************/
void UpdateFpsDisplay()
{
  g_frameCount++;

  ULONGLONG now = ::GetTickCount64();  
  if (now > g_lastTime + 1000)
  {
    double diff   = static_cast<double>(now - g_lastTime) 
                  / 1000.0;

    g_currentFps  = static_cast<double>(g_frameCount)
                  / diff;

    g_frameCount = 0;
    g_lastTime   = now;
  }
}

/******************************************************************************
Date:       7/9/2011
Purpose:    Creates the pair of clipping regions required for the animation.
Parameters: client[in]: The bounding rectangle for the entire animation.
            rc[in]: The bounding rectangle for the wiper that divides the screen.
            left[out]: A reference to the region handle to accept the left side.
              The caller is responsible to free this resource.
            right[out]: A reference to the region handle to accept the right side.
              The caller is responsible to free this resource.
*******************************************************************************/
void CreateAnimationClipRgns(const RECT &client, 
                             const RECT &rc, 
                             HRGN &hLeftRgn, 
                             HRGN &hRightRgn)
{
  //!!! Create two regions that are mutually exclusive over one larger region
  hLeftRgn  = ::CreateRectRgnIndirect(&client);

  // The region will be divided from the lower-left of the wiper,
  // to the upper-right of the wiper rectangle area.
  int segment = (rc.right - rc.left) / 3;
  int left    = rc.left  + segment;
  int right   = rc.right - segment;
  POINT rightAreaPts[4];
  rightAreaPts[0].x = left;
  rightAreaPts[0].y = rc.bottom;
  rightAreaPts[1].x = right;
  rightAreaPts[1].y = rc.top;
  rightAreaPts[2].x = client.right;
  rightAreaPts[2].y = client.top;
  rightAreaPts[3].x = client.right;
  rightAreaPts[3].y = client.bottom;

  hRightRgn = ::CreatePolygonRgn(rightAreaPts, 4, ALTERNATE);
  ::CombineRgn(hLeftRgn, hLeftRgn, hRightRgn, RGN_DIFF);
}

/******************************************************************************
Date:       7/4/2011
Purpose:    Generate a randomly placed and sized rectangle within the specified
            boundaries.
Parameters: width[in]: Every generated rectangle will be smaller and fit 
              within the boundary width.  
            height[in]: Every generated rectangle will be smaller and fit 
              within the boundary height.  
            count[in]: The number of rectangles to generate.
            randomRect[out]: The random rectangle output.
*******************************************************************************/
void GetRectangleGroup(const UINT width, 
                       const UINT height, 
                       const UINT count, 
                       RectList &list)
{
  // Generate randomly placed and sized rectangles.
  for (UINT i = 0; i < count; ++i)
  {
    RectInfo randInfo;
    GetRandomRectangle(width, height, randInfo);

    list.push_back(randInfo);
  }
}

/******************************************************************************
Date:       7/4/2011
Purpose:    Generate a randomly placed and sized rectangle within the specified
            boundaries.
Parameters: width[in]: Every generated rectangle will be smaller and fit 
              within the boundary width.  
            height[in]: Every generated rectangle will be smaller and fit 
              within the boundary height.  
            randomRect[out]: The random rectangle output.
*******************************************************************************/
void GetRandomRectangle(const UINT width, const UINT height, RectInfo &randomRect)
{
  // Calculate the Width and Height of the rectangle.
  double rectWidth  = ::rand() % (width / 2 - 1);
  double rectHeight = ::rand() % (height/ 2 - 1);
  
  // Calculate the placement.
  double range  = width - rectWidth;
  double domain = height- rectHeight;

  randomRect.x   = (::rand() % (int)(width  - 1)) / (double)width;
  randomRect.y   = (::rand() % (int)(height - 1)) / (double)height;
  randomRect.cx  = rectWidth / width;
  randomRect.cy  = rectHeight/ height;

  // Select a random color for the rectangle.
  int colorIdx = ::rand() % 5;
  switch(colorIdx)
  {
  case 0:
    randomRect.hBrush = g_hBrGray;
    break;
  case 1:
    randomRect.hBrush = g_hBrLightOrange;
    break;
  case 2:
    randomRect.hBrush = g_hBrDarkOrange;
    break;
  case 3:
    randomRect.hBrush = g_hBrLightGreen;
    break;
  case 4:
    randomRect.hBrush = g_hBrDarkGreen;
    break;
  default:
    randomRect.hBrush = (HBRUSH)::GetStockObject(WHITE_BRUSH);
    break;
  }
}

/******************************************************************************
Date:       7/6/2011
Purpose:    Draw a group of rectangles on the specified DC and possible clip rgn.
Parameters: hDC[in]:  Device context to paint upon.
            hClipRgn[in]: Clipping region, can be NULL.
            width[in]: Width of the area to paint in.
            height[in]: Height of the area to paint in.
            list[in]: List of rectangle info structures that contains the 
              information to draw each of the rectangles.
            isEllipse[in]: Indicates if ellipses should be drawn.  
              Otherwise rectangles will be used.
*******************************************************************************/
void DrawShapes(HDC hDC, 
                    HRGN hClipRgn, 
                    double width, 
                    double height, 
                    RectList &list,
                    bool isEllipse)
{
  int ctx = ::SaveDC(hDC);

  //!!! Set the clipping region, and draw
  ::SelectClipRgn(hDC, hClipRgn);
  RectListIter cur = list.begin();
  RectListIter end = list.end();
  for (; cur != end; ++cur)
  {
    ::SelectObject(hDC, cur->hBrush);
    if (isEllipse)
    {
      ::Ellipse(hDC, 
        (int)(cur->x * width), 
        (int)(cur->y * height), 
        (int)(cur->cx* width), 
        (int)(cur->cy* height));
    }
    else
    {
      ::Rectangle(hDC, 
        (int)(cur->x * width), 
        (int)(cur->y * height), 
        (int)(cur->cx* width), 
        (int)(cur->cy* height));
    }
  }

  ::RestoreDC(hDC, ctx);
}

/******************************************************************************
Date:       7/3/2011
Purpose:    Draw a gradient filled highlight in a specified rectangular area
            with a base and highlight color.
Parameters: hDC[in]:  
            rc[in]: 
            baseColor[in]:  
            hiColor[in]:  
******************************************************************************/
void DrawWiper(HDC hDC, const RECT &rc, COLORREF baseColor, COLORREF hiColor)
{ 
  int bmpWidth = rc.right - rc.left;
  int bmpHeight= rc.bottom - rc.top;

  HDC     hDCMem     = ::CreateCompatibleDC(hDC);
  HBITMAP hBlendMask = GetWiperAlphaBlendMask(hDC, rc, baseColor, hiColor);
  
  ::SelectObject(hDCMem, hBlendMask);

  //!!! Sample use of the blend function
  // Blend the two images, where:
  BLENDFUNCTION bFn;
  bFn.BlendOp = AC_SRC_OVER;
  bFn.BlendFlags = 0;
  bFn.SourceConstantAlpha = 0xFF; // The src constant blend will be fully opaque
  bFn.AlphaFormat = AC_SRC_ALPHA; // The alpha info is encode for each pixel.

  ::AlphaBlend( hDC, 
                rc.left, rc.top, 
                bmpWidth,
                bmpHeight,
                hDCMem,
                0,0,
                bmpWidth,
                bmpHeight,
                bFn);

  ::DeleteDC(hDCMem);
}

/* Gradient MACROS ***********************************************************/
//!!! MACROs to make formatting of color values for Gradient Fill more convenient.
// Convenience Macros to format the extract color parts into a 16-bit
// value which the Gradient fill structures require. GetXValue returns a byte.
#define GRADIENT_R_VAL(color) (GetRValue(color) << 8)
#define GRADIENT_G_VAL(color) (GetGValue(color) << 8)
#define GRADIENT_B_VAL(color) (GetBValue(color) << 8)


/******************************************************************************
Date:       7/8/2011
Purpose:    Functor to populate vertices of the wiper.
******************************************************************************/
template <int T>
struct VertexPopulator
{
  void operator()(TRIVERTEX v[T], 
    POINT pts[T],
    bool isOpaque[T], 
    COLORREF c1, 
    COLORREF c2)
  {
    for (int i = 0; i < T; ++i)
    {
      COLORREF  color = isOpaque[i] 
                      ? c1
                      : c2;
      USHORT    mask  = isOpaque[i] 
                      ? 0xFF00
                      : 0x0000;


      v[i].x      = pts[i].x;
      v[i].y      = pts[i].y;
      v[i].Red    = GRADIENT_R_VAL(color);
      v[i].Green  = GRADIENT_G_VAL(color);
      v[i].Blue   = GRADIENT_B_VAL(color);
      v[i].Alpha  = mask;
    }
  }
};

/******************************************************************************
Date:       7/8/2011
Purpose:    Use the gradientFill function to create a Alpha Blending mask
            for painting the transition wiper on the screen.
Parameters: hDC[in]: DC which the bitmap mask should be compatible with.
            rc[in]:  The bounding rectangle to create the wiper within.
Return:     A handle to a bitmap that contains the wiper alpha blend mask.
            The bitmap will be compatible with the DC that is passed in.
            This resource is cached for efficiency.
            A call to Term or FlushBackBuffer will Release the bitmap.
            NULL will be returned if the call fails.
******************************************************************************/
HBITMAP GetWiperAlphaBlendMask(HDC hDC, const RECT &rc, COLORREF color1, COLORREF color2)
{
  // If the Wiper mask already exists, return it now.
  if (g_hWiperMask)
  {
    return g_hWiperMask; 
  }

  VertexPopulator<8> populateVertices;

  // Adjust the rectangle to start from the origin (0,0).
  // This will simplify calculations, and reduce memory requirements 
  // for memory buffer that is created.
  RECT orgRect = rc;
  ::OffsetRect(&orgRect, -orgRect.left, 0);

  // Break the input rect into 3 parts horizontally to keep the highlight
  // gradient smooth, and within the specified boundaries.
  int width         = orgRect.right;    // rect is aligned at 0, right edge is width.
  int height        = orgRect.bottom;
  int segmentWidth  = width / 3;

  int left          = 0;
  int leftHilight   = left + segmentWidth;
  int rightHilight  = leftHilight + segmentWidth;
  int right         = left + width;   // which should also be: orgRect.right

  // There are 4 triangle surfaces to make a smooth transition from the background
  // color of white, to color A, switch to color B then transition back to white.
  // Vertices 1 and 4 are the same point with two different color definitions.
  // Vertices 2 and 7 are the same point with two different color definitions.
  //
  //    L 0-1  4-5
  //     /|/  /|/
  //    3-2  7-6 R
  //  
  //   L        The original left coordinate in the rectangle.
  //   R        The original right coordinate in the rectangle.
  //
  //  (0,1,6,2) or (0,4,6,7) or ...
  //            Creates the rectangle where the main highlight will be displayed.
  //  (4,5,6)   Creates the fade in transition.
  //  (0,2,3)   Creates the fade out transition.

  GRADIENT_TRIANGLE surface[4];
  surface[0].Vertex1 = 0;
  surface[0].Vertex2 = 1;
  surface[0].Vertex3 = 2;

  surface[1].Vertex1 = 0;
  surface[1].Vertex2 = 2;
  surface[1].Vertex3 = 3;

  surface[2].Vertex1 = 4;
  surface[2].Vertex2 = 5;
  surface[2].Vertex3 = 6;

  surface[3].Vertex1 = 4;
  surface[3].Vertex2 = 6;
  surface[3].Vertex3 = 7;

  // Interesting points, index mapping documented above.
  POINT pts[8] =
  {
    {leftHilight,   orgRect.top},
    {rightHilight,  orgRect.top},
    {leftHilight,   orgRect.bottom},
    {left,          orgRect.bottom},
    {rightHilight,  orgRect.top},
    {right,         orgRect.top},
    {rightHilight,  orgRect.bottom},
    {leftHilight,   orgRect.bottom}
  };

  // Mapping to indicate which vertices represent opaque points on the mask.
  bool maskOpaque[8] = {false, true, true, false, true, false, false, true};

  // Populate the vertex list for the Alpha mask.
  TRIVERTEX  mask[8];
  populateVertices(mask, pts, maskOpaque, color1, k_black);

  HDC hDCMask       = ::CreateCompatibleDC(hDC);
  HBITMAP hBmpMask  = ::CreateCompatibleBitmap(hDC, width, height);
  ::SelectObject(hDCMask, hBmpMask);

  ::BitBlt(hDCMask, 0, 0, width, height, hDC, orgRect.left, orgRect.top, BLACKNESS);
  ::GradientFill(hDCMask, mask, 8, &surface, 4, GRADIENT_FILL_TRIANGLE);

  // Delete the Memory DC, this will release its hold on the mask bitmap.
  ::DeleteDC(hDCMask);

  // Store the handle into the global location.
  g_hWiperMask = hBmpMask;

  return hBmpMask;
}



} // namespace anonymous

