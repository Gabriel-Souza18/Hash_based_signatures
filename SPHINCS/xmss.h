#ifndef XMSS_H
#define XMSS_H

/*
 * XMSS — uma camada da hiper-árvore do SPHINCS
 *
 * Cada árvore XMSS tem altura XMSS_H_LAYER = SPHINCS_H / SPHINCS_D folhas.
 * Cada folha é a raiz de uma L-tree de uma chave pública WOTS+.
 * Os nós internos são calculados com H_function (type=TREE).
 *
 * Referência: SPHINCS paper, Seção 3 / FIPS 205, Seção 6.
 */

#include <stdint.h>
#include "ltree.h"           /* LTREE_N, LTREE_L, H_function          */
#include "sphincs_address.h" /* adrs_for_wots_prf, adrs_for_tree, ... */

/* ── Parâmetros SPHINCS-256 ─────────────────────────────────────────────── */

#define XMSS_N          32   /* tamanho do hash em bytes (SHA-256)            */
#ifndef SPHINCS_H
#define SPHINCS_H       60   /* altura total da hiper-árvore                  */
#endif

#ifndef SPHINCS_D
#define SPHINCS_D       12   /* número de camadas (árvores XMSS)              */
#endif
#define XMSS_H_LAYER    (SPHINCS_H / SPHINCS_D)  /* = 5 folhas por árvore    */
#define XMSS_LEAVES     (1 << XMSS_H_LAYER)      /* = 32 folhas por árvore   */

/* ── Estruturas ─────────────────────────────────────────────────────────── */

/*
 * XMSSSignature — assinatura produzida por uma árvore XMSS
 *
 *   wots_sig : assinatura WOTS+ da mensagem (ou raiz da árvore abaixo)
 *   auth     : caminho de autenticação da folha usada até a raiz
 */
typedef struct {
    unsigned char wots_sig[LTREE_L][XMSS_N];   /* assinatura WOTS+          */
    unsigned char auth[XMSS_H_LAYER][XMSS_N];  /* caminho de autenticação   */
} XMSSSignature;

/* ── API ────────────────────────────────────────────────────────────────── */

/*
 * xmss_leaf — calcula a folha leaf_idx de uma árvore XMSS.
 *
 * Gera a chave WOTS+ da folha via PRF(SK_seed, ADRS),
 * computa sua chave pública e comprime com ltree_compress.
 *
 *   out      — raiz da L-tree (= folha XMSS), 32 bytes
 *   SK_seed  — semente secreta global (32 bytes)
 *   PK_seed  — semente pública global  (32 bytes)
 *   layer    — camada da hiper-árvore (0 .. SPHINCS_D-1)
 *   tree     — índice da árvore nessa camada
 *   leaf_idx — índice da folha dentro da árvore (0 .. XMSS_LEAVES-1)
 */
void xmss_leaf(unsigned char out[XMSS_N],
               const unsigned char SK_seed[XMSS_N],
               const unsigned char PK_seed[XMSS_N],
               uint32_t layer,
               uint64_t tree,
               uint32_t leaf_idx);

/*
 * xmss_root — calcula a raiz de uma árvore XMSS.
 *
 * Constrói todas as XMSS_LEAVES folhas e a árvore completa bottom-up.
 * A raiz é a chave pública dessa camada.
 *
 *   root     — saída: raiz da árvore, 32 bytes
 *   SK_seed  — semente secreta global
 *   PK_seed  — semente pública global
 *   layer    — camada da hiper-árvore
 *   tree     — índice da árvore nessa camada
 */
void xmss_root(unsigned char root[XMSS_N],
               const unsigned char SK_seed[XMSS_N],
               const unsigned char PK_seed[XMSS_N],
               uint32_t layer,
               uint64_t tree);

/*
 * xmss_sign — assina msg com a folha leaf_idx e coleta o caminho de auth.
 *
 *   sig      — saída: XMSSSignature preenchida
 *   msg      — mensagem a assinar (32 bytes — hash de algo ou raiz abaixo)
 *   SK_seed  — semente secreta global
 *   PK_seed  — semente pública global
 *   layer    — camada da hiper-árvore
 *   tree     — índice da árvore nessa camada
 *   leaf_idx — folha a usar (0 .. XMSS_LEAVES-1)
 */
void xmss_sign(XMSSSignature *sig,
               const unsigned char msg[XMSS_N],
               const unsigned char SK_seed[XMSS_N],
               const unsigned char PK_seed[XMSS_N],
               uint32_t layer,
               uint64_t tree,
               uint32_t leaf_idx);

/*
 * xmss_verify — reconstrói a raiz a partir de uma XMSSSignature.
 *
 * Não retorna pass/fail: apenas reconstrói a raiz.
 * O chamador compara root_out com a raiz conhecida.
 *
 *   root_out — saída: raiz reconstruída (32 bytes)
 *   msg      — mensagem que foi assinada
 *   sig      — assinatura XMSS
 *   PK_seed  — semente pública global
 *   layer    — camada da hiper-árvore
 *   tree     — índice da árvore nessa camada
 *   leaf_idx — folha que foi usada
 */
void xmss_verify(unsigned char root_out[XMSS_N],
                 const unsigned char msg[XMSS_N],
                 const XMSSSignature *sig,
                 const unsigned char PK_seed[XMSS_N],
                 const unsigned char SK_seed[XMSS_N],
                 uint32_t layer,
                 uint64_t tree,
                 uint32_t leaf_idx);

#endif /* XMSS_H */
