#include "client_handler.h"
 
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
 
            int written = snprintf(response, SERVER_BUFFER_SIZE, "echo: %s\n", message);
            if (written > 0)
            {
                size_t response_length = (size_t)written;
                if (response_length >= SERVER_BUFFER_SIZE)
                    response_length = SERVER_BUFFER_SIZE - 1; // truncamiento.
                server_send(server, client_fd, response, response_length);
            }
        }
    }
 
    free(buffer);
    free(message);
    free(response);
    close(client_fd);
    return NULL;
}