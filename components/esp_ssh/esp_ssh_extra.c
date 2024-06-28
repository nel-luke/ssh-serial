#include <pwd.h>
#include <unistd.h>
#include <sys/wait.h>
#include <net/if.h>

struct passwd* getpwnam (const char * p) {
    return NULL;
}

int getpwuid_r(uid_t t, struct passwd * pwd, char * o, size_t s, struct passwd ** p) {
    return 0;
}

int gethostname(char *__name, size_t __len) {
    return 0;
}

uid_t getuid (void) {
    return 0;
}

pid_t waitpid (pid_t, int *, int) {
    return 0;
}

unsigned int if_nametoindex(const char *ifname) {
    return 0;
}