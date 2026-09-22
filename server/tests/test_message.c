#include <criterion/criterion.h>
#include <stdio.h>
#include <string.h>

#include "../src/message.h"

Test(message_to_json, response_with_extra)
{
    Message msg;
    message_init(&msg);
    msg.type = MESSAGE_TYPE_RESPONSE;
    msg.operation = "IDENTIFY";
    msg.result = "SUCCESS";
    msg.extra = "Kimberly";

    char out[256];
    bool ok = message_to_json(&msg, out, sizeof(out));

    cr_assert(ok);
    cr_assert_str_eq(out, "{\"type\":\"RESPONSE\",\"operation\":\"IDENTIFY\",\"result\":\"SUCCESS\",\"extra\":\"Kimberly\"}");
}

Test(message_to_json, null_fields_are_omitted)
{
    Message msg;
    message_init(&msg);
    msg.type = MESSAGE_TYPE_RESPONSE;
    msg.operation = "INVALID";
    msg.result = "NOT_IDENTIFIED";

    char out[256];
    bool ok = message_to_json(&msg, out, sizeof(out));

    cr_assert(ok);
    cr_assert(strstr(out, "extra") == NULL);
}

Test(message_to_json, public_text_from)
{
    Message msg;
    message_init(&msg);
    msg.type = MESSAGE_TYPE_PUBLIC_TEXT_FROM;
    msg.username = "Kimberly";
    msg.text = "hola a todos";

    char out[256];
    bool ok = message_to_json(&msg, out, sizeof(out));

    cr_assert(ok);
    cr_assert_str_eq(out, "{\"type\":\"PUBLIC_TEXT_FROM\",\"username\":\"Kimberly\",\"text\":\"hola a todos\"}");
}

Test(message_to_json, invite_with_usernames_array)
{
    Message msg;
    message_init(&msg);
    msg.type = MESSAGE_TYPE_INVITE;
    msg.roomname = "Sala 1";

    char *usernames[] = {"miguel", "diaz", "canel"};
    msg.usernames = usernames;
    msg.usernames_count = 3;

    char out[256];
    bool ok = message_to_json(&msg, out, sizeof(out));

    cr_assert(ok);
    cr_assert_str_eq(out, "{\"type\":\"INVITE\",\"roomname\":\"Sala 1\",\"usernames\":[\"miguel\",\"diaz\",\"canel\"]}");


}

Test(message_to_json, users_object)
{
    Message msg;
    message_init(&msg);
    msg.type = MESSAGE_TYPE_INVITE;
    msg.roomname = "Sala 1";

    cJSON* users = cJSON_CreateObject();
    cJSON_AddStringToObject(users, "Luis", "ACTIVE");
    cJSON_AddStringToObject(users, "Antonio", "AWAY");
    cJSON_AddStringToObject(users, "Fernando", "BUSY");
    msg.users = users;

    char out[256];
    bool ok = message_to_json(&msg, out, sizeof(out));

    cr_assert(ok);
    cr_assert_str_eq(out, "{\"type\":\"INVITE\",\"roomname\":\"Sala 1\",\"users\":{\"Luis\":\"ACTIVE\",\"Antonio\":\"AWAY\",\"Fernando\":\"BUSY\"}}");

    cJSON_Delete(msg.users);
}

Test(message_to_json, message_with_no_extra_fields)
{
    Message msg;
    message_init(&msg);
    msg.type = MESSAGE_TYPE_DISCONNECT;

    char out[256];
    bool ok = message_to_json(&msg, out, sizeof(out));

    cr_assert(ok);
    cr_assert_str_eq(out, "{\"type\":\"DISCONNECT\"}");
}

Test(message_to_json, output_buffer_too_small_fails_safely)
{
    Message msg;
    message_init(&msg);
    msg.type = MESSAGE_TYPE_RESPONSE;
    msg.operation = "IDENTIFY";
    msg.result = "SUCCESS";
    msg.extra = "Kimberly";

    char tiny[5];
    bool ok = message_to_json(&msg, tiny, sizeof(tiny));

    cr_assert_not(ok);
}

Test(message_to_json, null_msg_is_rejected)
{
    char out[256];
    cr_assert_not(message_to_json(NULL, out, sizeof(out)));
}

