      *> ************************************************************
      *> Project : cobol-llm
      *> Module  : hello-llm.cob
      *> Desc    : Basic example - single prompt to an LLM provider
      *> Licence : GNU Lesser General Public License v2.1
      *>
      *> Copyright (c) 2026 Dennis Drown
      *> ************************************************************
       IDENTIFICATION DIVISION.
       PROGRAM-ID. HELLO-LLM.

       ENVIRONMENT DIVISION.
       CONFIGURATION SECTION.

       DATA DIVISION.
       WORKING-STORAGE SECTION.

       COPY 'LLM.cpy'.

       PROCEDURE DIVISION.

       000-MAIN.
           PERFORM 100-INIT
           IF LLM-STAT-OK
             PERFORM 200-CHAT
           END-IF.
           PERFORM 300-CLEANUP
           STOP RUN.

       100-INIT.
           MOVE 'OLLAMA'                  TO LLM-PROVIDER
           MOVE 'http://localhost:11434'  TO LLM-ENDPOINT-URL
           MOVE SPACES                    TO LLM-API-KEY
           MOVE 'llama3.2:3b'             TO LLM-MODEL
           CALL 'LLM-INIT' USING LLM-CONFIG
                                 LLM-STATUS
           IF NOT LLM-STAT-OK
             DISPLAY 'Init failed: ' LLM-STAT-MESSAGE
           END-IF.

       200-CHAT.
           MOVE 1                         TO LLM-REQ-MSG-COUNT
           SET LLM-ROLE-USER(1)           TO TRUE
           MOVE 'Hello! Please introduce yourself briefly.'
                                          TO LLM-REQ-MSG-CONTENT(1)
           CALL 'LLM-CHAT' USING LLM-CONFIG
                                 LLM-REQUEST
                                 LLM-RESPONSE
                                 LLM-STATUS
           IF LLM-STAT-OK AND LLM-RSP-OK
             DISPLAY 'Response: '
                     FUNCTION TRIM(LLM-RSP-CONTENT TRAILING)
             DISPLAY 'Tokens in:  ' LLM-RSP-TOKENS-IN
             DISPLAY 'Tokens out: ' LLM-RSP-TOKENS-OUT
           ELSE
             DISPLAY 'Failure: [' LLM-STAT-CODE '] ' LLM-STAT-MESSAGE
           END-IF.

       300-CLEANUP.
           CALL 'LLM-CLEANUP' USING LLM-CONFIG
                                    LLM-STATUS
           IF NOT LLM-STAT-OK
             DISPLAY 'Cleanup warning: ' LLM-STAT-MESSAGE
           END-IF.

