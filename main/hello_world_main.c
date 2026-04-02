/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <inttypes.h>
#include <stdio.h>

#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"

#include "bsp_qmi8658.h"
#include "bsp_qmi8658_utils.h"
#include "esp_log.h"
#include "lv_demos.h"
#include "lvgl.h"
#include "screen.h"
#include "uart_utils.h"

#include "config.h"

#include "esp_psram.h"

#include "adc.h"

void print_esp_info(void)
{
    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ", CONFIG_IDF_TARGET, chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);

    if (esp_flash_get_size(NULL, &flash_size) == 0)
    {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());
}

void test_spiram_psram(void)
{
    if (esp_psram_init() == ESP_OK)
    {
        ESP_LOGI("PSRAM", "PSRAM inizializzata correttamente.");
        ESP_LOGI("PSRAM", "Dimensione PSRAM: %d bytes", esp_psram_get_size());
    }
    else
    {
        ESP_LOGE("PSRAM", "Errore nell'inizializzazione della PSRAM.");
    }
}

void gpio_init(void)
{
    // Configure pin as output
    gpio_config_t io_conf_output = {.pin_bit_mask = (1ULL << GPIO_PIN),
                                    .mode = GPIO_MODE_OUTPUT,
                                    .pull_up_en = 0,
                                    .pull_down_en = 0,
                                    .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&io_conf_output);

    gpio_config_t io_conf_input = {.pin_bit_mask = (1ULL << GPIO_PIN_READ),
                                   .mode = GPIO_MODE_INPUT,
                                   .pull_up_en = 1,
                                   .pull_down_en = 0,
                                   .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&io_conf_input);
}

void app_main(void)
{
    lvgl_api_mux = xSemaphoreCreateRecursiveMutex();

    print_esp_info();
    gpio_init();
    uart_init();
    lv_init();
    display_init();
    touch_init();
    init_adc();
    ESP_ERROR_CHECK(bsp_qmi8658_init());
    lvgl_tick_timer_init(LVGL_TICK_PERIOD_MS);
    lv_port_disp_init();
    lv_port_indev_init();
    bsp_brightness_init();
    bsp_brightness_set_level(90);

    if (lvgl_lock(-1))
    {

        lvgl_test_ui_init(lv_scr_act());

        lvgl_unlock();
    }
    xTaskCreatePinnedToCore(task, "bsp_lv_port_task", 1024 * 20, NULL, 5, NULL, 1);
    xTaskCreate(read_adc_task, "read_adc_task", 2048, NULL, 5, NULL);
}
