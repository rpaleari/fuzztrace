.PHONY: all clean

CFLAGS=-Wall -std=c++11 -fPIC
LDFLAGS=

all: libtracer.a
clean:
	-rm $(objs) $(protobuf-files)

objs = bbtrace.pb.o bbmap.o exception.o serialize.o
protobuf-files = bbtrace.pb.cc bbtrace.pb.h

libtracer.a: $(objs)
	ar -rcs -o $@ $^ $(LDFLAGS)

%.o: %.cc logging.h
	$(CXX) $(CFLAGS) -c -o $@ $<

bbtrace.pb.h bbtrace.pb.cc: bbtrace.proto
	protoc --cpp_out=. $<
