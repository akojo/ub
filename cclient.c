#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "client.h"
#include "common.h"
#include "util.h"
#include "cJSON.h"
#include "strbuf.h"

#include <unistd.h>

typedef struct {
	char *msg_type;
	char *(*handler)(cJSON *);
} cmd_t;

void process_term_input(char *str);

char *handle_cmd();
char *handle_message();

void process_server_message(char *str);

char *cmd_msg(cJSON *data);
char *cmd_nick(cJSON *data);
char *cmd_error(cJSON *data);

static int term_fd = 0;
static int srv_fd;
static strbuf_t *sbuf;

static cmd_t cmd_handlers[] = {
	{ "MSG", cmd_msg },
	{ "NICK", cmd_nick },
	{ "ERROR", cmd_error }
};
static int ncmds = sizeof(cmd_handlers) / sizeof(cmd_t);

int main(void)
{
	client_t *client = client_new();

	sbuf = strbuf_new();
	srv_fd = client_connect(client, "localhost", CHAT_PORT, process_server_message);
	client_add_readfd(client, term_fd, process_term_input);
	client_run(client);

	client_delete(client);
	quit("bye");
}

void process_term_input(char *str)
{
	char *response;
	int len = strlen(str);
	while (isspace(str[len - 1])) {
		str[len - 1] = '\0';
		len--;
	}

    if (str[0] == '/')
        response = handle_cmd(str + 1);
    else
        response = handle_message(str);

	if (response) {
		write(srv_fd, response, strlen(response));
		free(response);
	}
}

char *handle_cmd(char *cmd)
{
    char *output;
    char *args = cmd;
    cJSON *json;

    cmd = strsep(&args, " ");

    json = cJSON_CreateObject();
	if (!strcmp(cmd, "nick")) {
		cJSON_AddStringToObject(json, "msgType", NICK_STR);
		cJSON_AddStringToObject(json, "data", args);
    } else {
        output = NULL;
        goto finish;
    }

    output = cJSON_PrintUnformatted(json);
finish:
    cJSON_Delete(json);
    return output;
}

char *handle_message(char *msg)
{
    char *output;
    cJSON *json;

    if (strlen(msg) == 0)
        return NULL;
   
    json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "msgType", MSG_STR);
    cJSON_AddStringToObject(json, "data", msg);

    output = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    return output;
}

void process_server_message(char *str)
{
	char *s = str;

	for (;;) {
		int i;
		cJSON *json, *type;

		while (*s != '\n' && *s != '\0')
			strbuf_append(sbuf, *s++);

		if (*s == '\0')
			return;

		json = cJSON_Parse(strbuf_buffer(sbuf));
		if (!json)
			return;

		type = cJSON_GetObjectItem(json, "msgType");
		for (i = 0; i < ncmds; i++) {
			if (!strcmp(type->valuestring, cmd_handlers[i].msg_type)) {
				cJSON *data = cJSON_GetObjectItem(json, "data");
				char *result = cmd_handlers[i].handler(data);

				if (result) {
					write(term_fd, result, strlen(result));
					free(result);
				}

				break;
			}
		}

		cJSON_Delete(json);
		strbuf_reset(sbuf);
	}
}

char *cmd_msg(cJSON *data)
{
	char *result = malloc(BUFSIZ);

	cJSON *nick = cJSON_GetObjectItem(data, "nick");
	cJSON *msg = cJSON_GetObjectItem(data, "message");

	snprintf(result, BUFSIZ, "<%s> %s\n",
			 nick->valuestring, msg->valuestring);

	return result;
}

char *cmd_nick(cJSON *data)
{
	char *result = malloc(BUFSIZ);

	cJSON *nick = cJSON_GetObjectItem(data, "nick");
	cJSON *prev = cJSON_GetObjectItem(data, "prev_nick");

	snprintf(result, BUFSIZ, "* '%s' is now known as '%s'\n",
			 prev->valuestring, nick->valuestring);

	return result;
}

char *cmd_error(cJSON *data)
{
	char *message = cJSON_PrintUnformatted(data);
	warning("error: '%s'", message);
	free(message);

	return NULL;
}
