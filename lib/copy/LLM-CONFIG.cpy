      *> ************************************************************
      *> Project : cobol-llm
      *> Module  : LLM-CONFIG.cpy
      *> Desc    : Provider configuration for LLM API access
      *> Licence : GNU Lesser General Public License v2.1
      *>
      *> Copyright (c) 2026 Dennis Drown
      *> ************************************************************
       01 LLM-CONFIG.
           05 LLM-PROVIDER          PIC X(20).
             88 LLM-PROVIDER-OLLAMA     VALUE 'OLLAMA'.
             88 LLM-PROVIDER-CLAUDE     VALUE 'CLAUDE'.
             88 LLM-PROVIDER-OPENAI     VALUE 'OPENAI'.
           05 LLM-ENDPOINT-URL      PIC X(256).
           05 LLM-API-KEY           PIC X(256).
           05 LLM-MODEL             PIC X(128).
           05 LLM-TIMEOUT-SECS      PIC 9(4) VALUE 30.

