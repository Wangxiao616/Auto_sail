# AUTO_SAIL

基于 STM32F103 的小船控制系统，通过红外传感器检测光电门方向，自动控制舵机转向和电机推进。

## 硬件平台

| 模块 | 型号/说明 | 接口 |
|------|-----------|------|
| 主控 | STM32F103C8T6 | — |
| 红外传感器 ×9 | IRM，环形排列于船身 | PA4~PA12（EXTI 中断） |
| 舵机 | SG90（转向控制） | PA1（TIM2 CH2，50Hz PWM） |
| 电机驱动 | DRV8833 双H桥 | PB6/PB7（TIM4 CH1/CH2） |
| OLED 显示屏 | GME12864-49（SSD1306，128×64） | PB14=SCL, PB15=SDA（软件 I2C） |
| 串口调试 | UART2 | 外接 CH340 串口模块 |
| 调试接口 | ST-Link | SWD |

## 项目结构

```
AUTO_SAIL/
├── Core/                    # STM32CubeMX 生成的片上外设代码
│   ├── Inc/                 # 头文件（main.h, gpio.h, tim.h, usart.h 等）
│   └── Src/                 # 源文件（main.c, gpio.c, tim.c, usart.c 等）
├── Drivers/                 # CMSIS 和 HAL 库
├── User/                    # 用户驱动代码
│   ├── DRV8833/             # 电机驱动（前进/后退/刹车/滑行）
│   ├── SG90/                # 舵机驱动（角度控制/加权映射）
│   ├── Infrared/            # 红外传感器（触发检测/加权计算）
│   └── OLED/                # OLED 显示驱动（SSD1306 软件 I2C）
├── cmake/                   # CMake 子项目（CubeMX 生成的编译规则）
├── build/                   # 构建输出
├── AUTO_SAIL.ioc            # CubeMX 工程文件
├── CMakeLists.txt           # 顶层 CMake 配置
├── CMakePresets.json        # CMake 预设
└── startup_stm32f103xb.s    # 启动汇编
```

## 控制逻辑

### 红外加权转向算法

9 个红外传感器呈环形排列：**IRM_5 正前方**、**IRM_2 正右方**、**IRM_8 正左方**，IRM_1 和 IRM_9 在后方（已屏蔽）。

每个传感器预设权重：

```
IRM_1:  0     IRM_2: +4    IRM_3: +3
IRM_4: +2     IRM_5:  0    IRM_6: -2
IRM_7: -3     IRM_8: -4    IRM_9:  0
```

- **右侧传感器**（IRM_2/3/4）正权重 → 船偏左 → 右转修正
- **左侧传感器**（IRM_6/7/8）负权重 → 船偏右 → 左转修正
- **正前方**（IRM_5）权重为 0，直行无修正

主循环流程：

1. 红外中断触发后记录传感器状态（15ms 防抖 + 电平确认）
2. 主循环计算权重和 `weight_sum`（被触发传感器的权重累加）
3. `SG90_CalcTargetAngle()` 将权重和映射为舵机角度偏移
4. `SG90_SetAngle()` 设置舵机角度
5. `Infrared_ResetTriggerStatus()` 重置状态，进入下一周期
6. 每 200ms 刷新一次 OLED 显示

### 角度映射

```
target_angle = SG90_INIT_ANGLE + weight_sum × (-7)
```

系数 -7 经下水实测调优得出。角度限制在 0°~180°，低角度对应右转，高角度对应左转。

### 电机控制

系统启动后持续前进（默认速度 50/100），支持前进、后退、刹车、滑行四种模式，可通过 DRV8833 的慢衰减/快衰减模式调节。

## 开发环境

| 工具 | 版本 |
|------|------|
| STM32CubeMX | 6.13.0 |
| CMake | ≥3.22 |
| ARM GCC 工具链 | arm-none-eabi-* |
| VS Code（推荐） | 含 ST-Link 调试插件 |
| MDK-ARM（兼容） | 5.41.0 |
| STM32 ST-Link Utility | 4.6.0.0 |

## 构建与烧录

### 命令行（CMake）

```bash
cd build/Debug
cmake --build .
```

### VS Code

在左侧栏选择"运行和调试"，选择 ST-Link 配置，直接编译并调试。

### Keil MDK

打开 `MDK-ARM/AUTO_SAIL.uvprojx` 工程文件编译，使用 ST-Link Utility 烧录 `AUTO_SAIL.hex`。

## 硬件连接注意事项

- **串口**：UART2 RX/TX 与串口模块交叉连接，并共地
- **电机驱动板**：STBY 引脚需接高电平（至电源拓展模块正极），并与系统板共地
- **OLED**：4 引脚直连（GND→GND, VDD→3.3V, SCK→PB14, SDA→PB15），无需电平转换
- **舵机**：上电前将 `SG90_INIT_ANGLE` 调至归中角度（当前为 98°）

## OLED 显示界面

```
┌──────────────────────────────┐
│ ████ ████ ████  ···  (9个)   │  ← 传感器状态条（实心=触发，空心=未触发）
│ 2 3 4                        │  ← 触发的传感器编号（6x8 字体）
│ W:+9 A:35                    │  ← 权重和 & 目标角度
└──────────────────────────────┘
```

无触发时显示 "All Clear"。触发状态保持 500ms 后自动清除。

## 调试

底板预留 UART2 针脚，代码中已定义 `DBG_PRINT` 宏用于串口输出。主循环中预留了已注释的调试日志代码，可在测试时启用。
