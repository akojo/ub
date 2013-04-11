#include "util.h"

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <arpa/inet.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

int listen_to(int port)
{
	int fd = socket(PF_INET, SOCK_STREAM, 0);
	int opt;
	int status;
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	status = bind(fd, (const struct sockaddr*)&addr,
				  sizeof(struct sockaddr_in));
	if (status < 0) error("failed to bind local address");

	status = listen(fd, 32);
	if (status < 0) error("listen to local socket failed");

	opt = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
	opt = 0;
	setsockopt(fd, SOL_SOCKET, SO_LINGER, &opt, sizeof(int));

	return fd;
}
