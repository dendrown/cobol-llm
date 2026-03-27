/**
 * @file cob_json.c
 * @brief cJSON-based JSON builder and parser implementation.
 *
 * Implements request serialization and response deserialization for
 * Ollama and Claude API formats. All memory allocated by cJSON is
 * freed before returning. Caller owns all input and output buffers.
 *
 * This module is intended as a temporary layer until GNUCobol's
 * JSON GENERATE and JSON PARSE statements are fully implemented,
 * at which point this module can be removed and the COBOL layer
 * refactored to use native JSON handling.
 *
 * @project  cobol-llm
 * @licence  GNU Lesser General Public License v2.1
 *
 * @copyright Copyright (c) 2026 Dennis Drown
 */
#include "cob_json.h"
#include <cjson/cJSON.h>
#include <string.h>
#include <stdio.h>

/* -------------------------------------------------------------------- */
/* Internal helpers                                                     */
/* -------------------------------------------------------------------- */
/**
 * @brief Copies a string into a fixed-length buffer with truncation
 *        protection.
 *
 * Always null-terminates @p dest. If @p src is longer than
 * @p dest_len - 1 bytes the string is truncated and
 * COB_JSON_ERR_TRUNCATED is returned, but the copy still succeeds.
 *
 * @param[out] dest      Destination buffer.
 * @param[in]  src       Source string.
 * @param[in]  dest_len  Total size of @p dest in bytes.
 *
 * @return COB_JSON_OK or COB_JSON_ERR_TRUNCATED.
 */
static int safe_copy(char *dest, const char *src, size_t dest_len)
{
    size_t src_len = strlen(src);
    size_t copy_len = src_len < dest_len - 1 ? src_len : dest_len - 1;
    memcpy(dest, src, copy_len);
    dest[copy_len] = '\0';
    return src_len >= dest_len ? COB_JSON_ERR_TRUNCATED : COB_JSON_OK;
}


/**
 * @brief Builds the JSON messages array common to both providers.
 *
 * Creates a cJSON array of message objects, each with "role" and
 * "content" string fields. The caller is responsible for attaching
 * the returned array to a parent object and eventually calling
 * cJSON_Delete() on the root object.
 *
 * @param[in]  messages   Array of CobLlmMessage structs.
 * @param[in]  msg_count  Number of messages in @p messages.
 *
 * @return Pointer to a cJSON array on success, NULL on failure.
 */
static cJSON *build_messages_array(
    const CobLlmMessage *messages,
    int32_t              msg_count)
{
    cJSON *array = cJSON_CreateArray();
    if (array == NULL) {
        return NULL;
    }

    int32_t i;
    for (i = 0; i < msg_count; i++) {
        cJSON *msg = cJSON_CreateObject();
        if (msg == NULL) {
            cJSON_Delete(array);
            return NULL;
        }
        if (cJSON_AddStringToObject(msg, "role",
                messages[i].role) == NULL ||
            cJSON_AddStringToObject(msg, "content",
                messages[i].content) == NULL) {
            cJSON_Delete(msg);
            cJSON_Delete(array);
            return NULL;
        }
        cJSON_AddItemToArray(array, msg);
    }

    return array;
}


/**
 * @brief Serializes a cJSON object to the output buffer.
 *
 * Renders @p root to a JSON string and copies it into @p json_out.
 * Always frees @p root before returning regardless of outcome.
 *
 * @param[in]  root         cJSON object to serialize.
 * @param[out] json_out     Output buffer.
 * @param[in]  json_out_max Size of @p json_out in bytes.
 * @param[out] json_out_len Bytes written on success.
 * @param[out] err_msg      Error message buffer.
 *
 * @return COB_JSON_OK on success, or a COB_JSON_ERR_* code.
 */
static int serialize_to_buffer(
    cJSON   *root,
    char    *json_out,
    int32_t  json_out_max,
    int32_t *json_out_len,
    char    *err_msg)
{
    char *rendered = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (rendered == NULL) {
        snprintf(err_msg, COB_JSON_ERR_MSG_LEN,
                 "cJSON_PrintUnformatted failed");
        return COB_JSON_ERR_BUILD;
    }

    int rc = safe_copy(json_out, rendered, (size_t)json_out_max);
    *json_out_len = (int32_t)strlen(json_out);
    cJSON_free(rendered);

    if (rc != COB_JSON_OK) {
        snprintf(err_msg, COB_JSON_ERR_MSG_LEN,
                 "JSON request truncated at %d bytes", *json_out_len);
        return COB_JSON_ERR_TRUNCATED;
    }

    return COB_JSON_OK;
}

