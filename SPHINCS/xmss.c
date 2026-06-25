#include "xmss.h"
#include "ltree.h"
#include "sphincs_address.h"
#include "../WOTS/keys.h"
#include "../WOTS/prf.h"
#include "../SHA256/sha256.h"

#include <string.h>
#include <stdlib.h>

/* ── Funções internas ───────────────────────────────────────────────────── */

/*
 * wots_gen_pk — gera a chave pública WOTS+ de uma folha via PRF + chain.
 *
 * Usa as funções já existentes em WOTS/keys.c, adaptadas para aceitar
 * SK_seed e PK_seed externos e o ADRS correto da hiper-árvore.
 *
 *   pk       — saída: L×N bytes da chave pública WOTS+
 *   SK_seed  — semente secreta global
 *   PK_seed  — semente pública global
 *   layer    — camada da hiper-árvore
 *   tree     — índice da árvore
 *   leaf_idx — índice da folha / par de chaves
 */
static void wots_gen_pk(unsigned char pk[LTREE_L][XMSS_N],
                        const unsigned char SK_seed[XMSS_N],
                        const unsigned char PK_seed[XMSS_N],
                        uint32_t layer, uint64_t tree, uint32_t leaf_idx)
{
    unsigned char adrs[32];
    unsigned char sk_i[XMSS_N];

    for (int i = 0; i < LTREE_L; i++) {
        /* 1. Gera SK[i] via PRF com ADRS tipo WOTS_PRF */
        adrs_for_wots_prf(adrs, layer, tree, leaf_idx, (uint32_t)i);
        PRF_SHA2(sk_i, PK_seed, SK_seed, adrs, XMSS_N);

        /* 2. Prepara ADRS tipo WOTS_HASH para a chain function */
        adrs_for_wots_hash(adrs, layer, tree, leaf_idx, (uint32_t)i, 0);

        /* 3. Deriva W-1 máscaras para este elemento */
        unsigned char masks[LTREE_W - 1][XMSS_N];
        for (int step = 0; step < LTREE_W - 1; step++) {
            unsigned char mask_adrs[32];
            /* Reutiliza adrs_for_wots_hash com hash_idx = step para máscaras */
            adrs_for_wots_hash(mask_adrs, layer, tree, leaf_idx,
                               (uint32_t)i, (uint32_t)step);
            PRF_SHA2(masks[step], PK_seed, SK_seed, mask_adrs, XMSS_N);
        }

        /* 4. Aplica chain function W-1 vezes: SK[i] → PK[i] */
        chainFunctionWOTSplus(sk_i, LTREE_W - 1,
                              pk[i], adrs, 0, masks);
    }
}

/*
 * wots_sign_msg — assina msg com a chave WOTS+ da folha leaf_idx.
 *
 *   sig      — saída: L×N bytes da assinatura WOTS+
 *   msg      — mensagem de N bytes a assinar
 *   SK_seed, PK_seed, layer, tree, leaf_idx — contexto
 */
static void wots_sign_msg(unsigned char sig[LTREE_L][XMSS_N],
                          const unsigned char msg[XMSS_N],
                          const unsigned char SK_seed[XMSS_N],
                          const unsigned char PK_seed[XMSS_N],
                          uint32_t layer, uint64_t tree, uint32_t leaf_idx)
{
    unsigned char adrs[32];
    unsigned char sk_i[XMSS_N];

    /* Converte msg para vetor de índices base-W (mensagem + checksum) */
    int b[LTREE_L];
    {
        int blocks[LTREE_L1], check[LTREE_L2];
        mensageForBlocks(msg, blocks);
        calcularChecksum(blocks, check);
        for (int i = 0; i < LTREE_L1; i++) b[i] = blocks[i];
        for (int i = 0; i < LTREE_L2; i++) b[LTREE_L1 + i] = check[i];
    }

    for (int i = 0; i < LTREE_L; i++) {
        /* Gera SK[i] */
        adrs_for_wots_prf(adrs, layer, tree, leaf_idx, (uint32_t)i);
        PRF_SHA2(sk_i, PK_seed, SK_seed, adrs, XMSS_N);

        /* ADRS para chain */
        adrs_for_wots_hash(adrs, layer, tree, leaf_idx, (uint32_t)i, 0);

        /* Máscaras */
        unsigned char masks[LTREE_W - 1][XMSS_N];
        for (int step = 0; step < LTREE_W - 1; step++) {
            unsigned char mask_adrs[32];
            adrs_for_wots_hash(mask_adrs, layer, tree, leaf_idx,
                               (uint32_t)i, (uint32_t)step);
            PRF_SHA2(masks[step], PK_seed, SK_seed, mask_adrs, XMSS_N);
        }

        /* Chain b[i] passos */
        chainFunctionWOTSplus(sk_i, b[i], sig[i], adrs, 0, masks);
    }
}

