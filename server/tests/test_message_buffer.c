#include <criterion/criterion.h>
#include <string.h>

#include "../src/message_buffer.h"

Test(message_buffer_init, resets_length_to_zero)
{
    Message_Buffer buf;
    buf.length = 999;
    message_buffer_init(&buf);
    cr_assert_eq(buf.length, 0);
}

Test(message_buffer_append, copies_bytes_at_correct_offset)
{
    Message_Buffer buf;
    message_buffer_init(&buf);
 
    message_buffer_append(&buf, "abc", 3);
    cr_assert_eq(buf.length, 3);
    cr_assert(memcmp(buf.data, "abc", 3) == 0);
}

Test(message_buffer_append, second_append_does_not_overwrite_first)
{
    Message_Buffer buf;
    message_buffer_init(&buf);
 
    message_buffer_append(&buf, "abc", 3);
    message_buffer_append(&buf, "def", 3);
 
    cr_assert_eq(buf.length, 6);
    cr_assert(memcmp(buf.data, "abcdef", 6) == 0);
}

Test(message_buffer_append, zero_length_chunk_is_a_no_op)
{
    Message_Buffer buf;
    message_buffer_init(&buf);
    message_buffer_append(&buf, "abc", 3);
 
    int result = message_buffer_append(&buf, "", 0);
 
    cr_assert_eq(result, 0);
    cr_assert_eq(buf.length, 3);
}

Test(message_buffer_append, succeeds_when_filling_exactly_to_the_limit)
{
    Message_Buffer buf;
    message_buffer_init(&buf);
 
    char chunk[SERVER_BUFFER_SIZE];
    memset(chunk, 'a', sizeof(chunk));
 
    int result = message_buffer_append(&buf, chunk, sizeof(chunk));
 
    cr_assert_eq(result, 0);
    cr_assert_eq(buf.length, SERVER_BUFFER_SIZE);
}

Test(message_buffer_append, fails_by_a_single_byte_over_the_limit)
{
    Message_Buffer buf;
    message_buffer_init(&buf);
 
    char chunk[SERVER_BUFFER_SIZE];
    memset(chunk, 'a', sizeof(chunk));
    message_buffer_append(&buf, chunk, sizeof(chunk));
 
    int result = message_buffer_append(&buf, "x", 1);
 
    cr_assert_eq(result, -1);
}

Test(message_buffer_append, failed_append_does_not_corrupt_existing_length)
{
    Message_Buffer buf;
    message_buffer_init(&buf);
    message_buffer_append(&buf, "abc", 3);
 
    char huge[SERVER_BUFFER_SIZE];
    memset(huge, 'x', sizeof(huge));
    int result = message_buffer_append(&buf, huge, sizeof(huge));
 
    cr_assert_eq(result, -1);
    
    cr_assert_eq(buf.length, 3);
    cr_assert(memcmp(buf.data, "abc", 3) == 0);
}

Test(message_buffer_extract, returns_false_when_no_newline_present)
{
    Message_Buffer buf;
    message_buffer_init(&buf);
    message_buffer_append(&buf, "sin final todavia", strlen("sin final todavia"));
 
    char out[64];
    size_t out_length;
    bool extracted = message_buffer_extract(&buf, out, sizeof(out), &out_length);
 
    cr_assert_not(extracted);
}

Test(message_buffer_extract, leaves_buffer_untouched_when_nothing_to_extract)
{
    Message_Buffer buf;
    message_buffer_init(&buf);
    message_buffer_append(&buf, "incompleto", strlen("incompleto"));
    size_t length_before = buf.length;
 
    char out[64];
    size_t out_length;
    message_buffer_extract(&buf, out, sizeof(out), &out_length);
 
    cr_assert_eq(buf.length, length_before);
    cr_assert(memcmp(buf.data, "incompleto", length_before) == 0);
}

Test(message_buffer_extract, extracts_message_at_start_of_buffer)
{
    Message_Buffer buf;
    message_buffer_init(&buf);
    message_buffer_append(&buf, "hola\n", strlen("hola\n"));
 
    char out[64];
    size_t out_length;
    cr_assert(message_buffer_extract(&buf, out, sizeof(out), &out_length));
    cr_assert_str_eq(out, "hola");
    cr_assert_eq(out_length, 4);
}

