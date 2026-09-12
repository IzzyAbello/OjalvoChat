#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#include "server.h"


typedef struct
{
    Server *server;
    int client_fd;
}
Client_Args;
 
static void* handle_client(void *arg)
{
    Client_Args* client_args = (Client_Args*)arg;
    int client_fd = client_args->client_fd;
    Server* server = client_args->server;

    printf(
        "[DEMO] Hilo %lu: empieza a atender al cliente (fd=%d)\n",
        (unsigned long)pthread_self(),
        client_fd
    );
    sleep(2); // probar que si funcionan los hilos.
 
    const char* message = "Message from the server to the client 'Hello Client'\n";
    server_send(server, client_fd, message, strlen(message));
 
    close(client_fd);
    free(client_args);
    return NULL;
}

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

    signal(SIGPIPE, SIG_IGN);

    for (;;)
    {
        int client_fd = server_accept_client(&server);
        if (client_fd < 0)
        {
            if (errno == EINTR) continue;
            printf("[SERVER - ERROR]: FALLA DEL SOCKET DEL SERVER.");
            break;
        }

        Client_Args* client_args = malloc(sizeof(Client_Args));
        if (client_args == NULL)
        {
            close(client_fd);
            continue;
        }
        client_args->server = &server;
        client_args->client_fd = client_fd;


        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, client_args) != 0)
        {
            close(client_fd);
            free(client_args);
            continue;
        }
        pthread_detach(thread_id);
    }
    
    server_close(&server);
    return EXIT_SUCCESS;
}