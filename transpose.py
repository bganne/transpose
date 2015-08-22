#!/usr/bin/python
import sys
import collections

def check(N, M, cycles):
    try:
        import numpy as np
    except:
        print>>sys.stderr, "Failed to import numpy, checks skipped."
        return
    A = np.arange(N*M)
    A_ref = np.copy(A).reshape(N,M).transpose()
    for clen, clist in cycles:
        for n in clist:
            cur = A[n]
            for i in range(clen):
                ni = (n % M) * N + n / M
                tmp = A[ni]
                A[ni] = cur
                cur = tmp
                n = ni
    A = A.reshape(M,N)
    assert(np.array_equal(A_ref, A))

try:
	N, M, ctype, unroll = sys.argv[1:]
except:
	print>>sys.stderr, "Usage: %s N M type unroll" % sys.argv[0]

N, M, UNROLL = int(N), int(M), int(unroll)

A = bytearray(N*M)
cycles = collections.defaultdict(list)

A[0] = 1
A[N*M-1] = 1
try:
    while True:
        n = A.index('\x00')
        start = n
        clen = 0
        while A[n] != 1:
            clen += 1
            A[n] = 1
            n = (n % M) * N + n / M
        cycles[clen].append(start)
except ValueError: pass # A.index(0) will raise ValueError when not found

# sort cycles by clen, then by smallest offset
cycles = sorted(((clen, clist) for clen, clist in cycles.iteritems()))

check(N, M, cycles)

def index_type(max_index):
	if max_index < (1<<8): return "uint8_t"
	elif max_index < (1<<16): return "uint16_t"
	elif max_index < (1<<32): return "uint32_t"
	else: return "size_t"

max_index = max((clist[-1] for clen, clist in cycles))
STORAGE_INDEX_TYPE=index_type(max_index)

def unroll1(TAB, clen, unroll, cycle_index, ctype, M, N, start = ""):
    print TAB + "size_t n[%i] = { %s };" % (unroll, ", ".join(("cycle%i[%s]" % (clen, start + cycle_index % i) for i in range(unroll))))
    print TAB + "%s cur[%i] = { %s };" % (ctype, unroll, ", ".join(("A[n[%i]]" % i for i in range(unroll))))
    print TAB + "for (size_t j=0; j<%i; j++) {" % clen
    TAB += " " * 4
    print TAB + "size_t ni[%i] = { %s };" % (unroll, ", ".join(("(n[%i] %% %i) * %i + n[%i] / %i" % (i, M, N, i, M) for i in range(unroll))))
    print TAB + "%s tmp[%i] = { %s };" % (ctype, unroll, ", ".join(("A[ni[%i]]" % i for i in range(unroll))))
    print "\n".join((TAB + "A[ni[%i]] = cur[%i]" % (i,i) + ";" for i in range(unroll)))
    print "\n".join((TAB + "cur[%i] = tmp[%i];" % (i,i) for i in range(unroll)))
    print "\n".join((TAB + "n[%i] = ni[%i];" % (i,i) for i in range(unroll)))
    TAB = " " * (len(TAB)-4);
    print TAB + "}"
    TAB = " " * (len(TAB)-4);
    print TAB + "}"
    return TAB


TAB=""
print TAB + "#ifndef TRANSPOSE_%sx%i_%ix%i_H__" % (ctype, UNROLL, N, M)
print TAB + "#define TRANSPOSE_%sx%i_%ix%i_H__" % (ctype, UNROLL, N, M)
print
print "#include <stddef.h>"
print "#include <stdint.h>"
print

print TAB + "static inline void transpose_%sx%i_%ix%i(%s *A)" % (ctype, UNROLL, N, M, ctype)
print TAB + "{"
TAB += " " * 4
for clen, clist in cycles:
    print TAB + "static const %s cycle%i[] = { %s };" % (STORAGE_INDEX_TYPE, clen, ", ".join((str(i) for i in clist)))
print
for clen, clist in cycles:
    if len(clist) > UNROLL:
        unroll = UNROLL
        cycle_index = "i+%i"
        print TAB + "for (size_t i=0; i<=%i-%i; i+=%i) {" % (len(clist),unroll,unroll)
    else:
        unroll = len(clist)
        cycle_index = "%i"
        print TAB + "{"
    TAB += " " * 4
    print TAB + "/* cycles of len %i (cf. cycle%i) */" % (clen, clen)
    TAB = unroll1(TAB, clen, unroll, cycle_index, ctype, M, N)
    if len(clist) > UNROLL and 0 != len(clist) % UNROLL:
        print TAB + "{"
        TAB += " " * 4
        print TAB + "/* take care of remaining for cycles of len %i (cf. cycle%i) */" % (clen, clen)
        TAB = unroll1(TAB, clen, len(clist) % UNROLL, "%i", ctype, M, N, "(%i-%i+1)+" % (len(clist), unroll))
TAB = " " * (len(TAB)-4);
print TAB + "}"
print
print "#endif    /* TRANSPOSE_%sx%i_%ix%i_H__ */" % (ctype, UNROLL, N, M)