Test(message_to_json, null_out_json_is_rejected)
{
    Message msg;
    message_init(&msg);
    msg.type = MESSAGE_TYPE_DISCONNECT;

    cr_assert_not(message_to_json(&msg, NULL, 256));
}

Test(message_to_json, round_trip_with_from_json)
{
    Message original;
    message_init(&original);
    original.type = MESSAGE_TYPE_TEXT_FROM;
    original.username = "Kimberly";
    original.text = "Hola Luis";

    char out[256];
    cr_assert(message_to_json(&original, out, sizeof(out)));

    Message parsed;
    cr_assert(message_from_json(out, &parsed));

    cr_assert_eq(parsed.type, MESSAGE_TYPE_TEXT_FROM);
    cr_assert_str_eq(parsed.username, "Kimberly");
    cr_assert_str_eq(parsed.text, "Hola Luis");

    message_destroy(&parsed);
}

Test(message_to_json, room_user_list_type_serializes_correctly)
{
    Message msg;
    message_init(&msg);
    msg.type = MESSAGE_TYPE_ROOM_USER_LIST;

    char out[256];
    cr_assert(message_to_json(&msg, out, sizeof(out)));
    cr_assert(strstr(out, "\"type\":\"ROOM_USER_LIST\"") != NULL);
}

Test(message_init, all_fields_start_empty)
{
    Message msg;
    memset(&msg, 0xAA, sizeof(msg));

    message_init(&msg);

    cr_assert_eq(msg.type, MESSAGE_TYPE_UNKNOWN);
    cr_assert_null(msg.username);
    cr_assert_null(msg.operation);
    cr_assert_null(msg.result);
    cr_assert_null(msg.extra);
    cr_assert_null(msg.status);
    cr_assert_null(msg.users);
    cr_assert_null(msg.text);
    cr_assert_null(msg.roomname);
    cr_assert_null(msg.usernames);
    cr_assert_eq(msg.usernames_count, 0);
}

Test(message_init, null_msg_does_not_crash)
{
    message_init(NULL);
}

Test(message_destroy, resets_fields_after_freeing)
{
    Message msg;
    message_init(&msg);
    cr_assert(message_from_json("{\"type\":\"IDENTIFY\",\"username\":\"kim\"}", &msg));

    message_destroy(&msg);

    cr_assert_null(msg.username);
    cr_assert_eq(msg.type, MESSAGE_TYPE_UNKNOWN);
}

Test(message_destroy, safe_to_call_on_already_freed_message)
{
    Message msg;
    message_init(&msg);
    message_destroy(&msg);
    message_destroy(&msg);
}

Test(message_destroy, null_msg_does_not_crash)
{
    message_destroy(NULL);
}

Test(message_destroy, frees_usernames_array_without_crashing)
{
    Message msg;
    message_init(&msg);
    cr_assert(message_from_json(
        "{\"type\":\"INVITE\",\"roomname\":\"Sala 1\",\"usernames\":[\"Luis\",\"Antonio\"]}",
        &msg));
    cr_assert_eq(msg.usernames_count, 2);

    message_destroy(&msg);

    cr_assert_null(msg.usernames);
    cr_assert_eq(msg.usernames_count, 0);
}

Test(message_destroy, frees_users_object_without_crashing)
{
    Message msg;
    message_init(&msg);
    cr_assert(message_from_json(
        "{\"type\":\"INVITE\",\"roomname\":\"Sala 1\",\"users\":{\"Luis\":\"ACTIVE\",\"Antonio\":\"AWAY\"}}",
        &msg)
    );

    cr_assert_eq(cJSON_GetArraySize(msg.users), 2);
    
    message_destroy(&msg);

    cr_assert_null(msg.users);
}

Test(message_from_json, parses_simple_fields)
{
    Message msg;
    message_init(&msg);

    bool ok = message_from_json("{\"type\":\"TEXT\",\"username\":\"Luis\",\"text\":\"hola\"}", &msg);

    cr_assert(ok);
    cr_assert_eq(msg.type, MESSAGE_TYPE_TEXT);
    cr_assert_str_eq(msg.username, "Luis");
    cr_assert_str_eq(msg.text, "hola");
    cr_assert_null(msg.roomname);

    message_destroy(&msg);
}

