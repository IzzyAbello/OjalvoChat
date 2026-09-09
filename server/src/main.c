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

#define DEFAULT_PORT 1234
#define BUFFER_SIZE 512

void notify(const char *format, ...) 
{
    va_list args;
    va_start(args, format);

    printf("[SERVER]: ");
    vprintf(format, args);
    printf("\n");

    va_end(args);
}

void notify_error(char* message) 
{
    perror(strcat("[SERVER - ERROR] ", message));
}

int set_port(int argc, char const* argv[]) 
{
    // Implement here a safe method...
    return DEFAULT_PORT;
}

int main(int argc, char const* argv[])
{
    int port = set_port(argc, argv);

    int servSockD = socket(AF_INET, SOCK_STREAM, 0);
    if (servSockD < 0)
    {
        notify_error("Error al crear el socket");
        return 1;
    }
    notify("Socket creado correctamente.\n");


    char serMsg[BUFFER_SIZE] = "Message from the server to the client \'Hello Client\' ";

    struct sockaddr_in servAddr;

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    notify("Vinculando socket al puerto %d...\n", port);
    if (bind(servSockD, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
    {
        notify_error("Error en bind");
        return 1;
    }
    notify("Socket vinculado al puerto %d.\n", port);

    notify("Esperando conexiones...\n");
    if (listen(servSockD, SOMAXCONN) < 0)
    {
        notify_error("Error en listen");
        return 1;
    }
    notify("Servidor escuchando en el puerto %d.\n", port);

    notify("Esperando a que se conecte un cliente...\n");
    int clientSocket = accept(servSockD, NULL, NULL);
    if (clientSocket < 0) {
        notify_error("Error en accept");
        return 1;
    }
    notify("¡Cliente conectado!\n");

    send(clientSocket, serMsg, sizeof(serMsg), 0);

    return 0;
}