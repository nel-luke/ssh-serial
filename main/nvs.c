#include "nvs.h"

#include <sdkconfig.h>

#include <esp_log.h>
#include <nvs_flash.h>

#define MAIN_NVS_NAMESPACE "main"
#define MAIN_NVS_HOSTNAME_KEY "hostname"

static const char* TAG = "MAIN_NVS";

/**
 * @brief Initializes the ESP32's non-volatile flash (NVS).
 *
 * @note If the NVS is corrupted, this function will reformat it.
 */
void main_nvs_init(void) {
    ESP_LOGI(TAG, "Initializing NVS");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition is truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

/**
 * @brief Get the hostname from NVS. If it doesn't exist, store the default hostname in it.
 *
 * @return - A pointer to a string buffer containing the hostname.
 * - NULL if there is an error.
 *
 * @note This pointer should be freed after use.
 */
char* main_nvs_get_hostname(void) {
    ESP_LOGI(TAG, "Retrieving hostname...");
    char* hostname = NULL;
    nvs_handle_t nvs;
    esp_err_t err = nvs_open(MAIN_NVS_NAMESPACE, NVS_READONLY, &nvs);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Hostname not initialized yet!");
        ESP_ERROR_CHECK(nvs_open(MAIN_NVS_NAMESPACE, NVS_READWRITE, &nvs));
        ESP_LOGI(TAG, "Setting host name to %s", CONFIG_SSC_HOSTNAME);
        ESP_ERROR_CHECK(nvs_set_str(nvs, MAIN_NVS_HOSTNAME_KEY, CONFIG_SSC_HOSTNAME));
        ESP_ERROR_CHECK(nvs_commit(nvs));
        nvs_close(nvs);
    } else {
        ESP_ERROR_CHECK(err);
        size_t len = 0;
        ESP_ERROR_CHECK(nvs_get_str(nvs, MAIN_NVS_HOSTNAME_KEY, NULL, &len));
        hostname = malloc(len);
        ESP_ERROR_CHECK(nvs_get_str(nvs, MAIN_NVS_HOSTNAME_KEY, hostname, &len));
        nvs_close(nvs);
    }
    ESP_LOGI(TAG, "Host name is %s", hostname);
    return hostname;
}