/*
 * wots_pk_from_sig — reconstrói a PK WOTS+ a partir de uma assinatura.
 *
 * Aplica os passos restantes (W-1-b[i]) da chain function.
 *
 *   pk   — saída: L×N bytes da PK reconstruída
 *   sig  — assinatura WOTS+ (L×N bytes)
 *   msg  — mensagem original (N bytes)
 *   PK_seed, layer, tree, leaf_idx — contexto
 */
static void wots_pk_from_sig(unsigned char pk[LTREE_L][XMSS_N],
                             const unsigned char sig[LTREE_L][XMSS_N],
                             const unsigned char msg[XMSS_N],
                             const unsigned char PK_seed[XMSS_N],
                             const unsigned char SK_seed[XMSS_N],
                             uint32_t layer, uint64_t tree, uint32_t leaf_idx)
{
    unsigned char adrs[32];

    int b[LTREE_L];
    {
        int blocks[LTREE_L1], check[LTREE_L2];
        mensageForBlocks(msg, blocks);
        calcularChecksum(blocks, check);
        for (int i = 0; i < LTREE_L1; i++) b[i] = blocks[i];
        for (int i = 0; i < LTREE_L2; i++) b[LTREE_L1 + i] = check[i];
    }

    for (int i = 0; i < LTREE_L; i++) {
        int remaining = LTREE_W - 1 - b[i];

        adrs_for_wots_hash(adrs, layer, tree, leaf_idx, (uint32_t)i, 0);

        unsigned char masks[LTREE_W - 1][XMSS_N];
        for (int step = 0; step < LTREE_W - 1; step++) {
            unsigned char mask_adrs[32];
            adrs_for_wots_hash(mask_adrs, layer, tree, leaf_idx,
                               (uint32_t)i, (uint32_t)step);
            PRF_SHA2(masks[step], PK_seed, SK_seed, mask_adrs, XMSS_N);
        }

        if (remaining > 0) {
            chainFunctionWOTSplus((unsigned char *)sig[i], remaining,
                                  pk[i], adrs, b[i], masks);
        } else {
            memcpy(pk[i], sig[i], XMSS_N);
        }
    }
}

/* ── API pública ────────────────────────────────────────────────────────── */

/*
 * xmss_leaf — calcula a folha leaf_idx de uma árvore XMSS.
 *
 * Processo:
 *   1. Gera PK WOTS+ da folha (wots_gen_pk)
 *   2. Comprime com L-tree (ltree_compress) → folha de 32 bytes
 */
void xmss_leaf(unsigned char out[XMSS_N],
               const unsigned char SK_seed[XMSS_N],
               const unsigned char PK_seed[XMSS_N],
               uint32_t layer, uint64_t tree, uint32_t leaf_idx)
{
    unsigned char pk[LTREE_L][XMSS_N];
    wots_gen_pk(pk, SK_seed, PK_seed, layer, tree, leaf_idx);
    ltree_compress(out, (const unsigned char (*)[XMSS_N])pk,
                   PK_seed, layer, tree, leaf_idx);
}

/*
 * xmss_root — calcula a raiz de uma árvore XMSS de altura XMSS_H_LAYER.
 *
 * Processo:
 *   1. Calcula todas as XMSS_LEAVES folhas
 *   2. Constrói a árvore de Merkle bottom-up com H_function (type=TREE)
 *   3. Retorna a raiz
 *
 * A árvore interna é armazenada em nodes[2*XMSS_LEAVES - 1][XMSS_N]:
 *   - nodes[0]              = raiz
 *   - nodes[leaf_offset + i] = folha i
 */
