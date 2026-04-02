#pragma once

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h" // se usi LVGL
#include <stdio.h>

#define ADC_CHANNEL ADC_CHANNEL_3 // GPIO4
#define ADC_UNIT ADC_UNIT_1

lv_obj_t *label_gpio_read_ADC;

static adc_oneshot_unit_handle_t adc_handle = NULL;
static adc_cali_handle_t adc_cali_handle = NULL;

void init_adc(void)
{
    // 1. Inizializza il driver ADC One-Shot
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &adc_handle));

    // 2. Configura il canale ADC
    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT, // tipicamente 12 bit
        .atten = ADC_ATTEN_DB_12,         // attenuazione 12 dB (0-3.6V circa)
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL, &chan_cfg));

    // 3. Crea il gestore calibrazione (curve fitting)
    adc_cali_curve_fitting_config_t cali_cfg = {
        .unit_id = ADC_UNIT,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    if (adc_cali_create_scheme_curve_fitting(&cali_cfg, &adc_cali_handle) != ESP_OK)
    {
        printf("Warning: ADC calibration not available, using raw readings\n");
        adc_cali_handle = NULL; // fallback
    }
}

void read_adc_task(void *arg)
{
    while (1)
    {
        int raw;
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL, &raw));

        int voltage_mv = 0;
        if (adc_cali_handle)
        {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle, raw, &voltage_mv));
        }
        else
        {
            voltage_mv = raw; // se calibrazione non disponibile, usa raw grezzo
        }

        char buf[128];
        snprintf(buf, sizeof(buf), "ADC -> RAW: %d V: %dmV", raw, voltage_mv);

        if (lvgl_lock(-1))
        {
            lv_label_set_text(label_gpio_read_ADC, buf);
            lvgl_unlock();
        }

        vTaskDelay(pdMS_TO_TICKS(200)); // lettura ogni 200 ms
    }
}
