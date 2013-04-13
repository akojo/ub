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

static int term_fd = 0;
static int srv_fd;
static strbuf_t *sbuf;

void process_response(char *str);
void process_input(char *str);

void handle_response(cJSON *json);
char *handle_cmd();
char *handle_message();

int main(void)
{
	client_t *client = client_new();

	sbuf = strbuf_new();
	srv_fd = client_connect(client, "localhost", CHAT_PORT, process_response);
	client_add_readfd(client, term_fd, process_input);
	client_run(client);

	client_delete(client);
	quit("bye");
}

void process_input(char *str)
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

void process_response(char *str)
{
	char *s = str;
	for (;;) {
		cJSON *json;

		while (*s != '\n' && *s != '\0')
			strbuf_append(sbuf, *s++);

		if (*s == '\0')
			return;

		json = cJSON_Parse(strbuf_buffer(sbuf));
		if (!json)
			return;

		handle_response(json);

		cJSON_Delete(json);
		strbuf_reset(sbuf);
	}
}

void handle_response(cJSON *json)
{
	static char result[BUFSIZ];

	int len;
	cJSON *type = cJSON_GetObjectItem(json, "msgType");
	cJSON *data = cJSON_GetObjectItem(json, "data");

	if (!strcmp(type->valuestring, NICK_STR)) {
		cJSON *nick = cJSON_GetObjectItem(data, "nick");
		cJSON *prev = cJSON_GetObjectItem(data, "prev_nick");

		len = snprintf(result, BUFSIZ, "* '%s' is now known as '%s'\n",
					   prev->valuestring, nick->valuestring);
	} else if (!strcmp(type->valuestring, MSG_STR)) {
		cJSON *nick = cJSON_GetObjectItem(data, "nick");
		cJSON *msg = cJSON_GetObjectItem(data, "message");

		len = snprintf(result, BUFSIZ, "<%s> %s\n",
					   nick->valuestring, msg->valuestring);
	} else {
		result[0] = '\0';
		len = 0;
	}

	if (len > 0)
		write(term_fd, result, len);
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
