idf_component_register(
    SRCS 
    "main.c" 
    "bsp_qmi8658.c"
    "bsp_qmi8658_utils.c"
    "screen.c"
    "uart_utils.c"
    "lvgl_utils.c"
    PRIV_REQUIRES esp_adc spi_flash
    INCLUDE_DIRS ".")
