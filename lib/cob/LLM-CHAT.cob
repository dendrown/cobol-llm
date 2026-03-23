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
       01 WS-CURL-RC                PIC S9(4) COMP.
       01 WS-HTTP-STATUS            PIC S9(9) COMP.
       01 WS-RESPONSE-LEN           PIC S9(9) COMP.
       01 WS-ERR-MSG                PIC X(256).

      *> ---- JSON working storage -----------------------------------
      *> Intermediate buffer for JSON request body passed to C shim
       01 WS-JSON-REQUEST           PIC X(8192).
       01 WS-JSON-REQUEST-LEN       PIC 9(6) COMP.

      *> ---- Ollama request structure -------------------------------
       01 WS-OLLAMA-REQUEST.
         05 WS-OLL-MODEL            PIC X(128).
         05 WS-OLL-STREAM           PIC X(5).
         05 WS-OLL-OPTIONS.
           10 WS-OLL-TEMPERATURE    PIC 9V9999.
         05 WS-OLL-MESSAGES         OCCURS 50 TIMES
                                    INDEXED BY WS-OLL-MSG-IDX.
           10 WS-OLL-MSG-ROLE       PIC X(16).
           10 WS-OLL-MSG-CONTENT    PIC X(4096).

      *> ---- Claude request structure ------------------------------
       01 WS-CLAUDE-REQUEST.
         05 WS-CLD-MODEL            PIC X(128).
         05 WS-CLD-MAX-TOKENS       PIC 9(6).
         05 WS-CLD-TEMPERATURE      PIC 9V9999.
         05 WS-CLD-MESSAGES         OCCURS 50 TIMES
                                    INDEXED BY WS-CLD-MSG-IDX.
           10 WS-CLD-MSG-ROLE       PIC X(16).
           10 WS-CLD-MSG-CONTENT    PIC X(4096).

      *> ---- Response parse structure ------------------------------
       01 WS-OLLAMA-RESPONSE.
         05 WS-OLL-RSP-MODEL        PIC X(128).
         05 WS-OLL-RSP-DONE         PIC X(5).
         05 WS-OLL-RSP-MESSAGE.
           10 WS-OLL-RSP-ROLE       PIC X(16).
           10 WS-OLL-RSP-CONTENT    PIC X(32768).
         05 WS-OLL-RSP-DONE-REASON  PIC X(32).
         05 WS-OLL-PROMPT-TOKENS    PIC 9(9).
         05 WS-OLL-COMPLETION-TOKENS PIC 9(9).

       01 WS-CLAUDE-RESPONSE.
         05 WS-CLD-RSP-ID           PIC X(64).
         05 WS-CLD-RSP-MODEL        PIC X(128).
         05 WS-CLD-RSP-STOP-REASON  PIC X(32).
         05 WS-CLD-RSP-CONTENT      OCCURS 1 TIMES
                                    INDEXED BY WS-CLD-RSP-IDX.
           10 WS-CLD-RSP-TYPE       PIC X(16).
           10 WS-CLD-RSP-TEXT       PIC X(32768).
         05 WS-CLD-USAGE.
           10 WS-CLD-INPUT-TOKENS   PIC 9(9).
           10 WS-CLD-OUTPUT-TOKENS  PIC 9(9).

      *> ---- loop index --------------------------------------------
       01 WS-MSG-IDX                PIC 9(3) COMP.

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
           EVALUATE TRUE
               WHEN LLM-PROVIDER-OLLAMA
                   PERFORM 210-BUILD-OLLAMA-REQUEST
               WHEN LLM-PROVIDER-CLAUDE
                   PERFORM 220-BUILD-CLAUDE-REQUEST
               WHEN OTHER
                   SET LLM-STAT-CONFIG-ERR  TO TRUE
                   MOVE 'LLM-CHAT: unsupported provider'
                                     TO LLM-STAT-MESSAGE
           END-EVALUATE.


      *> TODO: we'll probably need NAME clauses with JSON GENERATE
       210-BUILD-OLLAMA-REQUEST.
           INITIALIZE WS-OLLAMA-REQUEST
           MOVE LLM-MODEL            TO WS-OLL-MODEL
           MOVE LLM-REQ-TEMPERATURE  TO WS-OLL-TEMPERATURE
           MOVE 'false'              TO WS-OLL-STREAM

           PERFORM VARYING WS-MSG-IDX FROM 1 BY 1
               UNTIL WS-MSG-IDX > LLM-REQ-MSG-COUNT
               MOVE LLM-REQ-MSG-ROLE(WS-MSG-IDX)
                                     TO WS-OLL-MSG-ROLE(WS-MSG-IDX)
               MOVE LLM-REQ-MSG-CONTENT(WS-MSG-IDX)
                                     TO WS-OLL-MSG-CONTENT(WS-MSG-IDX)
           END-PERFORM

           INITIALIZE WS-JSON-REQUEST