void xmss_root(unsigned char root[XMSS_N],
               const unsigned char SK_seed[XMSS_N],
               const unsigned char PK_seed[XMSS_N],
               uint32_t layer, uint64_t tree)
{
    int num_nodes   = 2 * XMSS_LEAVES - 1;
    int leaf_offset = XMSS_LEAVES - 1;

    /* Aloca array de nós na stack (XMSS_LEAVES=32, num_nodes=63, 32B cada = 2016 B) */
    unsigned char nodes[2 * XMSS_LEAVES - 1][XMSS_N];

    /* 1. Calcula as folhas */
    for (int i = 0; i < XMSS_LEAVES; i++) {
        xmss_leaf(nodes[leaf_offset + i], SK_seed, PK_seed,
                  layer, tree, (uint32_t)i);
    }

    /* 2. Constrói nós internos bottom-up */
    for (int i = leaf_offset - 1; i >= 0; i--) {
        unsigned char adrs[32];

        /* Altura e índice do nó na árvore de Merkle
         * Na representação com raiz em 0:
         *   altura h satisfaz: leaf_offset >> h == i >> (h == 0 ? 0 : h)
         * Forma simples: contar quantos bits tem (i+1) */
        uint32_t h_idx = 0, pos = (uint32_t)i;
        /* nó i tem filhos 2i+1 e 2i+2.
         * A altura de i em relação às folhas: 
         *   leaf_offset = XMSS_LEAVES - 1
         *   um nó no nível k (contando folhas=0) tem índice [XMSS_LEAVES/2^(k+1)-1 .. XMSS_LEAVES/2^k - 2]
         * Forma direta: height = floor(log2(leaf_offset - i + 1))
         * Porém o mais simples é calcular pelo offset relativo ao nível */
        {
            int level_start = 0;
            int level_size  = 1;          /* raiz: 1 nó no nível 0 */
            int cur_height  = XMSS_H_LAYER; /* raiz está no topo   */
            while (level_start + level_size <= i) {
                level_start += level_size;
                level_size  *= 2;
                cur_height--;
            }
            h_idx    = (uint32_t)cur_height;
            pos      = (uint32_t)(i - level_start);
        }

        adrs_for_tree(adrs, layer, tree, h_idx, pos);

        H_function(nodes[i], PK_seed, adrs,
                   nodes[2 * i + 1], nodes[2 * i + 2]);
    }

    memcpy(root, nodes[0], XMSS_N);
    (void)num_nodes; /* suprime warning de variável não usada */
}

/*
 * xmss_sign — assina msg com a folha leaf_idx.
 *
 * Processo:
 *   1. Assina msg com WOTS+ → wots_sig
 *   2. Reconstrói a árvore e coleta o caminho de autenticação da folha
 */
void xmss_sign(XMSSSignature *sig,
               const unsigned char msg[XMSS_N],
               const unsigned char SK_seed[XMSS_N],
               const unsigned char PK_seed[XMSS_N],
               uint32_t layer, uint64_t tree, uint32_t leaf_idx)
{
    /* 1. Assinatura WOTS+ */
    wots_sign_msg(sig->wots_sig, msg, SK_seed, PK_seed,
                  layer, tree, leaf_idx);

    /* 2. Reconstrói a árvore para coletar o caminho de autenticação */
    unsigned char nodes[2 * XMSS_LEAVES - 1][XMSS_N];
    int leaf_offset = XMSS_LEAVES - 1;

    for (int i = 0; i < XMSS_LEAVES; i++) {
        xmss_leaf(nodes[leaf_offset + i], SK_seed, PK_seed,
                  layer, tree, (uint32_t)i);
    }

    for (int i = leaf_offset - 1; i >= 0; i--) {
        unsigned char adrs[32];
        uint32_t h_idx = 0, pos = 0;
        {
            int level_start = 0, level_size = 1;
            int cur_height  = XMSS_H_LAYER;
            while (level_start + level_size <= i) {
                level_start += level_size;
                level_size  *= 2;
                cur_height--;
            }
            h_idx = (uint32_t)cur_height;
            pos   = (uint32_t)(i - level_start);
        }
        adrs_for_tree(adrs, layer, tree, h_idx, pos);
        H_function(nodes[i], PK_seed, adrs,
                   nodes[2 * i + 1], nodes[2 * i + 2]);
    }

    /* 3. Coleta caminho de autenticação (irmãos do caminho leaf → raiz) */
    int node = leaf_offset + (int)leaf_idx;
    for (int lvl = 0; lvl < XMSS_H_LAYER; lvl++) {
        int sibling = (node % 2 == 0) ? node - 1 : node + 1;
        memcpy(sig->auth[lvl], nodes[sibling], XMSS_N);
        node = (node - 1) / 2;
    }
}

