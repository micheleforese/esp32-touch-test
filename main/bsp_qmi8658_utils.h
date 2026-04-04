#pragma once

#include "bsp_qmi8658.h"
#include "lvgl.h"
#include "lvgl_utils.h"

extern lv_obj_t *label_accel_x;
extern lv_obj_t *label_accel_y;
extern lv_obj_t *label_accel_z;
extern lv_obj_t *label_gyro_x;
extern lv_obj_t *label_gyro_y;
extern lv_obj_t *label_gyro_z;

void qmi8658_callback(lv_timer_t *timer);