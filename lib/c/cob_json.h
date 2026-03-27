/**
 * @file cob_json.h
 * @brief cJSON-based JSON builder and parser for COBOL LLM API access.
 *
 * Provides functions to serialize COBOL data structures into JSON
 * request bodies and deserialize JSON response bodies back into
 * fixed-length COBOL fields. Intended as a temporary layer until
 * GNUCobol's JSON GENERATE and JSON PARSE statements are fully
 * implemented, at which point this module can be removed and the
 * COBOL layer refactored to use native JSON handling.
 *
 * All string buffers are caller-allocated and fixed-length to match
 * the corresponding COBOL PIC fields defined in the LLM copybooks.
 *
 * @project  cobol-llm
 * @licence  GNU Lesser General Public License v2.1
 *
 * @copyright Copyright (c) 2026 Dennis Drown
 */
#ifndef COB_JSON_H
#define COB_JSON_H

#include <stdint.h>

/* -------------------------------------------------------------------- */
/* Buffer size constants                                                */
/* These must remain in sync with the COBOL copybooks.                  */
/* -------------------------------------------------------------------- */

/** Maximum number of messages in a request. Matches LLM-REQUEST.cpy    */
#define COB_JSON_MAX_MESSAGES     50

/** Message role field size. Matches LLM-REQ-MSG-ROLE PIC X(16)         */
#define COB_JSON_MSG_ROLE_LEN     16

/** Message content field size. Matches LLM-REQ-MSG-CONTENT PIC X(4096) */
#define COB_JSON_MSG_CONTENT_LEN  4096

/** Model field size. Matches LLM-MODEL PIC X(128)                      */
#define COB_JSON_MODEL_LEN        128

/** JSON request output buffer. Matches WS-JSON-REQUEST PIC X(8192)     */
#define COB_JSON_REQUEST_LEN      8192

/** Response content size. Matches LLM-RSP-CONTENT PIC X(32768)         */
#define COB_JSON_CONTENT_LEN      32768

/** Finish reason size. Matches LLM-RSP-FINISH-REASON PIC X(32)         */
#define COB_JSON_FINISH_REASON_LEN 32

/** Error message size. Matches LLM-STAT-MESSAGE PIC X(256)             */
#define COB_JSON_ERR_MSG_LEN      256

/* -------------------------------------------------------------------- */
/* Return codes                                                         */
/* -------------------------------------------------------------------- */
/**
 * @defgroup cob_json_return_codes Return Codes
 * @brief Return codes returned by cob_json functions.
 * @{
 */
#define COB_JSON_OK               0  /**< Success.                      */
#define COB_JSON_ERR_BUILD        1  /**< JSON build failed.            */
#define COB_JSON_ERR_PARSE        2  /**< JSON parse failed.            */
#define COB_JSON_ERR_TRUNCATED    3  /**< Output buffer too small.      */
#define COB_JSON_ERR_MISSING      4  /**< Expected field not found.     */
/** @} */

/* -------------------------------------------------------------------- */
/* Message structure                                                    */
/* -------------------------------------------------------------------- */
/**
 * @brief A single chat message, mirroring one occurrence of the
 *        LLM-REQ-MESSAGES table entry in LLM-REQUEST.cpy.
 *
 * Fields are fixed-length and null-terminated by the COBOL layer
 * before being passed to C. Role should be one of "user",
 * "assistant", or "system".
 */
typedef struct {
    char role   [COB_JSON_MSG_ROLE_LEN    + 1]; /**< Message role.      */
    char content[COB_JSON_MSG_CONTENT_LEN + 1]; /**< Message content.   */
} CobLlmMessage;

/* -------------------------------------------------------------------- */
/* Request builders                                                     */
/* -------------------------------------------------------------------- */
/**
 * @brief Builds an Ollama-compatible JSON request body.
 *
 * Serializes the provided model, messages, temperature, and stream
 * flag into a JSON string suitable for POST to the Ollama
 * /api/chat endpoint.
 *
 * @param[in]  model        Null-terminated model name.
 * @param[in]  messages     Array of CobLlmMessage structs.
 * @param[in]  msg_count    Number of messages in @p messages.
 * @param[in]  temperature  Sampling temperature (e.g. 0.7).
 * @param[in]  stream       0 for non-streaming, 1 for streaming.
 * @param[out] json_out     Caller-allocated output buffer for the
 *                          JSON string. Must be at least
 *                          COB_JSON_REQUEST_LEN bytes.
 * @param[out] json_out_len Number of bytes written to @p json_out.
 * @param[out] err_msg      Caller-allocated error message buffer.
 *                          Must be at least COB_JSON_ERR_MSG_LEN
 *                          bytes.
 *
 * @return COB_JSON_OK on success, or a COB_JSON_ERR_* code on
 *         failure.
 */