Test(message_from_json, missing_fields_stay_null)
{
    Message msg;
    message_init(&msg);

    bool ok = message_from_json("{\"type\":\"USERS\"}", &msg);

    cr_assert(ok);
    cr_assert_eq(msg.type, MESSAGE_TYPE_USERS);
    cr_assert_null(msg.username);
    cr_assert_null(msg.text);

    message_destroy(&msg);
}

Test(message_from_json, unrecognized_type_string_becomes_unknown)
{
    Message msg;
    message_init(&msg);

    bool ok = message_from_json("{\"type\":\"ALGO_QUE_NO_EXISTE\"}", &msg);

    cr_assert(ok);
    cr_assert_eq(msg.type, MESSAGE_TYPE_UNKNOWN);

    message_destroy(&msg);
}

Test(message_from_json, malformed_json_is_rejected)
{
    Message msg;
    message_init(&msg);

    bool ok = message_from_json("esto no es json", &msg);

    cr_assert_not(ok);
}

Test(message_from_json, null_arguments_are_rejected)
{
    Message msg;
    message_init(&msg);

    cr_assert_not(message_from_json(NULL, &msg));
    cr_assert_not(message_from_json("{}", NULL));
}

Test(message_from_json, parses_usernames_array)
{
    Message msg;
    message_init(&msg);

    bool ok = message_from_json(
        "{\"type\":\"INVITE\",\"roomname\":\"Sala 1\",\"usernames\":[\"miguel\",\"diaz\",\"canel\"]}",
        &msg
    );

    cr_assert(ok);
    cr_assert_eq(msg.usernames_count, 3);
    cr_assert_str_eq(msg.usernames[0], "miguel");
    cr_assert_str_eq(msg.usernames[1], "diaz");
    cr_assert_str_eq(msg.usernames[2], "canel");

    message_destroy(&msg);
}

Test(message_from_json, parses_users_object)
{
    Message msg;
    message_init(&msg);

    bool ok = message_from_json(
        "{\"type\":\"INVITE\",\"roomname\":\"Sala 1\",\"users\":{\"Luis\":\"AWAY\",\"Antonio\":\"BUSY\",\"Fernando\":\"ACTIVE\"}}",
        &msg
    );

    cr_assert(ok);
    cr_assert_eq(cJSON_GetArraySize(msg.users), 3);

    cJSON* item = cJSON_GetObjectItemCaseSensitive(msg.users, "Luis");
    cr_assert_str_eq(item->valuestring, "AWAY");

    item = cJSON_GetObjectItemCaseSensitive(msg.users, "Antonio");
    cr_assert_str_eq(item->valuestring, "BUSY");

    item = cJSON_GetObjectItemCaseSensitive(msg.users, "Fernando");
    cr_assert_str_eq(item->valuestring, "ACTIVE");

    message_destroy(&msg);
}

Test(message_from_json, missing_usernames_key_leaves_array_null)
{
    Message msg;
    message_init(&msg);

    bool ok = message_from_json("{\"type\":\"NEW_ROOM\",\"roomname\":\"Sala 1\"}", &msg);

    cr_assert(ok);
    cr_assert_null(msg.users);
    cr_assert_null(msg.usernames);
    cr_assert_eq(msg.usernames_count, 0);

    message_destroy(&msg);
}

Test(message_from_json, empty_usernames_array_leaves_count_zero)
{
    Message msg;
    message_init(&msg);

    bool ok = message_from_json("{\"type\":\"INVITE\",\"roomname\":\"Sala 1\",\"usernames\":[]}", &msg);

    cr_assert(ok);
    cr_assert_eq(msg.usernames_count, 0);

    message_destroy(&msg);
}

Test(message_from_json, empty_users_object_leaves_count_zero)
{
    Message msg;
    message_init(&msg);

    bool ok = message_from_json("{\"type\":\"INVITE\",\"roomname\":\"Sala 1\",\"users\":{}}", &msg);

    cr_assert(ok);
    cr_assert_eq(cJSON_GetArraySize(msg.users), 0);

    message_destroy(&msg);
}