# Project : cobol-llm
# Licence : GNU Lesser General Public License v2.1
#
# Copyright (c) 2026 Dennis Drown

lib_LTLIBRARIES = libcobllm.la

libcobllm_la_SOURCES = \
    cob_curl.c \
    cob_curl.h \
    cob_json.c \
    cob_json.h

libcobllm_la_CFLAGS = \
    $(CURL_CFLAGS) \
    $(CJSON_CFLAGS) \
    -Wall \
    -Wextra \
    -pedantic

libcobllm_la_LIBADD = \
    $(CURL_LIBS) \
    $(CJSON_LIBS)

libcobllm_la_LDFLAGS = \
    -version-info 0:1:0
