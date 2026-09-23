#include "message_handler.h"

#include <stdio.h>

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
    Users_Table* users, 
    Message* message, 
    int client_fd
)
{
    int mx_users = g_hash_table_size(users->table);
    int fds_to_disconnect[mx_users+1]; // +1 de guardia.
    for (int i = 0; i <= mx_users; i++)
        fds_to_disconnect[i] = -1;

    Send_Context ctx = {
    .server = server,
    .message = message,
    .client_fd = client_fd,
    .client_fds_to_disconnect = fds_to_disconnect,
    .index = 0
    };

    users_table_for_each(users, send_if_is_not_self, &ctx);

    int i = 0;
    while (fds_to_disconnect[i] != -1)
    {
        User user;
        if (users_table_find_by_client_fd(users, fds_to_disconnect[i], &user))
        {
            // ELIMINAR DE LA SALA <---
            /* 
                Tengo que implementar dentro de la clase room_table
                un metodo de eliminar de todas las salas. Que haga 
                un lock y a un forach mientras busca.
            */

            // ELIMINAR DE LA LISTA GENERAL
            users_table_remove(users, user.username);


            // ESTAS DOS COSAS PUEDEN SALIR EN UN STATIC REMOVE_FROM_DATA
            // UPDDATE DE STATE
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

        notify_all_but_self(server, &server->users, &notify_users, client_fd);

        // REMOVE DE LAS SALAS

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

            notify_all_but_self(server, &server->users, &notify_users, client_fd);

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

                notify_all_but_self(server, &server->users, &notify_users, client_fd);

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

    notify_all_but_self(server, &server->users, &public_text_from, client_fd);

    message_destroy(&public_text_from);
    
    return true;
}

typedef struct
{
    User* user;
    bool is_in_room;
    cJSON* users_obj;

}
Room_Visitor_Context;

static void room_add_creator (Room* room, void* context)
{
    Room_Visitor_Context* ctx = context;
    users_table_add
    (
        &room->members,
        ctx->user->username,
        ctx->user->socket_fd
    );
}

static void room_add_to_guests (Room* room, void* context)
{
    Room_Visitor_Context* ctx = context;
    users_table_add
    (
        &room->guests,
        ctx->user->username,
        ctx->user->socket_fd
    );
}

static bool process_new_room(
    Server* server,
    const Message* msg_in,
    int client_fd
)
{
    if (msg_in->roomname == NULL) return false;

    Message response;
    message_init(&response);
    response.type = MESSAGE_TYPE_RESPONSE;
    response.operation = strdup("NEW_ROOM");
    response.extra = strdup(msg_in->roomname);

    if (response.operation == NULL || response.extra == NULL)
    {
        message_destroy(&response);
        return false;
    }

    if (room_table_contains(&server->rooms, msg_in->roomname))
    {    
        response.result = strdup("ROOM_ALREADY_EXISTS");
        
        if (response.result == NULL)
        {
            message_destroy(&response);
            return false;
        }
    }
    else
    {
        response.result = strdup("SUCCESS");

        if (response.result == NULL)
        {
            message_destroy(&response);
            return false;
        }

        if(room_table_add(&server->rooms, msg_in->roomname))
        {
            User room_creator;
            if(users_table_find_by_client_fd(&server->users, client_fd, &room_creator))
            {
                Room_Visitor_Context ctx = { .user = &room_creator };
                room_table_with
                (
                    &server->rooms,
                    msg_in->roomname,
                    room_add_creator,
                    &ctx
                );
            }
            else
            {
                message_destroy(&response);
                return false;
            }
        }
        else
        {
            message_destroy(&response);
            return false;
        }
    }

    int sent = server_send(server, client_fd, &response);

    message_destroy(&response);

    if (sent < 0) return process_disconnect(server, client_fd);
    return true;
}

static void room_is_a_member (Room* room, void* context)
{
    Room_Visitor_Context* ctx = context;
    if (users_table_contains_by_client_fd(&room->members, ctx->user->socket_fd))
        ctx->is_in_room = true;
}

static void room_is_a_guest (Room* room, void* context)
{
    Room_Visitor_Context* ctx = context;
    if(users_table_contains_by_client_fd(&room->guests, ctx->user->socket_fd))
        ctx->is_in_room = true;
}

static bool process_invite(
    Server* server,
    const Message* msg_in,
    int client_fd
)
{
    if (msg_in->roomname == NULL || msg_in->usernames == NULL) 
        return false;

    Message response;
    message_init(&response);
    response.type = MESSAGE_TYPE_RESPONSE;
    response.operation = strdup("INVITE");

    if (response.operation == NULL)
    {
        message_destroy(&response);
        return false;
    }

    if (!room_table_contains(&server->rooms, msg_in->roomname))
    {   
        response.result = strdup("NO_SUCH_ROOM");
        response.extra = strdup(msg_in->roomname);

        if (response.result == NULL || response.extra == NULL)
        {
            message_destroy(&response);
            return false;
        }

        int sent = server_send(server, client_fd, &response);

        message_destroy(&response);

        if (sent < 0) return process_disconnect(server, client_fd);

        return true;
    }

    User user_from;
    if (users_table_find_by_client_fd(&server->users, client_fd, &user_from))
    {
        Room_Visitor_Context ctx = { .user = &user_from, .is_in_room = false };
        room_table_with(&server->rooms, msg_in->roomname, room_is_a_member, &ctx);
        if (ctx.is_in_room)
        {
            User invited_users[msg_in->usernames_count];
            for (int i = 0; i < msg_in->usernames_count; i++)
            {
                if 
                (
                    !users_table_find_by_username
                    (
                        &server->users,
                        msg_in->usernames[i],
                        &invited_users[i]
                    )
                )
                {
                    response.result = strdup("NO_SUCH_USER");
                    response.extra = strdup(msg_in->usernames[i]);

                    if (response.result == NULL || response.extra == NULL)
                    {
                        message_destroy(&response);
                        return false;
                    }

                    int sent = server_send(server, client_fd, &response);

                    message_destroy(&response);

                    if (sent < 0) return process_disconnect(server, client_fd);

                    return true;
                }
            }

            Message invitation;
            message_init(&invitation);
            invitation.type = MESSAGE_TYPE_INVITATION;
            invitation.username = strdup(user_from.username);
            invitation.roomname = strdup(msg_in->roomname);

            if (invitation.username == NULL || invitation.roomname == NULL)
            {
                message_destroy(&invitation);
                return false;
            }

            for (int i = 0; i < msg_in->usernames_count; i++)
            {
                Room_Visitor_Context ctx = { .user = &invited_users[i], .is_in_room = false }; 
                room_table_with(&server->rooms, msg_in->roomname, room_is_a_member, &ctx);
                room_table_with(&server->rooms, msg_in->roomname, room_is_a_guest, &ctx);

                if (ctx.is_in_room) return true;

                room_table_with(&server->rooms, msg_in->roomname, room_add_to_guests, &ctx);

                int sent = server_send(server, invited_users[i].socket_fd, &invitation);
                
                if (sent < 0) return process_disconnect(server, invited_users[i].socket_fd);
            }            

            message_destroy(&invitation);
        }
        else return process_disconnect(server, client_fd);
    }
    else return process_disconnect(server, client_fd);

    message_destroy(&response);
    return true;
}

static void room_from_guest_to_member(Room* room, void* context)
{
    Room_Visitor_Context* ctx = context;
    users_table_remove(&room->guests, ctx->user->username);
    users_table_add(&room->members, ctx->user->username, ctx->user->socket_fd);
}

static void room_notify_members(Room* room, void* context)
{
    Send_Context* ctx = context;
    notify_all_but_self(ctx->server, &room->members, ctx->message, ctx->client_fd);
}

static bool process_join_room(
    Server* server,
    const Message* msg_in,
    int client_fd
)
{
    if (msg_in->roomname == NULL) return false;

    Message response;
    message_init(&response);
    response.type = MESSAGE_TYPE_RESPONSE;
    response.operation = strdup("JOIN_ROOM");
    response.extra = strdup(msg_in->roomname);

    if (response.operation == NULL || response.extra == NULL)
    {
        message_destroy(&response);
        return false;
    }

    if (!room_table_contains(&server->rooms, msg_in->roomname))
    {
        response.result = strdup("NO_SUCH_ROOM");

        if (response.result == NULL)
        {
            message_destroy(&response);
            return false;
        }

        int sent = server_send(server, client_fd, &response);

        message_destroy(&response);

        if (sent < 0) return process_disconnect(server, client_fd);

        return true;
    }

    User user_from;
    if (users_table_find_by_client_fd(&server->users, client_fd, &user_from))
    {
        Room_Visitor_Context ctx = { .user = &user_from, .is_in_room = false };
        room_table_with(&server->rooms, msg_in->roomname, room_is_a_member, &ctx);
        if (!ctx.is_in_room)
        {
            room_table_with(&server->rooms, msg_in->roomname, room_is_a_guest, &ctx);
            if (ctx.is_in_room)
            {
                room_table_with
                (
                    &server->rooms, 
                    msg_in->roomname, 
                    room_from_guest_to_member, 
                    &ctx
                );

                response.result = strdup("SUCCESS");

                if (response.result == NULL)
                {
                    message_destroy(&response);
                    return false;
                }

                int sent = server_send(server, client_fd, &response);
                message_destroy(&response);
                if (sent < 0) return process_disconnect(server, client_fd);

                Message joined_room;
                message_init(&joined_room);
                joined_room.type = MESSAGE_TYPE_JOINED_ROOM;
                joined_room.roomname = strdup(msg_in->roomname);
                joined_room.username = strdup(user_from.username);

                Send_Context ctx = {
                    .server = server,
                    .message = &joined_room,
                    .client_fd = client_fd
                };
                room_table_for_each(&server->rooms, room_notify_members, &ctx);
                
                return true;
            }   
            else
            {
                response.result = strdup("NOT_INVITED");

                if (response.result == NULL)
                {
                    message_destroy(&response);
                    return false;
                }

                int sent = server_send(server, client_fd, &response);
                message_destroy(&response);
                if (sent < 0) return process_disconnect(server, client_fd);

                return true;
            } 
        }
    }
    else return process_disconnect(server, client_fd);

    return true;
}

static void room_users (Room* room, void* context)
{
    Room_Visitor_Context* ctx = context;

    Users_Table_Iter it;
    users_table_iter_begin(&room->members, &it);
    User user;
    while (users_table_iter_next(&it, &user))
        cJSON_AddStringToObject
        (
            ctx->users_obj,
            user.username,
            user_status_to_string(user.status)
        );
    users_table_iter_end(&it);
}

static bool process_room_users(
    Server* server,
    const Message* msg_in,
    int client_fd
)
{
    if (msg_in->roomname == NULL) return false;

    Message response;
    message_init(&response);
    response.type = MESSAGE_TYPE_RESPONSE;
    response.operation = strdup("ROOM_USERS");
    response.extra = strdup(msg_in->roomname);

    if (response.operation == NULL || response.extra == NULL)
    {
        message_destroy(&response);
        return false;
    }

    if (!room_table_contains(&server->rooms, msg_in->roomname))
    {
        response.result = strdup("NO_SUCH_ROOM");

        if (response.result == NULL)
        {
            message_destroy(&response);
            return false;
        }

        int sent = server_send(server, client_fd, &response);

        message_destroy(&response);

        if (sent < 0) return process_disconnect(server, client_fd);

        return true;
    }

    User user_from;
    if (users_table_find_by_client_fd(&server->users, client_fd, &user_from))
    {
        Room_Visitor_Context ctx = { .user = &user_from, .is_in_room = false };
        room_table_with(&server->rooms, msg_in->roomname, room_is_a_member, &ctx);

        if (ctx.is_in_room)
        {
            Message user_list;
            message_init(&user_list);
            user_list.type = MESSAGE_TYPE_ROOM_USER_LIST;
            user_list.roomname = strdup(msg_in->roomname);

            if (user_list.roomname == NULL)
            {
                message_destroy(&user_list);
                return false;
            }

            cJSON* users = cJSON_CreateObject();

            Room_Visitor_Context ctx = { .users_obj = users };
            room_table_with(&server->rooms, msg_in->roomname, room_users, &ctx);

            user_list.users = users;

            int sent = server_send(server, client_fd, &user_list);

            message_destroy(&user_list);

            if (sent < 0) return process_disconnect(server, client_fd);
        }
        else
        {
            response.result = strdup("NOT_JOINED");

            if (response.result == NULL)
            {
                message_destroy(&response);
                return false;
            }

            int sent = server_send(server, client_fd, &response);
            message_destroy(&response);
            if (sent < 0) return process_disconnect(server, client_fd);
        }
    }
    else return process_disconnect(server, client_fd);
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
        case MESSAGE_TYPE_NEW_ROOM:
            return process_new_room(server, msg_in, client_fd);
        case MESSAGE_TYPE_INVITE:
            return process_invite(server, msg_in, client_fd);    
        case MESSAGE_TYPE_JOIN_ROOM:
            return process_join_room(server, msg_in, client_fd);
        case MESSAGE_TYPE_ROOM_USERS:
            return process_room_users(server, msg_in, client_fd);    
        default:
            break;
    }

    return true;
}