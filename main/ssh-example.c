#include <libssh/callbacks.h>
#include <libssh/server.h>

#include <stdlib.h>
#include <stdio.h>
#include <esp_log.h>

#ifndef BUF_SIZE
#define BUF_SIZE 1048576
#endif

#define SESSION_END (SSH_CLOSED | SSH_CLOSED_ERROR)
#define AUTH_KEYS_MAX_LINE_SIZE 2048

#define DEF_STR_SIZE 1024
char authorizedkeys[DEF_STR_SIZE] = {0};
char username[128] = "myuser";
char password[128] = "mypassword";

#define TAG "SSH"

/* An userdata struct for session. */
struct session_data_struct {
    /* Pointer to the channel the session will allocate. */
    ssh_channel channel;
    int auth_attempts;
    int authenticated;
};

static int data_function(ssh_session session, ssh_channel channel, void *data,
                         uint32_t len, int is_stderr, void *userdata) {
    (void) session;
    (void) channel;
    (void) len;
    (void) is_stderr;
    (void) userdata;

    char* msg = (char*) data;

    if (msg[len-1] == 3) {
        ssh_channel_close(channel);
        goto finish;
    }

    putchar(msg[len-1]);
    fflush(stdout);
    ssh_channel_write(channel, data+len-1, 1);

finish:
    return SSH_OK;
}

static int pty_request(ssh_session session, ssh_channel channel,
                       const char *term, int cols, int rows, int py, int px,
                       void *userdata) {
    (void) session;
    (void) channel;
    (void) userdata;

    ESP_LOGI(TAG, "Pty requested: %s - cols: %d, rows: %d, py: %d, px: %d", term, cols, rows, py, px);
    return SSH_OK;
}

static int shell_request(ssh_session session, ssh_channel channel,
                         void *userdata) {

    (void) session;
    (void) channel;
    (void) userdata;

    return SSH_OK;
}

static int auth_password(ssh_session session, const char *user,
                         const char *pass, void *userdata) {
    struct session_data_struct *sdata = (struct session_data_struct *) userdata;

    (void) session;

    if (strcmp(user, username) == 0 && strcmp(pass, password) == 0) {
        sdata->authenticated = 1;
        return SSH_AUTH_SUCCESS;
    }

    sdata->auth_attempts++;
    return SSH_AUTH_DENIED;
}

static int auth_publickey(ssh_session session,
                          const char *user,
                          struct ssh_key_struct *pubkey,
                          char signature_state,
                          void *userdata)
{
    struct session_data_struct *sdata = (struct session_data_struct *) userdata;
    ssh_key key = NULL;
    FILE *fp = NULL;
    char line[AUTH_KEYS_MAX_LINE_SIZE] = {0};
    char *p = NULL;
    const char *q = NULL;
    unsigned int lineno = 0;
    int result;
    int i;
    enum ssh_keytypes_e type;

    (void) user;
    (void) session;

    if (signature_state == SSH_PUBLICKEY_STATE_NONE) {
        return SSH_AUTH_SUCCESS;
    }

    if (signature_state != SSH_PUBLICKEY_STATE_VALID) {
        return SSH_AUTH_DENIED;
    }

    fp = fopen(authorizedkeys, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: opening authorized keys file %s failed, reason: %s\n",
                authorizedkeys, strerror(errno));
        return SSH_AUTH_DENIED;
    }

    while (fgets(line, sizeof(line), fp)) {
        lineno++;

        /* Skip leading whitespace and ignore comments */
        p = line;

        for (i = 0; i < AUTH_KEYS_MAX_LINE_SIZE; i++) {
            if (!isspace((int)p[i])) {
                break;
            }
        }

        if (i >= AUTH_KEYS_MAX_LINE_SIZE) {
            fprintf(stderr,
                    "warning: The line %d in %s too long! Skipping.\n",
                    lineno,
                    authorizedkeys);
            continue;
        }

        if (p[i] == '#' || p[i] == '\0' || p[i] == '\n') {
            continue;
        }

        q = &p[i];
        for (; i < AUTH_KEYS_MAX_LINE_SIZE; i++) {
            if (isspace((int)p[i])) {
                p[i] = '\0';
                break;
            }
        }

        type = ssh_key_type_from_name(q);

        i++;
        if (i >= AUTH_KEYS_MAX_LINE_SIZE) {
            fprintf(stderr,
                    "warning: The line %d in %s too long! Skipping.\n",
                    lineno,
                    authorizedkeys);
            continue;
        }

        q = &p[i];
        for (; i < AUTH_KEYS_MAX_LINE_SIZE; i++) {
            if (isspace((int)p[i])) {
                p[i] = '\0';
                break;
            }
        }

        result = ssh_pki_import_pubkey_base64(q, type, &key);
        if (result != SSH_OK) {
            fprintf(stderr,
                    "Warning: Cannot import key on line no. %d in authorized keys file: %s\n",
                    lineno,
                    authorizedkeys);
            continue;
        }

        result = ssh_key_cmp(key, pubkey, SSH_KEY_CMP_PUBLIC);
        ssh_key_free(key);
        if (result == 0) {
            sdata->authenticated = 1;
            fclose(fp);
            return SSH_AUTH_SUCCESS;
        }
    }
    if (ferror(fp) != 0) {
        fprintf(stderr,
                "Error: Reading from authorized keys file %s failed, reason: %s\n",
                authorizedkeys, strerror(errno));
    }
    fclose(fp);

    /* no matches */
    return SSH_AUTH_DENIED;
}