TODO       JSON GENERATE WS-JSON-REQUEST
               FROM WS-OLLAMA-REQUEST
               COUNT IN WS-JSON-REQUEST-LEN
               ON EXCEPTION
                   SET LLM-STAT-JSON-ERR TO TRUE
                   MOVE 'LLM-CHAT: JSON GENERATE failed (Ollama)'
                                     TO LLM-STAT-MESSAGE
           END-JSON.


      *> TODO: we'll probably need NAME clauses with JSON GENERATE
       220-BUILD-CLAUDE-REQUEST.
           INITIALIZE WS-CLAUDE-REQUEST
           MOVE LLM-MODEL            TO WS-CLD-MODEL
           MOVE LLM-REQ-MAX-TOKENS   TO WS-CLD-MAX-TOKENS
           MOVE LLM-REQ-TEMPERATURE  TO WS-CLD-TEMPERATURE

           PERFORM VARYING WS-MSG-IDX FROM 1 BY 1
               UNTIL WS-MSG-IDX > LLM-REQ-MSG-COUNT
               MOVE LLM-REQ-MSG-ROLE(WS-MSG-IDX)
                                     TO WS-CLD-MSG-ROLE(WS-MSG-IDX)
               MOVE LLM-REQ-MSG-CONTENT(WS-MSG-IDX)
                                     TO WS-CLD-MSG-CONTENT(WS-MSG-IDX)
           END-PERFORM

           INITIALIZE WS-JSON-REQUEST
TODO       JSON GENERATE WS-JSON-REQUEST
               FROM WS-CLAUDE-REQUEST
               COUNT IN WS-JSON-REQUEST-LEN
               ON EXCEPTION
                   SET LLM-STAT-JSON-ERR TO TRUE
                   MOVE 'LLM-CHAT: JSON GENERATE failed (Claude)'
                                         TO LLM-STAT-MESSAGE
           END-JSON.


       300-CALL-CURL.
           INITIALIZE WS-ERR-MSG
           INITIALIZE LLM-RESPONSE

           CALL 'cob_curl_post' USING
               BY REFERENCE LLM-ENDPOINT-URL
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
           INITIALIZE WS-OLLAMA-RESPONSE
           JSON PARSE WS-OLLAMA-RESPONSE
               FROM LLM-RSP-CONTENT
               ON EXCEPTION
                   SET LLM-STAT-JSON-ERR TO TRUE
                   MOVE 'LLM-CHAT: JSON PARSE failed (Ollama)'
                                         TO LLM-STAT-MESSAGE
                   GOBACK
           END-JSON

           MOVE WS-OLL-RSP-CONTENT       TO LLM-RSP-CONTENT
           MOVE WS-OLL-RSP-MODEL         TO LLM-RSP-MODEL
           MOVE WS-OLL-RSP-DONE-REASON   TO LLM-RSP-FINISH-REASON
           MOVE WS-OLL-PROMPT-TOKENS     TO LLM-RSP-TOKENS-IN
           MOVE WS-OLL-COMPLETION-TOKENS TO LLM-RSP-TOKENS-OUT.


       420-PARSE-CLAUDE-RESPONSE.
           INITIALIZE WS-CLAUDE-RESPONSE
           JSON PARSE WS-CLAUDE-RESPONSE
               FROM LLM-RSP-CONTENT
               ON EXCEPTION
                   SET LLM-STAT-JSON-ERR TO TRUE
                   MOVE 'LLM-CHAT: JSON PARSE failed (Claude)'
                                         TO LLM-STAT-MESSAGE
                   GOBACK
           END-JSON

           MOVE WS-CLD-RSP-TEXT(1)       TO LLM-RSP-CONTENT
           MOVE WS-CLD-RSP-MODEL         TO LLM-RSP-MODEL
           MOVE WS-CLD-RSP-STOP-REASON   TO LLM-RSP-FINISH-REASON
           MOVE WS-CLD-INPUT-TOKENS      TO LLM-RSP-TOKENS-IN
           MOVE WS-CLD-OUTPUT-TOKENS     TO LLM-RSP-TOKENS-OUT.

