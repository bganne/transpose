#include <stdio.h>
#include <stddef.h>
#include "trans.h"

TYPE A[N][M];
TYPE B[M][N];

static void init(TYPE A[N][M], TYPE B[M][N])
{
    for (size_t i=0; i<N; i++) {
        for (size_t j=0; j<M; j++) {
            A[i][j] = i * M + j;
            B[j][i] = -1;
        }
    }
}

static void check(const char *str, const TYPE B[M][N])
{
    for (size_t j=0; j<M; j++) {
        for (size_t i=0; i<N; i++) {
            const TYPE exp = i * M + j;
            if (exp != B[j][i]) {
                fprintf(stderr, "%s: B[%zu][%zu] == %g != %g\n",
                        str, j, i, (double)B[j][i], exp);
            }
        }
    }
}

int main(void)
{
#define CHECK_(f, a, b, c) \
    do { \
        void *A_ = (a), *B_ = (b), *C_ = (c); \
        init(A_, B_); \
        f(A_, B_); \
        check(# f, C_); \
    } while(0)
#define CHECK(f, a, b)  CHECK_(f, (a), (b), (b))

    CHECK(trans_REF, A, B);
    CHECK(trans_REF_UNROLL, A, B);
    CHECK(trans_UNROLL_AND_JAM, A, B);
    CHECK(trans_LINEAR, A, B);
    CHECK(trans_LINEAR_UNROLL, A, B);
    CHECK_(trans_INPLACE, A, B, A);

    return 0;
}
