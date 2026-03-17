      *> ************************************************************
      *> Project : cobol-llm
      *> Module  : LLM-RESPONSE.cpy
      *> Desc    : Response data structure for LLM API calls
      *> Licence : GNU Lesser General Public License v2.1
      *>
      *> Copyright (c) 2026 Dennis Drown
      *> ************************************************************
       01 LLM-RESPONSE.
           05 LLM-RSP-STATUS        PIC 9(4) VALUE 0.
             88 LLM-RSP-OK              VALUE 200.
             88 LLM-RSP-UNAUTH          VALUE 401.
             88 LLM-RSP-RATE-LIMIT      VALUE 429.
             88 LLM-RSP-SERVER-ERR      VALUE 500.
           05 LLM-RSP-FINISH-REASON PIC X(32).
             88 LLM-FINISH-COMPLETE     VALUE 'stop'.
             88 LLM-FINISH-MAX-TOK      VALUE 'max_tokens'              OLLAMA
                                              'length'.                 CLAUDE
TODO       05 LLM-RSP-CONTENT       PIC X(32768).
           05 LLM-RSP-CONTENT-LEN   PIC 9(6) VALUE 0.
           05 LLM-RSP-TOKENS-IN     PIC 9(9) VALUE 0.
           05 LLM-RSP-TOKENS-OUT    PIC 9(9) VALUE 0.
           05 LLM-RSP-MODEL         PIC X(128).

