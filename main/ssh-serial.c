#include "ifaddrs.h"
#include <sys/cdefs.h>
#include <stdlib.h>
#include <stdio.h>

#include <sdkconfig.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <driver/gpio.h>
#include <nvs_flash.h>

#include <lwip/sockets.h>

#include <wifi.h>

#include <libssh/server.h>
#include <pthread.h>

extern void *session_thread(void *arg);

static const char private_key[] =
        "-----BEGIN OPENSSH PRIVATE KEY-----\n"
        "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\n"
        "-----END OPENSSH PRIVATE KEY-----";

#define HOST_NAME_KEY "host_name"
#define MAX_HOST_NAME_LEN 16
#define DEFAULT_HOST_NAME "SSH-Serial"

#define INTR_FLAG_DEFAULT 0

static QueueHandle_t gpio_evt_queue = NULL;

static const char TAG[] = "main";

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

_Noreturn static void handle_gpio(void* arg) {
    (void) arg;
    uint32_t io_num;
    while (1) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            printf("GPIO %d pressed!\n", CONFIG_SSC_WIFI_ON_DEMAND_PIN);
        }
    }
}

void set_bind_options(ssh_bind sshbind) {
    int verbosity = SSH_LOG_TRACE;
    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_IMPORT_KEY_STR, private_key);
    bool f = false;
    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_PROCESS_CONFIG, &f);
}

void app_main(void)
#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"
{
//    printf("Hello world v.%s!\n", CONFIG_APP_PROJECT_VER);

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



    ESP_LOGI(TAG, "Initializing NVS");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition is truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    char host_name[MAX_HOST_NAME_LEN];
    size_t length = MAX_HOST_NAME_LEN;

    ESP_LOGI(TAG, "Retrieving host name...");
    nvs_handle_t nvs;
    err = nvs_open("storage", NVS_READONLY, &nvs);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Host name not initialized yet!");
        ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvs));
        ESP_LOGI(TAG, "Setting host name to %s", DEFAULT_HOST_NAME);
        ESP_ERROR_CHECK(nvs_set_str(nvs, HOST_NAME_KEY, DEFAULT_HOST_NAME));
        ESP_ERROR_CHECK(nvs_commit(nvs));
        nvs_close(nvs);
//        strcpy(host_name, DEFAULT_HOST_NAME);
        //ESP_ERROR_CHECK(nvs_get_str(nvs, "host_name", host_name, &length));
    } else {
        ESP_ERROR_CHECK(err);
        ESP_ERROR_CHECK(nvs_get_str(nvs, HOST_NAME_KEY, host_name, &length));
        nvs_close(nvs);
    }
    ESP_LOGI(TAG, "Host name is %s", host_name);

    ESP_LOGI(TAG, "Starting event loop");
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "Starting Wi-Fi");
    // Initialize networking stack
    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_t *netif = esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_netif_set_hostname(netif, host_name));
    wifi_start(host_name);

//    ESP_LOGI(TAG, "Starting SNTP");
//    misc_start_sntp();

    ESP_LOGI(TAG, "Retrieving MAC address...");
    uint8_t mac_addr[6];
    ESP_ERROR_CHECK(esp_netif_get_mac(netif, mac_addr));
    ESP_LOGI(TAG, "MAC address is %x:%x:%x:%x:%x:%x",
             mac_addr[0], mac_addr[1], mac_addr[2],
             mac_addr[3], mac_addr[4], mac_addr[5]);

    esp_netif_ip_info_t ip_info = { 0 };
    esp_netif_get_ip_info(netif, &ip_info);
    struct in_addr ip_struct = { 0 };
    inet_addr_from_ip4addr(&ip_struct, &ip_info.ip);

    char ip_addr[INET_ADDRSTRLEN];
    lwip_inet_ntop(AF_INET, &ip_struct, ip_addr, INET_ADDRSTRLEN);


    ssh_bind sshbind;
    ssh_session session;
    int rc;

    rc = ssh_init();
    if (rc < 0) {
        fprintf(stderr, "ssh_init failed\n");
        return;
    }

    sshbind = ssh_bind_new();
    if (sshbind == NULL) {
        fprintf(stderr, "ssh_bind_new failed\n");
        ssh_finalize();
        return;
    }

    set_bind_options(sshbind);

    if(ssh_bind_listen(sshbind) < 0) {
        fprintf(stderr, "%s\n", ssh_get_error(sshbind));
        ssh_bind_free(sshbind);
        ssh_finalize();
        return;
    }


    printf("Waiting\n");
    while (1) {
        service_wifi();

        session = ssh_new();
        if (session == NULL) {
            fprintf(stderr, "Failed to allocate session\n");
            continue;
        }

        /* Blocks until there is a new incoming connection. */
        if(ssh_bind_accept(sshbind, session) != SSH_ERROR) {
            pthread_t tid;

            rc = pthread_create(&tid, NULL, session_thread, session);
            if (rc == 0) {
                pthread_detach(tid);
                continue;
            }
            fprintf(stderr, "Failed to pthread_create\n");
        } else {
            fprintf(stderr, "%s\n", ssh_get_error(sshbind));
        }
        /* Since the session has been passed to a child fork, do some cleaning
         * up at the parent process. */
        ssh_disconnect(session);
        ssh_free(session);
        ESP_LOGI(TAG, "Waiting again");
    }

    ssh_bind_free(sshbind);
    ssh_finalize();
}
#pragma clang diagnostic pop
