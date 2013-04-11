#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"
#include "common.h"
#include "util.h"
#include "cJSON.h"

#include <unistd.h>

static int term_fd = 0;
static int srv_fd;

ssize_t process_response(char *str);
ssize_t process_input(char *str);

char *handle_cmd();
char *handle_message();

int main(void)
{
	client_t *client = client_new();
	srv_fd = client_connect(client, "localhost", CHAT_PORT, process_response);
	client_add_readfd(client, term_fd, process_input);
	client_run(client);

	client_delete(client);
	quit("bye");
}

ssize_t process_input(char *str)
{
	char *response;

    if (str[0] == '/')
        response = handle_cmd(str + 1);
    else
        response = handle_message(str);

	if (response) {
		write(srv_fd, response, strlen(response));
		free(response);
	}
    return strlen(str);
}

ssize_t process_response(char *str)
{
	int len;
	static char result[1024];
	cJSON *json = cJSON_Parse(str);
	cJSON *response = cJSON_GetObjectItem(json, "response");

	if (!strcmp(response->valuestring, NICK_STR)) {
		cJSON *nick = cJSON_GetObjectItem(json, "nick");
		cJSON *prev = cJSON_GetObjectItem(json, "prev_nick");
		snprintf(result, 1024, "* '%s' is now known as '%s'\n", prev->valuestring, nick->valuestring);
	} else if (!strcmp(response->valuestring, MSG_STR)) {
		cJSON *nick = cJSON_GetObjectItem(json, "nick");
		cJSON *msg = cJSON_GetObjectItem(json, "message");
		snprintf(result, 1024, "<%s> %s\n", nick->valuestring, msg->valuestring);
	} else {
		result[0] = '\0';
	}

	if ((len = strlen(result)) > 0)
		write(term_fd, result, len);

	cJSON_Delete(json);
    return strlen(str);
}

char *handle_cmd(char *cmd)
{
    char *output;
    char *args = cmd;
    cJSON *json;

    cmd = strsep(&args, " ");

    json = cJSON_CreateObject();
	if (!strcmp(cmd, "nick")) {
		cJSON_AddStringToObject(json, "cmd", NICK_STR);
		cJSON_AddStringToObject(json, "nick", args);
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

    cJSON_AddStringToObject(json, "cmd", MSG_STR);
    cJSON_AddStringToObject(json, "message", msg);

    output = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    return output;
}
