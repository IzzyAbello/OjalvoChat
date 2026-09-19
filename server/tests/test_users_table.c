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
    bool found = users_table_find_by_username(&table, "alice", &out_user);
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
    users_table_find_by_username(&table, "bob", &out_user);
    cr_assert_eq(out_user.socket_fd, 11);
}

Test(users_table, find_by_username_non_existent_user, .init = setup, .fini = destroy)
{
    User out_user;
    bool found = users_table_find_by_username(&table, "casper", &out_user);
    cr_assert_not(found);
}

Test(users_table_find_by_client_fd, finds_user_added_by_add)
{
    Users_Table table;
    users_table_init(&table);
    users_table_add(&table, "fidel", 42);
 
    User found;
    bool exists = users_table_find_by_client_fd(&table, 42, &found);
 
    cr_assert(exists);
    cr_assert_str_eq(found.username, "fidel");
    cr_assert_eq(found.socket_fd, 42);
 
    users_table_destroy(&table);
}
 
Test(users_table_find_by_client_fd, unknown_fd_is_not_found)
{
    Users_Table table;
    users_table_init(&table);
    users_table_add(&table, "putin", 42);
 
    User found;
    bool exists = users_table_find_by_client_fd(&table, 999, &found);
 
    cr_assert_not(exists);
 
    users_table_destroy(&table);
}

Test(users_table_find_by_client_fd, agrees_with_find_by_username)
{
    Users_Table table;
    users_table_init(&table);
    users_table_add(&table, "bibi", 67);
 
    User by_username, by_fd;
    users_table_find_by_username(&table, "bibi", &by_username);
    users_table_find_by_client_fd(&table, 67, &by_fd);
 
    cr_assert_eq(by_username.socket_fd, by_fd.socket_fd);
    cr_assert_str_eq(by_username.username, by_fd.username);
 
    users_table_destroy(&table);
}
 
Test(users_table_find_by_client_fd, removed_user_disappears_from_fd_index_too)
{
    Users_Table table;
    users_table_init(&table);
    users_table_add(&table, "claudia", 42);
 
    cr_assert(users_table_remove(&table, "claudia"));
 
    User found;
    cr_assert_not(users_table_find_by_client_fd(&table, 42, &found));
 
    users_table_destroy(&table);
}
 
Test(users_table_find_by_client_fd, multiple_users_are_indexed_independently)
{
    Users_Table table;
    users_table_init(&table);
    users_table_add(&table, "kim", 1);
    users_table_add(&table, "jong", 2);
    users_table_add(&table, "un", 3);
 
    User found;
    cr_assert(users_table_find_by_client_fd(&table, 1, &found));
    cr_assert_str_eq(found.username, "kim");
 
    cr_assert(users_table_find_by_client_fd(&table, 2, &found));
    cr_assert_str_eq(found.username, "jong");
 
    cr_assert(users_table_find_by_client_fd(&table, 3, &found));
    cr_assert_str_eq(found.username, "un");
 
    users_table_destroy(&table);
}

Test(users_table, remove_user, .init = setup, .fini = destroy)
{
    users_table_add(&table, "fidel", 15);

    bool removed = users_table_remove(&table, "fidel");
    cr_assert(removed);

    User out_user;
    bool found = users_table_find_by_username(&table, "fidel", &out_user);
    cr_assert_not(found);

    bool remove_again = users_table_remove(&table, "fidel");
    cr_assert_not(remove_again);
}

Test(users_table_iter, iterates_over_all_users)
{
    Users_Table table;
    users_table_init(&table);
 
    users_table_add(&table, "fidel", 1);
    users_table_add(&table, "raul", 2);
    users_table_add(&table, "canel", 3);
 
    int seen_count = 0;
    bool saw_fidel = false, saw_raul = false, saw_canel = false;
 
    Users_Table_Iter iter;
    users_table_iter_begin(&table, &iter);
 
    User user;
    while (users_table_iter_next(&iter, &user))
    {
        seen_count++;
        if (strcmp(user.username, "fidel") == 0) saw_fidel = true;
        if (strcmp(user.username, "raul") == 0) saw_raul = true;
        if (strcmp(user.username, "canel") == 0) saw_canel = true;
    }
 
    users_table_iter_end(&iter);
 
    cr_assert_eq(seen_count, 3);
    cr_assert(saw_fidel);
    cr_assert(saw_raul);
    cr_assert(saw_canel);
 
    users_table_destroy(&table);
}
 
Test(users_table_iter, empty_table_yields_nothing)
{
    Users_Table table;
    users_table_init(&table);
 
    Users_Table_Iter iter;
    users_table_iter_begin(&table, &iter);
 
    User user;
    bool got_any = users_table_iter_next(&iter, &user);
 
    users_table_iter_end(&iter);
 
    cr_assert_not(got_any);
    users_table_destroy(&table);
}
 
Test(users_table_iter, table_is_usable_again_after_iteration_ends)
{
    Users_Table table;
    users_table_init(&table);
    users_table_add(&table, "jeffry", 1);
 
    Users_Table_Iter iter;
    users_table_iter_begin(&table, &iter);
    User user;
    while (users_table_iter_next(&iter, &user)) {}
    users_table_iter_end(&iter);
 
    cr_assert(users_table_add(&table, "epstein", 2));
 
    User found;
    cr_assert(users_table_find_by_username(&table, "epstein", &found));
 
    users_table_destroy(&table);
}
 
Test(users_table_iter, copies_are_independent_of_the_table)
{
    Users_Table table;
    users_table_init(&table);
    users_table_add(&table, "bibi", 1);
 
    Users_Table_Iter iter;
    users_table_iter_begin(&table, &iter);
    User user;
    users_table_iter_next(&iter, &user);
    users_table_iter_end(&iter);
 
    user.socket_fd = 999;
 
    User found;
    users_table_find_by_username(&table, "bibi", &found);
    cr_assert_eq(found.socket_fd, 1);
 
    users_table_destroy(&table);
}

static void mark_all_busy(User* user, void* context)
{
    (void)context;
    user->status = USER_STATUS_BUSY;
}
 
Test(users_table_for_each, mutates_the_real_entries)
{
    Users_Table table;
    users_table_init(&table);
    users_table_add(&table, "fidel", 1);
    users_table_add(&table, "raul", 2);
 
    users_table_for_each(&table, mark_all_busy, NULL);
 
    User found;
    users_table_find_by_username(&table, "fidel", &found);
    cr_assert_eq(found.status, USER_STATUS_BUSY);
    users_table_find_by_username(&table, "raul", &found);
    cr_assert_eq(found.status, USER_STATUS_BUSY);
 
    users_table_destroy(&table);
}
 
static void count_visitor(User* user, void* context)
{
    (void)user;
    int* count = (int*)context;
    (*count)++;
}
 
Test(users_table_for_each, context_pointer_is_passed_through)
{
    Users_Table table;
    users_table_init(&table);
    users_table_add(&table, "kim", 1);
    users_table_add(&table, "jong", 2);
    users_table_add(&table, "un", 3);
 
    int count = 0;
    users_table_for_each(&table, count_visitor, &count);
 
    cr_assert_eq(count, 3);
    users_table_destroy(&table);
}
 
Test(users_table_for_each, table_is_usable_again_afterwards)
{
    Users_Table table;
    users_table_init(&table);
    users_table_add(&table, "bibi", 1);
 
    users_table_for_each(&table, mark_all_busy, NULL);
 
    cr_assert(users_table_add(&table, "claudia", 2));
 
    users_table_destroy(&table);
}