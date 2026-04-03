/**
 * @file cob_utils.c
 * @brief Shared utility function implementations.
 *
 * @project  cobol-llm
 * @licence  GNU Lesser General Public License v2.1
 *
 * @copyright Copyright (c) 2026 Dennis Drown
 */
#include "cob_utils.h"
#include <string.h>

void cobol_to_c_string(
    char       *dst,
    const char *src,
    size_t      src_len,
    size_t      dst_len)
{
    /* Guard: valid destination buffer */
    if (!dst || !dst_len) {
        return;
    }

    /* Guard: valid source */
    if (!src || !src_len) {
        dst[0] = '\0';
        return;
    }

    size_t max_len = dst_len - 1;
    size_t cpy_len = src_len;

    /* Trim trailing spaces */
    while (cpy_len > 0 && src[cpy_len - 1] == ' ') {
        cpy_len--;
    }

    /* Guard against destination buffer overrun */
    if (cpy_len > max_len) {
        cpy_len = max_len;
    }

    memcpy(dst, src, cpy_len);
    dst[cpy_len] = '\0';
}
