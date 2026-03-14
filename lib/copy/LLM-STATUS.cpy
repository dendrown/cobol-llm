      *> ************************************************************
      *> Project : cobol-llm
      *> Module  : LLM-STATUS.cpy
      *> Desc    : Library-level status and error reporting
      *> Licence : GNU Lesser General Public License v2.1
      *>
      *> Copyright (c) 2026 Dennis Drown
      *> ************************************************************
       01 LLM-STATUS.
         05 LLM-STAT-CODE         PIC 9(4) VALUE 0.
           88 LLM-STAT-OK             VALUE 0.
           88 LLM-STAT-CURL-ERR       VALUE 1.
           88 LLM-STAT-JSON-ERR       VALUE 2.
           88 LLM-STAT-CONFIG-ERR     VALUE 3.
           88 LLM-STAT-TIMEOUT        VALUE 4.
         05 LLM-STAT-MESSAGE      PIC X(256).

