idf_component_register(SRCS ssh-serial.c ssh-example.c gpio.c nvs.c
        INCLUDE_DIRS "."
        PRIV_REQUIRES driver nvs_flash esp_netif esp_wifi wifi esp_ssh)

component_compile_options("-Wno-unknown-pragmas")