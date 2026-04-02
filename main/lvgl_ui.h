#pragma once

#include "bsp_qmi8658_utils.h"
#include "demos/lv_demos.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_touch_cst816s.h"
#include "esp_log.h"
#include "lv_demos.h"
#include "lvgl.h"
#include "screen.h"
#include "uart_utils.h"

#define LVGL_TICK_PERIOD_MS 2
#define LVGL_TASK_MAX_DELAY_MS 500
#define LVGL_TASK_MIN_DELAY_MS 1

#define LV_DRAW_COMPLEX 1
#define LV_USE_ANTIALIAS 1

#define NUM_LABELS (3)

extern const char *TAG;
extern SemaphoreHandle_t lvgl_api_mux;
extern lv_timer_t *qmi8658_timer;

extern lv_obj_t *labels[NUM_LABELS];
extern lv_obj_t *label_gpio_read_status;
extern lv_obj_t *label_gpio_read_ADC;

void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);
void lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data);
void lv_port_disp_init(void);
void lv_port_indev_init(void);
void lvgl_increase_tick(void *arg);
void lvgl_tick_timer_init(uint32_t ms);
void lvgl_unlock(void);
bool lvgl_lock(int timeout_ms);
void btn_event_cb(lv_event_t *e);
void lvgl_test_ui_init(lv_obj_t *parent);
void lvgl_qmi8658_ui_init(lv_obj_t *parent);
void task(void *param);