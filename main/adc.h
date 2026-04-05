#pragma once

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h" // se usi LVGL
#include <stdio.h>

const char *TAG = "adc";

#define ADC_CHANNEL ADC_CHANNEL_3 // GPIO4
#define ADC_UNIT ADC_UNIT_1

lv_obj_t *label_gpio_read_ADC;

static adc_oneshot_unit_handle_t adc_handle = NULL;
static adc_cali_handle_t adc_cali_handle = NULL;

static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten,
                                 adc_cali_handle_t *out_handle);

void adc_init(void)
{
    //-------------ADC1 Init---------------//
    adc_oneshot_unit_init_cfg_t adc_init_cfg = {
        .unit_id = ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_init_cfg, &adc_handle));

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t adc_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL, &adc_config));

    //-------------ADC1 Calibration Init---------------//
    adc_cali_handle_t adc_cali_handle = NULL;
    bool do_calibration =
        adc_calibration_init(ADC_UNIT, ADC_CHANNEL, ADC_ATTEN_DB_12, &adc_cali_handle);
}

bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten,
                          adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

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

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated)
    {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK)
        {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated)
    {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK)
        {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "Calibration Success");
    }
    else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated)
    {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    }
    else
    {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

void adc_read_task(void *arg)
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
