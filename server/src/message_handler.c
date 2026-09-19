#include "message_handler.h"

static Message disconnect_msg;

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

static void notify_all_but_self (Server* server, Message message, int client_fd)
{
    Send_Context ctx = {
    .server = server,
    .message = &message,
    .client_fd = client_fd
    };
    users_table_for_each(&server->users, send_if_is_not_self, &ctx);
}

static bool process_disconnect(Server* server, int client_fd)
{
    User user;
    if(users_table_find_by_client_fd(&server->users, client_fd, &user))
    {
        Message notify_users;
        message_init(&notify_users);
        notify_users.type = MESSAGE_TYPE_DISCONNECTED;
        notify_users.username = strdup(user.username);

        notify_all_but_self(server, notify_users, client_fd);

        users_table_remove(&server->users, user.username);

        message_destroy(&notify_users);
    }
    //NO HACER FREE USER
    server_disconnect_client(server, client_fd);
    return true;
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
            
            //desconectar ???
            process_disconnect(server, client_fd);
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

            notify_all_but_self(server, notify_users, client_fd);

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

    if (msg_in->type == MESSAGE_TYPE_DISCONNECT)
        return process_disconnect(server, client_fd);

    if (msg_in->type == MESSAGE_TYPE_IDENTIFY)
        return process_identify(server, msg_in, client_fd);

    message_init(&disconnect_msg);
    disconnect_msg.type = MESSAGE_TYPE_DISCONNECT;
    // A partir de estos, si no se identifican toca desconectarlos...

    return true;
}