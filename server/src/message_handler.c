#include "message_handler.h"

typedef struct 
{
    Server* server;
    Message* message;
    int client_fd;
    int* client_fds_to_disconnect;
    int index;
}
Send_Context;

static void send_if_is_not_self (User* user, void* context)
{
    Send_Context* ctx = context;
    if(user->socket_fd != ctx->client_fd)
    {
        if (
            server_send(
                ctx->server,
                user->socket_fd,
                ctx->message
            ) < 0)
        {
            ctx->client_fds_to_disconnect[ctx->index] = user->socket_fd;
            ctx->index++;
        } 
    }
}

static void notify_all_but_self (
    Server* server, 
    Message message, 
    int client_fd
)
{
    int mx_users = g_hash_table_size(server->users.table);
    int fds_to_disconnect[mx_users+1]; // +1 de guardia.
    for (int i = 0; i <= mx_users; i++)
        fds_to_disconnect[i] = -1;

    Send_Context ctx = {
    .server = server,
    .message = &message,
    .client_fd = client_fd,
    .client_fds_to_disconnect = fds_to_disconnect,
    .index = 0
    };

    users_table_for_each(&server->users, send_if_is_not_self, &ctx);

    int i = 0;
    while (fds_to_disconnect[i] != -1)
    {
        User user;
        if (users_table_find_by_client_fd(&server->users, fds_to_disconnect[i], &user))
        {
            // ELIMINAR DE LA SALA <---
            /* 
                Tengo que implementar dentro de la clase room_table
                un metodo de eliminar de todas las salas. Que haga 
                un lock y a un forach mientras busca.
            */

            // ELIMINAR DE LA LISTA GENERAL
            users_table_remove(&server->users, user.username);
        }
        i++;
    }
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
    if (users_table_find_by_client_fd(&server->users, client_fd, &user))
    {
        if (strcmp(user.username, username) != 0)
            return process_disconnect(server, client_fd);
        //ignorar
        return true;
    }
    
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
            
            int sent = server_send(server, client_fd, &response);
            
            message_destroy(&response);

            if (sent < 0) return process_disconnect(server, client_fd);

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
}

static bool process_status(
    Server* server,
    const Message* msg_in,
    int client_fd
)
{
    if (msg_in->status == NULL)
    {
        // manejar error
        return false;
    }

    User_Status msg_status;
    if(user_status_from_string(strdup(msg_in->status), &msg_status))
    {
        User user;
        if (users_table_find_by_client_fd(&server->users, client_fd, &user))
        {
            if (user.status != msg_status)
            {
                users_table_change_status_by_client_fd(&server->users, client_fd, msg_status);
                
                Message notify_users;
                message_init(&notify_users);
                notify_users.type = MESSAGE_TYPE_NEW_STATUS;
                notify_users.username = strdup(user.username);
                notify_users.status = strdup(msg_in->status);

                notify_all_but_self(server, notify_users, client_fd);

                message_destroy(&notify_users);
            }
        }
        return true;
    }
    else return process_disconnect(server, client_fd);
}

static bool process_users(Server* server, int client_fd)
{
    Message user_list;
    message_init(&user_list);
    user_list.type = MESSAGE_TYPE_USER_LIST;

    cJSON* users = cJSON_CreateObject();

    Users_Table_Iter it;
    users_table_iter_begin(&server->users, &it);
    User user;
    while (users_table_iter_next(&it, &user))
        cJSON_AddStringToObject(
            users,
            user.username,
            user_status_to_string(user.status)
        );
    users_table_iter_end(&it);

    user_list.users = users;

    int sent = server_send(server, client_fd, &user_list);

    message_destroy(&user_list);

    if (sent < 0) return process_disconnect(server, client_fd);

    return true;
}

static bool process_text(
    Server* server,
    const Message* msg_in,
    int client_fd
)
{
    User user_to;
    if (users_table_find_by_username(&server->users, msg_in->username, &user_to))
    {
        Message text_from;
        message_init(&text_from);
        text_from.type = MESSAGE_TYPE_TEXT_FROM;

        User user_from;
        if (!users_table_find_by_client_fd(&server->users, client_fd, &user_from))
        {
            message_destroy(&text_from);
            return false;
        }

        text_from.username = strdup(user_from.username);
        text_from.text = strdup(msg_in->text);

        if (text_from.username == NULL || text_from.text == NULL)
        {
            message_destroy(&text_from);
            return false;
        }

        int sent = server_send(server, user_to.socket_fd, &text_from);

        message_destroy(&text_from);
    
        if (sent < 0) return process_disconnect(server, client_fd);
    }
    else
    {
        Message response;
        message_init(&response);

        response.type = MESSAGE_TYPE_RESPONSE;
        response.operation = strdup("TEXT");
        response.result = strdup("NO_SUCH_USER");
        response.extra = strdup(msg_in->username);

        if (response.operation == NULL || response.result == NULL || response.extra == NULL)
        {
            message_destroy(&response);
            return false;
        }

        int sent = server_send(server, client_fd, &response);

        message_destroy(&response);
    
        if (sent < 0) return process_disconnect(server, client_fd);
    }
    return true;
}

static bool process_public_text(
    Server* server,
    const Message* msg_in,
    int client_fd
)
{
    Message public_text_from;
    message_init(&public_text_from);
    public_text_from.type = MESSAGE_TYPE_PUBLIC_TEXT_FROM;

    User user_from;
    if (!users_table_find_by_client_fd(&server->users, client_fd, &user_from))
    {
        message_destroy(&public_text_from);
        return false;
    }

    public_text_from.username = strdup(user_from.username);
    public_text_from.text = strdup(msg_in->text);

    if (public_text_from.username == NULL || public_text_from.text == NULL)
    {
        message_destroy(&public_text_from);
        return false;
    }

    notify_all_but_self(server, public_text_from, client_fd);

    message_destroy(&public_text_from);
    
    return true;
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

    if (!users_table_contains_by_client_fd(&server->users, client_fd))
        return process_disconnect(server, client_fd);

    switch (msg_in->type)
    {
        case MESSAGE_TYPE_STATUS:
            return process_status(server, msg_in, client_fd);
        case MESSAGE_TYPE_USERS:
            return process_users(server, client_fd);
        case MESSAGE_TYPE_TEXT:
            return process_text(server, msg_in, client_fd);
        case MESSAGE_TYPE_PUBLIC_TEXT:
            return process_public_text(server, msg_in, client_fd);
        default:
            break;
    }

    return true;
}