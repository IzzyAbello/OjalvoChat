#include <criterion/criterion.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "../src/users_table.h"

#define THREAD_COUNT 20
#define STRESS_ITERATIONS 200

static int count_users(Users_Table* table)
{
    int count = 0;
    User user;
    Users_Table_Iter iter;

    users_table_iter_begin(table, &iter);
    while (users_table_iter_next(&iter, &user)) count++;
    users_table_iter_end(&iter);

    return count;
}

typedef struct
{
    Users_Table *table;
    int index;
    bool result;
}
Add_Task;

static void* add_distinct_user(void* arg)
{
    Add_Task* task = (Add_Task*)arg;
    char username[USER_USERNAME_MAX_LENGTH + 1];
    snprintf(username, sizeof(username), "u%d", task->index);
    task->result = users_table_add(task->table, username, task->index);
    return NULL;
}

Test(users_table_concurrency, distinct_usernames_all_succeed)
{
    Users_Table table;
    users_table_init(&table);

    pthread_t threads[THREAD_COUNT];
    Add_Task tasks[THREAD_COUNT];

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        tasks[i].table = &table;
        tasks[i].index = i;
        tasks[i].result = false;
        pthread_create(&threads[i], NULL, add_distinct_user, &tasks[i]);
    }
    for (int i = 0; i < THREAD_COUNT; i++)
        pthread_join(threads[i], NULL);

    for (int i = 0; i < THREAD_COUNT; i++)
        cr_assert(
            tasks[i].result,
            "[TEST] El hilo %d no pudo agregar su usuario",
            i
        );

    cr_assert_eq(count_users(&table), THREAD_COUNT);

    users_table_destroy(&table);
}


typedef struct
{
    Users_Table* table;
    int socket_fd;
    bool result;
}
Same_Add_Task;

static void* add_same_client(void* arg)
{
    Same_Add_Task* task = (Same_Add_Task*)arg;
    task->result = users_table_add(task->table, "fidel", task->socket_fd);
    return NULL;
}

Test(users_table_concurrency, same_username_only_one_wins)
{
    Users_Table table;
    users_table_init(&table);

    pthread_t threads[THREAD_COUNT];
    Same_Add_Task tasks[THREAD_COUNT];

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        tasks[i].table = &table;
        tasks[i].socket_fd = i;
        tasks[i].result = false;
        pthread_create(&threads[i], NULL, add_same_client, &tasks[i]);
    }
    for (int i = 0; i < THREAD_COUNT; i++)
        pthread_join(threads[i], NULL);

    int successes = 0;
    for (int i = 0; i < THREAD_COUNT; i++)
        if (tasks[i].result) successes++;

    cr_assert_eq(successes, 1);
    cr_assert_eq(count_users(&table), 1);

    users_table_destroy(&table);
}


typedef struct
{
    Users_Table* table;
    int thread_index;
}
Stress_Task;

static void* stress_client(void* arg)
{
    Stress_Task *task = (Stress_Task*)arg;
    char username[USER_USERNAME_MAX_LENGTH + 1];
    snprintf(username, sizeof(username), "s%d", task->thread_index);

    for (int i = 0; i < STRESS_ITERATIONS; i++)
    {
        users_table_add(task->table, username, task->thread_index);

        User found;
        users_table_find_by_username(task->table, username, &found);

        users_table_remove(task->table, username);
    }
    return NULL;
}

Test(users_table_concurrency, stress_add_find_by_username_remove_no_crash)
{
    Users_Table table;
    users_table_init(&table);

    pthread_t threads[THREAD_COUNT];
    Stress_Task tasks[THREAD_COUNT];

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        tasks[i].table = &table;
        tasks[i].thread_index = i;
        pthread_create(&threads[i], NULL, stress_client, &tasks[i]);
    }
    for (int i = 0; i < THREAD_COUNT; i++)
        pthread_join(threads[i], NULL);

    cr_assert_eq(count_users(&table), 0);

    users_table_destroy(&table);
}