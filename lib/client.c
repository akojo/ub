#include "array.h"
#include "util.h"
#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <arpa/inet.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <netdb.h>

struct client {
	fd_set read_fds;
	int maxfd;
	array_t *handlers;
    size_t bufsize;
    char *buf;
};

static int connect_to(char *hostname, int port);

client_t *client_new()
{
	client_t *new_cl = calloc(1, sizeof(client_t));
	new_cl->handlers = array_new();
    new_cl->bufsize = BUFSIZ;
    new_cl->buf = malloc(new_cl->bufsize);
	return new_cl;
}

int client_connect(client_t *cl, char *hostname, int port, handler_t handler)
{
	int fd = connect_to(hostname, port);

	array_ensure(cl->handlers, fd);
	array_set(cl->handlers, fd, (void *)handler);

	FD_ZERO(&cl->read_fds);
	FD_SET(fd, &cl->read_fds);
	if (fd > cl->maxfd)
		cl->maxfd = fd;

	return fd;
}

void client_delete(client_t *cl)
{
    free(cl->buf);
    array_delete(cl->handlers);
	free(cl);
}

int client_add_readfd(client_t *cl, int fd, handler_t handler)
{
	array_ensure(cl->handlers, fd);
	array_set(cl->handlers, fd, (void *)handler);

	FD_SET(fd, &cl->read_fds);
	if (fd > cl->maxfd)
		cl->maxfd = fd;

	return fd;
}

void client_run(client_t *cl)
{
	fd_set read_fds = cl->read_fds;

	while (select(cl->maxfd + 1, &read_fds, NULL, NULL, NULL) > 0) {
		int fd;

		for (fd = 0; fd < cl->maxfd + 1; fd++) {
            handler_t handler = (handler_t)array_get(cl->handlers, fd);
			if (FD_ISSET(fd, &read_fds) && handler) {
				int n;
				char *p;

                n = read(fd, cl->buf, cl->bufsize - 1);
				if (n == 0) return;
				else if (n < 0) error("failed to read descriptor");

                if ((p = strpbrk(cl->buf, "\n\r")))
					*p = '\0';
				else
                    cl->buf[n] = '\0';

                handler(cl->buf);
			}
		}
		read_fds = cl->read_fds;
	}
}

static int connect_to(char *hostname, int port)
{
	int status;
	int fd;
	char portstr[32];
	struct addrinfo hint;
	struct addrinfo *info;

	hint.ai_family = PF_INET;
	hint.ai_socktype = SOCK_STREAM;

	sprintf(portstr, "%d", port);

	status = getaddrinfo(hostname, portstr, &hint, &info); 
	if (status < 0) error("failed to get address");

	fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0) error("failed to create socket");

	status = connect(fd, info->ai_addr, info->ai_addrlen);
	if (status < 0) error("failed to connect to server");

	return fd;
}

