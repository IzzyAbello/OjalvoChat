#include "message_handler.h"

typedef struct 
{
    Server* server;
    Message* message;
    int client_fd;
}
Send_Context;

static void send_if_is_not_self (User* user, void* context)
{
    Send_Context* ctx = context;
    if(user->socket_fd != ctx->client_fd)
        server_send(
            ctx->server,
            user->socket_fd,
            ctx->message
        );
}

static bool process_identify(
    Server* server,
    const Message* msg_in,
    int client_fd
)
{
    if (msg_in->username == NULL)
    {
        // manejar error
        return false;
    }

    char* username = strdup(msg_in->username);

    User user;
    if(users_table_find_by_username(&server->users, username, &user))
    {
        if (user.socket_fd != client_fd)
        {
            Message response;
            message_init(&response);
            response.type = MESSAGE_TYPE_RESPONSE;
            response.operation = strdup("IDENTIFY");
            response.result = strdup("USER_ALREADY_EXISTS");
            response.extra = strdup(username);
            
            server_send(server, client_fd, &response);
            
            message_destroy(&response);
        }
        // ignorar
        return true;
    }
    else
    {
        if (users_table_add(&server->users, username, client_fd))
        {
            Message response;
            message_init(&response);
            response.type = MESSAGE_TYPE_RESPONSE;
            response.result = strdup("SUCCESS");
            response.extra = strdup(username);
            
            server_send(server, client_fd, &response);
            
            message_destroy(&response);

            Message notify_users;
            message_init(&notify_users);
            notify_users.type = MESSAGE_TYPE_NEW_USER;
            notify_users.username = strdup(username);

            Send_Context ctx = {
                .server = server,
                .message = &notify_users,
                .client_fd = client_fd
            };
            users_table_for_each(&server->users, send_if_is_not_self, &ctx);

            message_destroy(&notify_users);

            return true;
        }
        // error al agregar.
        return false;
    }
    free(&user);
}


bool message_handler_process(
    Server* server,
    const Message* msg_in,
    int client_fd
)
{
    if (msg_in == NULL || msg_in->type == MESSAGE_TYPE_UNKNOWN)
    {
        // manejar error...
        return false;
    }

    if (msg_in->type == MESSAGE_TYPE_IDENTIFY)
        return process_identify(server, msg_in, client_fd);

    // IMPLEMENTAR LOS DEMAS
    return true;
}