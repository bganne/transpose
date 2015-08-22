#include <stddef.h>
#include "trans.h"

TYPE A[N][M];
TYPE B[M][N];

int main(void)
{
    for (size_t i=0; i<ROUND; i++) {
        TRANS((void *)A, (void *)B);
    }
    return 0;
}
