#include "Handleball.h"
#include "ball.h"
#include <math.h>



#define COL_WHITE (RGB(255,255,255))
#define COL_BLUE (RGB(0,0,255))
#define COL_GREEN (RGB(0,255,0))


#define PI	3.1415926		// pi定义
#define AngToRad(x)	(((float)(x))/180*PI)			// 角度制转化为弧度制
#define RadToAng(x) (double(x)/PI*180)			    // 弧度制转化为角度制
#define DISTANCE(pt1,pt2) sqrt((float)(pt1.x-pt2.x)*(pt1.x-pt2.x)+(pt1.y-pt2.y)*(pt1.y-pt2.y))	   // 计算两点之间的距离 


/*==================HandleBall类成员函数的实现==================*/
/*********************公有成员函数实现部分***********************/
bool HandleBall::init(HWND hWnd,int ball_r,int MoveLen)
{
	int i;
	POINT pt;							// 临时保存各球中心位置坐标
	RECT rt;							// 保存窗体矩形区域

	SYSTEMTIME stime;
	GetSystemTime(&stime);
	WORD wCount=stime.wMilliseconds%100;
	while(wCount--)
		rand();				// 以系统时间的微秒数为种子初始化rand函数，从而保证每一次得到的都不是一样的布局

	this->m_hWnd=hWnd;
	this->m_ball_r=ball_r;
	this->m_MoveLen=MoveLen;

	// 得到窗体大小
	GetClientRect(hWnd,&rt);

	for(i=0;i<BALLNUMBER;i++)
	{
		// 生成球i以及其相应的移动方向角度
		this->m_pBalls[i]=new ball();
		this->m_BallAngles[i]=rand()%360;

		// 初始化球i：
		pt.x=(LONG)((abs(rand())%(int)(rt.right-2*ball_r))+ball_r);
		pt.y=(LONG)((abs(rand())%(int)(rt.bottom-2*ball_r))+ball_r);

		this->m_pBalls[i]->init(pt,ball_r,RGB(rand()%255,rand()%255,rand()%255));
		this->m_pBalls[i]->SetId(i);					// 设置此球唯一标识
	}
	return true;
}

HandleBall::~HandleBall()
{
	for(int i=0;i<BALLNUMBER;i++)
		delete this->m_pBalls[i];
}

void HandleBall::Show(COLORREF col,bool isErase)
{
	int i;
	HDC hdc=GetDC(m_hWnd);
	if(false==isErase)
	{	for(i=0;i<BALLNUMBER;i++)
			this->m_pBalls[i]->draw(hdc,0,false);	
	}
	else
	{
		for(i=0;i<BALLNUMBER;i++)
			this->m_pBalls[i]->draw(hdc,col,true);
	}
	ReleaseDC(m_hWnd,hdc);
}

void HandleBall::Move(int mLen, bool UseDefault)
{
	if(true==UseDefault)
		mLen=this->m_MoveLen;
	for(int i=0;i<BALLNUMBER;i++)
		this->MoveOneBall(this->m_pBalls[i],this->m_BallAngles[i],mLen);
}

/*********************私有成员函数实现部分***********************/

/*
*******************移动球**********************
对pBall指向的球做如下移动：
Nx=Ox+len*cos(Angle)
Ny=Oy+len*sin(Angle)
其中(Ox,Oy)为移动前的位置，而(Nx,Ny)为移动后的位置
*/
void HandleBall::MoveOneBall(ball *pBall,int Angle,int len)
{
	POINT OldPos,NewPos,tmpPos;
	RECT rt = {0};
	int tempAngle = 0;
	OldPos=pBall->GetPos();

	NewPos.x=OldPos.x+len*cos((float)AngToRad(Angle));
	NewPos.y=OldPos.y+len*sin((float)AngToRad(Angle));

	// 考虑与四壁碰撞情况
	GetClientRect(this->m_hWnd,&rt);
	if(this->isHitWall(pBall,Angle,len,rt,&tempAngle,&tmpPos))
	{
		this->m_BallAngles[pBall->GetId()]=tempAngle;		// 改变球的运动方向

		NewPos.x=OldPos.x+len*cos((float)AngToRad(tempAngle));
		NewPos.y=OldPos.y+len*sin((float)AngToRad(tempAngle));

		pBall->SetPos(NewPos);

	}
	else
		pBall->SetPos(NewPos);

	//	考虑球之间碰撞问题
	int aActive,aPassive;
	for(int i=0;i<BALLNUMBER;i++)
	{
		// 扫描全部不是pBall的球，并进行判断
		if(i!=pBall->GetId() && this->isTwoBallHit(pBall,this->m_pBalls[i],&aActive,&aPassive))
		{
			this->m_BallAngles[pBall->GetId()]=aActive;
			this->m_BallAngles[i]=aPassive;
		}
	}
}

/*
**********************反射算法*******************
1、以aRef(亦即阻挡方向)为x轴正方向建立坐标系
2、(360-aRef)即为新坐标轴按顺时针旋转与标准坐标轴重合所需要的最小角度
*/
int HandleBall::Reflect(int aSrc, int aRef)
{
	int aResult;					// 
	int n_aResult;					// 新坐标轴下aResult角度
	int offset=(360-aRef)%360;		// 两坐标轴之间的偏移量，参见以上定义
	int n_aSrc=(aSrc+offset)%360;	// 新坐标轴下aSrc角度

	// 相当则不存在反射的可能，返回-1代表出错
	if(aSrc==aRef)
		return -1;

	n_aResult=360-n_aSrc;

	// 新坐标向标准坐标转换	
	aResult=abs(n_aResult-offset);	
	return aResult;
}

