#ifndef CLIENT_H
#define CLIENT_H

struct client;
typedef struct client client_t;

client_t *client_new(void);
int client_connect(client_t *client, char *hostname, int port, void (*handler)(char *));
void client_delete(client_t *client);
int client_add_readfd(client_t *client, int fd, void (*handler)(char *));
void client_run(client_t *client);

#endif /* end of include guard: CLIENT_H */