Test(message_buffer_extract, handles_empty_message_leading_newline)
{
    Message_Buffer buf;
    message_buffer_init(&buf);
    message_buffer_append(&buf, "\nhola\n", strlen("\nhola\n"));
 
    char out[64];
    size_t out_length;
 
    cr_assert(message_buffer_extract(&buf, out, sizeof(out), &out_length));
    cr_assert_eq(out_length, 0);
    cr_assert_str_eq(out, "");
 
    cr_assert(message_buffer_extract(&buf, out, sizeof(out), &out_length));
    cr_assert_str_eq(out, "hola");
}

Test(message_buffer_extract, leaves_trailing_partial_data_after_last_newline)
{
    Message_Buffer buf;
    message_buffer_init(&buf);
    message_buffer_append(&buf, "uno\ndos\ntre", strlen("uno\ndos\ntre"));
 
    char out[64];
    size_t out_length;
 
    cr_assert(message_buffer_extract(&buf, out, sizeof(out), &out_length));
    cr_assert_str_eq(out, "uno");
    cr_assert(message_buffer_extract(&buf, out, sizeof(out), &out_length));
    cr_assert_str_eq(out, "dos");
    
    cr_assert_not(message_buffer_extract(&buf, out, sizeof(out), &out_length));
 
    cr_assert_eq(buf.length, 3);
    cr_assert(memcmp(buf.data, "tre", 3) == 0);
}

Test(message_buffer_extract, resets_length_to_zero_after_consuming_everything)
{
    Message_Buffer buf;
    message_buffer_init(&buf);
    message_buffer_append(&buf, "unico\n", strlen("unico\n"));
 
    char out[64];
    size_t out_length;
    message_buffer_extract(&buf, out, sizeof(out), &out_length);
 
    cr_assert_eq(buf.length, 0);
}

Test(message_buffer_extract, truncates_when_out_capacity_is_too_small)
{
    Message_Buffer buf;
    message_buffer_init(&buf);
    message_buffer_append(&buf, "mensaje-largo\n", strlen("mensaje-largo\n"));
 
    char out[6]; /* "mensa" + '\0' */
    size_t out_length;
    cr_assert(message_buffer_extract(&buf, out, sizeof(out), &out_length));
 
    cr_assert_str_eq(out, "mensa");
    cr_assert_eq(out_length, strlen("mensaje-largo"));
}

Test(message_buffer_extract, advances_buffer_correctly_even_when_truncated)
{
    Message_Buffer buf;
    message_buffer_init(&buf);
    message_buffer_append(&buf, "mensaje-largo\nsiguiente\n", strlen("mensaje-largo\nsiguiente\n"));
 
    char out[6];
    size_t out_length;
    message_buffer_extract(&buf, out, sizeof(out), &out_length);
 
    cr_assert(message_buffer_extract(&buf, out, sizeof(out), &out_length));
    cr_assert_str_eq(out, "sigui");
}

Test(message_buffer_extract, out_capacity_of_one_writes_only_terminator)
{
    Message_Buffer buf;
    message_buffer_init(&buf);
    message_buffer_append(&buf, "algo\n", strlen("algo\n"));
 
    char out[1];
    size_t out_length;
    cr_assert(message_buffer_extract(&buf, out, sizeof(out), &out_length));
    cr_assert_eq(out[0], '\0');
    cr_assert_eq(out_length, strlen("algo"));
}

Test(message_buffer_extract, out_capacity_zero_returns_false_safely)
{
    Message_Buffer buf;
    message_buffer_init(&buf);
    message_buffer_append(&buf, "algo\n", strlen("algo\n"));
 
    char out[1];
    size_t out_length;
    cr_assert_not(message_buffer_extract(&buf, out, 0, &out_length));
}

Test(message_buffer_extract, out_length_pointer_is_optional)
{
    Message_Buffer buf;
    message_buffer_init(&buf);
    message_buffer_append(&buf, "hola\n", strlen("hola\n"));
 
    char out[64];
    cr_assert(message_buffer_extract(&buf, out, sizeof(out), NULL));
    cr_assert_str_eq(out, "hola");
}

Test(message_is_valid, accepts_plain_text)
{
    char text[] = "hola pedrito";
    cr_assert(message_is_valid(text, strlen(text)));
}

Test(message_is_valid, accepts_empty_text)
{
    cr_assert(message_is_valid("", 0));
}
 
