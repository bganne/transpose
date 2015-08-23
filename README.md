# Non-square matrix transpose (NxM)

Various experiments with non-square matrix transpose (NxM), including in-place
transpose.
If interested, more in-depth analysis available here: http://www.benou.fr/~ben/non-square-matrix-in-place-transpose.html

The input matrix 'A' is defined as a NxM matrix in row-major (C array semantic,
A[N][M]).
The output matrix 'B' is obviously a MxN matrix (B[M][N]).

The algorithms implemented are:
-	REF: naive, out-of-place
-	UNROLL: naive, out-of-place, inner loop unrolled
-	UNROLL-AND-JAM: naive, out-of-place, outer loop unrolled
-	BLOCK: cache-blocking, out-of-place, inner loop unrolled
-	LINEAR: instead of using 2 loops over N and M, use a single loop over N*M, out-of-place
-	LINEAR_UNROLLED: same as LINEAR but loop unrolled
-	INPLACE: in-place transpose

To build:

	make

To run sanity checks:

	make check

To run performance measures:

	make perf

You can control several parameters of the various implemented algorithms
through make variables (default values in parenthesis):
-	N (256): the matrix column number
-	M (2): the matrix row number
-	TYPE (float): type of the elements
-	UNROLL (4): unrolling factor
-	BLOCK_N (256): column blocking factor
-	BLOCK_M (8): row blocking factor
-	ROUND (100): number of time to loop over a test to measure performance

For example, to measure performance of a 4096x128 matrix transpose with
default settings but a column blocking factor of 64:

	make N=4096 M=128 BLOCK_N=64 perf
