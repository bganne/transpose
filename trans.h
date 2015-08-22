#include <stddef.h>

#define TRANS__(x)      trans_ ## x
#define TRANS_(x)       TRANS__(x)
#define TRANS(A, B)     TRANS_(FLAVOUR) ((A), (B))

#define trans_INPLACE__(n, m, t, u)     transpose_ ## t ## x ## u ## _ ## n ## x ## m
#define trans_INPLACE_(n, m, t, u)      trans_INPLACE__(n, m, t, u)
#define trans_INPLACE(A, B)             trans_INPLACE_(N, M, TYPE, UNROLL) ((A))

#define MIN(a, b)       ((a) < (b) ? (a) : (b))

#define UNROLL_N        MIN(UNROLL, N)
#define UNROLL_M        MIN(UNROLL, M)
#define UNROLL_NM       MIN(UNROLL, N*M)

static inline void trans_REF(const TYPE (*restrict A)[M], TYPE (*restrict B)[N])
{
    for (size_t i=0; i<N; i++) {
        for (size_t j=0; j<M; j++) {
            B[j][i] = A[i][j];
        }
    }
}

#pragma GCC push_options
#pragma GCC optimize ("unroll-loops")
static inline void trans_REF_UNROLL__(const TYPE (*restrict A)[M], TYPE (*restrict B)[N],
        size_t i, size_t j, size_t unroll)
{
    for (size_t k=0; k<unroll; k++) {
        B[j+k][i] = A[i][j+k];
    }
}
#pragma GCC pop_options

static inline void trans_REF_UNROLL(const TYPE (*restrict A)[M], TYPE (*restrict B)[N])
{
    for (size_t i=0; i<N; i++) {
        size_t j;
        for (j=0; j<=M-UNROLL_M; j+=UNROLL_M) {
            trans_REF_UNROLL__(A, B, i, j, UNROLL_M);
        }
        trans_REF_UNROLL__(A, B, i, j, M%UNROLL_M);
    }
}

#pragma GCC push_options
#pragma GCC optimize ("unroll-loops")
static inline void trans_REF_UNROLL_AND_JAM__(const TYPE (*restrict A)[M], TYPE (*restrict B)[N],
        size_t i, size_t j, size_t unroll)
{
    for (size_t k=0; k<unroll; k++) {
        B[j][i+k] = A[i+k][j];
    }
}
#pragma GCC pop_options

static inline void trans_REF_UNROLL_AND_JAM(const TYPE (*restrict A)[M], TYPE (*restrict B)[N])
{
    size_t i;
    for (i=0; i<=N-UNROLL_N; i+=UNROLL_N) {
        for (size_t j=0; j<M; j++) {
            trans_REF_UNROLL_AND_JAM__(A, B, i, j, UNROLL_N);
        }
    }
    for (size_t j=0; j<M; j++) {
        trans_REF_UNROLL_AND_JAM__(A, B, i, j, N%UNROLL_N);
    }
}

static inline void trans_LINEAR(const TYPE *restrict A, TYPE *restrict B)
{
    for (size_t i=0; i<N*M; i++) {
        B[(i % M) * N + i / M] = A[i]; 
    }
}

#pragma GCC push_options
#pragma GCC optimize ("unroll-loops")
static inline void trans_LINEAR_UNROLL__(const TYPE *restrict A, TYPE *restrict B,
        size_t i, size_t unroll)
{
    for (size_t k=0; k<unroll; k++) {
        B[((i+k) % M) * N + (i+k) / M] = A[i+k]; 
    }
}
#pragma GCC pop_options

static inline void trans_LINEAR_UNROLL(const TYPE *restrict A, TYPE *restrict B)
{
    size_t i;
    for (i=0; i<=N*M-UNROLL_NM; i+=UNROLL_NM) {
        trans_LINEAR_UNROLL__(A, B, i, UNROLL_NM);
    }
    trans_LINEAR_UNROLL__(A, B, i, (N*M)%UNROLL_NM);
}
