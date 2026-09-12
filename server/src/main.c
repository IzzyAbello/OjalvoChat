#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "server.h"

static void show_server_help(void)
{
    printf("USO: ojalvochat [opciones]\n\n");
    printf(
        "  -p, --port, --puerto <n>   Puerto a escuchar (default %d)\n",
        SERVER_DEFAULT_PORT
    );
    printf("  -l, --log                  Activa mensajes de log\n");
    printf("  -h, --help                 Muestra esta ayuda\n");
}

int main(int argc, char const *argv[])
{
    Server_Options options = server_parse_args(argc, argv);

    if (options.show_help)
    {
        show_server_help();
        return EXIT_SUCCESS;
    }

    int port = server_resolve_port(&options);

    Server server;
    if (server_init(&server, port, options.log_enabled) != 0)
        return EXIT_FAILURE;

    if (server_bind_and_listen(&server, SOMAXCONN) != 0) 
    {
        server_close(&server);
        return EXIT_FAILURE;
    }

    int client_fd = server_accept_client(&server);
    if (client_fd < 0) 
    {
        server_close(&server);
        return EXIT_FAILURE;
    }

    char* message = "Message from the server to the client 'Hello Client'\n";
    server_send(&server, client_fd, message, strlen(message));

    close(client_fd);
    server_close(&server);
    return EXIT_SUCCESS;
}