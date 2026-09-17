#include "client_handler.h"
#include "message.h"
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
 
#define RECV_CHUNK_SIZE 1024

void* handle_client(void* arg)
{
    Client_Args* client_args = (Client_Args*)arg;
    int client_fd = client_args->client_fd;
    Server* server = client_args->server;
    free(client_args);
 
    const char* greeting = "Message from the server to the client 'Hello Client'\n";
    server_send(server, client_fd, greeting, strlen(greeting));
 
    Message_Buffer* buffer = malloc(sizeof(Message_Buffer));
    char *message = malloc(SERVER_BUFFER_SIZE);
    char *response = malloc(SERVER_BUFFER_SIZE);
 
    if (buffer == NULL || message == NULL || response == NULL)
    {
        free(buffer);
        free(message);
        free(response);
        close(client_fd);
        return NULL;
    }

    message_buffer_init(buffer);
 
    char chunk[RECV_CHUNK_SIZE];
 
    for (;;)
    {
        ssize_t received = recv(client_fd, chunk, sizeof(chunk), 0);
 
        if (received <= 0 || message_buffer_append(buffer, chunk, (size_t)received) != 0)
            break;
 
        size_t message_length;
        while (message_buffer_extract(buffer, message, SERVER_BUFFER_SIZE, &message_length))
        {
            message_buffer_sanitize(message, &message_length);
 
            if (message_length == 0) continue;

            // Parsear JSON ------ ARREGLAR 
            Message msg_in;
            message_init(&msg_in);

            if (!message_from_json(message, &msg_in))
            {
                // Manejar error
            }

            // Analizar msg_in

            Message msg_out;
            message_init(&msg_out);
            
            // SOLO POR AHORA (ECHO)
            msg_out = msg_in;

            if (message_to_json(&msg_out, response, SERVER_BUFFER_SIZE))
            {
                size_t response_length = strlen(response);
                if (response_length + 1 < SERVER_BUFFER_SIZE)
                {
                    response[response_length] = '\n';
                    response[response_length + 1] = '\0';
                    response_length++;
                }
                else
                {
                    response[SERVER_BUFFER_SIZE - 2] = '\n';
                    response[SERVER_BUFFER_SIZE - 1] = '\0';
                    response_length = SERVER_BUFFER_SIZE - 1;
                }
                server_send(server, client_fd, response, response_length);
            }
            else
            {
                // Manejar error
            }

            message_destroy(&msg_in);
            // QUITAR COMENTARIOS DESPUES 
            //message_destroy(&msg_out);
        }
    }
 
    free(buffer);
    free(message);
    free(response);
    close(client_fd);
    return NULL;
}