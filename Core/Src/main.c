/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sg90.h"
#include "drv8833.h"
#include "infrared.h" // 新增的红外传感器控制加权逻辑的头文件
#include "oled.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_SPEED 100 // 最大速度值
#define UART_BUF_SIZE 64
#define DBG_PRINT(fmt, ...) do { \
    char buf[UART_BUF_SIZE]; \
    sprintf(buf, fmt, ##__VA_ARGS__); \
    HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), 100); \
} while(0)
#define IR_DEBOUNCE_MS 15U // 防抖时间（毫秒）用于中断回调函数

/* ==== 反射抗性遮蔽参数 ==== */
#define MASK_PHASE_MS       8000U   /* 单侧遮蔽持续时间（毫秒），根据门间时间调整 */
#define MASK_FIRST_RIGHT    1       /* 1=先遮蔽右侧, 0=先遮蔽左侧（根据赛道S弯方向修改） */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t speed = 50;//电机速度
static uint32_t ir_last_tick[IR_SENSOR_NUM] = {0};
volatile uint8_t ir_dbg_pending = 0;
volatile uint8_t ir_dbg_idx = 0xFF;
// 红外传感器调试掩码
static const uint8_t ir_enable_mask[IR_SENSOR_NUM] = {
    0,1,1,1,1,1,1,1,0
};

