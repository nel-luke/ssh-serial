#include <pwd.h>
#include <unistd.h>
#include <sys/wait.h>
#include <net/if.h>
#include <esp_log.h>

static const char* TAG = "SSH_Fakes";

struct passwd* getpwnam (const char * p) {
    ESP_LOGW(TAG, "%s called", __func__);
    return NULL;
}

int getpwuid_r(uid_t t, struct passwd * pwd, char * o, size_t s, struct passwd ** p) {
    ESP_LOGW(TAG, "%s called", __func__);
    return 0;
}

int gethostname(char *__name, size_t __len) {
    ESP_LOGW(TAG, "%s called", __func__);
    return 0;
}

uid_t getuid (void) {
    ESP_LOGW(TAG, "%s called", __func__);
    return 0;
}

pid_t waitpid (pid_t, int *, int) {
    ESP_LOGW(TAG, "%s called", __func__);
    return 0;
}

unsigned int if_nametoindex(const char *ifname) {
    ESP_LOGW(TAG, "%s called", __func__);
    return 0;
}

const char *gai_strerror(int ecode) {
    ESP_LOGW(TAG, "%s called", __func__);
    return "HELP!";
}