/*
 * xmss_verify — reconstrói a raiz a partir de msg + XMSSSignature.
 *
 * Processo:
 *   1. Reconstrói PK WOTS+ a partir de wots_sig (passos restantes da chain)
 *   2. Comprime PK com L-tree → folha reconstruída
 *   3. Sobe a árvore usando o caminho de autenticação → raiz
 */
void xmss_verify(unsigned char root_out[XMSS_N],
                 const unsigned char msg[XMSS_N],
                 const XMSSSignature *sig,
                 const unsigned char PK_seed[XMSS_N],
                 const unsigned char SK_seed[XMSS_N],
                 uint32_t layer, uint64_t tree, uint32_t leaf_idx)
{
    /* 1. Reconstrói PK WOTS+ */
    unsigned char pk[LTREE_L][XMSS_N];
    wots_pk_from_sig(pk,
                     (const unsigned char (*)[XMSS_N])sig->wots_sig,
                     msg, PK_seed, SK_seed, layer, tree, leaf_idx);

    /* 2. Comprime com L-tree → folha reconstruída */
    unsigned char node[XMSS_N];
    ltree_compress(node, (const unsigned char (*)[XMSS_N])pk,
                   PK_seed, layer, tree, leaf_idx);

    /* 3. Sobe o caminho de autenticação
     *
     * Convenção de altura em xmss_root:
     *   nível 0 = folhas (h_idx = XMSS_H_LAYER na representação top-down da
     *             função xmss_root, mas aqui usamos bottom-up a partir de 1):
     *   lvl=0 → combina duas folhas → nó pai tem h_idx=1 no ADRS
     *   lvl=1 → combina esses pais  → nó avô tem h_idx=2
     *   ...
     *   lvl=XMSS_H_LAYER-1 → produz a raiz com h_idx=XMSS_H_LAYER
     *
     * Na árvore indexada com raiz em 0 (usada em xmss_root):
     *   nó interno no nível k acima das folhas tem índice
     *   no range [XMSS_LEAVES/2^k - 1 .. XMSS_LEAVES/2^(k-1) - 2]
     *   e pos = índice relativo ao início desse nível.
     *
     * Mapeamos da mesma forma que xmss_root: o level_start/size do nó pai.
     */
    int idx = (int)leaf_idx;
    for (int lvl = 0; lvl < XMSS_H_LAYER; lvl++) {
        unsigned char adrs[32];
        /* Altura do nó pai (1-indexed acima das folhas) */
        uint32_t h_idx = (uint32_t)(lvl + 1);
        /* Posição do nó pai dentro desse nível */
        uint32_t pos   = (uint32_t)(idx / 2);

        adrs_for_tree(adrs, layer, tree, h_idx, pos);

        unsigned char parent[XMSS_N];
        if (idx % 2 == 0) {
            /* nó atual é filho esquerdo */
            H_function(parent, PK_seed, adrs, node, sig->auth[lvl]);
        } else {
            /* nó atual é filho direito */
            H_function(parent, PK_seed, adrs, sig->auth[lvl], node);
        }
        memcpy(node, parent, XMSS_N);
        idx /= 2;
    }

    memcpy(root_out, node, XMSS_N);
}
