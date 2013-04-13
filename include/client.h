#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>

struct client;
typedef struct client client_t;

typedef void (*handler_t)(char *);

client_t *client_new(void);
int client_connect(client_t *client, char *hostname, int port, handler_t handler);
void client_delete(client_t *client);
int client_add_readfd(client_t *client, int fd, handler_t handler);
void client_run(client_t *client);

#endif /* end of include guard: CLIENT_H */
