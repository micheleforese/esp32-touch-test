#pragma once

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_touch_cst816s.h"
#include "esp_log.h"
#include "lvgl_ui.h"

#define PIN_NUM_LCD_SCLK 39
#define PIN_NUM_LCD_MOSI 38

#define SPI_HOST SPI2_HOST

#define I2C_NUM 0 // I2C number
#define PIN_NUM_I2C_INT 46
#define PIN_NUM_I2C_SCL 47
#define PIN_NUM_I2C_SDA 48

#define LCD_PIXEL_CLOCK_HZ (40 * 1000 * 1000)

#define PIN_NUM_LCD_DC 42
// #define PIN_NUM_LCD_RST -1
#define PIN_NUM_LCD_RST 0
#define PIN_NUM_LCD_CS 45

#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8

#define LCD_V_RES 320
#define LCD_H_RES 240
#define LCD_BUF_HEIGHT 40
#define LCD_BUF_LENGTH (LCD_H_RES * LCD_BUF_HEIGHT)

#define PIN_NUM_BK_LIGHT 1

#define LCD_BL_LEDC_TIMER LEDC_TIMER_0
#define LCD_BL_LEDC_MODE LEDC_LOW_SPEED_MODE

#define LCD_BL_LEDC_CHANNEL LEDC_CHANNEL_0
#define LCD_BL_LEDC_DUTY_RES LEDC_TIMER_10_BIT // Set duty resolution to 13 bits
#define LCD_BL_LEDC_DUTY (1024)                // Set duty to 50%. (2 ** 13) * 50% = 4096
#define LCD_BL_LEDC_FREQUENCY (10000)          // Frequency in Hertz. Set frequency at 5 kHz

extern lv_indev_drv_t indev_drv;
extern lv_disp_drv_t disp_drv;
extern esp_lcd_panel_handle_t panel_handle;
extern esp_lcd_touch_handle_t tp;

bool lvgl_notify_flush_ready(esp_lcd_panel_io_handle_t panel_io,
                             esp_lcd_panel_io_event_data_t *edata, void *user_ctx);

void display_init(void);

void touch_init(void);

void bsp_brightness_init(void);

void bsp_brightness_set_level(uint8_t level);