#include <criterion/criterion.h>

#include "../src/server.h"

Test(server_parse_args, no_args_defaults) 
{
    char const *argv[] = {"server"};
    Server_Options options = server_parse_args(1, argv);

    cr_assert(options.show_help == false);
    cr_assert(options.log_enabled == false);
    cr_assert(options.port_arg == NULL);
}

Test(server_parse_args, help_flag_and_alias) 
{
    char const *argv_short[] = {"server", "-h"};
    cr_assert(server_parse_args(2, argv_short).show_help == true);

    char const *argv_long[] = {"server", "--help"};
    cr_assert(server_parse_args(2, argv_long).show_help == true);
}

Test(server_parse_args, log_flag_and_alias)
{
    char const *argv_short[] = {"server", "-l"};
    cr_assert(server_parse_args(2, argv_short).log_enabled == true);

    char const *argv_long[] = {"server", "--log"};
    cr_assert(server_parse_args(2, argv_long).log_enabled == true);
}

Test(server_parse_args, port_flag_variants)
{
    char const *argv_short[] = {"server", "-p", "5000"};
    cr_assert_str_eq(server_parse_args(3, argv_short).port_arg, "5000");

    char const *argv_long[] = {"server", "--port", "6060"};
    cr_assert_str_eq(server_parse_args(3, argv_long).port_arg, "6060");

    char const *argv_es[] = {"server", "--puerto", "7645"};
    cr_assert_str_eq(server_parse_args(3, argv_es).port_arg, "7645");
}

Test(server_parse_args, port_flag_without_value_does_not_crash)
{
    char const *argv[] = {"server", "-p"};
    Server_Options options = server_parse_args(2, argv);
    cr_assert_null(options.port_arg);
}

Test(server_parse_args, combined_flags)
{
    char const *argv[] = {"server", "-l", "--port", "9001"};
    Server_Options options = server_parse_args(4, argv);

    cr_assert(options.log_enabled == true);
    cr_assert(options.show_help == false);
    cr_assert_str_eq(options.port_arg, "9001");
}

Test(server_resolve_port, defaults_when_missing)
{
    Server_Options options = {.show_help = false, .log_enabled = false, .port_arg = NULL};
    cr_assert_eq(server_resolve_port(&options), SERVER_DEFAULT_PORT);
}

Test(server_resolve_port, parses_valid_value)
{
    Server_Options options = {.show_help = false, .log_enabled = false, .port_arg = "8080"};
    cr_assert_eq(server_resolve_port(&options), 8080);
}

Test(server_resolve_port, falls_back_on_garbage)
{
    Server_Options options = {.show_help = false, .log_enabled = false, .port_arg = "no-es-un-puerto"};
    cr_assert_eq(server_resolve_port(&options), SERVER_DEFAULT_PORT);
}

Test(server_resolve_port, falls_back_on_overflow)
{
    Server_Options options = {.show_help = false, .log_enabled = false, .port_arg = "99999999999999999999"};
    cr_assert_eq(server_resolve_port(&options), SERVER_DEFAULT_PORT);
}