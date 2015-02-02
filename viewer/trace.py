"""
Execution trace object.

Copyright 2014, Roberto Paleari (@rpaleari)
"""

import datetime
import hashlib
import logging
import os
import struct

import bbtrace_pb2

class MemoryRegion(object):
    """Mapped memory region, possibly associated to a filename."""
    def __init__(self, obj):
        self.base = obj.base
        self.size = obj.size
        self.name = obj.name

    def get_lower(self):
        return self.base

    def get_upper(self):
        return self.base + self.size - 1

    def get_name(self):
        return self.name

    def has(self, addr):
        """Check if the specified address belongs to this region."""
        return self.get_lower() <= addr <= self.get_upper()

class CrashException(object):
    """Exception observed during program execution."""
    def __init__(self, obj, timestamp):
        """Constructor for the CrashException class."""
        self.timestamp = timestamp
        self.exc_type = obj.type
        self.exc_pc = obj.pc
        self.faultyaddr = obj.faultyaddr
        self.access = obj.access
        self.hashz = None

        # Update hash value for this exception
        self.__update_hash()
        assert self.hashz is not None

    def is_valid(self):
        """Return True if this exception object is valid."""
        if self.hashz is None:
            return False

        return True

    def __update_hash(self):
        """Update the hash value that uniquely characterizes this object."""
        md5 = hashlib.md5()
        md5.update("%d" % self.exc_type)
        md5.update("%x" % self.exc_pc)
        md5.update("%x" % self.faultyaddr)
        md5.update("%d" % self.access)
        self.hashz = md5.hexdigest()

    def __str__(self):
        s = "type: %d, pc: 0x%x, addr: 0x%x, access: %d" % (
            self.exc_type, self.exc_pc, self.faultyaddr, self.access)
        return s

