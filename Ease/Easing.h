
// 22 种移动特效
// http://shakddoo.tistory.com/entry/Easing-code [jd Code Snippet]

#include <MATH.H>

#ifndef Pi
#define Pi 3.1415926
#endif


enum EASE_TYPE
{
	ease_linear = 0,
	ease_QuadraticIn,
	ease_QuadraticOut,
	ease_QuadraticInOut,
	ease_CubicIn,
	ease_CubicOut,
	ease_CubicInOut,
	ease_QuarticIn,
	ease_QuarticOut,
	ease_QuarticInOut,
	ease_QuinticIn,
	ease_QuinticOut,
	ease_QuinticInOut,
	ease_SinIn,
	ease_SinOut,
	ease_SinInOut,
	ease_ExponentialIn,
	ease_ExponentialOut,
	ease_ExponentialInOut,
	ease_CircularIn,
	ease_CircularOut,
	ease_CircularInOut,
}EaseType;

// 参数一 当前帧
// 参数二 开始时的位置
// 参数三 数值越小，速度越快
// 参数四 移动距离，越大移动的距离越多

inline float easeLinear(float current_frame, float start_value, float end_frame, float change_value)
{
	return change_value*current_frame/end_frame + start_value;
}

inline float easeQuadraticIn(float current_frame, float start_value, float end_frame, float change_value)
{
	current_frame /= end_frame;
	return change_value*current_frame*current_frame + start_value;
}

inline float easeQuadraticOut(float current_frame, float start_value, float end_frame, float change_value)
{
	current_frame /= end_frame;
	return -change_value * current_frame*(current_frame-2) + start_value;
}

inline float easeQuadraticInOut(float current_frame, float start_value, float end_frame, float change_value)
{
	current_frame /= end_frame/2;
	if (current_frame < 1)
		return change_value/2*current_frame*current_frame + start_value;
	current_frame--;
	return -change_value/2 * (current_frame*(current_frame-2) - 1) + start_value;
}

inline float easeCubicIn(float current_frame, float start_value, float end_frame, float change_value)
{
	current_frame /= end_frame;
	return change_value*current_frame*current_frame*current_frame + start_value;
}

inline float easeCubicOut(float current_frame, float start_value, float end_frame, float change_value)
{
	current_frame /= end_frame;
	current_frame--;
	return change_value*(current_frame*current_frame*current_frame + 1) + start_value;
}

inline float easeCubicInOut(float current_frame, float start_value, float end_frame, float change_value)
{
	current_frame /= end_frame/2;
	if (current_frame < 1)
		return change_value/2*current_frame*current_frame*current_frame + start_value;
	current_frame -= 2;
	return change_value/2*(current_frame*current_frame*current_frame + 2) + start_value;
}

inline float easeQuarticIn(float current_frame, float start_value, float end_frame, float change_value)
{
	current_frame /= end_frame;
	return change_value*current_frame*current_frame*current_frame*current_frame + start_value;
}

inline float easeQuarticOut(float current_frame, float start_value, float end_frame, float change_value)
{
	current_frame /= end_frame;
	current_frame--;
	return -change_value * (current_frame*current_frame*current_frame*current_frame - 1) + start_value;
}

inline float easeQuarticInOut(float current_frame, float start_value, float end_frame, float change_value)
{
	current_frame /= end_frame/2;
	if (current_frame < 1)
		return change_value/2*current_frame*current_frame*current_frame*current_frame + start_value;
	current_frame -= 2;
	return -change_value/2 * (current_frame*current_frame*current_frame*current_frame - 2) + start_value;
}

inline float easeQuinticIn(float current_frame, float start_value, float end_frame, float change_value)
{
	current_frame /= end_frame;
	return change_value*current_frame*current_frame*current_frame*current_frame*current_frame + start_value;
}

inline float easeQuinticOut(float current_frame, float start_value, float end_frame, float change_value)
{
	current_frame /= end_frame;
	current_frame--;
	return change_value*(current_frame*current_frame*current_frame*current_frame*current_frame + 1) + start_value;
}

inline float easeQuinticInOut(float current_frame, float start_value, float end_frame, float change_value)
{
	current_frame /= end_frame/2;
	if (current_frame < 1)
		return change_value/2*current_frame*current_frame*current_frame*current_frame*current_frame + start_value;
	current_frame -= 2;
	return change_value/2 * (current_frame*current_frame*current_frame*current_frame*current_frame + 2) + start_value;
}


inline float easeSinIn(float current_frame, float start_value, float end_frame, float change_value)
{
	return   -change_value * cosf(current_frame/end_frame * (Pi/2)) + change_value + start_value;
}

inline float easeSinOut(float current_frame, float start_value, float end_frame, float change_value)
{
	return   change_value * sinf(current_frame/end_frame * (Pi/2)) + start_value;
}

inline float easeSinInOut(float current_frame, float start_value, float end_frame, float change_value)
{
	return -change_value/2 * (cosf(Pi*current_frame/end_frame) - 1) + start_value;
}

inline float easeExponentialIn(float current_frame, float start_value, float end_frame, float change_value)
{
	return change_value * powf( 2, 10 * (current_frame/end_frame - 1) ) + start_value;
}

inline float easeExponentialOut(float current_frame, float start_value, float end_frame, float change_value)
{
	return change_value * ( -powf( 2, -10 * current_frame/end_frame ) + 1 ) + start_value;
}

