#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sdkconfig.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include <driver/gpio.h>

#define INTR_FLAG_DEFAULT 0

static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void handle_gpio(void* arg) {
    uint32_t io_num;
    while (1) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            printf("GPIO %d pressed!\n", CONFIG_SSC_WIFI_ON_DEMAND_PIN);
        }
    }
}

void app_main(void)
{
    printf("Hello world v.%s!\n", CONFIG_APP_PROJECT_VER);

    gpio_config_t io_config = {
            .intr_type = GPIO_INTR_NEGEDGE,
            .mode = GPIO_MODE_INPUT,
            .pin_bit_mask = (1ULL<<CONFIG_SSC_WIFI_ON_DEMAND_PIN),
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&io_config);

    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(handle_gpio, "gpio handler", 2048, NULL, 10, NULL);

    gpio_install_isr_service(INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(CONFIG_SSC_WIFI_ON_DEMAND_PIN, gpio_isr_handler, (void*) CONFIG_SSC_WIFI_ON_DEMAND_PIN);

    printf("Waiting\n");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
