.PHONY: all clean

CFLAGS=-Wall -std=c++11 -I..
LDFLAGS=-L../common/

libtracer=../common/libtracer.a

all: bts_trace
clean:
	-rm $(objs) bts_trace

objs = bts_trace.o perf.o monitor.o

bts_trace: $(objs) $(libtracer)
	$(CXX) $(CFLAGS) -o $@ $(objs) $(LDFLAGS) -lprotobuf -ltracer

.PHONY: $(libtracer)
$(libtracer):
	@$(MAKE) -C $(dir $(libtracer))

%.o: %.cc ../common/logging.h
	$(CXX) $(CFLAGS) -c -o $@ $<