inline float easeExponentialInOut(float current_frame, float start_value, float end_frame, float change_value)
{
	current_frame /= end_frame/2;
	if (current_frame < 1)
		return change_value/2 * powf( 2, 10 * (current_frame - 1) ) + start_value;
	current_frame --;
	return change_value/2 * ( -powf( 2, -10 * current_frame) + 2 ) + start_value;
}

inline float easeCircularIn(float current_frame, float start_value, float end_frame, float change_value)
{
	current_frame /= end_frame;
	return -change_value * (sqrtf(1 - current_frame*current_frame) - 1) + start_value;
}

inline float easeCircularOut(float current_frame, float start_value, float end_frame, float change_value)
{
	current_frame /= end_frame;
	current_frame--;
	return change_value * sqrtf(1 - current_frame*current_frame) + start_value;
}

inline float easeCircularInOut(float current_frame, float start_value, float end_frame, float change_value)
{
	current_frame /= end_frame/2;
	if (current_frame < 1)
		return -change_value/2 * (sqrtf(1 - current_frame*current_frame) - 1) + start_value;
	current_frame -= 2;
	return change_value/2 * (sqrtf(1 - current_frame*current_frame) + 1) + start_value;
}

inline float doEase(EASE_TYPE type, float current_frame, float start_value, float end_frame, float change_value)
{
	switch (type)
	{
	case ease_linear:
		return easeLinear(current_frame, start_value, end_frame, change_value);
		break;
	case ease_QuadraticIn:
		return easeQuarticIn(current_frame, start_value, end_frame, change_value);
		break;
	case ease_QuadraticOut:
		return easeQuadraticOut(current_frame, start_value, end_frame, change_value);
		break;
	case ease_QuadraticInOut:
		return easeQuadraticInOut(current_frame, start_value, end_frame, change_value);
		break;
	case ease_CubicIn:
		return easeCubicIn(current_frame, start_value, end_frame, change_value);
		break;
	case ease_CubicOut:
		return easeCubicOut(current_frame, start_value, end_frame, change_value);
		break;
	case ease_CubicInOut:
		return easeCubicInOut(current_frame, start_value, end_frame, change_value);
		break;
	case ease_QuarticIn:
		return easeQuarticIn(current_frame, start_value, end_frame, change_value);
		break;
	case ease_QuarticOut:
		return easeQuarticOut(current_frame, start_value, end_frame, change_value);
		break;
	case ease_QuarticInOut:
		return easeQuarticInOut(current_frame, start_value, end_frame, change_value);
		break;
	case ease_QuinticIn:
		return easeQuinticIn(current_frame, start_value, end_frame, change_value);
		break;
	case ease_QuinticOut:
		return easeQuinticOut(current_frame, start_value, end_frame, change_value);
		break;
	case ease_QuinticInOut:
		return easeQuinticInOut(current_frame, start_value, end_frame, change_value);
		break;
	case ease_SinIn:
		return easeSinIn(current_frame, start_value, end_frame, change_value);
		break;
	case ease_SinOut:
		return easeSinOut(current_frame, start_value, end_frame, change_value);
		break;
	case ease_SinInOut:
		return easeSinInOut(current_frame, start_value, end_frame, change_value);
		break;
	case ease_ExponentialIn:
		return easeExponentialIn(current_frame, start_value, end_frame, change_value);
		break;
	case ease_ExponentialOut:
		return easeExponentialOut(current_frame, start_value, end_frame, change_value);
		break;
	case ease_ExponentialInOut:
		return easeExponentialInOut(current_frame, start_value, end_frame, change_value);
		break;
	case ease_CircularIn:
		return easeCircularIn(current_frame, start_value, end_frame, change_value);
		break;
	case ease_CircularOut:
		return easeCircularOut(current_frame, start_value, end_frame, change_value);
		break;
	case ease_CircularInOut:
		return easeCircularInOut(current_frame, start_value, end_frame, change_value);
		break;
	default:
		return 0.0f;
		break;
	}
}



inline char* GetEaseName(EASE_TYPE type)
{
	switch (type)
	{
	case ease_linear:
		return "easeLinear";
		break;
	case ease_QuadraticIn:
		return "easeQuarticIn";
		break;
	case ease_QuadraticOut:
		return "easeQuadraticOut";
		break;
	case ease_QuadraticInOut:
		return "easeQuadraticInOut";
		break;
	case ease_CubicIn:
		return "easeCubicIn";
		break;
	case ease_CubicOut:
		return "easeCubicOut";
		break;
	case ease_CubicInOut:
		return "easeCubicInOut";
		break;
	case ease_QuarticIn:
		return "easeQuarticIn";
		break;
	case ease_QuarticOut:
		return "easeQuarticOut";
		break;
	case ease_QuarticInOut:
		return "easeQuarticInOut";
		break;
	case ease_QuinticIn:
		return "easeQuinticIn";
		break;
	case ease_QuinticOut:
		return "easeQuinticOut";
		break;
	case ease_QuinticInOut:
		return "easeQuinticInOut";
		break;
	case ease_SinIn:
		return "easeSinIn";
		break;
	case ease_SinOut:
		return "easeSinOut";
		break;
	case ease_SinInOut:
		return "easeSinInOut";
		break;
	case ease_ExponentialIn:
		return "easeExponentialIn";
		break;
	case ease_ExponentialOut:
		return "easeExponentialOut";
		break;
	case ease_ExponentialInOut:
		return "easeExponentialInOut";
		break;
	case ease_CircularIn:
		return "easeCircularIn";
		break;
	case ease_CircularOut:
		return "easeCircularOut";
		break;
	case ease_CircularInOut:
		return "easeCircularInOut";
		break;
	default:
		return "";
		break;
	}
}