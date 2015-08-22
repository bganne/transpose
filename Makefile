N?=256
M?=2
TYPE?=float
UNROLL?=4
BLOCK_N?=256
BLOCK_M?=8
ROUND?=100

FLAVOURS:=REF UNROLL UNROLL_AND_JAM BLOCK LINEAR LINEAR_UNROLL INPLACE

PERF_EVENTS:=cycles,stalled-cycles-frontend,stalled-cycles-backend,instructions
PERF_EVENTS+=,cache-references,cache-misses
PERF_EVENTS+=,branches,branch-misses

CFLAGS:=-O3 -g -Wall -Werror -std=c99
CFLAGS += -DROUND=$(ROUND)
CFLAGS += -DN=$(N) -DM=$(M) -DTYPE=$(TYPE) -DUNROLL_=$(UNROLL) -DBLOCK_N_=$(BLOCK_N) -DBLOCK_M_=$(BLOCK_M)
LDFLAGS:=-g

PREFIX:=trans_$(TYPE)x$(UNROLL)_$(N)x$(M)_$(BLOCK_N)x$(BLOCK_M)
CHECK:=$(PREFIX)_check
TRANSH:=$(PREFIX).h
BIN:=$(addprefix $(PREFIX)_, $(FLAVOURS))

all: $(BIN) $(CHECK)

define target =
$(PREFIX)_$(1): trans.c trans.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -DFLAVOUR=$(1) $(LDFLAGS) $(LOADLIBES) $(LDLIBS) $$^ -o $$@
endef

$(foreach flavour,$(FLAVOURS),$(eval $(call target,$(flavour))))

$(PREFIX)_INPLACE: trans.c trans.h $(TRANSH)
	$(CC) $(CPPFLAGS) $(CFLAGS) -DFLAVOUR=INPLACE -include $(TRANSH) $(LDFLAGS) $(LOADLIBES) $(LDLIBS) $^ -o $@

$(TRANSH): transpose.py
	./$< $(N) $(M) $(TYPE) $(UNROLL) > "$@"

perf: $(addprefix perf-,$(FLAVOURS))

perf-%: $(PREFIX)_%
	perf stat -e "$(PERF_EVENTS)" -r10 ./$^

$(CHECK): check.c trans.h $(TRANSH)
	$(CC) $(CPPFLAGS) $(CFLAGS) -include $(TRANSH) $(LDFLAGS) $(LOADLIBES) $(LDLIBS) $^ -o $@

$(FLAVOURS): %: $(PREFIX)_%

check: $(CHECK)
	./$^

clean:
	$(RM) $(BIN) $(CHECK) $(TRANSH)

.PHONY: all clean check $(FLAVOURS)