class ExecutionTrace(object):
    """Single execution of a target process."""

    MAP_EXCEPTION_TYPE = {
        bbtrace_pb2.Exception.TYPE_UNKNOWN:   "unknown",
        bbtrace_pb2.Exception.TYPE_VIOLATION: "violation",
    }

    MAP_EXCEPTION_ACCESS = {
        bbtrace_pb2.Exception.ACCESS_UNKNOWN: "unknown",
        bbtrace_pb2.Exception.ACCESS_READ:    "read",
        bbtrace_pb2.Exception.ACCESS_WRITE:   "write",
        bbtrace_pb2.Exception.ACCESS_EXECUTE: "execute",
    }

    def __init__(self, cmdline, tracefile, inputdata = None, deps = None):
        """Constructor for the ExecutionTrace class.

        Keyword arguments:
        cmdline -- process command line (list of arguments).
        tracefile -- name of the trace file (must exist).
        inputdata -- data feed to the program via stdin (can be None).
        deps -- other file names this execution depends on (can be None).
        """
        self.cmdline = cmdline
        self.inputdata = inputdata
        self.deps = set()

        # Ensure all dependecies actually exist
        if deps is not None:
            assert all([os.path.exists(x) for x in deps])
            self.deps |= deps

        # Read data
        self.tracefile = tracefile
        f = open(tracefile, "rb")
        data = f.read()
        f.close()

        # Parse object
        obj = bbtrace_pb2.Trace()
        obj.ParseFromString(data)
        assert obj.header.magic == bbtrace_pb2.TraceHeader.TRACE_MAGIC

        self.timestamp = datetime.datetime.fromtimestamp(obj.header.timestamp)
        self.hashz = "%x" % obj.header.hash

        # Prepare the list of CFG edges observed in this execution
        # NOTE: We ignore the execution order of CFG edges
        self.edges = list(set([(e.prev, e.next, e.hit) for e in obj.edge]))
        self.edges.sort()

        # Create the list of CrashException object, representing exceptions
        # risen during this execution
        self.exceptions = []
        for e in obj.exception:
            exception = CrashException(e, self.timestamp)
            self.exceptions.append(exception)

        # Parse memory-mapped regions
        self.regions = []
        for m in obj.region:
            region = MemoryRegion(m)
            self.regions.append(region)
        self.regions.sort(cmp=lambda a,b: cmp(a.base, b.base))

    @staticmethod
    def exception_type_str(n):
        return ExecutionTrace.MAP_EXCEPTION_TYPE.get(n)

    @staticmethod
    def exception_access_str(n):
        return ExecutionTrace.MAP_EXCEPTION_ACCESS.get(n)

    def purge(self):
        """Deletes all the dependency files of this execution trace."""
        if os.path.isfile(self.tracefile):
            logging.debug("Removing trace file %s", self.tracefile)
            os.unlink(self.tracefile)

        for dep in self.deps:
            logging.debug("Removing dependency file %s", dep)
            os.unlink(dep)

    def is_valid(self):
        """Return True if this execution trace is valid."""
        if not os.path.isfile(self.tracefile):
            return False

        for dep in self.deps:
            if not os.path.isfile(dep):
                return False

        return True

    def update_hash(self):
        """Re-compute the hash value for this execution trace."""
        # Compute the hash as MD5 of CFG edges
        md5 = hashlib.md5()
        for bb_prev, bb_next, hit in self.edges:
            md5.update(struct.pack("L", bb_prev))
            md5.update(struct.pack("L", bb_next))
        self.hashz = md5.hexdigest()

    def __str__(self):
        """Return a concise string representation of this execution trace."""
        s = ("[%s] cmd: %s, data: %d bytes, time: %s, "
             "hash: %s, edges(s): %d, exception(s): %d" %
             (self.tracefile, " ".join(self.cmdline),
              len(self.inputdata) if self.inputdata is not None else 0,
              self.timestamp, self.hashz, len(self.edges),
              len(self.exceptions)))
        return s

    def pretty_print(self):
        """
        Print this Trace object to standard output.

        Compared to __str__(), this method provides a more verbose
        representation of the object.
        """

        print "#### Trace '%s' ####" % self.tracefile
        print trace
        print

        print " - CFG edges"
        for e_prev, e_next, e_hit in trace.edges:
            print " [%08x -> %08x] %d hit" % (e_prev, e_next, e_hit)

        if len(trace.exceptions) > 0:
            print
            print " - Exceptions"
            for exc in trace.exceptions:
                print (" type: %s, pc: 0x%08x, faultyaddr: 0x%08x, "
                       "access: %s" % (
                           ExecutionTrace.exception_type_str(exc.exc_type),
                           exc.exc_pc, exc.faultyaddr,
                           ExecutionTrace.exception_access_str(exc.access)))

        if len(trace.regions) > 0:
            print
            print " - Memory regions"
            for region in trace.regions:
                print " [0x%08x, 0x%08x] %s" % (region.get_lower(),
                                                region.get_upper(),
                                                region.get_name())


if __name__ == "__main__":
    # Parse an execution trace and print it out, optionally performing a "diff"
    # between two traces
    import argparse
    import sys

    # Parse command-line arguments
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-d", "--diff", default=False, action="store_true",
                        help="perform a diff between two trace files")
    parser.add_argument("tracefiles", metavar="TRACE", nargs="+",
                        help="trace files")
    args = parser.parse_args()

    # Parse trace files
    traces = []
    for filename in args.tracefiles:
        trace = ExecutionTrace(["test",], filename)
        traces.append(trace)

    if args.diff:
        assert len(traces) == 2
        print ("#### Comparing %s vs %s ####" %
               (traces[0].tracefile, traces[1].tracefile))
        print

        edge0 = set([(e_prev, e_next) for e_prev, e_next, _ in traces[0].edges])
        edge1 = set([(e_prev, e_next) for e_prev, e_next, _ in traces[1].edges])

        for filename, setz in (
                (traces[0].tracefile, edge0 - edge1),
                (traces[1].tracefile, edge1 - edge0),
        ):
            print "- Only in %s, %d edge(s)" % (filename, len(setz))
            edges = list(setz)
            edges.sort()
            for edge_prev, edge_next in edges:
                print "- 0x%08x -> 0x%08x" % (edge_prev, edge_next)
            print

    else:
        # Just print traces to stdout
        for trace in traces:
            trace.pretty_print()
