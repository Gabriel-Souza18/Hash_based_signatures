#ifndef LTREE_H
#define LTREE_H

/*
 * L-tree: comprime os L=67 vetores de N=32 bytes da chave pública WOTS+
 * em uma única raiz de 32 bytes.
 *
 * É uma árvore de Merkle de tamanho variável sobre L folhas,
 * usando a função H com ADRS do tipo WOTS_PK (type=1).
 *
 * Referência: SPHINCS paper, Seção 3 / FIPS 205, Seção 6.
 */

#define LTREE_N  32          /* tamanho do hash em bytes (SHA-256)          */
#define LTREE_W  16          /* parâmetro Winternitz                         */
#define LTREE_L1 64          /* ceil(8*N / log2(W)) = 64                    */
#define LTREE_L2  3          /* floor(log2(L1*(W-1)) / log2(W)) + 1 = 3     */
#define LTREE_L  (LTREE_L1 + LTREE_L2)  /* = 67 entradas na PK WOTS+       */

/*
 * H_function — hash de dois filhos (nós internos da L-tree e da árvore XMSS)
 *
 * H(PK_seed, ADRS, left || right) =
 *   SHA-256(PK_seed || zeros(32) || ADRSc(22) || left(32) || right(32))
 *
 * Parâmetros:
 *   out     — saída de 32 bytes
 *   PK_seed — semente pública de 32 bytes
 *   adrs    — ADRS de 32 bytes (identifica posição na árvore)
 *   left    — filho esquerdo (32 bytes)
 *   right   — filho direito  (32 bytes)
 */
void H_function(unsigned char out[LTREE_N],
                const unsigned char PK_seed[LTREE_N],
                const unsigned char adrs[32],
                const unsigned char left[LTREE_N],
                const unsigned char right[LTREE_N]);

/*
 * ltree_compress — comprime pk[L][N] → root[N]
 *
 * Parâmetros:
 *   root    — saída: raiz da L-tree (32 bytes)
 *   pk      — entrada: L=67 vetores de N=32 bytes (PK completa do WOTS+)
 *   PK_seed — semente pública (32 bytes)
 *   layer   — camada da hiper-árvore SPHINCS (para o ADRS)
 *   tree    — índice da árvore na camada (para o ADRS)
 *   keypair — índice da folha/par de chaves na árvore (para o ADRS)
 */
void ltree_compress(unsigned char root[LTREE_N],
                    const unsigned char pk[LTREE_L][LTREE_N],
                    const unsigned char PK_seed[LTREE_N],
                    unsigned int layer,
                    unsigned long long tree,
                    unsigned int keypair);

#endif /* LTREE_H */