Test(message_is_valid, rejects_embedded_null_byte)
{
    char text[] = {'h', 'o', 'l', 'a', '\0', 'x'};
    cr_assert_not(message_is_valid(text, sizeof(text)));
}
 
Test(message_is_valid, rejects_embedded_newline) 
{
    char text[] = "hola\nmundo";
    cr_assert_not(message_is_valid(text, strlen(text)));
}
 
Test(message_is_valid, null_byte_at_the_very_end_is_still_rejected) 
{
    char text[] = {'h', 'o', 'l', 'a', '\0'};
    cr_assert_not(message_is_valid(text, sizeof(text)));
}

Test(message_buffer_sanitize, leaves_clean_text_unchanged)
{
    char text[32] = "hola pedrito";
    size_t length = strlen(text);
 
    message_buffer_sanitize(text, &length);
 
    cr_assert_str_eq(text, "hola pedrito");
    cr_assert_eq(length, strlen("hola pedrito"));
}

Test(message_buffer_sanitize, removes_single_embedded_null_byte)
{
    char text[32];
    memcpy(text, "ho\0la", 5);
    size_t length = 5;
 
    message_buffer_sanitize(text, &length);
 
    cr_assert_eq(length, 4);
    cr_assert_str_eq(text, "hola");
}

Test(message_buffer_sanitize, removes_multiple_scattered_null_bytes)
{
    char text[32];
    memcpy(text, "a\0b\bc\rd", 7);
    size_t length = 7;
 
    message_buffer_sanitize(text, &length);
 
    cr_assert_eq(length, 4);
    cr_assert_str_eq(text, "abcd");
}

Test(message_buffer_sanitize, text_entirely_of_null_bytes_becomes_empty) 
{
    char text[4] = {'\0', '\t', '\r', 'x'};
    size_t length = 3;
 
    message_buffer_sanitize(text, &length);
 
    cr_assert_eq(length, 0);
    cr_assert_str_eq(text, "");
}

Test(message_buffer_sanitize, result_is_always_valid_afterwards) 
{
    char text[32];
    memcpy(text, "a\ab\bc", 5);
    size_t length = 5;
 
    message_buffer_sanitize(text, &length);

    cr_assert(message_is_valid(text, length));
}

Test(message_buffer, extracted_message_with_null_byte_gets_cleaned_not_dropped)
{
    Message_Buffer buf;
    message_buffer_init(&buf);
 
    char raw[] = "ho\0la\n";
    message_buffer_append(&buf, raw, sizeof(raw) - 1);
 
    char out[64];
    size_t out_length;
    cr_assert(message_buffer_extract(&buf, out, sizeof(out), &out_length));
    cr_assert_eq(out_length, 5);
 
    message_buffer_sanitize(out, &out_length);
 
    cr_assert_eq(out_length, 4);
    cr_assert_str_eq(out, "hola");
}

Test(message_buffer, message_split_across_two_appends)
{
    Message_Buffer buf;
    message_buffer_init(&buf);
    message_buffer_append(&buf, "hola pedrito\n adios ", strlen("hola pedrito\n adios "));

    char out[128];
    size_t out_length;

    cr_assert(message_buffer_extract(&buf, out, sizeof(out), &out_length));
    cr_assert_str_eq(out, "hola pedrito");

    cr_assert_not(message_buffer_extract(&buf, out, sizeof(out), &out_length));

    message_buffer_append(&buf, "pedrito\n", strlen("pedrito\n"));
    cr_assert(message_buffer_extract(&buf, out, sizeof(out), &out_length));
    cr_assert_str_eq(out, " adios pedrito");
}

Test(message_buffer, multiple_messages_in_one_append)
{
    Message_Buffer buf;
    message_buffer_init(&buf);
    message_buffer_append(&buf, "uno\ndos\ntres\n", strlen("uno\ndos\ntres\n"));
 
    char out[128];
    size_t out_length;
 
    cr_assert(message_buffer_extract(&buf, out, sizeof(out), &out_length));
    cr_assert_str_eq(out, "uno");
    cr_assert(message_buffer_extract(&buf, out, sizeof(out), &out_length));
    cr_assert_str_eq(out, "dos");
    cr_assert(message_buffer_extract(&buf, out, sizeof(out), &out_length));
    cr_assert_str_eq(out, "tres");
    cr_assert_not(message_buffer_extract(&buf, out, sizeof(out), &out_length));
    cr_assert_eq(buf.length, 0);
}