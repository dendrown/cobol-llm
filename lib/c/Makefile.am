# Project : cobol-llm
# Licence : GNU Lesser General Public License v2.1
#
# Copyright (c) 2026 Dennis Drown

COBCOPYDIR = $(top_srcdir)/lib/copy

lib_LTLIBRARIES = libcobllm.la

# COBOL sources
cob_modules = LLM-INIT LLM-CHAT LLM-CLEANUP
cob_sources = $(cob_modules:=.cob)

# Generated C files from COBOL
cob_c_files = $(cob_modules:=.c)

# Libtool object files from COBOL-generated C
cob_objects = $(cob_modules:=.lo)

libcobllm_la_SOURCES = \
    cob_utils.c \
    cob_utils.h \
    cob_curl.c \
    cob_curl.h \
    cob_json.c \
    cob_json.h

libcobllm_la_LIBADD = \
    $(CURL_LIBS) \
    $(CJSON_LIBS) \
    $(cob_objects)

libcobllm_la_CFLAGS = \
    $(CURL_CFLAGS) \
    $(CJSON_CFLAGS) \
    -Wall \
    -Wextra \
    -pedantic

libcobllm_la_LDFLAGS = \
    -version-info 0:1:0

# Pattern rule: COBOL -> C
%.c: $(top_srcdir)/lib/cob/%.cob
	$(COBC) $(COBCFLAGS) -C \
	    -I $(COBCOPYDIR) \
	    -o $@ $<

# Pattern rule: COBOL-generated C -> libtool object
%.lo: %.c
	$(LIBTOOL) --mode=compile $(CC) $(CFLAGS) \
	    `$(COB_CONFIG) --cflags` \
	    -c -o $@ $<

# Ensure COBOL objects are built before the library
all-local: $(cob_objects)

clean-local:
	rm -f $(cob_c_files) $(cob_objects)

EXTRA_DIST = $(cob_sources)

# vim: set ft=make :
