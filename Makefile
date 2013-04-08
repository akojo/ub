CFLAGS += -I include -g
PROGRAMS = cclient cserver
MODULES = lib $(PROGRAMS)

lib_OBJS = $(subst .c,.o,$(wildcard lib/*.c))
cclient_OBJS = $(subst .c,.o,cclient.c)
cserver_OBJS = $(subst .c,.o,cserver.c)

all_OBJS = $(foreach mod,$(MODULES),$($(mod)_OBJS))

.SECONDEXPANSION:
all: $(PROGRAMS)

$(PROGRAMS): $$($$@_OBJS) $(lib_OBJS)

$(all_OBJS): Makefile $(foreach d,. lib,$(wildcard $(d)/*.h))

.PHONY: clean realclean
clean:
	rm -f $(all_OBJS)

realclean: clean
	rm -f $(PROGRAMS)
