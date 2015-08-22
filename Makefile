N?=256
M?=2
TYPE?=float
UNROLL?=4

FLAVOURS:=REF REF_UNROLL REF_UNROLL_AND_JAM LINEAR LINEAR_UNROLL INPLACE

PREFIX:=trans_$(TYPE)x$(UNROLL)_$(N)x$(M)
TRANSH:=$(PREFIX).h


PERF_EVENTS:=cycles,stalled-cycles-frontend,stalled-cycles-backend,instructions
PERF_EVENTS+=,cache-references,cache-misses
PERF_EVENTS+=,branches,branch-misses

CFLAGS:=-O3 -g -Wall -Werror -std=c99
CFLAGS += -DROUND=100
CFLAGS += -DN=$(N) -DM=$(M) -DTYPE=$(TYPE) -DUNROLL=$(UNROLL)
LDFLAGS:=-g

BIN:=$(addprefix $(PREFIX)_, $(FLAVOURS))

all: $(BIN) check

define target =
$(PREFIX)_$(1): trans.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -DFLAVOUR=$(1) $(LDFLAGS) $(LOADLIBES) $(LDLIBS) $$< -o $$@
endef

$(foreach flavour,$(FLAVOURS),$(eval $(call target,$(flavour))))

$(PREFIX)_INPLACE: trans.c $(TRANSH)
	$(CC) $(CPPFLAGS) $(CFLAGS) -DFLAVOUR=INPLACE -include $(TRANSH) $(LDFLAGS) $(LOADLIBES) $(LDLIBS) $^ -o $@

$(TRANSH): transpose.py
	./$< $(N) $(M) $(TYPE) $(UNROLL) > "$@"

perf: $(addprefix perf-,$(BIN))

perf-%: %
	perf stat -e "$(PERF_EVENTS)" -r10 ./$<

check: check.c $(TRANSH)
	$(CC) $(CPPFLAGS) $(CFLAGS) -include $(TRANSH) $(LDFLAGS) $(LOADLIBES) $(LDLIBS) $^ -o $@

clean:
	$(RM) $(BIN) check $(TRANSH)

.PHONY: all clean
