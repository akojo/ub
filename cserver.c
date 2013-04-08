#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "common.h"
#include "net.h"
#include "util.h"
#include "cJSON.h"

#include <unistd.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>

array_t *nicks;
array_t *clients;

char *process_message(int fd, char *msg);
char *handle_message(int fd, cJSON* json);
char *handle_nick(int fd, cJSON *json);
void write_message(void *val, int idx, void *message);

int main(void)
{
	char buf[BUFSIZ];
	fd_set rfds, wait_fds;

	int listen_fd = listen_to(CHAT_PORT);
	int max_fd = listen_fd;

	FD_ZERO(&rfds);
	FD_SET(listen_fd, &rfds);

	nicks = array_new(free);
	clients = array_new(NULL);

	wait_fds = rfds;
	while (select(max_fd + 1, &wait_fds, NULL, NULL, NULL) > 0) {
		for (int fd = 0; fd < max_fd + 1; fd++) {
			if (!FD_ISSET(fd, &wait_fds))
				continue;

			if (fd == listen_fd) {
				struct sockaddr_in addr;
				socklen_t len = sizeof(struct sockaddr_in);
				int sock = accept(fd, (struct sockaddr *)&addr, &len);

				FD_SET(sock, &rfds);

				array_ensure(clients, sock);
				array_set(clients, sock, (void *)1);

				if (sock >= max_fd)
					max_fd = sock;
			} else {
				int nread = read(fd, buf, BUFSIZ);
				if (nread <= 0) {
					FD_CLR(fd, &rfds);
					array_set(clients, fd, (void *)0);

					free(array_get(nicks, fd));
					array_set(nicks, fd, NULL);

					close(fd);
				} else {
					char *response = process_message(fd, buf);
					if (response) {
						array_foreach_idx(clients, write_message, response);
						free(response);
					}
				}
			}
		}
		wait_fds = rfds;
	}

	error("select failed");
	return 1;
}

char *process_message(int fd, char *msg)
{
	char *output = NULL;
	cJSON *json = cJSON_Parse(msg);
	cJSON *cmd;

	if (!json) {
		warning("failed to parse json '%s'\n", msg);
		return NULL;
	}

	cmd = cJSON_GetObjectItem(json, "cmd");
	if (!cmd || cmd->type != cJSON_String) {
		warning("missing 'cmd' from message '%s'\n", msg);
		goto finish;
	}

	if (!strcmp(cmd->valuestring, MSG_STR)) {
		output = handle_message(fd, json);
	} else if (!strcmp(cmd->valuestring, NICK_STR)) {
		output = handle_nick(fd, json);
	}

finish:
	cJSON_Delete(json);
	return output;
}

char *handle_message(int fd, cJSON *json)
{
	char *output = NULL;
	char *nick = array_get(nicks, fd);

	cJSON *response = cJSON_CreateObject();

	if (!nick) {
		cJSON_AddStringToObject(response, "response", ERROR_STR);
		cJSON_AddStringToObject(response, "message", "not registered");
	} else {
		cJSON *message = cJSON_GetObjectItem(json, "message");
		if (!message) {
			warning("missing 'message'\n");
			goto finish;
		}
		cJSON_AddStringToObject(response, "response", MSG_STR);
		cJSON_AddStringToObject(response, "nick", nick);
		cJSON_AddStringToObject(response, "message", message->valuestring);
	}

	output = cJSON_PrintUnformatted(response);
	array_foreach_idx(clients, write_message, output);
	free(output);
finish:
	cJSON_Delete(response);
	return NULL;
}

char *handle_nick(int fd, cJSON *json)
{
	char *old;
	char *output;
	cJSON *nick = cJSON_GetObjectItem(json, "nick");
	cJSON *response;

	if (!nick || nick->type != cJSON_String) {
		warning("missing 'nick' from");
		return NULL;
	}

	array_ensure(nicks, fd);
	old = array_get(nicks, fd);

	array_set(nicks, fd, strdup(nick->valuestring));

	if (!old)
		return NULL;

	response = cJSON_CreateObject();
	cJSON_AddStringToObject(response, "response", NICK_STR);
	cJSON_AddStringToObject(response, "nick", nick->valuestring);
	cJSON_AddStringToObject(response, "prev_nick", old);

	output = cJSON_PrintUnformatted(response);

	free(old);
	cJSON_Delete(response);
	return output;

}

void write_message(void *val, int fd, void *context)
{
	char *message = context;
	if (val)
		write(fd, message, strlen(message));
}
