#include "lvgl_ui.h"
#include "bsp_qmi8658_utils.h"
#include "esp_timer.h"
#include "lvgl.h"

#include "config.h"

const char *TAG_LVGL = "lvgl_ui";
SemaphoreHandle_t lvgl_api_mux = NULL;
lv_obj_t *labels[NUM_LABELS];
lv_timer_t *qmi8658_timer;

lv_obj_t *label_gpio_read_status;

void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // copy a buffer's content to a specific area of the display

    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1,
                              color_map);
}

void lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    uint16_t touchpad_x[1] = {0};
    uint16_t touchpad_y[1] = {0};
    uint8_t touchpad_cnt = 0;
    esp_lcd_touch_read_data(tp);
    /* Get coordinates */
    bool touchpad_pressed =
        esp_lcd_touch_get_coordinates(tp, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);

    if (touchpad_pressed && touchpad_cnt > 0)
    {
        data->point.x = touchpad_x[0];
        data->point.y = touchpad_y[0];
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

void lv_port_disp_init(void)
{

    static lv_disp_draw_buf_t draw_buf;

    lv_color_t *buf1 =
        heap_caps_malloc(LCD_BUF_LENGTH * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);

    if (!buf1)
    {
        ESP_LOGE("lv_port_disp_init", "ERRORE FATALE: impossibile allocare buf1!");
        abort();
    }

    lv_color_t *buf2 =
        heap_caps_malloc(LCD_BUF_LENGTH * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);

    if (!buf2)
    {
        ESP_LOGE("lv_port_disp_init", "ERRORE FATALE: impossibile allocare buf2!");
        abort();
    }
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LCD_BUF_LENGTH); /*Initialize the display buffer*/

    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    lv_disp_drv_init(&disp_drv); /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = LCD_H_RES;
    disp_drv.ver_res = LCD_V_RES;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = lvgl_flush_cb;

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf;

    /*Required for Example 3)*/
    disp_drv.full_refresh = 0;
    // disp_drv.direct_mode = 1;

    /* Fill a memory array with a color if you have GPU.
     * Note that, in lv_conf.h you can enable GPUs that has built-in support in
     * LVGL. But if you have a different GPU you can use with this callback.*/
    // disp_drv.gpu_fill_cb = gpu_fill;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

void lv_port_indev_init(void)
{
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_touch_cb;
    indev_drv.user_data = tp;

    lv_indev_drv_register(&indev_drv);
}

void lvgl_increase_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

void lvgl_tick_timer_init(uint32_t ms)
{
    ESP_LOGI(TAG_LVGL, "Install LVGL tick timer");
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {.callback = &lvgl_increase_tick,
                                                          .name = "lvgl_tick"};
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, ms * 1000));
}

void lvgl_unlock(void)
{
    xSemaphoreGiveRecursive(lvgl_api_mux);
}

bool lvgl_lock(int timeout_ms)
{
    // Convert timeout in milliseconds to FreeRTOS ticks
    // If `timeout_ms` is set to -1, the program will block until the condition is
    // met
    const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTakeRecursive(lvgl_api_mux, timeout_ticks) == pdTRUE;
}

void btn_event_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    const char *txt = lv_label_get_text(lv_obj_get_child(btn, 0));

    // Controlla se è toggle
    bool is_toggle = lv_obj_has_flag(btn, LV_OBJ_FLAG_CHECKABLE);

    char msg[128];

    if (is_toggle)
    {
        bool state = lv_obj_has_state(btn, LV_STATE_CHECKED);

        snprintf(msg, sizeof(msg), "LAB1: \"%s %s\"\n", txt, state ? "ON" : "OFF");
    }
    else
    {
        snprintf(msg, sizeof(msg), "LAB0: \"BUTTON %s PRESSED\"\n", txt);
    }
    uart_write_bytes(UART_NUM, msg, strlen(msg));

    printf(msg);
}

void btn_gpio_set_high_level_cb(lv_event_t *e)
{
    gpio_set_level(GPIO_PIN, 1); // HIGH
    char msg[128];
    snprintf(msg, sizeof(msg), "GPIO STATUS: %s\n", "HIGH");
    printf(msg);
}

void btn_gpio_set_low_level_cb(lv_event_t *e)
{
    gpio_set_level(GPIO_PIN, 0); // LOW
    char msg[128];
    snprintf(msg, sizeof(msg), "GPIO STATUS: %s\n", "LOW");
    printf(msg);
}

void btn_gpio_read(lv_event_t *e)
{
    int button_state = gpio_get_level(GPIO_PIN_READ);
    if (lvgl_lock(-1))
    {
        lv_label_set_text_fmt(label_gpio_read_status, "GPIO STATUS: %d\n", button_state);

        lvgl_unlock();
    }
}

