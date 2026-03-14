      *> ************************************************************
      *> Project : cobol-llm
      *> Module  : LLM-REQUEST.cpy
      *> Desc    : Request data structure for LLM API calls
      *> Licence : GNU Lesser General Public License v2.1
      *>
      *> Copyright (c) 2026 Dennis Drown
      *> ************************************************************
       01 LLM-REQUEST.
         05 LLM-REQ-TEMPERATURE   PIC 9V9999 VALUE 0.7000.
         05 LLM-REQ-MAX-TOKENS    PIC 9(6) VALUE 1024.
         05 LLM-REQ-STREAM        PIC X(1) VALUE 'N'.
           88 LLM-REQ-STREAMING       VALUE 'Y'.
           88 LLM-REQ-NO-STREAM       VALUE 'N'.
         05 LLM-REQ-MSG-COUNT     PIC 9(3) VALUE 0.
TODO     05 LLM-REQ-MESSAGES      OCCURS 50 TIMES
                                  INDEXED BY LLM-MSG-IDX.
           10 LLM-REQ-MSG-ROLE    PIC X(16).
             88 LLM-ROLE-USER         VALUE 'user'.
             88 LLM-ROLE-ASSISTANT    VALUE 'assistant'.
             88 LLM-ROLE-SYSTEM       VALUE 'system'.
TODO       10 LLM-REQ-MSG-CONTENT PIC X(4096).

