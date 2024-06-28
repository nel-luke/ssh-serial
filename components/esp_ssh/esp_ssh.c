#include "esp_ssh.h"

#include <libssh/libssh.h>

static ssh_channel t;

void test(void) {
    ssh_channel_close(t);
}