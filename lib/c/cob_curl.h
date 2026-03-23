/**
 * @file cob_curl.h
 * @brief libcurl HTTP shim for COBOL-LLM API access.
 *
 * Provides a minimal C interface for sending HTTP POST requests
 * and capturing responses, designed to be called from GNUCobol
 * via its C interoperability mechanism. JSON construction and
 * parsing remain in the COBOL layer; this shim handles only
 * HTTP transport.
 *
 * Lifecycle:
 * @code
 *   cob_curl_init();
 *   cob_curl_post(...);  // called once per LLM-CHAT invocation
 *   cob_curl_cleanup();
 * @endcode
 *
 * @project  cobol-llm
 * @licence  GNU Lesser General Public License v2.1
 *
 * @copyright Copyright (c) 2026 Dennis Drown
 */

#ifndef COB_CURL_H
#define COB_CURL_H

#include <stdint.h>

/**
 * @defgroup return_codes Return Codes
 * @brief Return codes returned by cob_curl functions.
 *
 * These values mirror the 88-level condition codes defined in
 * LLM-STATUS.cpy to allow direct mapping at the COBOL layer.
 * @{
 */
#define COB_CURL_OK           0  /**< Success.                        */
#define COB_CURL_ERR_INIT     1  /**< curl initialisation failed.     */
#define COB_CURL_ERR_PERFORM  2  /**< curl perform failed.            */
#define COB_CURL_ERR_TIMEOUT  3  /**< Request timed out.              */
#define COB_CURL_ERR_BUFFER   4  /**< Response exceeded buffer size.  */
/** @} */

/**
 * @brief Initialises the global curl environment and session handle.
 *
 * Must be called once before any call to cob_curl_post(). Calls
 * curl_global_init() followed by curl_easy_init() and retains the
 * resulting handle for reuse across requests.
 *
 * @return COB_CURL_OK on success, COB_CURL_ERR_INIT on failure.
 */
int cob_curl_init(void);

/**
 * @brief Sends an HTTP POST request and captures the response body.
 *
 * Uses the persistent curl handle created by cob_curl_init(). The
 * response body is written directly into @p response_buf, which is
 * caller-allocated. JSON construction and parsing are the
 * responsibility of the calling COBOL layer.
 *
 * @param[in]  url          Null-terminated endpoint URL.
 * @param[in]  api_key      Null-terminated API key. May be empty
 *                          for providers that require no key (e.g.
 *                          Ollama).
 * @param[in]  request_body Null-terminated JSON request body.
 * @param[out] response_buf Caller-allocated buffer for the response
 *                          body. Should be sized to match
 *                          LLM-RSP-CONTENT (32768 bytes).
 * @param[out] response_len Bytes written into @p response_buf on
 *                          return.
 * @param[out] http_status  HTTP status code on return (e.g. 200,
 *                          401, 429, 500).
 * @param[in]  timeout_secs Request timeout in seconds. Should match
 *                          LLM-TIMEOUT-SECS from LLM-CONFIG.
 * @param[out] err_msg      Caller-allocated buffer for a
 *                          null-terminated error message. Must be
 *                          at least 256 bytes, matching
 *                          LLM-STAT-MESSAGE in LLM-STATUS.cpy.
 *
 * @return COB_CURL_OK on success, or one of the COB_CURL_ERR_*
 *         codes on failure.
 */
int cob_curl_post(
    const char  *url,
    const char  *api_key,
    const char  *request_body,
    char        *response_buf,
    int32_t     *response_len,
    int32_t     *http_status,
    int32_t      timeout_secs,
    char        *err_msg
);

/**
 * @brief Releases the curl session handle and global curl resources.
 *
 * Must be called once when LLM access is no longer required. Calls
 * curl_easy_cleanup() followed by curl_global_cleanup() in the
 * correct order.
 */
void cob_curl_cleanup(void);

#endif /* COB_CURL_H */
