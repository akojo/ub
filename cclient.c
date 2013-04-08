#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "net.h"
#include "util.h"
#include "cJSON.h"

#include <unistd.h>

#include <sys/select.h>

static char buf[BUFSIZ];

char *process_input(char *str);
char *process_response(char *str);

char *handle_cmd();
char *handle_message();

int main(void)
{
    fd_set rfds, wait_fds;
    int term_fd = 0;
    int srv_fd = connect_to("localhost", CHAT_PORT);

    FD_ZERO(&rfds);
    FD_SET(term_fd, &rfds);
    FD_SET(srv_fd, &rfds);

    wait_fds = rfds;
    while (select(srv_fd + 1, &wait_fds, NULL, NULL, NULL) > 0) {
        int fd;
		for (fd = 0; fd < srv_fd + 1; fd++) {
			char *output = NULL;
			char *p;
			int out_fd, n;

			if (!FD_ISSET(fd, &wait_fds))
				continue;

			n = read(fd, buf, BUFSIZ);
			if (n == 0) quit("bye");
			else if (n < 0) error("failed to read descriptor");

			if ((p = strpbrk(buf, "\n\r")))
				*p = '\0';
			else
				buf[n] = '\0';

			if (fd == term_fd) {
				output = process_input(buf);
				out_fd = srv_fd;
			}
			else if (fd == srv_fd) {
				output = process_response(buf);
				out_fd = term_fd;
			}

			if (output) {
				write(out_fd, output, strlen(output));
				free(output);
			}
		}

        wait_fds = rfds;
    }
    error("error communicating with server");
}

char *process_input(char *str)
{
    if (buf[0] == '/')
        return handle_cmd(buf + 1);
    else
        return handle_message(buf);
}

char *process_response(char *str)
{
	char *result = malloc(1024);
	cJSON *json = cJSON_Parse(str);
	cJSON *response = cJSON_GetObjectItem(json, "response");

	if (!strcmp(response->valuestring, NICK_STR)) {
		cJSON *nick = cJSON_GetObjectItem(json, "nick");
		cJSON *prev = cJSON_GetObjectItem(json, "prev_nick");
		sprintf(result, "* '%s' is now known as '%s'\n", prev->valuestring, nick->valuestring);
	} else if (!strcmp(response->valuestring, MSG_STR)) {
		cJSON *nick = cJSON_GetObjectItem(json, "nick");
		cJSON *msg = cJSON_GetObjectItem(json, "message");
		sprintf(result, "<%s> %s\n", nick->valuestring, msg->valuestring);
	} else {
		result[0] = '\0';
	}

    return result;
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
    cJSON_AddStringToObject(json, "message", buf);

    output = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    return output;
}
