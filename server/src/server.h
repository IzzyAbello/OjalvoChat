#ifndef OJALVO_SERVER_H
#define OJALVO_SERVER_H

#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#define SERVER_DEFAULT_PORT 1234
#define SERVER_BUFFER_SIZE 1024

typedef struct 
{
    bool show_help;
    bool log_enabled;
    const char *port_arg;
} 
Server_Options;

Server_Options server_parse_args(int argc, char const *argv[]);
int server_resolve_port(const Server_Options *options);

typedef struct
{
    int socket_fd;
    int port;
    bool log_enabled;
    struct sockaddr_in address;
}
Server;

int server_init(Server *server, int port, bool log_enabled);
int server_bind_and_listen(Server *server, int backlog);
int server_accept_client(const Server *server);
ssize_t server_send(
    const Server *server,
    int client_fd,
    const char *message,
    size_t length
);
void server_close(Server *server);

#endif