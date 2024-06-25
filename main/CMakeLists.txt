idf_component_register(SRCS "ssh-serial.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES wifi driver nvs_flash esp_netif esp_wifi)
