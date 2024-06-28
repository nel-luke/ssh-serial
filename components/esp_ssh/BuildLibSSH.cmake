macro(install)
endmacro()

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(WITH_ZLIB OFF)
set(WITH_SFTP OFF)
set(WITH_DEBUG_CALLTRACE OFF)
set(WITH_MBEDTLS ON)
set(WITH_PCAP OFF)
set(BUILD_SHARED_LIBS OFF)
set(WITH_EXAMPLES OFF)
set(WITH_SYMBOL_VERSIONING OFF)

#set(ZLIB_BUILD_EXAMPLES OFF)
#set(SKIP_INSTALL_ALL TRUE)
#set(UNIX TRUE)
#add_subdirectory(./external/zlib EXCLUDE_FROM_ALL)

set(MBEDTLS_ROOT_DIR ${IDF_PATH}/components/mbedtls/mbedtls ${COMPONENT_DIR}/fake_lib)
set(MBEDTLS_INCLUDE_DIR ${IDF_PATH}/components/mbedtls/mbedtls/include ${COMPONENT_DIR}/include)

set(HAVE_GETADDRINFO TRUE)
set(HAVE_POLL TRUE)
set(HAVE_TERMIOS_H TRUE)
list(APPEND SUPPORTED_COMPILER_FLAGS "-Wno-error=char-subscripts" "-Wno-error=format=")

add_subdirectory(${COMPONENT_DIR}/external/libssh EXCLUDE_FROM_ALL)
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/external/libssh/config.h "#include <esp_ssh_extras.h>")

target_link_libraries(${COMPONENT_LIB} INTERFACE ssh)