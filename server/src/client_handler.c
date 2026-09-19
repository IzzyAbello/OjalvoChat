#include "client_handler.h"
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "message.h"
#include "message_handler.h"
 
#define RECV_CHUNK_SIZE 1024

void* handle_client(void* arg)
{
    Client_Args* client_args = (Client_Args*)arg;
    int client_fd = client_args->client_fd;
    Server* server = client_args->server;
    free(client_args);
 
    Message_Buffer* buffer = malloc(sizeof(Message_Buffer));
    char *message = malloc(SERVER_BUFFER_SIZE);
 
    if (buffer == NULL || message == NULL)
    {
        free(buffer);
        free(message);
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

            Message msg_in;
            message_init(&msg_in);
            if (!message_from_json(message, &msg_in))
            {
                User* user;
                if (users_table_find_by_client_fd(&server->users, client_fd, user))
                {
                    Message disconnect_msg;
                    message_init(&disconnect_msg);
                    disconnect_msg.type = MESSAGE_TYPE_DISCONNECT;

                    message_handler_process(server, &disconnect_msg, client_fd);
                    message_destroy(&disconnect_msg);
                }

                server_disconnect_client(server, client_fd);
                free(&user);
                return NULL;
            }

            message_handler_process(server, &msg_in, client_fd);

            message_destroy(&msg_in);
        }
    }
 
    free(buffer);
    free(message);
    return NULL;
}