#include <criterion/criterion.h>
#include <stdbool.h>

#include "../src/users_table.h"

static Users_Table table;

void setup(void)
{
    users_table_init(&table);
}

void destroy(void)
{
    users_table_destroy(&table);
}

Test(users_table, init_and_destroy)
{
    Users_Table t;
    users_table_init(&t);
    cr_assert_not_null(t.table);
    users_table_destroy(&t);
}

Test(users_table, add_valid_user, .init = setup, .fini = destroy)
{
    bool added = users_table_add(&table, "alice", 10);
    cr_assert(added);

    User out_user;
    bool found = users_table_find(&table, "alice", &out_user);
    cr_assert(found);
    cr_assert_eq(out_user.socket_fd, 10);
}

Test(users_table, add_duplicate_user, .init = setup, .fini = destroy)
{
    bool first_add = users_table_add(&table, "bob", 11);
    cr_assert(first_add);

    bool second_add = users_table_add(&table, "bob", 12);
    cr_assert_not(second_add);

    User out_user;
    users_table_find(&table, "bob", &out_user);
    cr_assert_eq(out_user.socket_fd, 11);
}

Test(users_table, find_non_existent_user, .init = setup, .fini = destroy)
{
    User out_user;
    bool found = users_table_find(&table, "casper", &out_user);
    cr_assert_not(found);
}

Test(users_table, remove_user, .init = setup, .fini = destroy)
{
    users_table_add(&table, "fidel", 15);

    bool removed = users_table_remove(&table, "fidel");
    cr_assert(removed);

    User out_user;
    bool found = users_table_find(&table, "fidel", &out_user);
    cr_assert_not(found);

    bool remove_again = users_table_remove(&table, "fidel");
    cr_assert_not(remove_again);
}

// --------------------------------------------------------------------------
// IMPLEMENTAR --> prueba de concurrencia
// --------------------------------------------------------------------------