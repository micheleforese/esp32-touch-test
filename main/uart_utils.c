#include "uart_utils.h"

static const char *TAG_UART_RECEIVER = "UART_RECEIVER";
static QueueHandle_t uart_queue = NULL;

void my_uart_callback(Message *msg)
{
    ESP_LOGI("UART", "Ricevuto LAB%d: %s", msg->label_num, msg->data);

    if (lvgl_lock(-1))
    {
        // Esempio con array di label
        if (msg->label_num < NUM_LABELS)
        {
            lv_label_set_text(labels[msg->label_num], msg->data);
        }
        lvgl_unlock();
    }
}

bool parse_message(const char *line, Message *msg_out)
{
    if (strncmp(line, "LAB", 3) != 0)
    {
        ESP_LOGI(TAG_UART_RECEIVER, "Messaggio ignorato: %s", line);
        return false;
    }

    uint8_t label_num;
    char content[MAX_MESSAGE_LENGTH];

    if (sscanf(line, "LAB%hhd: \"%63[^\"]\"", &label_num, content) == 2)
    {
        msg_out->label_num = label_num;
        strncpy(msg_out->data, content, MAX_MESSAGE_LENGTH - 1);
        msg_out->data[MAX_MESSAGE_LENGTH - 1] = '\0';

        return true;
    }
    else
    {
        ESP_LOGW(TAG_UART_RECEIVER, "Formato non valido: %s", line);
    }
    return false;
}

void uart_event_task(void *pvParameters)
{
    uint8_t data[UART_BUFF_SIZE];
    uart_event_t event;

    while (1)
    {
        if (xQueueReceive(uart_queue, &event, portMAX_DELAY))
        {
            if (event.type == UART_PATTERN_DET)
            {
                int pos = uart_pattern_pop_pos(UART_NUM);
                if (pos != -1 && pos < UART_BUFF_SIZE)
                {
                    int len = uart_read_bytes(UART_NUM, data, pos + 1, 100 / portTICK_PERIOD_MS);
                    if (len > 0)
                    {
                        data[len - 1] = '\0'; // rimuove '\n'
                        Message msg;
                        if (parse_message((char *)data, &msg))
                        {
                            my_uart_callback(&msg);
                        }
                    }
                }
            }
        }
    }
}

void uart_init()
{
    const char *TAG = "UART";

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        // .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, // Meglio disattivare per debug
    };

    ESP_ERROR_CHECK(
        uart_driver_install(UART_NUM, UART_BUFF_SIZE * 2, UART_BUFF_SIZE * 2, 20, &uart_queue, 0));
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));

    ESP_ERROR_CHECK(
        uart_set_pin(UART_NUM, UART_PIN_TX, UART_PIN_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_ERROR_CHECK(
        uart_enable_pattern_det_baud_intr(UART_NUM, UART_PATTERN_ENDING_MSG_CHAR, 1, 9, 0, 0));
    ESP_ERROR_CHECK(uart_pattern_queue_reset(UART_NUM, 20));

    xTaskCreate(uart_event_task, "uart_event_task", UART_BUFF_SIZE * 4, NULL, 10, NULL);
    ESP_LOGI(TAG, "UART init");
}