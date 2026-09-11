#include "server.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static void log_info(bool enabled, const char *format, ...)
{
    va_list args;
    if (!enabled) return;

    va_start(args, format);
    printf("[SERVER]: ");
    vprintf(format, args);
    printf("\n");
    va_end(args);
}

static void log_error(const char *message)
{
    char full_message[256];
    snprintf(
        full_message,
        sizeof(full_message),
        "[SERVER - ERROR] %s",
        message
    );
    perror(full_message);
}

Server_Options server_parse_args(int argc, char const *argv[])
{
    Server_Options options = {
        .show_help = false,
        .log_enabled = false,
        .port_arg = NULL,
    };

    for (int i = 0; i < argc; i++) 
    {
        char const *arg = argv[i];

        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
        {
            options.show_help = true;
            continue;
        }
        if (strcmp(arg, "-l") == 0 || strcmp(arg, "--log") == 0)
        {
            options.log_enabled = true;
            continue;
        }
        if ((strcmp(arg, "-p") == 0 || strcmp(arg, "--port") == 0 ||
            strcmp(arg, "--puerto") == 0) && i + 1 < argc)
        {
            options.port_arg = argv[++i];
            continue;
        }
    }

    return options;
}

int server_resolve_port(const Server_Options *options)
{
    if (options->port_arg == NULL)
        return SERVER_DEFAULT_PORT;

    char *endptr;
    errno = 0;
    long parsed = strtol(options->port_arg, &endptr, 10);

    if (endptr == options->port_arg)
    {
        log_info(options->log_enabled, "Puerto inválido, levantando en puerto %d.", SERVER_DEFAULT_PORT);
        return SERVER_DEFAULT_PORT;
    }
    if (*endptr != '\0')
    {
        log_info(options->log_enabled, "Advertencia: caracteres extra: %s", endptr);
    }
    if (errno != 0)
    {
        log_info(options->log_enabled, "Puerto inválido, levantando en puerto %d.", SERVER_DEFAULT_PORT);
        return SERVER_DEFAULT_PORT;
    }
    if (parsed < 1 || parsed > 65535)
    {
        log_info(options->log_enabled, "Puerto fuera de rango, levantando en puerto %d.", SERVER_DEFAULT_PORT);
        return SERVER_DEFAULT_PORT;
    }

    return (int)parsed;
}

int server_init(Server *server, int port, bool log_enabled)
{
    server->port = port;
    server->log_enabled = log_enabled;

    log_info(server->log_enabled, "Levantando servidor en el puerto %d...", port);

    server->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->socket_fd < 0) 
    {
        log_error("Error al crear el socket");
        return -1;
    }
    log_info(server->log_enabled, "Socket creado correctamente.");

    memset(&server->address, 0, sizeof(server->address));
    server->address.sin_family = AF_INET;
    server->address.sin_port = htons((uint16_t)port);
    server->address.sin_addr.s_addr = INADDR_ANY;

    return 0;
}

int server_bind_and_listen(Server *server, int backlog) 
{
    log_info(server->log_enabled, "Vinculando socket al puerto %d...", server->port);
    if (bind(server->socket_fd,
            (struct sockaddr *)&server->address,
            sizeof(server->address)) < 0) 
    {
        log_error("Error en bind");
        return -1;
    }
    log_info(server->log_enabled, "Socket vinculado al puerto %d.", server->port);

    log_info(server->log_enabled, "Esperando conexiones...");
    if (listen(server->socket_fd, backlog) < 0) 
    {
        log_error("Error en listen...");
        return -1;
    }
    log_info(server->log_enabled, "Servidor escuchando en el puerto %d.", server->port);

    return 0;
}

int server_accept_client(const Server *server) 
{
    log_info(server->log_enabled, "Esperando a que se conecte un cliente...");
    int client_fd = accept(server->socket_fd, NULL, NULL);
    if (client_fd < 0) 
    {
        log_error("Error en accept...");
        return -1;
    }
    log_info(server->log_enabled, "¡Cliente conectado!");
    return client_fd;
}

ssize_t server_send(
        const Server *server,
        int client_fd,
        const char *message,
        size_t length
    ) 
{
    ssize_t sent = send(client_fd, message, length, 0);
    if (sent < 0) log_error("Error al enviar el mensaje");
    else log_info(server->log_enabled, "Enviados %zd bytes al cliente.", sent);
    
    return sent;
}

void server_close(Server *server) 
{
    if (server->socket_fd >= 0)
    {
        close(server->socket_fd);
        server->socket_fd = -1;
    }
}