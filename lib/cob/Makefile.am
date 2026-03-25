# Project : cobol-llm
# Licence : GNU Lesser General Public License v2.1
#
# Copyright (c) 2026 Dennis Drown

COBCOPYDIR = $(srcdir)/../copy

cob_sources = \
    LLM-INIT.cob \
    LLM-CHAT.cob \
    LLM-CLEANUP.cob

cob_modules = \
    LLM-INIT.so \
    LLM-CHAT.so \
    LLM-CLEANUP.so

all-local: $(cob_modules)

LLM-INIT.so: LLM-INIT.cob
	$(COBC) $(COBCFLAGS) $(COBCFLAGS_MODULE) \
	    -I $(COBCOPYDIR) \
	    -L $(top_builddir)/lib/c/.libs \
	    -lcobllm \
	    -o $@ $<

LLM-CHAT.so: LLM-CHAT.cob
	$(COBC) $(COBCFLAGS) $(COBCFLAGS_MODULE) \
	    -I $(COBCOPYDIR) \
	    -L $(top_builddir)/lib/c/.libs \
	    -lcobllm \
	    -o $@ $<

LLM-CLEANUP.so: LLM-CLEANUP.cob
	$(COBC) $(COBCFLAGS) $(COBCFLAGS_MODULE) \
	    -I $(COBCOPYDIR) \
	    -L $(top_builddir)/lib/c/.libs \
	    -lcobllm \
	    -o $@ $<

clean-local:
	rm -f $(cob_modules)

EXTRA_DIST = $(cob_sources)
