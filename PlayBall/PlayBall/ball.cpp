#include "ball.h"

#define COL_RED (RGB(255,0,0))

// ball类成员函数实现
ball::ball()
{
	this->col=COL_RED;
	this->pos.x=0;
	this->pos.y=0;
	this->r=0;
}

BOOL ball::init(POINT pos,unsigned int r, COLORREF col)
{
	this->pos=pos;
	this->r=r;
	this->col=col;
	return TRUE;
}
POINT ball::SetPos(POINT newPos)
{
	POINT tmpPos=this->pos;
	this->pos=newPos;
	return tmpPos;
}
POINT ball::GetPos()
{
	return this->pos;
}

unsigned int ball::SetR(unsigned int newR)
{
	unsigned int tempR=this->r;
	this->r=newR;
	return tempR;
}
unsigned int ball::GetR()
{
	return this->r;
}

COLORREF ball::SetCol(COLORREF newCol)
{
	COLORREF tempCol=this->col;
	this->col=newCol;
	return tempCol;
}
COLORREF ball::GetCol()
{
	return this->col;
}

void ball::SetId(int id)
{
	this->id=id;
}

int ball::GetId()
{
	return this->id;
}

BOOL ball::draw(HDC hdc,COLORREF col,bool isErase)
{
	BOOL bSuccess;

	HPEN hNewPen,hOldPen;
	HBRUSH hNewBrush,hOldBrush;

	// 如果不进行擦除，则用此球对象中存放的颜色创建画刷
	if(false==isErase)
		col=this->col;

	hNewPen=CreatePen(PS_NULL,0,0);
	hNewBrush=CreateSolidBrush(col);

	hOldPen=(HPEN) SelectObject(hdc,hNewPen);
	hOldBrush=(HBRUSH) SelectObject(hdc,hNewBrush);
	bSuccess=Ellipse(hdc,this->pos.x-this->r,this->pos.y-this->r,this->pos.x+this->r,this->pos.y+this->r);
	if(FALSE==bSuccess)
		return FALSE;

	SelectObject(hdc,hOldPen);
	SelectObject(hdc,hOldBrush);

	DeleteObject(hNewBrush);
	DeleteObject(hNewPen);
	return TRUE;
}