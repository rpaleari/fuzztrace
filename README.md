FuzzTrace is a "general-purpose" tracing tool for closed-source applications,
aimed at generating a concise execution trace that can be used to support the
fuzz-testing activity or other analyses.

At the time of writing, we provide two tracing back-ends, based on Intel BTS
and PIN respectively. In any case, the execution trace is serialized to a
[protobuf](https://code.google.com/p/protobuf/) object, that can then be
processed off-line. Available back-ends are briefly described in the next
paragraphs, together with some usage examples.

On a Debian/Ubuntu system, use the following commands to install the required
dependencies (these are common to all the back-ends available):

	roby@gimli:~$ sudo apt-get install protobuf-compiler python-protobuf libprotobuf-dev

## Back-ends ##

### BTS-based execution tracers ###

The BTS back-end is an efficient tracer that leverages Intel "Branch Trace
Store" (BTS) technology. The source code for this back-end is located under
`tracer/bts`. To compile this back-end module enter directory `tracer/bin` and
run `make`.

Hopefully, everything will go fine. You should now be able to trace your target
application using the `bts_trace` binary:

	roby@gimli:~/projects/fuzztrace/tracer/bts$ ./bts_trace -f /dev/shm/trace.bin -- /bin/ls -la >/dev/null
	[*] Got 108684 events (4967 CFG edges)
	[*] Serializing to /dev/shm/trace.bin

### PIN-based execution tracers ###

The PIN back-end is a
[PIN](https://software.intel.com/en-us/articles/pin-a-dynamic-binary-instrumentation-tool)
extension to monitor the execution of a binary application and record its
"execution trace". FuzzTrace/PIN lives in directory `fuzztrace/tracer/pin`. To
compile the PIN module, set the `PIN_ROOT` environment variable and launch
`make` from the `tracer/pin` directory, e.g.:

	roby@gimli:~/apps/pin$ export PIN_ROOT=$(pwd)
	roby@gimli:~/apps/pin$ cd ~/projects/fuzztrace/tracer/pin
	roby@gimli:~/projects/fuzztrace/tracer/pin$ make


You should be able to trace your target program using the `pintrace` PIN tool:

	roby@gimli:~/projects/fuzztrace/tracer/pin$ ${PIN_ROOT}/pin.sh -t obj-intel64/pintrace.so -f /dev/shm/trace.bin -- /bin/ls

## Trace viewer ##

The `viewer` directory provides a basic trace viewer, which parses a saved
execution traces and displays recorded branches.

Before using the viewer, compile the `bbtrace.proto` file:

	roby@gimli:~/projects/fuzztrace/viewer$ protoc --python_out=. bbtrace.proto

After that, usage is quite straightforward:

	roby@gimli:~/projects/fuzztrace/viewer$ python trace.py /dev/shm/trace.bin
	#### Trace '/dev/shm/trace.bin' ####
	[/dev/shm/trace.bin] cmd: test, data: 0 bytes, time: 2015-01-29 22:47:52, hash: 3c31f, edges(s): 708, exception(s): 0

	 - CFG edges
	 [00402168 -> 00402178] 1 hit
	 [00402178 -> 0040217d] 1 hit
	 [0040217d -> 00412513] 1 hit
	 [00402190 -> 004124e0] 1 hit
	 ...