bool HandleBall::isHitWall(ball *pBall,int bdAngle, int len, RECT rt, int *Angle, POINT *Pos)
{
	POINT newPos;		// 按规则移动后球的位置
	newPos.x=pBall->GetPos().x+len*cos((float)AngToRad(bdAngle));
	newPos.y=pBall->GetPos().y+len*sin((float)AngToRad(bdAngle));

	// pos初始化，返回此表明无效
	Pos->x=-1;
	Pos->y=-1;

	// 首先考虑与四个角碰撞的情况
	if(newPos.y<=rt.top && newPos.x<=rt.left)
	{
		// 左上角
		*Angle=this->Reflect(bdAngle,135);
		return true;

	}
	if(newPos.y<=rt.top && newPos.x>=rt.right)
	{
		// 右上角
		*Angle=this->Reflect(bdAngle,225);
		return true;
	}
	if(newPos.y>=rt.bottom && newPos.x<=rt.left)
	{
		// 左下角
		*Angle=this->Reflect(bdAngle,45);
		return true;
	}
	if(newPos.y>=rt.bottom && newPos.x>=rt.right)
	{
		// 右下角
		*Angle=this->Reflect(bdAngle,315);
		return true;
	}
	// 处理四壁碰撞的情况
	if(newPos.y<=rt.top)
	{
		// 上
		*Angle=this->Reflect(bdAngle,0);
		return true;

	}
	if(newPos.y>=rt.bottom)
	{
		// 下
		*Angle=this->Reflect(bdAngle,0);
		return true;
	}
	if(newPos.x<=rt.left)
	{
		// 左
		*Angle=this->Reflect(bdAngle,90);
		return true;
	}
	if(newPos.x>=rt.right)
	{
		// 右
		*Angle=this->Reflect(bdAngle,270);
		return true;
	}
	return false;
}

/*************************求两角平分线算法****************************
1、当两角度同向（即差的绝对值小于180）：
aResult=(a1+a2)/2
2、当两角度反向：
aResult=((a1+a2)/2+180)%360
*/
int HandleBall::Bisector(int a1, int a2)
{
	int Result;
	abs(a1-a2)>180? Result=(((a1+a2)/2+180)%360):Result=((a1+a2)/2);
	return Result;
}

/***********************************************************
此中仍然采用自定义的坐标轴，因此所提到的象限定义如下：
自右下方顺时针依次为一、二、三、四象限。
*/
int HandleBall::DirectTwoPoint(POINT pt_src,POINT pt_dest)
{
	float temp;
	//	分四象限考虑情况
	if(pt_dest.x>pt_src.x && pt_dest.y>=pt_src.y)
	{
		// 第一象限，包括0度
		temp=atan((float)((float)(pt_dest.y-pt_src.y)/(float)(pt_dest.x-pt_src.x)));

		return (int)RadToAng(temp);
	}
	if(pt_dest.x<=pt_src.x && pt_dest.y>pt_src.y)
	{
		// 第二象限，包括90度
		temp=atan((float)((float)(pt_src.x-pt_dest.x)/(float)(pt_dest.y-pt_src.y)));
		return (int)RadToAng(temp)+90;
	}
	if(pt_dest.x<pt_src.x && pt_dest.y<=pt_src.y)
	{
		// 第三象限，包括180度
		temp=(atan((float)((float)(pt_src.y-pt_dest.y)/(float)(pt_src.x-pt_dest.x))));
		return (int)RadToAng(temp)+180;
	}
	if(pt_dest.x>=pt_src.x && pt_dest.y<pt_src.y)
	{
		// 第四象限，包括270度
		temp=(atan((float)((float)(pt_dest.x-pt_src.x)/(float)(pt_src.y-pt_dest.y))));
		return (int)RadToAng(temp)+270;
	}
	return -1;
}

bool HandleBall::isTwoBallHit(ball *pActive, ball *pPassive, int *aActive, int *aPassive)
{
	int RefDirect;		// 两球相撞时，主动球的反射方向
	int aPerDirect;		// 主动球先前方向
	int pPerDirect;		// 被动球先前方向

	// 当两球心之间的距离小于两球半径之和时即相撞
	if(DISTANCE(pActive->GetPos(),pPassive->GetPos())<=(pActive->GetR()+pPassive->GetR()))
	{	
		aPerDirect=this->m_BallAngles[pActive->GetId()];
		pPerDirect=this->m_BallAngles[pPassive->GetId()];

		RefDirect=this->DirectTwoPoint(pPassive->GetPos(),pActive->GetPos());

		// 主动球返回方向由两球连线方向与自己原本方向所决定
		*aActive=this->Bisector(RefDirect,aPerDirect);

		// 被动球返回方向由主动球方向与自己原本方向所决定
		*aPassive=this->Bisector(aPerDirect,pPerDirect);
		return true;

	}
	else
		return false;
}