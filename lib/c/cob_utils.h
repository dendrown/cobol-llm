/**
 * @file cob_utils.h
 * @brief Shared utilities and COBOL field size constants.
 *
 * Centralises all size constants that must remain in sync with the
 * COBOL copybooks, and declares utility functions shared across the
 * cobol-llm C modules. Any value defined here that corresponds to a
 * COBOL PIC field size must be updated in both this file and the
 * relevant copybook if the size changes.
 *
 * @project  cobol-llm
 * @licence  GNU Lesser General Public License v2.1
 *
 * @copyright Copyright (c) 2026 Dennis Drown
 */
#ifndef COB_UTILS_H
#define COB_UTILS_H

#include <stddef.h>

/* -------------------------------------------------------------------- */
/* COBOL field size constants                                           */
/* These must remain in sync with the LLM copybooks.                    */
/* -------------------------------------------------------------------- */
/**
 * @defgroup cobol_sizes COBOL Field Size Constants
 * @brief Size constants mirroring PIC field lengths in the copybooks.
 *
 * Each constant documents which copybook and field it corresponds to.
 * @{
 */

/** LLM-CONFIG.cpy : LLM-PROVIDER          PIC X(20)    */
#define SIZE_LLM_PROVIDER           20

/** LLM-CONFIG.cpy : LLM-ENDPOINT-URL      PIC X(300)   */
#define SIZE_LLM_ENDPOINT_URL      300

/** LLM-CONFIG.cpy : LLM-API-KEY           PIC X(256)   */
#define SIZE_LLM_API_KEY           256

/** LLM-CONFIG.cpy : LLM-MODEL             PIC X(128)   */
#define SIZE_LLM_MODEL             128

/** LLM-REQUEST.cpy : LLM-REQ-MSG-ROLE     PIC X(16)    */
#define SIZE_LLM_MSG_ROLE           16

/** LLM-REQUEST.cpy : LLM-REQ-MSG-CONTENT  PIC X(4096)  */
#define SIZE_LLM_MSG_CONTENT      4096

/** LLM-REQUEST.cpy : LLM-REQ-MESSAGES     OCCURS 50    */
#define SIZE_LLM_MSG_MAX            50

/** LLM-RESPONSE.cpy : LLM-RSP-CONTENT     PIC X(32768) */
#define SIZE_LLM_RSP_CONTENT      32768

/** LLM-RESPONSE.cpy : LLM-RSP-FINISH-REASON PIC X(32)  */
#define SIZE_LLM_FINISH_REASON      32

/** LLM-RESPONSE.cpy : LLM-RSP-MODEL       PIC X(128)   */
#define SIZE_LLM_RSP_MODEL         128

/** LLM-STATUS.cpy : LLM-STAT-MESSAGE      PIC X(256)   */
#define SIZE_LLM_STAT_MESSAGE      256

/** WS-JSON-REQUEST in LLM-CHAT.cob        PIC X(8192)  */
#define SIZE_LLM_JSON_REQUEST     8192

/** @} */

/* -------------------------------------------------------------------- */
/* Utility function declarations                                        */
/* -------------------------------------------------------------------- */
/**
 * @brief Converts a space-padded COBOL string to a null-terminated
 *        C string by trimming trailing spaces.
 *
 * Handles all edge cases including NULL pointers, zero-length
 * buffers, and fully space-padded source strings. The destination
 * buffer is always null-terminated if @p dst and @p dst_len are
 * valid.
 *
 * @param[out] dst      Destination buffer.
 * @param[in]  src      Space-padded COBOL source string.
 * @param[in]  src_len  Length of @p src in bytes.
 * @param[in]  dst_len  Total size of @p dst in bytes including
 *                      space for the null terminator.
 */
void cobol_to_c_string(
    char       *dst,
    const char *src,
    size_t      src_len,
    size_t      dst_len);

#endif /* COB_UTILS_H */
