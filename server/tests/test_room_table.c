#include <criterion/criterion.h>
#include <string.h>

#include "../src/room_table.h"

Test(room_table_add, adding_and_contains)
{
    Room_Table table;
    room_table_init(&table);

    cr_assert(room_table_add(&table, "Sala 1"));
    cr_assert(room_table_contains(&table, "Sala 1"));
    cr_assert_not(room_table_contains(&table, "Sala 2"));

    room_table_destroy(&table);
}

Test(room_table_add, duplicate_roomname_is_rejected)
{
    Room_Table table;
    room_table_init(&table);

    cr_assert(room_table_add(&table, "Sala 1"));
    cr_assert_not(room_table_add(&table, "Sala 1"));

    room_table_destroy(&table);
}

Test(room_table_add, invalid_roomname_is_rejected)
{
    Room_Table table;
    room_table_init(&table);

    cr_assert_not(room_table_add(&table, "12345678901234567"));
    cr_assert_not(room_table_add(&table, ""));
    cr_assert_not(room_table_add(&table, NULL));

    room_table_destroy(&table);
}

Test(room_table_remove, removing_existing_room)
{
    Room_Table table;
    room_table_init(&table);
    room_table_add(&table, "Sala 1");

    cr_assert(room_table_remove(&table, "Sala 1"));
    cr_assert_not(room_table_contains(&table, "Sala 1"));

    room_table_destroy(&table);
}

Test(room_table_remove, removing_nonexistent_room_returns_false)
{
    Room_Table table;
    room_table_init(&table);

    cr_assert_not(room_table_remove(&table, "Sala fantasma"));

    room_table_destroy(&table);
}

Test(room_table_add, roomname_can_be_reused_after_removal)
{
    Room_Table table;
    room_table_init(&table);

    room_table_add(&table, "Sala 1");
    room_table_remove(&table, "Sala 1");

    cr_assert(room_table_add(&table, "Sala 1"));

    room_table_destroy(&table);
}

static void add_fidel_only(Room* room, void* context)
{
    (void)context;
    users_table_add(&room->members, "fidel", 1);
}

static void add_and_verify_both(Room* room, void* context)
{
    (void)context;

    users_table_add(&room->members, "fidel", 1);
    users_table_add(&room->guests, "fidel", 2);

    User member, guest;
    cr_assert(users_table_find_by_username(&room->members, "fidel", &member));
    cr_assert(users_table_find_by_username(&room->guests, "fidel", &guest));
    cr_assert_eq(member.socket_fd, 1);
    cr_assert_eq(guest.socket_fd, 2);
}

Test(room_table_with, existing_room_runs_action_and_mutates_it_for_real)
{
    Room_Table table;
    room_table_init(&table);
    room_table_add(&table, "Sala 1");

    bool existed = room_table_with(&table, "Sala 1", add_and_verify_both, NULL);

    cr_assert(existed);

    room_table_destroy(&table);
}

static bool action_was_called = false;

static void mark_called(Room* room, void* context)
{
    (void)room;
    (void)context;
    action_was_called = true;
}

Test(room_table_with, nonexistent_room_does_not_run_action)
{
    Room_Table table;
    room_table_init(&table);
    action_was_called = false;

    bool existed = room_table_with(&table, "Sala fantasma", mark_called, NULL);

    cr_assert_not(existed);
    cr_assert_not(action_was_called);

    room_table_destroy(&table);
}

static char last_seen_username[16] = {0};

static void record_first_member(Room* room, void* context)
{
    (void)context;
    Users_Table_Iter iter;
    users_table_iter_begin(&room->members, &iter);
    User user;
    if (users_table_iter_next(&iter, &user))
        strncpy(last_seen_username, user.username, sizeof(last_seen_username) - 1);
    users_table_iter_end(&iter);
}

Test(room_table_with, changes_persist_across_separate_calls)
{
    Room_Table table;
    room_table_init(&table);
    room_table_add(&table, "Sala 1");

    room_table_with(&table, "Sala 1", add_fidel_only, NULL);
    room_table_with(&table, "Sala 1", record_first_member, NULL);

    cr_assert_str_eq(last_seen_username, "fidel");

    room_table_destroy(&table);
}

static void count_visitor(Room* room, void* context)
{
    (void)room;
    int* count = (int*)context;
    (*count)++;
}

Test(room_table_for_each, visits_all_rooms)
{
    Room_Table table;
    room_table_init(&table);
    room_table_add(&table, "Sala 1");
    room_table_add(&table, "Sala 2");
    room_table_add(&table, "Sala 3");

    int count = 0;
    room_table_for_each(&table, count_visitor, &count);

    cr_assert_eq(count, 3);

    room_table_destroy(&table);
}

Test(room_table_for_each, empty_table_visits_nothing)
{
    Room_Table table;
    room_table_init(&table);

    int count = 0;
    room_table_for_each(&table, count_visitor, &count);

    cr_assert_eq(count, 0);

    room_table_destroy(&table);
}

Test(room_table_for_each, mutates_the_real_rooms)
{
    Room_Table table;
    room_table_init(&table);
    room_table_add(&table, "Sala 1");
    room_table_add(&table, "Sala 2");

    room_table_for_each(&table, add_fidel_only, NULL);

    last_seen_username[0] = '\0';
    room_table_with(&table, "Sala 1", record_first_member, NULL);
    cr_assert_str_eq(last_seen_username, "fidel");

    last_seen_username[0] = '\0';
    room_table_with(&table, "Sala 2", record_first_member, NULL);
    cr_assert_str_eq(last_seen_username, "fidel");

    room_table_destroy(&table);
}