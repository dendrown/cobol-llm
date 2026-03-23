      *> ************************************************************
      *> Project : cobol-llm
      *> Module  : LLM-INIT.cob
      *> Desc    : Initialise the LLM provider connection
      *> Licence : GNU Lesser General Public License v2.1
      *>
      *> Copyright (c) 2026 Dennis Drown
      *> ************************************************************
       IDENTIFICATION DIVISION.
       PROGRAM-ID. LLM-INIT.

       ENVIRONMENT DIVISION.
       CONFIGURATION SECTION.

       DATA DIVISION.
       WORKING-STORAGE SECTION.

       01 WS-CURL-RC                PIC S9(4) COMP.

       LINKAGE SECTION.

       COPY 'LLM-CONFIG.cpy'.
       COPY 'LLM-STATUS.cpy'.


       PROCEDURE DIVISION USING LLM-CONFIG
                                LLM-STATUS.

       000-MAIN.
           PERFORM 100-VALIDATE-CONFIG
           IF LLM-STAT-OK
               PERFORM 200-CURL-INIT
           END-IF
           GOBACK.


       100-VALIDATE-CONFIG.
           INITIALIZE LLM-STATUS

           IF LLM-PROVIDER = SPACES
               SET LLM-STAT-CONFIG-ERR   TO TRUE
               MOVE 'LLM-INIT: LLM-PROVIDER not set'
                                         TO LLM-STAT-MESSAGE
               GOBACK
           END-IF

           IF NOT LLM-PROVIDER-OLLAMA
           AND NOT LLM-PROVIDER-CLAUDE
           AND NOT LLM-PROVIDER-OPENAI
               SET LLM-STAT-CONFIG-ERR   TO TRUE
               MOVE 'LLM-INIT: unrecognised LLM-PROVIDER'
                                         TO LLM-STAT-MESSAGE
               GOBACK
           END-IF

           IF LLM-ENDPOINT-URL = SPACES
               SET LLM-STAT-CONFIG-ERR   TO TRUE
               MOVE 'LLM-INIT: LLM-ENDPOINT-URL not set'
                                         TO LLM-STAT-MESSAGE
               GOBACK
           END-IF

           IF LLM-MODEL = SPACES
               SET LLM-STAT-CONFIG-ERR   TO TRUE
               MOVE 'LLM-INIT: LLM-MODEL not set'
                                         TO LLM-STAT-MESSAGE
               GOBACK
           END-IF

           IF LLM-PROVIDER-CLAUDE
           AND LLM-API-KEY = SPACES
               SET LLM-STAT-CONFIG-ERR   TO TRUE
               MOVE 'LLM-INIT: LLM-API-KEY required for Claude'
                                         TO LLM-STAT-MESSAGE
               GOBACK
           END-IF.


       200-CURL-INIT.
           CALL 'cob_curl_init' RETURNING WS-CURL-RC

           EVALUATE WS-CURL-RC
               WHEN 0
                   INITIALIZE LLM-STATUS
               WHEN 1
                   SET LLM-STAT-CURL-ERR TO TRUE
                   MOVE 'LLM-INIT: curl initialisation failed'
                                         TO LLM-STAT-MESSAGE
               WHEN OTHER
                   SET LLM-STAT-CURL-ERR TO TRUE
                   MOVE 'LLM-INIT: unexpected curl return code'
                                         TO LLM-STAT-MESSAGE
           END-EVALUATE.
