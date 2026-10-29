#if defined(BENCH_MSS)

#define N WOTS_N
#define main mss_cli_main
#include "../../../../MSS/Arvore.c"
#include "../../../../MSS/remet_mss.c"
#undef main

No *mssBenchmarkTree(Folha *folhas) {
    return mssTree(folhas);
}

#endif
