#ifndef ESP_SSH_EXTRAS_H_
#define ESP_SSH_EXTRAS_H_

#define MBEDTLS_GCM_ALT
#define MBEDTLS_THREADING_C
#define MBEDTLS_THREADING_PTHREAD
#include <gcm_alt.h>

#define IMAXBEL 50
#define ECHOCTL 1
#define ECHOKE 2
#define PENDIN 3
#define VEOL2 3
#define VREPRINT 4
#define VWERASE 5
#define VLNEXT 6
#define VDISCARD 7

const char *gai_strerror(int ecode);

int socketpair(int domain, int type, int protocol, int sv[2]);
#define PF_UNIX 0

#endif /* ESP_SSH_EXTRAS_H_ */