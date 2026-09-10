/** 
 * Me basé en el código de:
 * https://www.geeksforgeeks.org/computer-networks/simple-client-server-application-in-c/
 */

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#define DEFAULT_PORT 1234
#define BUFFER_SIZE 1024

int help_fl = 0;
int  log_fl = 0;
char const* port_fl = NULL;

void notify(const char *format, ...) 
{
    va_list args;
    va_start(args, format);

    if (log_fl == 1)
    {
        printf("[SERVER]: ");
        vprintf(format, args);
        printf("\n");
    }

    va_end(args);
}

void notify_error(char* message) 
{
    perror(strcat("[SERVER - ERROR] ", message));
}

void show_help ()
{
    printf("Mostrar mensaje de ayuda...");
}

void get_input(int argc, char const* argv[])
{
    for (int i = 0; i < argc; i++)
    {
        char const* param = argv[i]; 
        if (strcmp(param, "-h") == 0 || strcmp(param, "--help") == 0)
        {
            show_help();
            exit(0);
        }

        if (strcmp(param, "-l") == 0 || strcmp(param, "--log") == 0)
            log_fl = 1;

        if ((strcmp(param, "-p") == 0 || strcmp(param, "--port") == 0 || strcmp(param, "--puerto") == 0)
            && argc != i)
            port_fl = argv[++i];
    }
}

int set_port() 
{
    if (port_fl == NULL) 
        return DEFAULT_PORT;

    char *endptr;
    errno = 0;  

    long input = strtol(port_fl, &endptr, 10);

    if (endptr == port_fl) 
    {
        notify_error("No se encontró ningún número de puerto válido.");
        notify("Puerto inválido, levantando en puerto %d.", DEFAULT_PORT);
        return DEFAULT_PORT;
    } 
    if (*endptr != '\0')
    {
        notify("Advertencia: Se encontraron caracteres no numéricos extra: %s\n", endptr);
    }
    if (errno != 0) 
    {
        perror("Error de rango/desbordamiento.");
        notify("Puerto inválido, levantando en puerto %d.", DEFAULT_PORT);
        return DEFAULT_PORT;
    }

    return (int)input;
}

int main(int argc, char const* argv[])
{
    get_input(argc, argv);

    int port = set_port();

    notify("Levantando servidor en el puerto %d...", port);

    int servSockD = socket(AF_INET, SOCK_STREAM, 0);
    if (servSockD < 0)
    {
        notify_error("Error al crear el socket");
        return 1;
    }
    notify("Socket creado correctamente.");


    char serMsg[BUFFER_SIZE] = "Message from the server to the client \'Hello Client\' ";

    struct sockaddr_in servAddr;

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    notify("Vinculando socket al puerto %d...", port);
    if (bind(servSockD, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
    {
        notify_error("Error en bind");
        return 1;
    }
    notify("Socket vinculado al puerto %d.", port);

    notify("Esperando conexiones...");
    if (listen(servSockD, SOMAXCONN) < 0)
    {
        notify_error("Error en listen");
        return 1;
    }
    notify("Servidor escuchando en el puerto %d.", port);

    notify("Esperando a que se conecte un cliente...");
    int clientSocket = accept(servSockD, NULL, NULL);
    if (clientSocket < 0) {
        notify_error("Error en accept");
        return 1;
    }
    notify("¡Cliente conectado!");

    send(clientSocket, serMsg, sizeof(serMsg), 0);

    return 0;
}