int cob_json_build_ollama_request(
    const char        *model,
    const CobLlmMessage *messages,
    int32_t            msg_count,
    double             temperature,
    int32_t            stream,
    char              *json_out,
    int32_t           *json_out_len,
    char              *err_msg
);

/**
 * @brief Builds a Claude-compatible JSON request body.
 *
 * Serializes the provided model, messages, temperature, and
 * max_tokens into a JSON string suitable for POST to the Anthropic
 * /v1/messages endpoint.
 *
 * @param[in]  model        Null-terminated model name.
 * @param[in]  messages     Array of CobLlmMessage structs.
 * @param[in]  msg_count    Number of messages in @p messages.
 * @param[in]  temperature  Sampling temperature (e.g. 0.7).
 * @param[in]  max_tokens   Maximum tokens in the response.
 * @param[out] json_out     Caller-allocated output buffer for the
 *                          JSON string. Must be at least
 *                          COB_JSON_REQUEST_LEN bytes.
 * @param[out] json_out_len Number of bytes written to @p json_out.
 * @param[out] err_msg      Caller-allocated error message buffer.
 *                          Must be at least COB_JSON_ERR_MSG_LEN
 *                          bytes.
 *
 * @return COB_JSON_OK on success, or a COB_JSON_ERR_* code on
 *         failure.
 */
int cob_json_build_claude_request(
    const char          *model,
    const CobLlmMessage *messages,
    int32_t              msg_count,
    double               temperature,
    int32_t              max_tokens,
    char                *json_out,
    int32_t             *json_out_len,
    char                *err_msg
);

/* -------------------------------------------------------------------- */
/* Response parsers                                                     */
/* -------------------------------------------------------------------- */
/**
 * @brief Parses an Ollama JSON response body.
 *
 * Extracts content, model, finish reason, and token counts from
 * an Ollama /api/chat response. All output buffers are
 * caller-allocated and fixed-length to match the corresponding
 * COBOL fields in LLM-RESPONSE.cpy.
 *
 * @param[in]  json           Null-terminated JSON response string.
 * @param[out] content        Extracted response text. Must be at
 *                            least COB_JSON_CONTENT_LEN bytes.
 * @param[out] model          Extracted model name. Must be at least
 *                            COB_JSON_MODEL_LEN bytes.
 * @param[out] finish_reason  Extracted finish reason. Must be at
 *                            least COB_JSON_FINISH_REASON_LEN bytes.
 * @param[out] tokens_in      Prompt token count.
 * @param[out] tokens_out     Completion token count.
 * @param[out] err_msg        Caller-allocated error message buffer.
 *                            Must be at least COB_JSON_ERR_MSG_LEN
 *                            bytes.
 *
 * @return COB_JSON_OK on success, or a COB_JSON_ERR_* code on
 *         failure.
 */
int cob_json_parse_ollama_response(
    const char *json,
    char       *content,
    char       *model,
    char       *finish_reason,
    int32_t    *tokens_in,
    int32_t    *tokens_out,
    char       *err_msg
);

/**
 * @brief Parses a Claude JSON response body.
 *
 * Extracts content, model, finish reason, and token counts from
 * an Anthropic /v1/messages response. All output buffers are
 * caller-allocated and fixed-length to match the corresponding
 * COBOL fields in LLM-RESPONSE.cpy.
 *
 * @param[in]  json           Null-terminated JSON response string.
 * @param[out] content        Extracted response text. Must be at
 *                            least COB_JSON_CONTENT_LEN bytes.
 * @param[out] model          Extracted model name. Must be at least
 *                            COB_JSON_MODEL_LEN bytes.
 * @param[out] finish_reason  Extracted finish reason. Must be at
 *                            least COB_JSON_FINISH_REASON_LEN bytes.
 * @param[out] tokens_in      Prompt token count.
 * @param[out] tokens_out     Completion token count.
 * @param[out] err_msg        Caller-allocated error message buffer.
 *                            Must be at least COB_JSON_ERR_MSG_LEN
 *                            bytes.
 *
 * @return COB_JSON_OK on success, or a COB_JSON_ERR_* code on
 *         failure.
 */
int cob_json_parse_claude_response(
    const char *json,
    char       *content,
    char       *model,
    char       *finish_reason,
    int32_t    *tokens_in,
    int32_t    *tokens_out,
    char       *err_msg
);

#endif /* COB_JSON_H */
