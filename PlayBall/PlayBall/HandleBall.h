

//定义新类型操作一些球以达到动画的效果
/*
***************************坐标系如下****************************
1、x轴正方向	——右
2、y轴正方向	——下
3、角度正方向	——顺时针
*/
#ifndef __HANDLE_BALL
#define __HANDLE_BALL

#include <Windows.h>

#define BALLNUMBER 30		// 被操作的球数目

class ball;

class HandleBall
{
private:
	ball *m_pBalls[BALLNUMBER];		// 指向球类的指针数组
	int m_BallAngles[BALLNUMBER];		// 对应球移动的偏移角度
	HWND m_hWnd;						// 窗口句柄

	int m_MoveLen;					// 球移动的位移量
	int m_ball_r;						// 各球的半径

	//	私有函数定义
	int Reflect(int aSrc,int aRef);													// 返回aSrc方向按照aRef方向反射之后的方向 
	void MoveOneBall(ball* pBall,int Angle,int len);								// 按照给定的角度Angle,给定的位移len移动给定的球pBall;
	bool isHitWall(ball* pBall,int bdAngle,int len,RECT rt,int *Angle,POINT *Pos);	// 球pBall向前移动len长度是否与rt四壁相撞，如果相撞则Angle中存放的是反射回的方向，Pos中存放的是球恰与墙相撞时的圆心位置
	int Bisector(int a1,int a2);													// 将a1、a2按照物理规则进行合成之后返回
	bool isTwoBallHit(ball* pActive,ball* pPassive,int *aActive,int *aPassive);		// 考察主动球pActive是否与被动球pPassive相撞，相撞则将两球撞后方向分别填入aActive和aPassive之中
	//	int DirectTwoPoint(POINT pt_src,POINT pt_dest);									// 返回以pt_src为起点，指向pt_dest的射线方向
public:		
	HandleBall(){;}								//
	bool init(HWND hWnd,int ball_r,int MoveLen);		// 初始化函数完成成员变量的初始化以及相应对象的创建
	~HandleBall();										// 完成垃圾清理工作
	void Show(COLORREF col,bool isErase);				// 如果isErase为真则用col显示所有的球（可以用来擦除）,否则col无效
	void Move(int mLen,bool UseDefault);				// 移动此对象所有的球，如果UseDefault为真则用对象内部的移动位移量，为假则用mLen
	int DirectTwoPoint(POINT pt_src,POINT pt_dest);
};
#endif