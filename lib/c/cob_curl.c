/**
 * @file cob_curl.c
 * @brief libcurl HTTP shim implementation for COBOL-LLM API access.
 *
 * Implements the three-function lifecycle interface declared in
 * cob_curl.h. Manages a single persistent curl session handle
 * for connection reuse across requests. All memory for request
 * and response data is owned by the calling COBOL layer.
 *
 * @project  cobol-llm
 * @licence  GNU Lesser General Public License v2.1
 *
 * @copyright Copyright (c) 2026 Dennis Drown
 */
#include "cob_curl.h"

#include <curl/curl.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/* -------------------------------------------------------------------- */
/* COBOL-LLM/cob_curl alignment: C must match COBOL                     */
/* -------------------------------------------------------------------- */
#define SIZE_LLM_API_KEY        256     /**< Matches LLM-CONFIG.cpy     */
#define SIZE_LLM_STAT_MESSAGE   256     /**< Matches LLM-STATUS.cpy     */
#define SIZE_LLM_RSP_CONTENT  32768     /**< Matches LLM-RESPONSE.cpy   */

#define SIZE_HTTP_AUTH_HEADER   (64 + SIZE_LLM_API_KEY)


/* -------------------------------------------------------------------- */
/* Internal types                                                       */
/* -------------------------------------------------------------------- */
/**
 * @brief Write callback context passed to libcurl.
 *
 * Tracks the caller-supplied response buffer, its total capacity,
 * and how many bytes have been written so far.
 */
typedef struct {
    char     *buf;       /**< Caller-allocated response buffer.     */
    int32_t   capacity;  /**< Total size of buf in bytes.           */
    int32_t   written;   /**< Bytes written into buf so far.        */
} WriteContext;


/* -------------------------------------------------------------------- */
/* Static state                                                         */
/* -------------------------------------------------------------------- */
/** @brief Persistent curl session handle. NULL until cob_curl_init(). */
static CURL *s_curl_handle = NULL;


/* -------------------------------------------------------------------- */
/* Internal functions                                                   */
/* -------------------------------------------------------------------- */
/**
 * @brief libcurl write callback. Appends received data to the buffer.
 *
 * Called by libcurl zero or more times per request as response data
 * arrives. Writes into the caller-supplied buffer tracked via
 * @p userdata. Stops writing if the buffer would be exceeded, but
 * allows the transfer to complete so the HTTP status code is still
 * captured.
 *
 * @param[in]  ptr      Pointer to received data.
 * @param[in]  size     Always 1 (libcurl convention).
 * @param[in]  nmemb    Number of bytes in this chunk.
 * @param[out] userdata Pointer to a WriteContext.
 *
 * @return Number of bytes consumed. Returning less than nmemb signals
 *         an error to libcurl and aborts the transfer.
 */
static size_t write_callback(
    char   *ptr,
    size_t  size,
    size_t  nmemb,
    void   *userdata)
{
    WriteContext *ctx       = (WriteContext *)userdata;
    size_t        incoming  = size * nmemb;
    size_t        available = (size_t)(ctx->capacity - ctx->written - 1);

    /* TODO: when the response exceeds the buffer capacity, write_callback
     * silently * truncates rather than returning an error that would abort
     * the transfer. This means we still get the HTTP status code, which is
     * useful diagnostic information.
     *
     * The caller is notified via COB_CURL_ERR_BUFFER and the message in err_msg,
     * but it's a soft error rather than a hard failure.
     */
    if (incoming > available) {
        incoming = available;
    }

    if (incoming > 0) {
        memcpy(ctx->buf + ctx->written, ptr, incoming);
        ctx->written += (int32_t)incoming;
        ctx->buf[ctx->written] = '\0';
    }

    return size * nmemb;
}


/* -------------------------------------------------------------------- */
/* Public functions                                                     */
/* -------------------------------------------------------------------- */

int cob_curl_init(void)
{
    CURLcode rc;

    rc = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (rc != CURLE_OK) {
        return COB_CURL_ERR_INIT;
    }

    s_curl_handle = curl_easy_init();
    if (s_curl_handle == NULL) {
        curl_global_cleanup();
        return COB_CURL_ERR_INIT;
    }

    return COB_CURL_OK;
}

int cob_curl_post(
    const char    *url,
    const char    *api_key,
    const char    *request_body,
    char          *response_buf,
    int32_t       *response_len,
    int32_t       *http_status,
    int32_t        timeout_secs,
    char          *err_msg)
{
    CURLcode         rc;
    struct curl_slist *headers   = NULL;
    long             http_code   = 0;
    int              result      = COB_CURL_OK;
    char             auth_header[SIZE_HTTP_AUTH_HEADER];
    WriteContext     ctx;

    /* Initialise output parameters */
    *response_len        = 0;
    *http_status         = 0;
    err_msg[0]           = '\0';
    response_buf[0]      = '\0';

    /* Set up write context */
    ctx.buf      = response_buf;
    ctx.capacity = 32768;
    ctx.written  = 0;

    /* Reset handle state from any previous request */
    curl_easy_reset(s_curl_handle);

    /* Set URL */
    curl_easy_setopt(s_curl_handle, CURLOPT_URL, url);

    /* Set POST body */
    curl_easy_setopt(s_curl_handle, CURLOPT_POSTFIELDS, request_body);

    /* Build headers */
    headers = curl_slist_append(headers,
                  "Content-Type: application/json");

    if (api_key != NULL && api_key[0] != '\0') {
        snprintf(auth_header, sizeof(auth_header),
                 "Authorization: Bearer %s", api_key);
        headers = curl_slist_append(headers, auth_header);
    }

    curl_easy_setopt(s_curl_handle, CURLOPT_HTTPHEADER, headers);

    /* Set write callback */
    curl_easy_setopt(s_curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(s_curl_handle, CURLOPT_WRITEDATA, &ctx);

    /* Set timeout */
    curl_easy_setopt(s_curl_handle, CURLOPT_TIMEOUT, (long)timeout_secs);

    /* Perform request */
    rc = curl_easy_perform(s_curl_handle);

    if (rc == CURLE_OPERATION_TIMEDOUT) {
        snprintf(err_msg, SIZE_LLM_STAT_MESSAGE, "Request timed out after %d seconds",
                 timeout_secs);
        result = COB_CURL_ERR_TIMEOUT;
        goto cleanup;
    }

    if (rc != CURLE_OK) {
        snprintf(err_msg, SIZE_LLM_STAT_MESSAGE, "curl error: %s",
                 curl_easy_strerror(rc));
        result = COB_CURL_ERR_PERFORM;
        goto cleanup;
    }

    /* Retrieve HTTP status code */
    curl_easy_getinfo(s_curl_handle, CURLINFO_RESPONSE_CODE,
                      &http_code);
    *http_status  = (int32_t)http_code;
    *response_len = ctx.written;

    /* Flag truncation but do not treat as a hard error */
    if (ctx.written >= ctx.capacity - 1) {
        snprintf(err_msg, SIZE_LLM_STAT_MESSAGE,
                 "Response truncated at %d bytes", ctx.written);
        result = COB_CURL_ERR_BUFFER;
    }

cleanup:
    curl_slist_free_all(headers);
    return result;
}

void cob_curl_cleanup(void)
{
    if (s_curl_handle != NULL) {
        curl_easy_cleanup(s_curl_handle);
        s_curl_handle = NULL;
    }
    curl_global_cleanup();
}
