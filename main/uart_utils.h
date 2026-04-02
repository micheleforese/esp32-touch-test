#pragma once

#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl_ui.h"

#define UART_NUM UART_NUM_1
#define UART_PIN_TX (GPIO_NUM_43)
#define UART_PIN_RX (GPIO_NUM_44)
#define UART_BUFF_SIZE (1024)
#define UART_PATTERN_ENDING_MSG_CHAR ('\n')

#define MAX_MESSAGE_LENGTH (64)

typedef struct
{
    uint8_t label_num;
    char data[MAX_MESSAGE_LENGTH];
} Message;

void uart_init();
void uart_event_task(void *pvParameters);
bool parse_message(const char *line, Message *msg_out);
void my_uart_callback(Message *msg);