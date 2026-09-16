#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#include "server.h"
#include "client_handler.h"

static volatile sig_atomic_t g_shutdown_requested = 0;

static void handle_sigint(int signum) 
{
    (void)signum;

    const char bye[] = "\n¡Hasta pronto!\n";
    write(STDOUT_FILENO, bye, sizeof(bye) - 1);
    
    g_shutdown_requested = 1;
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

    struct sigaction sigint_action;
    memset(&sigint_action, 0, sizeof(sigint_action));
    sigint_action.sa_handler = handle_sigint; // puntero a función
    sigemptyset(&sigint_action.sa_mask);
    sigint_action.sa_flags = 0;
    sigaction(SIGINT, &sigint_action, NULL);

    for (;;)
    {
        int client_fd = server_accept_client(&server);
        if (client_fd < 0)
        {
            if (g_shutdown_requested) break;
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