/* -------------------------------------------------------------------- */
/* Request builders                                                     */
/* -------------------------------------------------------------------- */
int cob_json_build_ollama_request(
    const char          *model,
    const CobLlmMessage *messages,
    int32_t              msg_count,
    double               temperature,
    int32_t              stream,
    char                *json_out,
    int32_t             *json_out_len,
    char                *err_msg)
{
    err_msg[0]    = '\0';
    *json_out_len = 0;

    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        snprintf(err_msg, COB_JSON_ERR_MSG_LEN,
                 "cJSON_CreateObject failed");
        return COB_JSON_ERR_BUILD;
    }

    /* model */
    if (cJSON_AddStringToObject(root, "model", model) == NULL) {
        goto build_error;
    }

    /* messages */
    cJSON *msg_array = build_messages_array(messages, msg_count);
    if (msg_array == NULL) {
        goto build_error;
    }
    cJSON_AddItemToObject(root, "messages", msg_array);

    /* stream */
    if (cJSON_AddBoolToObject(root, "stream", stream) == NULL) {
        goto build_error;
    }

    /* options.temperature */
    cJSON *options = cJSON_CreateObject();
    if (options == NULL) {
        goto build_error;
    }
    if (cJSON_AddNumberToObject(options, "temperature",
                                temperature) == NULL) {
        cJSON_Delete(options);
        goto build_error;
    }
    cJSON_AddItemToObject(root, "options", options);

    return serialize_to_buffer(root, json_out, COB_JSON_REQUEST_LEN,
                               json_out_len, err_msg);

build_error:
    snprintf(err_msg, COB_JSON_ERR_MSG_LEN,
             "Failed to build Ollama JSON request");
    cJSON_Delete(root);
    return COB_JSON_ERR_BUILD;
}

int cob_json_build_claude_request(
    const char          *model,
    const CobLlmMessage *messages,
    int32_t              msg_count,
    double               temperature,
    int32_t              max_tokens,
    char                *json_out,
    int32_t             *json_out_len,
    char                *err_msg)
{
    err_msg[0]    = '\0';
    *json_out_len = 0;

    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        snprintf(err_msg, COB_JSON_ERR_MSG_LEN,
                 "cJSON_CreateObject failed");
        return COB_JSON_ERR_BUILD;
    }

    /* model */
    if (cJSON_AddStringToObject(root, "model", model) == NULL) {
        goto build_error;
    }

    /* max_tokens */
    if (cJSON_AddNumberToObject(root, "max_tokens",
                                (double)max_tokens) == NULL) {
        goto build_error;
    }

    /* temperature */
    if (cJSON_AddNumberToObject(root, "temperature",
                                temperature) == NULL) {
        goto build_error;
    }

    /* messages */
    cJSON *msg_array = build_messages_array(messages, msg_count);
    if (msg_array == NULL) {
        goto build_error;
    }
    cJSON_AddItemToObject(root, "messages", msg_array);

    return serialize_to_buffer(root, json_out, COB_JSON_REQUEST_LEN,
                               json_out_len, err_msg);

build_error:
    snprintf(err_msg, COB_JSON_ERR_MSG_LEN,
             "Failed to build Claude JSON request");
    cJSON_Delete(root);
    return COB_JSON_ERR_BUILD;
}

/* -------------------------------------------------------------------- */
/* Response parsers                                                     */
/* -------------------------------------------------------------------- */
int cob_json_parse_ollama_response(
    const char *json,
    char       *content,
    char       *model,
    char       *finish_reason,
    int32_t    *tokens_in,
    int32_t    *tokens_out,
    char       *err_msg)
{
    err_msg[0] = '\0';
    content[0] = '\0';
    model[0]   = '\0';
    finish_reason[0] = '\0';
    *tokens_in  = 0;
    *tokens_out = 0;

    cJSON *root = cJSON_Parse(json);
    if (root == NULL) {
        const char *ep = cJSON_GetErrorPtr();
        snprintf(err_msg, COB_JSON_ERR_MSG_LEN,
                 "JSON parse failed near: %.64s",
                 ep != NULL ? ep : "unknown");
        return COB_JSON_ERR_PARSE;
    }

    int rc = COB_JSON_OK;

    /* message.content */
    cJSON *message = cJSON_GetObjectItemCaseSensitive(root, "message");
    if (cJSON_IsObject(message)) {
        cJSON *text = cJSON_GetObjectItemCaseSensitive(message,
                                                       "content");
        if (cJSON_IsString(text) && text->valuestring != NULL) {
            if (safe_copy(content, text->valuestring,
                          COB_JSON_CONTENT_LEN) != COB_JSON_OK) {
                rc = COB_JSON_ERR_TRUNCATED;
                snprintf(err_msg, COB_JSON_ERR_MSG_LEN,
                         "Ollama response content truncated");
            }
        }
    } else {
        snprintf(err_msg, COB_JSON_ERR_MSG_LEN,
                 "Ollama response missing 'message' object");
        cJSON_Delete(root);
        return COB_JSON_ERR_MISSING;
    }

    /* model */
    cJSON *model_item = cJSON_GetObjectItemCaseSensitive(root, "model");
    if (cJSON_IsString(model_item) && model_item->valuestring != NULL) {
        safe_copy(model, model_item->valuestring, COB_JSON_MODEL_LEN);
    }

    /* done_reason */
    cJSON *done_reason = cJSON_GetObjectItemCaseSensitive(root,
                                                          "done_reason");
    if (cJSON_IsString(done_reason) &&
            done_reason->valuestring != NULL) {
        safe_copy(finish_reason, done_reason->valuestring,
                  COB_JSON_FINISH_REASON_LEN);
    }

    /* prompt_eval_count / eval_count */
    cJSON *prompt_tokens = cJSON_GetObjectItemCaseSensitive(root,
                               "prompt_eval_count");
    if (cJSON_IsNumber(prompt_tokens)) {
        *tokens_in = (int32_t)prompt_tokens->valuedouble;
    }

    cJSON *completion_tokens = cJSON_GetObjectItemCaseSensitive(root,
                                   "eval_count");
    if (cJSON_IsNumber(completion_tokens)) {
        *tokens_out = (int32_t)completion_tokens->valuedouble;
    }

    cJSON_Delete(root);
    return rc;
}

