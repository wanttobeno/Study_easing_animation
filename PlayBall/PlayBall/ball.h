// 定义球类ball
#ifndef __BALL
#define __BALL
#include <Windows.h>

class ball
{
private:
	POINT pos;		// 圆心位置
	unsigned int r;	// 半径长度
	COLORREF col;	// 颜色
	int id;			// 唯一标识此球
public:
	ball();
	BOOL init(POINT pos,unsigned int r, COLORREF col);	// 对象初始化

	// 基本属性操作函数
	POINT SetPos(POINT newPos);							// 设置新位置为newPos，返回oldPos;
	POINT GetPos();										// 返回pos;
	unsigned int SetR(unsigned int newR);				// 设置新半径，返回旧半径
	unsigned int GetR();								// 返回半径R
	COLORREF SetCol(COLORREF newCol);					// 设置新颜色，返回旧颜色
	COLORREF GetCol();									// 返回颜色col
	void SetId(int id);					
	int GetId();
	// 行为表现函数
	BOOL draw(HDC hdc,COLORREF col,bool isErase);		// 将此球用hdc画出
};
#endif
