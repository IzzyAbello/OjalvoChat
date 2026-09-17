#include <criterion/criterion.h>
#include <string.h>
#include "../src/user.h"

Test(user_init, success_valid_parameters)
{
    User user;
    bool result = user_init(&user, "alice", 5);

    cr_assert(result);
    cr_assert_str_eq(user.username, "alice");
    cr_assert_eq(user.socket_fd, 5);
    cr_assert_eq(user.status, USER_STATUS_ACTIVE);
}

Test(user_init, fail_null_username)
{
    User user;
    bool result = user_init(&user, NULL, 5);

    cr_assert_not(result);
}

Test(user_init, fail_empty_username)
{
    User user;
    bool result = user_init(&user, "", 5);

    cr_assert_not(result);
}

Test(user_init, success_max_length_username)
{
    User user;
    char max_username[USER_USERNAME_MAX_LENGTH + 1];
    memset(max_username, 'a', USER_USERNAME_MAX_LENGTH);
    max_username[USER_USERNAME_MAX_LENGTH] = '\0';

    bool result = user_init(&user, max_username, 10);

    cr_assert(result);
    cr_assert_str_eq(user.username, max_username);
}

Test(user_init, fail_exceeds_max_length)
{
    User user;
    char long_username[USER_USERNAME_MAX_LENGTH + 2];
    memset(long_username, 'a', USER_USERNAME_MAX_LENGTH + 1);
    long_username[USER_USERNAME_MAX_LENGTH + 1] = '\0';

    bool result = user_init(&user, long_username, 10);

    cr_assert_not(result);
}

Test(user_status_to_string, valid_statuses)
{
    cr_assert_str_eq(user_status_to_string(USER_STATUS_ACTIVE), "ACTIVE");
    cr_assert_str_eq(user_status_to_string(USER_STATUS_AWAY), "AWAY");
    cr_assert_str_eq(user_status_to_string(USER_STATUS_BUSY), "BUSY");
}

Test(user_status_to_string, invalid_enum_value_defaults_to_active)
{
    User_Status invalid_status = (User_Status)999;
    cr_assert_str_eq(user_status_to_string(invalid_status), "ACTIVE");
}

Test(user_status_from_string, success_valid_strings)
{
    User_Status status;

    cr_assert(user_status_from_string("ACTIVE", &status));
    cr_assert_eq(status, USER_STATUS_ACTIVE);

    cr_assert(user_status_from_string("AWAY", &status));
    cr_assert_eq(status, USER_STATUS_AWAY);

    cr_assert(user_status_from_string("BUSY", &status));
    cr_assert_eq(status, USER_STATUS_BUSY);
}

Test(user_status_from_string, fail_null_text)
{
    User_Status status;
    bool result = user_status_from_string(NULL, &status);

    cr_assert_not(result);
}

Test(user_status_from_string, fail_invalid_strings)
{
    User_Status status;

    cr_assert_not(user_status_from_string("active", &status));
    cr_assert_not(user_status_from_string("UNKNOWN", &status));
    cr_assert_not(user_status_from_string("", &status));
}