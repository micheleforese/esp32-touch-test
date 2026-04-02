#include "bsp_qmi8658_utils.h"

lv_obj_t *label_accel_x = NULL;
lv_obj_t *label_accel_y = NULL;
lv_obj_t *label_accel_z = NULL;
lv_obj_t *label_gyro_x = NULL;
lv_obj_t *label_gyro_y = NULL;
lv_obj_t *label_gyro_z = NULL;

void qmi8658_callback(lv_timer_t *timer)
{
    qmi8658_data_t data;
    bsp_qmi8658_read_data(&data);
    if (lvgl_lock(-1))
    {
        lv_label_set_text_fmt(label_accel_x, "%d", data.acc_x);
        lv_label_set_text_fmt(label_accel_y, "%d", data.acc_y);
        lv_label_set_text_fmt(label_accel_z, "%d", data.acc_z);
        lv_label_set_text_fmt(label_gyro_x, "%d", data.gyr_x);
        lv_label_set_text_fmt(label_gyro_y, "%d", data.gyr_y);
        lv_label_set_text_fmt(label_gyro_z, "%d", data.gyr_z);
        lvgl_unlock();
    }
}