void lvgl_test_ui_init(lv_obj_t *parent)
{

    // Contenitore principale (verticale)
    lv_obj_t *main_container = lv_obj_create(parent);
    lv_obj_set_size(main_container, 240,
                    320); // adatta alle dimensioni del tuo schermo
    lv_obj_center(main_container);
    lv_obj_set_flex_flow(main_container, LV_FLEX_FLOW_COLUMN); // verticale
    lv_obj_set_style_pad_all(main_container, 10, 0);

    // Contenitore per le 3 label
    lv_obj_t *label_container = lv_obj_create(main_container);
    lv_obj_set_size(label_container, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(label_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(label_container, 5, 0);

    // 3 label
    // Label 1
    labels[0] = lv_label_create(label_container);
    lv_label_set_text(labels[0], "Label 0");

    // Label 2
    labels[1] = lv_label_create(label_container);
    lv_label_set_text(labels[1], "Label 1");

    // Label 3
    labels[2] = lv_label_create(label_container);
    lv_label_set_text(labels[2], "Label 2");

    // Contenitore griglia 2x2 per i bottoni
    lv_obj_t *btn_grid = lv_obj_create(main_container);
    lv_obj_set_size(btn_grid, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_layout(btn_grid, LV_LAYOUT_GRID);
    lv_obj_set_align(btn_grid, LV_ALIGN_BOTTOM_MID);

    static lv_coord_t col_dsc[] = {LV_PCT(50), LV_PCT(50), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {50, 50, 50, 50, LV_GRID_TEMPLATE_LAST};

    lv_obj_set_grid_dsc_array(btn_grid, col_dsc, row_dsc);
    lv_obj_set_style_pad_all(btn_grid, 5, 0);
    lv_obj_set_style_pad_column(btn_grid, 5, 0);
    lv_obj_set_style_pad_row(btn_grid, 5, 0);

    // Crea i 4 bottoni
    for (int i = 0; i < 4; i++)
    {
        lv_obj_t *btn;
        if (i >= 2)
        {
            btn = lv_btn_create(btn_grid);
            lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE); // toggle
        }
        else
        {
            btn = lv_btn_create(btn_grid); // normale
        }

        lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_STRETCH, i % 2, 1, LV_GRID_ALIGN_STRETCH, i / 2, 1);

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text_fmt(label, "Btn %d", i + 1);
        lv_obj_center(label);

        lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
    }

    // Button GPIO HIGH
    lv_obj_t *btn_set_gpio_high = lv_btn_create(btn_grid);
    lv_btn_create(btn_grid);
    lv_obj_set_grid_cell(btn_set_gpio_high, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2,
                         1);

    lv_obj_t *label_gpio_high = lv_label_create(btn_set_gpio_high);
    lv_label_set_text_fmt(label_gpio_high, "GPIO HIGH");
    lv_obj_center(label_gpio_high);

    lv_obj_add_event_cb(btn_set_gpio_high, btn_gpio_set_high_level_cb, LV_EVENT_CLICKED, NULL);

    // Button GPIO LOW
    lv_obj_t *btn_set_gpio_low = lv_btn_create(btn_grid);
    lv_btn_create(btn_grid);
    lv_obj_set_grid_cell(btn_set_gpio_low, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 2,
                         1);

    lv_obj_t *label_gpio_low = lv_label_create(btn_set_gpio_low);
    lv_label_set_text_fmt(label_gpio_low, "GPIO LOW");
    lv_obj_center(label_gpio_low);

    lv_obj_add_event_cb(btn_set_gpio_low, btn_gpio_set_low_level_cb, LV_EVENT_CLICKED, NULL);

    // Button GPIO READ
    lv_obj_t *btn_read_gpio = lv_btn_create(btn_grid);
    lv_btn_create(btn_grid);
    lv_obj_set_grid_cell(btn_read_gpio, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_STRETCH, 3, 1);

    lv_obj_t *label_gpio_read = lv_label_create(btn_read_gpio);
    lv_label_set_text_fmt(label_gpio_read, "GPIO READ");
    lv_obj_center(label_gpio_read);

    lv_obj_add_event_cb(btn_read_gpio, btn_gpio_read, LV_EVENT_CLICKED, NULL);

    // Label GPIO READ
    label_gpio_read_status = lv_label_create(main_container);
    lv_label_set_text(label_gpio_read_status, "GPIO STATUS: UNKNOWN");

    label_gpio_read_ADC = lv_label_create(main_container);
    lv_label_set_text(label_gpio_read_ADC, "ADC: UNKNOWN");

    lvgl_qmi8658_ui_init(main_container);
}

void lvgl_qmi8658_ui_init(lv_obj_t *parent)
{
    lv_obj_t *list = lv_list_create(parent);
    lv_obj_set_size(list, lv_pct(100), lv_pct(100));

    lv_obj_t *list_item = lv_list_add_btn(list, NULL, "accel_x");
    label_accel_x = lv_label_create(list_item);
    lv_label_set_text(label_accel_x, "0");

    list_item = lv_list_add_btn(list, NULL, "accel_y");
    label_accel_y = lv_label_create(list_item);
    lv_label_set_text(label_accel_y, "0");

    list_item = lv_list_add_btn(list, NULL, "accel_z");
    label_accel_z = lv_label_create(list_item);
    lv_label_set_text(label_accel_z, "0");

    list_item = lv_list_add_btn(list, NULL, "gyro_x");
    label_gyro_x = lv_label_create(list_item);
    lv_label_set_text(label_gyro_x, "0");

    list_item = lv_list_add_btn(list, NULL, "gyro_y");
    label_gyro_y = lv_label_create(list_item);
    lv_label_set_text(label_gyro_y, "0");

    list_item = lv_list_add_btn(list, NULL, "gyro_z");
    label_gyro_z = lv_label_create(list_item);
    lv_label_set_text(label_gyro_z, "0");

    qmi8658_timer = lv_timer_create(qmi8658_callback, 100, NULL);
}

void task(void *param)
{
    while (1)
    {
        uint32_t task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
        while (1)
        {
            // Lock the mutex due to the LVGL APIs are not thread-safe
            if (lvgl_lock(-1))
            {
                task_delay_ms = lv_timer_handler();
                // Release the mutex
                lvgl_unlock();
            }
            if (task_delay_ms > LVGL_TASK_MAX_DELAY_MS)
            {
                task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
            }
            else if (task_delay_ms < LVGL_TASK_MIN_DELAY_MS)
            {
                task_delay_ms = LVGL_TASK_MIN_DELAY_MS;
            }
            vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
        }
    }
}