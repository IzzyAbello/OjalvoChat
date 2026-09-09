/** 
 * Me basé en el código de:
 * https://www.geeksforgeeks.org/computer-networks/simple-client-server-application-in-c/
 */

#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>

int main(int argc, char const* argv[])
{

    // create server socket similar to what was done in
    // client program
    int servSockD = socket(AF_INET, SOCK_STREAM, 0);
    if (servSockD < 0)
    {
        perror("[SERVER] Error al crear el socket");
        return 1;
    }
    printf("[SERVER] Socket creado correctamente.\n");


    // string store data to send to client
    char serMsg[255] = "Message from the server to the "
                       "client \'Hello Client\' ";

    // define server address
    struct sockaddr_in servAddr;

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(9001);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    // bind socket to the specified IP and port
    printf("[SERVER] Vinculando socket al puerto 9001...\n");
    if (bind(servSockD, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("[SERVER] Error en bind");
        return 1;
    }
    printf("[SERVER] Socket vinculado al puerto 9001.\n");

    // listen for connections
    printf("[SERVER] Esperando conexiones...\n");
    if (listen(servSockD, 1) < 0)
    {
        perror("[SERVER] Error en listen");
        return 1;
    }
    printf("[SERVER] Servidor escuchando en el puerto 9001.\n");

    // integer to hold client socket.
    printf("[SERVER] Esperando a que se conecte un cliente...\n");
    int clientSocket = accept(servSockD, NULL, NULL);
    if (clientSocket < 0) {
        perror("[SERVER] Error en accept");
        return 1;
    }
    printf("[SERVER] ¡Cliente conectado!\n");

    // send's messages to client socket
    send(clientSocket, serMsg, sizeof(serMsg), 0);

    return 0;
}