#include "ltree.h"
#include "sphincs_address.h"
#include "../SHA256/sha256.h"

#include <string.h>

/*
 * H_function — hash de dois filhos concatenados (nós internos da L-tree e XMSS)
 *
 * H(PK_seed, ADRS, left || right) =
 *   SHA-256( PK_seed || zeros(32) || ADRSc(22) || left(N) || right(N) )
 *
 * Tamanho total da entrada: 32 + 32 + 22 + 32 + 32 = 150 bytes
 *
 * Referência: FIPS 205, Seção 11.2.2 (Hmsg para SHA2-256s)
 */
void H_function(unsigned char out[LTREE_N],
                const unsigned char PK_seed[LTREE_N],
                const unsigned char adrs[32],
                const unsigned char left[LTREE_N],
                const unsigned char right[LTREE_N])
{
    unsigned char buf[32 + 32 + 22 + 32 + 32]; /* 150 bytes */
    unsigned char adrs_c[22];
    int pos = 0;

    /* 1. PK_seed (32 bytes) */
    memcpy(buf + pos, PK_seed, LTREE_N);  pos += LTREE_N;

    /* 2. Padding de zeros (32 bytes) — para alinhar ao bloco SHA-256 */
    memset(buf + pos, 0, 32);             pos += 32;

    /* 3. ADRS comprimido (22 bytes) */
    adrs_compress(adrs, adrs_c);
    memcpy(buf + pos, adrs_c, 22);        pos += 22;

    /* 4. Filho esquerdo || Filho direito (32 + 32 = 64 bytes) */
    memcpy(buf + pos, left,  LTREE_N);    pos += LTREE_N;
    memcpy(buf + pos, right, LTREE_N);    pos += LTREE_N;

    sha256_bytes(buf, pos, out);
}

/*
 * ltree_compress — comprime os L=67 vetores PK do WOTS+ em uma raiz de 32 bytes
 *
 * Algoritmo:
 *   - Copia pk[0..L-1] numa camada temporária (buffer local)
 *   - A cada nível, pares de nós vizinhos são combinados com H_function
 *   - Se o número de nós é ímpar, o último sobe diretamente (sem hash)
 *   - Termina quando resta apenas 1 nó: a raiz
 *
 * O ADRS muda a cada nível (campo "height") e a cada par (campo "index"),
 * garantindo que nenhum hash seja reutilizado em posições diferentes.
 *
 * Referência: SPHINCS paper, Seção 3 — "compress PK via L-tree"
 */
void ltree_compress(unsigned char root[LTREE_N],
                    const unsigned char pk[LTREE_L][LTREE_N],
                    const unsigned char PK_seed[LTREE_N],
                    unsigned int layer,
                    unsigned long long tree,
                    unsigned int keypair)
{
    /*
     * Buffer local: começa com L=67 nós, vai sendo reduzido pela metade
     * a cada nível até restar 1 nó (a raiz).
     * Usamos LTREE_L como tamanho máximo (67 nós × 32 bytes).
     */
    unsigned char buf[LTREE_L][LTREE_N];
    int n_nodes = LTREE_L;  /* número de nós no nível atual */
    unsigned char adrs[32];

    /* Copia as PKs do WOTS+ para o buffer local */
    for (int i = 0; i < LTREE_L; i++) {
        memcpy(buf[i], pk[i], LTREE_N);
    }

    /* height = nível atual da L-tree (0 = folhas, sobe até restar 1 nó) */
    for (unsigned int height = 0; n_nodes > 1; height++) {

        int next_nodes = n_nodes / 2;   /* pares que serão combinados */

        for (int i = 0; i < next_nodes; i++) {
            /* Monta ADRS para este nó: (layer, tree, keypair, height, index=i) */
            adrs_for_ltree(adrs, layer, tree, keypair, height, (unsigned int)i);

            /* H(buf[2i] || buf[2i+1]) → buf[i] */
            H_function(buf[i], PK_seed, adrs, buf[2 * i], buf[2 * i + 1]);
        }

        /* Se n_nodes é ímpar, o último nó sobe sem ser combinado */
        if (n_nodes % 2 == 1) {
            memcpy(buf[next_nodes], buf[n_nodes - 1], LTREE_N);
            next_nodes++;
        }

        n_nodes = next_nodes;
    }

    /* O único nó restante é a raiz */
    memcpy(root, buf[0], LTREE_N);
}
