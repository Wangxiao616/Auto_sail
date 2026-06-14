#include "oled.h"
#include "oledfont.h"
#include "main.h"
#include <string.h>

/* 软件I2C引脚 — PB14=SCL, PB15=SDA */
#define OLED_SCL_Pin    GPIO_PIN_14
#define OLED_SDA_Pin    GPIO_PIN_15
#define OLED_I2C_Port   GPIOB

#define OLED_I2C_ADDR   0x78  /* 7位地址0x3C左移1位 */
#define OLED_GRAM_SIZE  (128 * 8)

static uint8_t OLED_GRAM[OLED_GRAM_SIZE];

/* ── 软件I2C底层操作 ── */

static void OLED_I2C_Delay(void)
{
    volatile uint8_t i = 15;
    while (i--) { __NOP(); }
}

static void OLED_I2C_SCL(uint8_t level)
{
    HAL_GPIO_WritePin(OLED_I2C_Port, OLED_SCL_Pin, level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void OLED_I2C_SDA(uint8_t level)
{
    HAL_GPIO_WritePin(OLED_I2C_Port, OLED_SDA_Pin, level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void OLED_I2C_Start(void)
{
    OLED_I2C_SDA(1);
    OLED_I2C_SCL(1);
    OLED_I2C_Delay();
    OLED_I2C_SDA(0);
    OLED_I2C_Delay();
    OLED_I2C_SCL(0);
}

static void OLED_I2C_Stop(void)
{
    OLED_I2C_SDA(0);
    OLED_I2C_SCL(1);
    OLED_I2C_Delay();
    OLED_I2C_SDA(1);
    OLED_I2C_Delay();
}

static void OLED_I2C_SendByte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++) {
        OLED_I2C_SDA((byte & 0x80) ? 1 : 0);
        byte <<= 1;
        OLED_I2C_Delay();
        OLED_I2C_SCL(1);
        OLED_I2C_Delay();
        OLED_I2C_SCL(0);
    }
    /* 第9个时钟(ACK) — 释放SDA，不检查应答 */
    OLED_I2C_SDA(1);
    OLED_I2C_Delay();
    OLED_I2C_SCL(1);
    OLED_I2C_Delay();
    OLED_I2C_SCL(0);
}

/* ── OLED命令/数据写入 ── */

static void OLED_WriteCmd(uint8_t cmd)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(OLED_I2C_ADDR);
    OLED_I2C_SendByte(0x00); /* 控制字节: 命令 */
    OLED_I2C_SendByte(cmd);
    OLED_I2C_Stop();
}

static void OLED_WriteBytes(uint8_t ctrl, const uint8_t *data, uint8_t len)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(OLED_I2C_ADDR);
    OLED_I2C_SendByte(ctrl);
    for (uint8_t i = 0; i < len; i++) {
        OLED_I2C_SendByte(data[i]);
    }
    OLED_I2C_Stop();
}

/* ── 公开API ── */

void OLED_Init(void)
{
    /* 初始化SCL/SDA引脚为推挽输出 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = OLED_SCL_Pin | OLED_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OLED_I2C_Port, &GPIO_InitStruct);

    /* 总线初始状态 */
    OLED_I2C_SCL(1);
    OLED_I2C_SDA(1);

    HAL_Delay(100);

    OLED_WriteCmd(0xAE); /* Display OFF */

    OLED_WriteCmd(0xD5); OLED_WriteCmd(0x80); /* 时钟分频 */
    OLED_WriteCmd(0xA8); OLED_WriteCmd(0x3F); /* 复用比 1/64 */
    OLED_WriteCmd(0xD3); OLED_WriteCmd(0x00); /* 显示偏移 */
    OLED_WriteCmd(0x40);                      /* 起始行 */
    OLED_WriteCmd(0x8D); OLED_WriteCmd(0x14); /* 电荷泵使能 */
    OLED_WriteCmd(0x20); OLED_WriteCmd(0x00); /* 水平寻址模式 */
    OLED_WriteCmd(0xA1);                      /* 段重映射 (左右镜像) */
    OLED_WriteCmd(0xC8);                      /* COM扫描 (上下镜像) */
    OLED_WriteCmd(0xDA); OLED_WriteCmd(0x12); /* COM引脚配置 */
    OLED_WriteCmd(0x81); OLED_WriteCmd(0xCF); /* 对比度 */
    OLED_WriteCmd(0xD9); OLED_WriteCmd(0xF1); /* 预充电周期 */
    OLED_WriteCmd(0xDB); OLED_WriteCmd(0x40); /* VCOMH */
    OLED_WriteCmd(0xA4);                      /* 显示全部恢复 */
    OLED_WriteCmd(0xA6);                      /* 正常显示(非反色) */

    OLED_WriteCmd(0xAF); /* Display ON */

    OLED_Clear();
}

void OLED_Clear(void)
{
    memset(OLED_GRAM, 0x00, OLED_GRAM_SIZE);
    OLED_Refresh();
}

void OLED_Fill(uint8_t fill)
{
    memset(OLED_GRAM, fill, OLED_GRAM_SIZE);
    OLED_Refresh();
}

void OLED_DisplayOn(void)
{
    OLED_WriteCmd(0x8D); OLED_WriteCmd(0x14);
    OLED_WriteCmd(0xAF);
}

void OLED_DisplayOff(void)
{
    OLED_WriteCmd(0x8D); OLED_WriteCmd(0x10);
    OLED_WriteCmd(0xAE);
}

void OLED_Refresh(void)
{
    for (uint8_t page = 0; page < 8; page++) {
        OLED_WriteCmd(0xB0 + page);
        OLED_WriteCmd(0x00);
        OLED_WriteCmd(0x10);
        OLED_WriteBytes(0x40, &OLED_GRAM[page * 128], 128);
    }
}

void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t mode)
{
    if (x >= 128 || y >= 64) return;

    if (mode) {
        OLED_GRAM[x + (y / 8) * 128] |=  (1 << (y % 8));
    } else {
        OLED_GRAM[x + (y / 8) * 128] &= ~(1 << (y % 8));
    }
}

static void OLED_ShowChar6x8(uint8_t x, uint8_t y, char chr)
{
    uint8_t idx = chr - ' ';
    if (idx > 94) idx = 0;

    for (uint8_t col = 0; col < 6; col++) {
        uint8_t line = F6x8[idx][col];
        for (uint8_t row = 0; row < 8; row++) {
            OLED_DrawPoint(x + col, y + row, (line >> row) & 0x01);
        }
    }
}

static void OLED_ShowChar8x16(uint8_t x, uint8_t y, char chr)
{
    uint8_t idx = chr - ' ';
    if (idx > 94) idx = 0;

    for (uint8_t col = 0; col < 8; col++) {
        uint8_t line_h = F8x16[idx][col];
        uint8_t line_l = F8x16[idx][col + 8];
        for (uint8_t row = 0; row < 8; row++) {
            OLED_DrawPoint(x + col, y + row,      (line_h >> row) & 0x01);
            OLED_DrawPoint(x + col, y + row + 8,  (line_l >> row) & 0x01);
        }
    }
}

void OLED_ShowChar(uint8_t x, uint8_t y, char chr, uint8_t size)
{
    if (size == 12) {
        OLED_ShowChar6x8(x, y, chr);
    } else {
        OLED_ShowChar8x16(x, y, chr);
    }
}

void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size)
{
    uint8_t char_width = (size == 12) ? 6 : 8;
    while (*str) {
        if (x + char_width > 128) break;
        if (y >= 64) break;
        OLED_ShowChar(x, y, *str, size);
        x += char_width;
        str++;
    }
}
