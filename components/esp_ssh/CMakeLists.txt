set(COMPONENT_ADD_INCLUDEDIRS
        ./include
        ./external/libssh/include
        ${IDF_PATH}/components/mbedtls/port/include
        ${CMAKE_CURRENT_BINARY_DIR}/external/libssh/include
)

set(COMPONENT_REQUIRES mbedtls)
set(COMPONENT_PRIV_REQUIRES esp_http_server nvs_flash esp_netif esp_wifi esp_eth driver vfs console)

set(COMPONENT_SRCS
        ./esp_ssh.c
)

register_component()

include(BuildLibSSH.cmake)
target_link_libraries(${COMPONENT_LIB} INTERFACE ssh)
