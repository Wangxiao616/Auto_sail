/*
 * @file sg90.c 
 * @brief 舵机控制实现
 * @note 舵机控制PWM信号频率50Hz，占空比范围2.5%到12.5%
 */
#include "sg90.h"
#include "tim.h"

#include "usart.h"
#include <stdio.h>

/* @brief 控制舵机的Timer*/
#define SG90_TIM &htim2
/* @brief 控制舵机的Timer通道*/
#define SG90_CHANNEL TIM_CHANNEL_2

/* @brief 舵机周期（单位：ms）*/
#define SG90_PERIOD 20
/* @brief 舵机最小占空比（百分比）*/
#define SG90_MIN_DUTY 2.5f
/* @brief 舵机最大占空比（百分比）*/
#define SG90_MAX_DUTY 12.5f
/* 
 * @brief 舵机占空比修正值（百分比）
 * @note 该值根据实际角度与预期偏转角度差值进行调整
 */
#define SG90_MODIFER 0.0f
/* @brief 舵机初始角度*/
#define SG90_INIT_ANGLE 98 //这里你们试一下设成多少舵是正的
/* @brief 舵机向左转向时的角度*/
#define SG90_LEFT_ANGLE 60 //这里是左转的时候设置的角度
/* @brief 舵机向右转向时的角度*/
#define SG90_RIGHT_ANGLE 120 //这里是右转的时候的角度

#define SG90_INIT_DUTY (SG90_INIT_ANGLE / 18.0f) + 2.5



/* 
 * @brief 设置舵机控制PWM占空比
 * @param duty 占空比值，范围在2.5到12.5之间
 */
void __SG90_SetDuty(float duty)
{

    if (duty < SG90_MIN_DUTY) duty = SG90_MIN_DUTY;
    if (duty > SG90_MAX_DUTY) duty = SG90_MAX_DUTY;//防越界

    /* 瞬时到位：直接设置目标比较值，不做步进平滑 */
    uint32_t target = (uint32_t)((duty + SG90_MODIFER) * SG90_PERIOD);
    __HAL_TIM_SET_COMPARE(SG90_TIM, SG90_CHANNEL, target);
}
/* 
 * @brief 舵机初始化函数 
 */
void SG90_Init(void)
{
    HAL_TIM_PWM_Start(SG90_TIM, SG90_CHANNEL);
	SG90_SetAngle(SG90_INIT_ANGLE);
}

/*
 * @brief 设置舵机角度
 * @param angle 舵机角度，0-180之间，90度为正
 */
void SG90_SetAngle(int angle)
{
	if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;//限制角度范围0-180之间

	float duty = (angle / 18.0f) + 2.5;
	__SG90_SetDuty(duty);
	return;
}

/* 
 * @brief 舵机向左转向
 */
void SG90_TurnLeft()
{
	SG90_SetAngle(SG90_LEFT_ANGLE);
}

/* 
 * @brief 舵机向右转向
 */
void SG90_TurnRight()
{
	SG90_SetAngle(SG90_RIGHT_ANGLE);
}

/* 
 * @brief 舵机归中
 */
void SG90_Reset()
{
	SG90_SetAngle(SG90_INIT_ANGLE);
}
/* 
 * @brief 舵机根据加权和计算目标角度
 * @param weight_sum 红外传感器加权和
 */
 int SG90_CalcTargetAngle(int8_t weight_sum)
{
    /* 基础角度（归中） */
    int target_angle = SG90_INIT_ANGLE;
    
    /* 权重和映射到角度偏移（负号因舵机物理方向与代码假设相反，系数可调） */
    int angle_offset = weight_sum * (-7);
    target_angle += angle_offset;
    
    /* 限制角度范围0~180° */
    if(target_angle < 0)
    {
        target_angle = 0;
    }
    else if(target_angle > 180)
    {
        target_angle = 180;
    }
    
    return target_angle;
}