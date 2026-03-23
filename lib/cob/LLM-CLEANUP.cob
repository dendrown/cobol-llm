      *> ************************************************************
      *> Project : cobol-llm
      *> Module  : LLM-CLEANUP.cob
      *> Desc    : Release LLM provider connection resources
      *> Licence : GNU Lesser General Public License v2.1
      *>
      *> Copyright (c) 2026 Dennis Drown
      *> ************************************************************
       IDENTIFICATION DIVISION.
       PROGRAM-ID. LLM-CLEANUP.

       ENVIRONMENT DIVISION.
       CONFIGURATION SECTION.

       DATA DIVISION.
       WORKING-STORAGE SECTION.

       LINKAGE SECTION.

       COPY 'LLM-CONFIG.cpy'.
       COPY 'LLM-STATUS.cpy'.


       PROCEDURE DIVISION USING LLM-CONFIG
                                LLM-STATUS.

       000-MAIN.
           INITIALIZE LLM-STATUS
           CALL 'cob_curl_cleanup'
           GOBACK.

