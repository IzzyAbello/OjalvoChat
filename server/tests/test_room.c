#include <criterion/criterion.h>
#include <string.h>

#include "../src/room.h"

Test(room_init, valid_roomname_sets_fields)
{
    Room room;
    bool ok = room_init(&room, "Sala 1");

    cr_assert(ok);
    cr_assert_str_eq(room.roomname, "Sala 1");

    room_destroy(&room);
}

Test(room_init, members_and_guests_start_empty)
{
    Room room;
    room_init(&room, "Sala 1");

    int members_count = 0;
    Users_Table_Iter iter;
    users_table_iter_begin(&room.members, &iter);
    User user;
    while (users_table_iter_next(&iter, &user)) members_count++;
    users_table_iter_end(&iter);

    int guests_count = 0;
    users_table_iter_begin(&room.guests, &iter);
    while (users_table_iter_next(&iter, &user)) guests_count++;
    users_table_iter_end(&iter);

    cr_assert_eq(members_count, 0);
    cr_assert_eq(guests_count, 0);

    room_destroy(&room);
}

Test(room_init, roomname_at_exactly_the_limit_is_valid)
{
    Room room;
    bool ok = room_init(&room, "1234567890123456");

    cr_assert(ok);
    cr_assert_str_eq(room.roomname, "1234567890123456");

    room_destroy(&room);
}

Test(room_init, roomname_over_the_limit_is_rejected)
{
    Room room;
    bool ok = room_init(&room, "12345678901234567");

    cr_assert_not(ok);
}

Test(room_init, empty_roomname_is_rejected)
{
    Room room;
    bool ok = room_init(&room, "");

    cr_assert_not(ok);
}

Test(room_init, null_roomname_is_rejected)
{
    Room room;
    bool ok = room_init(&room, NULL);

    cr_assert_not(ok);
}

Test(room_init, roomname_with_spaces_and_accents_is_valid)
{
    Room room;
    bool ok = room_init(&room, "Saló ñ");

    cr_assert(ok);
    cr_assert_str_eq(room.roomname, "Saló ñ");

    room_destroy(&room);
}

Test(room_init, members_and_guests_are_independent_tables)
{
    Room room;
    room_init(&room, "Sala 1");

    users_table_add(&room.members, "fidel", 1);

    User found;
    cr_assert(users_table_find_by_username(&room.members, "fidel", &found));
    cr_assert_not(users_table_find_by_username(&room.guests, "fidel", &found));

    room_destroy(&room);
}

Test(room_destroy, does_not_crash_on_fresh_room)
{
    Room room;
    room_init(&room, "Sala 1");
    room_destroy(&room);
}

Test(room_destroy, does_not_crash_with_members_and_guests_inside)
{
    Room room;
    room_init(&room, "Sala 1");
    users_table_add(&room.members, "fidel", 1);
    users_table_add(&room.guests, "raul", 2);

    room_destroy(&room);
}