int cob_json_parse_claude_response(
    const char *json,
    char       *content,
    char       *model,
    char       *finish_reason,
    int32_t    *tokens_in,
    int32_t    *tokens_out,
    char       *err_msg)
{
    err_msg[0] = '\0';
    content[0] = '\0';
    model[0]   = '\0';
    finish_reason[0] = '\0';
    *tokens_in  = 0;
    *tokens_out = 0;

    cJSON *root = cJSON_Parse(json);
    if (root == NULL) {
        const char *ep = cJSON_GetErrorPtr();
        snprintf(err_msg, COB_JSON_ERR_MSG_LEN,
                 "JSON parse failed near: %.64s",
                 ep != NULL ? ep : "unknown");
        return COB_JSON_ERR_PARSE;
    }

    int rc = COB_JSON_OK;

    /* content[0].text */
    cJSON *content_array = cJSON_GetObjectItemCaseSensitive(root,
                                                            "content");
    if (cJSON_IsArray(content_array)) {
        cJSON *first = cJSON_GetArrayItem(content_array, 0);
        if (cJSON_IsObject(first)) {
            cJSON *text = cJSON_GetObjectItemCaseSensitive(first,
                                                           "text");
            if (cJSON_IsString(text) && text->valuestring != NULL) {
                if (safe_copy(content, text->valuestring,
                              COB_JSON_CONTENT_LEN) != COB_JSON_OK) {
                    rc = COB_JSON_ERR_TRUNCATED;
                    snprintf(err_msg, COB_JSON_ERR_MSG_LEN,
                             "Claude response content truncated");
                }
            }
        }
    } else {
        snprintf(err_msg, COB_JSON_ERR_MSG_LEN,
                 "Claude response missing 'content' array");
        cJSON_Delete(root);
        return COB_JSON_ERR_MISSING;
    }

    /* model */
    cJSON *model_item = cJSON_GetObjectItemCaseSensitive(root, "model");
    if (cJSON_IsString(model_item) && model_item->valuestring != NULL) {
        safe_copy(model, model_item->valuestring, COB_JSON_MODEL_LEN);
    }

    /* stop_reason */
    cJSON *stop_reason = cJSON_GetObjectItemCaseSensitive(root,
                                                          "stop_reason");
    if (cJSON_IsString(stop_reason) &&
            stop_reason->valuestring != NULL) {
        safe_copy(finish_reason, stop_reason->valuestring,
                  COB_JSON_FINISH_REASON_LEN);
    }

    /* usage.input_tokens / usage.output_tokens */
    cJSON *usage = cJSON_GetObjectItemCaseSensitive(root, "usage");
    if (cJSON_IsObject(usage)) {
        cJSON *input = cJSON_GetObjectItemCaseSensitive(usage,
                                                        "input_tokens");
        if (cJSON_IsNumber(input)) {
            *tokens_in = (int32_t)input->valuedouble;
        }
        cJSON *output = cJSON_GetObjectItemCaseSensitive(usage,
                                                         "output_tokens");
        if (cJSON_IsNumber(output)) {
            *tokens_out = (int32_t)output->valuedouble;
        }
    }

    cJSON_Delete(root);
    return rc;
}