/* ==== 反射抗性状态变量 ==== */
static uint32_t mask_start_tick = 0;    /* 遮蔽计时起点（上电即开始） */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint8_t idx = 0xFF;

    if      (GPIO_Pin == IRM_1_Pin) idx = 0;
    else if (GPIO_Pin == IRM_2_Pin) idx = 1;
    else if (GPIO_Pin == IRM_3_Pin) idx = 2;
    else if (GPIO_Pin == IRM_4_Pin) idx = 3;
    else if (GPIO_Pin == IRM_5_Pin) idx = 4;
    else if (GPIO_Pin == IRM_6_Pin) idx = 5;
    else if (GPIO_Pin == IRM_7_Pin) idx = 6;
    else if (GPIO_Pin == IRM_8_Pin) idx = 7;
    else if (GPIO_Pin == IRM_9_Pin) idx = 8;
    else return;

    //红外传感器调试代码
    if (!ir_enable_mask[idx]) return;

    // 反射抗性：被遮蔽方向的传感器直接忽略，不触发
    if (Infrared_IsSensorMasked(idx)) return;

    uint32_t now = HAL_GetTick();//获取当前时间戳
    if ((now - ir_last_tick[idx]) < IR_DEBOUNCE_MS) {
        return; // 防抖时间窗内，忽略抖动
    }

    // 非阻塞电平确认
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_Pin) != GPIO_PIN_RESET) {
        return;
    }

    ir_last_tick[idx] = now;

    if (ir_trigger_status[idx] == 0) { // 只有上一轮状态是0，才更新为1

        Infrared_SetTriggerStatus(idx, 1);
    }
    // 只置标志，不在中断里串口打印
    ir_dbg_idx = idx;
    ir_dbg_pending = 1;

    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  SG90_Init();
  DRV8833_Init();
  Infrared_Init(); // 新增红外初始化
  OLED_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  DRV8833_Forward(speed); // 开始前进

  

  while (1)
  {
    /* ==== 反射抗性：时间驱动交替遮蔽 ==== */
    uint32_t now = HAL_GetTick();
    uint32_t phase = (now - mask_start_tick) / MASK_PHASE_MS;

    #if MASK_FIRST_RIGHT
        /* 先遮蔽右侧: phase 0,2,4...遮蔽右侧, phase 1,3,5...遮蔽左侧 */
        Infrared_SetMaskSide((phase & 1) ? IR_MASK_LEFT : IR_MASK_RIGHT);
    #else
        /* 先遮蔽左侧: phase 0,2,4...遮蔽左侧, phase 1,3,5...遮蔽右侧 */
        Infrared_SetMaskSide((phase & 1) ? IR_MASK_RIGHT : IR_MASK_LEFT);
    #endif

    /* 1. 计算红外加权和（被遮蔽传感器不参与） */
    int8_t weight_sum = Infrared_CalcWeightSum();

    /* 2. 计算目标舵机角度 */
    int target_angle = SG90_CalcTargetAngle(weight_sum);

    /* 3. 设置舵机角度 */
    SG90_SetAngle(target_angle);

    /* ==== OLED显示红外传感器状态 ==== */
    static uint8_t oled_disp_status[IR_SENSOR_NUM] = {0};
    static uint32_t oled_disp_tick[IR_SENSOR_NUM] = {0};
    static uint32_t oled_last_refresh = 0;
    uint32_t now_oled = HAL_GetTick();

    for (uint8_t i = 0; i < IR_SENSOR_NUM; i++) {
        if (ir_trigger_status[i]) {
            oled_disp_status[i] = 1;
            oled_disp_tick[i] = now_oled;
        }
    }
    /* 超时500ms后清除显示 */
    for (uint8_t i = 0; i < IR_SENSOR_NUM; i++) {
        if (oled_disp_status[i] && (now_oled - oled_disp_tick[i]) > 500) {
            oled_disp_status[i] = 0;
        }
    }

    /* 每200ms刷新一次OLED */
    if ((now_oled - oled_last_refresh) >= 200) {
        oled_last_refresh = now_oled;

        OLED_Clear();

        /* 第1行：9传感器状态条（12x10方块，实心=触发，空心=未触发） */
        for (uint8_t i = 0; i < IR_SENSOR_NUM; i++) {
            uint8_t x0 = i * 14;
            uint8_t f  = oled_disp_status[i];
            for (uint8_t dx = 1; dx < 13; dx++) {
                for (uint8_t dy = 0; dy < 10; dy++) {
                    OLED_DrawPoint(x0 + dx, dy, f);
                }
            }
        }

        /* 第2行：触发的传感器编号（6x8字体，最多可显示全部9个） */
        char buf[32];
        uint8_t pos = 0;
        for (uint8_t i = 0; i < IR_SENSOR_NUM; i++) {
            if (oled_disp_status[i]) {
                pos += sprintf(buf + pos, "%d ", i + 1);
            }
        }
        if (pos == 0) {
            OLED_ShowString(0, 16, "All Clear", 12);
        } else {
            OLED_ShowString(0, 16, buf, 12);
        }

        /* 底行显示权重、角度和遮蔽状态 */
        const char *mask_str;
        switch (Infrared_GetMaskSide()) {
            case IR_MASK_LEFT:  mask_str = "ML"; break;
            case IR_MASK_RIGHT: mask_str = "MR"; break;
            default:            mask_str = "--"; break;
        }
        sprintf(buf, "W:%d A:%d P%lu %s", weight_sum, target_angle, phase, mask_str);
        OLED_ShowString(0, 48, buf, 12);

        OLED_Refresh();
    }

    // // 中断事件日志（你前面加的 pending 标志）
    // if (ir_dbg_pending) {
    //     uint8_t idx = ir_dbg_idx;
    //     ir_dbg_pending = 0;
    //     DBG_PRINT("IR_TRIG: IRM%d\r\n", (int)(idx + 1));
    // }
    // // 周期状态日志：每200ms输出一次，避免刷屏阻塞
    // uint32_t now = HAL_GetTick();
    // if ((now - last_log_tick) >= 200U) {
    //     last_log_tick = now;
    //     char ir_stat[10] = {0}; // 9位+结束符
    //     for (uint8_t i = 0; i < 9; i++) {
    //         ir_stat[i] = ir_trigger_status[i] ? '1' : '0';
    //     }
    //     DBG_PRINT("WEIGHT:%d ANGLE:%d IR:[%s]\r\n", weight_sum, target_angle, ir_stat);
    // }


    /* 4. 重置传感器状态（避免持续触发） */
    Infrared_ResetTriggerStatus();

    /* 5. 控制周期（可调整，建议20~50ms） */
    HAL_Delay(30);


    //DEBUG部分代码

    // 核心板上LED以1s为周期闪烁，表示程序正常运行
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin); 
    


    // DRV8833_Forward(speed); // 控制电机前进
    // speed += 10; // 增加速度
    // if (speed > MAX_SPEED) // 如果速度超过MAX_SPEED，重置为0
    // {
    //   speed = 0;
    // }
    // HAL_Delay(1000); // 延时1秒


    // HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin); // Toggle the LED on PC13
    // SG90_TurnLeft(); // 向左转
    //  HAL_Delay(1000); // 延时1秒
    //  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin); // Toggle the LED on PC13
    //  SG90_TurnRight(); // 向右转
    //  HAL_Delay(1000); // 延时1秒
    // SG90_SetAngle(98); // 设置舵机角度居中
    //  HAL_Delay(1000); // 延时1秒
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
