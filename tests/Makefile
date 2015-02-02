TESTS_ASM := $(basename $(wildcard *.S))
TESTS_C := $(basename $(wildcard *.c))
TESTS = $(TESTS_ASM) $(TESTS_C)
TRACES = $(TESTS:=.trace)

tests: $(TESTS)
all: tests
clean:
	-rm $(TESTS)

traces: $(TRACES)

$(TRACES): %.trace: %
	../tracer/bts/bts_trace	-f /dev/shm/$@ ./$^

$(TESTS_ASM): %: %.S
	$(CC) -nostdlib -o $@ $^

$(TESTS_C): %: %.c
	$(CC) -o $@ $^
