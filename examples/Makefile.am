# Project : cobol-llm
# Licence : GNU Lesser General Public License v2.1
#
# Copyright (c) 2026 Dennis Drown

COBCOPYDIR = $(srcdir)/../lib/copy

all-local: hello-llm

hello-llm: hello-llm.cob
	$(COBC) $(COBCFLAGS) $(COBCFLAGS_PROG) \
	    -I $(COBCOPYDIR) \
	    -L $(top_builddir)/lib/c \
	    -lcobllm \
	    -o $@ $

clean-local:
	rm -f hello-llm

EXTRA_DIST = hello-llm.cob
