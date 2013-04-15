CFLAGS += -I include -g -Wall -Werror -pedantic
LDFLAGS += -g
LDLIBS += -lm
PROGRAMS = cclient cserver
TEST_PROGRAMS = $(subst .c,,$(wildcard test/*.c))
MODULES = lib test $(PROGRAMS)

lib_OBJS = $(subst .c,.o,$(wildcard lib/*.c))
test_OBJS = $(subst .c,.o,$(wildcard test/*.c))
cclient_OBJS = $(subst .c,.o,cclient.c)
cserver_OBJS = $(subst .c,.o,cserver.c)

all_OBJS = $(foreach mod,$(MODULES),$($(mod)_OBJS))

.SECONDEXPANSION:
all: $(PROGRAMS)

$(PROGRAMS): $$($$@_OBJS) $(lib_OBJS)
$(TEST_PROGRAMS): $$@.o $(lib_OBJS)

$(all_OBJS): Makefile $(foreach d,. lib,$(wildcard $(d)/*.h))

.PHONY: clean realclean test
test: $(TEST_PROGRAMS)
	$(foreach tp,$(TEST_PROGRAMS),$(tp))

clean:
	rm -f $(all_OBJS)

realclean: clean
	rm -f $(PROGRAMS) $(TEST_PROGRAMS)
