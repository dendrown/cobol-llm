      *> ************************************************************
      *> Project : cobol-llm
      *> Module  : LLM-CHAT.cob
      *> Desc    : Send a chat request to the LLM provider
      *> Licence : GNU Lesser General Public License v2.1
      *>
      *> Copyright (c) 2026 Dennis Drown
      *> ************************************************************
       IDENTIFICATION DIVISION.
       PROGRAM-ID. LLM-CHAT.

       ENVIRONMENT DIVISION.
       CONFIGURATION SECTION.

       DATA DIVISION.
       WORKING-STORAGE SECTION.

      *> ---- curl shim interface ------------------------------------
       01 WS-CURL-RC                 PIC S9(4) COMP-5.
       01 WS-FULL-URL                PIC X(300).
       01 WS-HTTP-STATUS             PIC S9(9) COMP-5.
       01 WS-RESPONSE-LEN            PIC S9(9) COMP-5.
       01 WS-ERR-MSG                 PIC X(256).

      *> ---- JSON shim interface ------------------------------------
       01 WS-JSON-RC                 PIC S9(4) COMP-5.
       01 WS-JSON-REQUEST            PIC X(8192).
       01 WS-JSON-REQUEST-LEN        PIC S9(9) COMP-5.
       01 WS-JSON-CONTENT            PIC X(32768).
       01 WS-JSON-MODEL              PIC X(128).
       01 WS-JSON-FINISH-REASON      PIC X(32).
       01 WS-JSON-TOKENS-IN          PIC S9(9) COMP-5.
       01 WS-JSON-TOKENS-OUT         PIC S9(9) COMP-5.

      *> ---- message array for C shim ------------------------------
      *> Mirrors CobLlmMessage struct in cob_json.h
       01 WS-MESSAGES.
           05 WS-MESSAGE             OCCURS 50 TIMES
                                     INDEXED BY WS-MSG-IDX.
             10 WS-MSG-ROLE          PIC X(16).
             10 WS-MSG-CONTENT       PIC X(4096).

      *> ---- temperature as double ---------------------------------
       01 WS-TEMPERATURE             COMP-2.

       LINKAGE SECTION.

       COPY 'LLM-CONFIG.cpy'.
       COPY 'LLM-REQUEST.cpy'.
       COPY 'LLM-RESPONSE.cpy'.
       COPY 'LLM-STATUS.cpy'.

       PROCEDURE DIVISION USING LLM-CONFIG
                                LLM-REQUEST
                                LLM-RESPONSE
                                LLM-STATUS.

       000-MAIN.
           INITIALIZE LLM-STATUS
           INITIALIZE LLM-RESPONSE
           PERFORM 100-VALIDATE-REQUEST
           IF LLM-STAT-OK
               PERFORM 200-BUILD-REQUEST
           END-IF
           IF LLM-STAT-OK
               PERFORM 300-CALL-CURL
           END-IF
           IF LLM-STAT-OK
               PERFORM 400-PARSE-RESPONSE
           END-IF
           GOBACK.

       100-VALIDATE-REQUEST.
           IF LLM-REQ-MSG-COUNT = 0
               SET LLM-STAT-CONFIG-ERR   TO TRUE
               MOVE 'LLM-CHAT: no messages in request'
                                         TO LLM-STAT-MESSAGE
               GOBACK
           END-IF

           IF LLM-REQ-MSG-COUNT > 50
               SET LLM-STAT-CONFIG-ERR   TO TRUE
               MOVE 'LLM-CHAT: message count exceeds maximum (50)'
                                         TO LLM-STAT-MESSAGE
               GOBACK
           END-IF

           PERFORM VARYING WS-MSG-IDX FROM 1 BY 1
               UNTIL WS-MSG-IDX > LLM-REQ-MSG-COUNT
               IF LLM-REQ-MSG-ROLE(WS-MSG-IDX) = SPACES
                   SET LLM-STAT-CONFIG-ERR   TO TRUE
                   MOVE 'LLM-CHAT: message role not set'
                                             TO LLM-STAT-MESSAGE
                   GOBACK
               END-IF
               IF LLM-REQ-MSG-CONTENT(WS-MSG-IDX) = SPACES
                   SET LLM-STAT-CONFIG-ERR   TO TRUE
                   MOVE 'LLM-CHAT: message content not set'
                                             TO LLM-STAT-MESSAGE
                   GOBACK
               END-IF
           END-PERFORM.

       200-BUILD-REQUEST.
      *> Copy messages from LLM-REQUEST table to WS-MESSAGES
      *> for passing to C shim (space-padded, no conversion needed)
           PERFORM VARYING WS-MSG-IDX FROM 1 BY 1
               UNTIL WS-MSG-IDX > LLM-REQ-MSG-COUNT
               MOVE LLM-REQ-MSG-ROLE(WS-MSG-IDX)
                                         TO WS-MSG-ROLE(WS-MSG-IDX)
               MOVE LLM-REQ-MSG-CONTENT(WS-MSG-IDX)
                                         TO WS-MSG-CONTENT(WS-MSG-IDX)
           END-PERFORM

           MOVE LLM-REQ-TEMPERATURE      TO WS-TEMPERATURE

           EVALUATE TRUE
               WHEN LLM-PROVIDER-OLLAMA
                   PERFORM 210-BUILD-OLLAMA-REQUEST
               WHEN LLM-PROVIDER-CLAUDE
                   PERFORM 220-BUILD-CLAUDE-REQUEST
               WHEN OTHER
                   SET LLM-STAT-CONFIG-ERR   TO TRUE
                   MOVE 'LLM-CHAT: unsupported provider'
                                             TO LLM-STAT-MESSAGE
           END-EVALUATE.

       210-BUILD-OLLAMA-REQUEST.
           CALL 'cob_json_build_ollama_request' USING
               BY REFERENCE LLM-MODEL
               BY REFERENCE WS-MESSAGES
               BY VALUE     LLM-REQ-MSG-COUNT
               BY VALUE     WS-TEMPERATURE
               BY VALUE     0
               BY REFERENCE WS-JSON-REQUEST
               BY REFERENCE WS-JSON-REQUEST-LEN
               BY REFERENCE WS-ERR-MSG
               RETURNING WS-JSON-RC

           IF WS-JSON-RC NOT = 0
               SET LLM-STAT-JSON-ERR     TO TRUE
               MOVE WS-ERR-MSG           TO LLM-STAT-MESSAGE
           END-IF.

       220-BUILD-CLAUDE-REQUEST.
           CALL 'cob_json_build_claude_request' USING
               BY REFERENCE LLM-MODEL
               BY REFERENCE WS-MESSAGES
               BY VALUE     LLM-REQ-MSG-COUNT
               BY VALUE     WS-TEMPERATURE
               BY VALUE     LLM-REQ-MAX-TOKENS
               BY REFERENCE WS-JSON-REQUEST
               BY REFERENCE WS-JSON-REQUEST-LEN
               BY REFERENCE WS-ERR-MSG
               RETURNING WS-JSON-RC

           IF WS-JSON-RC NOT = 0
               SET LLM-STAT-JSON-ERR     TO TRUE
               MOVE WS-ERR-MSG           TO LLM-STAT-MESSAGE
           END-IF.

       300-CALL-CURL.
           INITIALIZE WS-ERR-MSG
           EVALUATE TRUE
             WHEN LLM-PROVIDER-OLLAMA
               STRING FUNCTION
                 TRIM(LLM-ENDPOINT-URL TRAILING) DELIMITED SIZE
                 '/api/chat'                     DELIMITED SIZE
                 INTO WS-FULL-URL
             WHEN LLM-PROVIDER-CLAUDE
               STRING FUNCTION
                 TRIM(LLM-ENDPOINT-URL TRAILING) DELIMITED SIZE
                 '/v1/messages'                  DELIMITED SIZE
                 INTO WS-FULL-URL
           END-EVALUATE

           CALL 'cob_curl_post' USING
               BY REFERENCE WS-FULL-URL
               BY REFERENCE LLM-API-KEY
               BY REFERENCE WS-JSON-REQUEST
               BY REFERENCE LLM-RSP-CONTENT
               BY REFERENCE WS-RESPONSE-LEN
               BY REFERENCE WS-HTTP-STATUS
               BY VALUE     LLM-TIMEOUT-SECS
               BY REFERENCE WS-ERR-MSG
               RETURNING WS-CURL-RC

           MOVE WS-HTTP-STATUS           TO LLM-RSP-STATUS
           MOVE WS-RESPONSE-LEN          TO LLM-RSP-CONTENT-LEN

           EVALUATE WS-CURL-RC
               WHEN 0
                   CONTINUE
               WHEN 3
                   SET LLM-STAT-TIMEOUT  TO TRUE
                   MOVE WS-ERR-MSG       TO LLM-STAT-MESSAGE
                   GOBACK
               WHEN OTHER
                   SET LLM-STAT-CURL-ERR TO TRUE
                   MOVE WS-ERR-MSG       TO LLM-STAT-MESSAGE
                   GOBACK
           END-EVALUATE

           IF NOT LLM-RSP-OK
               SET LLM-STAT-CURL-ERR     TO TRUE
               MOVE WS-ERR-MSG           TO LLM-STAT-MESSAGE
           END-IF.

       400-PARSE-RESPONSE.
           EVALUATE TRUE
               WHEN LLM-PROVIDER-OLLAMA
                   PERFORM 410-PARSE-OLLAMA-RESPONSE
               WHEN LLM-PROVIDER-CLAUDE
                   PERFORM 420-PARSE-CLAUDE-RESPONSE
               WHEN OTHER
                   SET LLM-STAT-CONFIG-ERR   TO TRUE
                   MOVE 'LLM-CHAT: unsupported provider'
                                             TO LLM-STAT-MESSAGE
           END-EVALUATE.

       410-PARSE-OLLAMA-RESPONSE.
           CALL 'cob_json_parse_ollama_response' USING
               BY REFERENCE LLM-RSP-CONTENT
               BY REFERENCE WS-JSON-CONTENT
               BY REFERENCE WS-JSON-MODEL
               BY REFERENCE WS-JSON-FINISH-REASON
               BY REFERENCE WS-JSON-TOKENS-IN
               BY REFERENCE WS-JSON-TOKENS-OUT
               BY REFERENCE WS-ERR-MSG
               RETURNING WS-JSON-RC

           IF WS-JSON-RC NOT = 0
               SET LLM-STAT-JSON-ERR     TO TRUE
               MOVE WS-ERR-MSG           TO LLM-STAT-MESSAGE
               GOBACK
           END-IF

           MOVE WS-JSON-CONTENT          TO LLM-RSP-CONTENT
           MOVE WS-JSON-MODEL            TO LLM-RSP-MODEL
           MOVE WS-JSON-FINISH-REASON    TO LLM-RSP-FINISH-REASON
           MOVE WS-JSON-TOKENS-IN        TO LLM-RSP-TOKENS-IN
           MOVE WS-JSON-TOKENS-OUT       TO LLM-RSP-TOKENS-OUT.

       420-PARSE-CLAUDE-RESPONSE.
           CALL 'cob_json_parse_claude_response' USING
               BY REFERENCE LLM-RSP-CONTENT
               BY REFERENCE WS-JSON-CONTENT
               BY REFERENCE WS-JSON-MODEL
               BY REFERENCE WS-JSON-FINISH-REASON
               BY REFERENCE WS-JSON-TOKENS-IN
               BY REFERENCE WS-JSON-TOKENS-OUT
               BY REFERENCE WS-ERR-MSG
               RETURNING WS-JSON-RC

           IF WS-JSON-RC NOT = 0
               SET LLM-STAT-JSON-ERR     TO TRUE
               MOVE WS-ERR-MSG           TO LLM-STAT-MESSAGE
               GOBACK
           END-IF

           MOVE WS-JSON-CONTENT          TO LLM-RSP-CONTENT
           MOVE WS-JSON-MODEL            TO LLM-RSP-MODEL
           MOVE WS-JSON-FINISH-REASON    TO LLM-RSP-FINISH-REASON
           MOVE WS-JSON-TOKENS-IN        TO LLM-RSP-TOKENS-IN
           MOVE WS-JSON-TOKENS-OUT       TO LLM-RSP-TOKENS-OUT.