static ssh_channel channel_open(ssh_session session, void *userdata) {
    struct session_data_struct *sdata = (struct session_data_struct *) userdata;

    sdata->channel = ssh_channel_new(session);
    return sdata->channel;
}

static void handle_session(ssh_event event, ssh_session session) {
    int n;

    /* Our struct holding information about the session. */
    struct session_data_struct sdata = {
            .channel = NULL,
            .auth_attempts = 0,
            .authenticated = 0
    };

    struct ssh_channel_callbacks_struct channel_cb = {
            .userdata = NULL,
            .channel_pty_request_function = pty_request,
            .channel_shell_request_function = shell_request,
            .channel_data_function = data_function,
    };

    struct ssh_server_callbacks_struct server_cb = {
            .userdata = &sdata,
            .auth_password_function = auth_password,
            .channel_open_request_session_function = channel_open,

    };

    if (authorizedkeys[0]) {
        server_cb.auth_pubkey_function = auth_publickey;
        ssh_set_auth_methods(session, SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_PUBLICKEY);
    } else
        ssh_set_auth_methods(session, SSH_AUTH_METHOD_PASSWORD);

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EmptyDeclOrStmt"
    ssh_callbacks_init(&server_cb);
    ssh_callbacks_init(&channel_cb);
#pragma clang diagnostic pop

    ssh_set_server_callbacks(session, &server_cb);

    if (ssh_handle_key_exchange(session) != SSH_OK) {
        fprintf(stderr, "%s\n", ssh_get_error(session));
        return;
    }

    ssh_event_add_session(event, session);

    n = 0;
    while (sdata.authenticated == 0 || sdata.channel == NULL) {
        /* If the user has used up all attempts, or if he hasn't been able to
         * authenticate in 10 seconds (n * 100ms), disconnect. */
        if (sdata.auth_attempts >= 3 || n >= 100) {
            return;
        }

        if (ssh_event_dopoll(event, 100) == SSH_ERROR) {
            fprintf(stderr, "%s\n", ssh_get_error(session));
            return;
        }
        n++;
    }

    ssh_set_channel_callbacks(sdata.channel, &channel_cb);

    ESP_LOGI("SSH", "Ready for whatever");

    do {
        /* Poll the main event which takes care of the session, the channel and
         * even our child process's stdout/stderr (once it's started). */
        if (ssh_event_dopoll(event, -1) == SSH_ERROR) {
            ssh_channel_close(sdata.channel);
        }
    } while(ssh_channel_is_open(sdata.channel));

    ESP_LOGI(TAG, "Closing channel");

    ssh_channel_send_eof(sdata.channel);
    ssh_channel_close(sdata.channel);

    /* Wait up to 5 seconds for the client to terminate the session. */
    for (n = 0; n < 50 && (ssh_get_status(session) & SESSION_END) == 0; n++) {
        ssh_event_dopoll(event, 100);
    }
}

void *session_thread(void *arg) {
    ssh_session session = arg;
    ssh_event event;

    event = ssh_event_new();
    if (event != NULL) {
        /* Blocks until the SSH session ends by either
         * child thread exiting, or client disconnecting. */
        handle_session(event, session);
        ssh_event_free(event);
    } else {
        fprintf(stderr, "Could not create polling context\n");
    }
    ssh_disconnect(session);
    ssh_free(session);
    return NULL;
}
