#include <string.h>
#include <stdlib.h>
#include <sodium.h>
#include <openssl/evp.h>

#include "sphincs.h"
#include "sphincs_address.h"
#include "xmss.h"
#include "ltree.h"
#include "../WOTS/prf.h"
#include "../HORST/keys.h"
#include "../HORST/horst_tree.h"
#include "../SHA256/sha256.h"

/* Sementes globais do WOTS */
extern unsigned char PK_seed[32];
extern unsigned char SK_seed[32];

/* ── Auxiliares HORST ─────────────────────────────────────────────────────── */

static int selecionarIndicesLocal(const unsigned char *hash, int *indices) {
    int bits_por_indice = HORST_BITS_PER_INDEX;
    int hash_bits = 256;
    int bit_pos = 0;

    for (int i = 0; i < HORST_K; i++) {
        int index = 0;

        for (int j = 0; j < bits_por_indice; j++) {
            if (bit_pos >= hash_bits) break;

            int byte_index = bit_pos / 8;
            int bit_index = 7 - (bit_pos % 8);

            int bit = (hash[byte_index] >> bit_index) & 1;
            index = (index << 1) | bit;

            bit_pos++;
        }
        indices[i] = index;
    }

    return HORST_K;
}

static void sphincs_horst_expand_seed(unsigned char SKeys[HORST_T][HORST_N],
                                      const unsigned char horst_seed[HORST_N],
                                      const unsigned char PK_seed_in[HORST_N],
                                      uint32_t layer, uint64_t tree, uint32_t leaf_idx) {
    unsigned char adrs_exp[32];
    for (int i = 0; i < HORST_T; i++) {
        adrs_for_wots_prf(adrs_exp, layer, tree, leaf_idx, (uint32_t)i);
        PRF_SHA2(SKeys[i], PK_seed_in, horst_seed, adrs_exp, HORST_N);
    }
}

static int sphincs_horst_verify(unsigned char pk_horst[SPHINCS_N],
                                const unsigned char D[SPHINCS_N],
                                const Assinatura *horst_sig) {
    int horst_indices[HORST_K];
    selecionarIndicesLocal(D, horst_indices);

    unsigned char first_root[SPHINCS_N];
    int root_set = 0;

    for (int i = 0; i < HORST_K; i++) {
        int idx_horst = horst_indices[i];

        /* Computa a folha (hash do segredo) */
        unsigned char leaf_hash[HORST_N];
        sha256_bytes(horst_sig->components[i].sk, HORST_N, leaf_hash);

        /* Reconstrói a raiz usando o caminho */
        unsigned char hash_atual[HORST_N];
        memcpy(hash_atual, leaf_hash, HORST_N);
        int temp_idx = idx_horst;

        for (int lvl = 0; lvl < HORST_H; lvl++) {
            unsigned char concat[HORST_N * 2];
            if (temp_idx % 2 == 0) {
                memcpy(concat, hash_atual, HORST_N);
                memcpy(concat + HORST_N, horst_sig->components[i].auth_path.path[lvl], HORST_N);
            } else {
                memcpy(concat, horst_sig->components[i].auth_path.path[lvl], HORST_N);
                memcpy(concat + HORST_N, hash_atual, HORST_N);
            }
            sha256_bytes(concat, HORST_N * 2, hash_atual);
            temp_idx /= 2;
        }

        if (!root_set) {
            memcpy(first_root, hash_atual, SPHINCS_N);
            root_set = 1;
        } else {
            if (memcmp(first_root, hash_atual, SPHINCS_N) != 0) {
                return 0; /* Inconsistente */
            }
        }
    }

    memcpy(pk_horst, first_root, SPHINCS_N);
    return 1;
}

/* ── API Pública ──────────────────────────────────────────────────────────── */

void sphincs_keygen(SphincsPublicKey *pk, SphincsSecretKey *sk) {
    if (sodium_init() < 0) {
        /* Já inicializado ou erro */
    }

    randombytes_buf(sk->SK_seed, SPHINCS_N);
    randombytes_buf(sk->SK_prf, SPHINCS_N);
    randombytes_buf(sk->PK_seed, SPHINCS_N);
    memcpy(pk->PK_seed, sk->PK_seed, SPHINCS_N);

    /* Atualiza sementes globais do WOTS */
    memcpy(SK_seed, sk->SK_seed, SPHINCS_N);
    memcpy(PK_seed, sk->PK_seed, SPHINCS_N);

    /* Constrói a raiz da hiper-árvore na camada d-1 */
    xmss_root(pk->root, sk->SK_seed, sk->PK_seed, SPHINCS_D - 1, 0);
}

void sphincs_sign(SphincsSignature *sig,
                  const unsigned char *msg, size_t msg_len,
                  const SphincsSecretKey *sk) {
    /* Atualiza sementes globais do WOTS */
    memcpy(SK_seed, sk->SK_seed, SPHINCS_N);
    memcpy(PK_seed, sk->PK_seed, SPHINCS_N);

    /* 1. R = F(M, SK2) usando SHA-512 via EVP_Digest */
    unsigned char R[64];
    unsigned char *buf1 = malloc(SPHINCS_N + msg_len);
    if (!buf1) return;
    memcpy(buf1, sk->SK_prf, SPHINCS_N);
    memcpy(buf1 + SPHINCS_N, msg, msg_len);

    unsigned int out_len = 64;
    EVP_Digest(buf1, SPHINCS_N + msg_len, R, &out_len, EVP_sha512(), NULL);
    free(buf1);

    memcpy(sig->R, R, SPHINCS_N);
    unsigned char *R2 = R + 32;

    /* 2. D = H(R1, M) usando SHA-256 via EVP_Digest */
    unsigned char D[SPHINCS_N];
    unsigned char *buf2 = malloc(SPHINCS_N + msg_len);
    if (!buf2) return;
    memcpy(buf2, sig->R, SPHINCS_N);
    memcpy(buf2 + SPHINCS_N, msg, msg_len);

    out_len = 32;
    EVP_Digest(buf2, SPHINCS_N + msg_len, D, &out_len, EVP_sha256(), NULL);
    free(buf2);

    /* 3. idx = Chop(R2, 60 bits) */
    uint64_t idx = 0;
    for (int i = 0; i < 8; i++) {
        idx = (idx << 8) | R2[i];
    }
    idx &= 0x0FFFFFFFFFFFFFFFULL; /* máscara de 60 bits */
    sig->idx = idx;

    /* 4. Determina tree e leaf na camada 0 */
    uint64_t tree = idx >> 5;
    uint32_t leaf = idx & 0x1F;

    /* 5. Gera a semente do HORST correspondente */
    unsigned char adrs_horst[32];
    adrs_for_wots_prf(adrs_horst, SPHINCS_D, tree, leaf, 0);

    unsigned char horst_seed[SPHINCS_N];
    PRF_SHA2(horst_seed, sk->PK_seed, sk->SK_seed, adrs_horst, SPHINCS_N);

    /* 6. Expande semente do HORST em T chaves secretas */
    unsigned char horst_skeys[HORST_T][HORST_N];
    sphincs_horst_expand_seed(horst_skeys, horst_seed, sk->PK_seed, SPHINCS_D, tree, leaf);

    /* 7. Constrói a árvore HORST */
    ArvoreHorst *horst_tree = criarArvoreHorst((const unsigned char (*)[HORST_N])horst_skeys);

    /* 8. Chave pública do HORST (raiz) */
    unsigned char pk_horst[SPHINCS_N];
    obterRaizArvoreHorst(horst_tree, pk_horst);

    /* 9. Assina D com HORST */
    int horst_indices[HORST_K];
    selecionarIndicesLocal(D, horst_indices);
    for (int i = 0; i < HORST_K; i++) {
        int idx_horst = horst_indices[i];
        memcpy(sig->horst_sig.components[i].sk, horst_skeys[idx_horst], HORST_N);
        obterCaminhoAutenticacaoHorst(horst_tree, idx_horst, &sig->horst_sig.components[i].auth_path);
    }
    liberarArvoreHorst(horst_tree);

    /* 10. Assina a chave pública do HORST e as raízes subsequentes usando a hiper-árvore XMSS */
    unsigned char current_msg[SPHINCS_N];
    memcpy(current_msg, pk_horst, SPHINCS_N);

    for (int j = 0; j < SPHINCS_D; j++) {
        uint64_t xmss_tree = idx >> ((j + 1) * 5);
        uint32_t xmss_leaf = (idx >> (j * 5)) & 0x1F;

        /* Assina current_msg usando a folha xmss_leaf da árvore xmss_tree na camada j */
        xmss_sign(&sig->xmss_sigs[j], current_msg, sk->SK_seed, sk->PK_seed, j, xmss_tree, xmss_leaf);

        /* Reconstrói o root da camada j para servir de mensagem para a camada j + 1 */
        if (j < SPHINCS_D - 1) {
            unsigned char root_rec[SPHINCS_N];
            xmss_verify(root_rec, current_msg, &sig->xmss_sigs[j], sk->PK_seed, sk->SK_seed, j, xmss_tree, xmss_leaf);
            memcpy(current_msg, root_rec, SPHINCS_N);
        }
    }
}

int sphincs_verify(const SphincsSignature *sig,
                   const unsigned char *msg, size_t msg_len,
                   const SphincsPublicKey *pk,
                   const unsigned char SK_seed_val[SPHINCS_N]) {
    /* Atualiza sementes globais do WOTS */
    memcpy(SK_seed, SK_seed_val, SPHINCS_N);
    memcpy(PK_seed, pk->PK_seed, SPHINCS_N);

    /* 1. D = H(R1, M) */
    unsigned char D[SPHINCS_N];
    unsigned char *buf2 = malloc(SPHINCS_N + msg_len);
    if (!buf2) return 0;
    memcpy(buf2, sig->R, SPHINCS_N);
    memcpy(buf2 + SPHINCS_N, msg, msg_len);

    unsigned int out_len = 32;
    EVP_Digest(buf2, SPHINCS_N + msg_len, D, &out_len, EVP_sha256(), NULL);
    free(buf2);

    /* 2. Reconstrói a chave pública do HORST a partir do digest D e da assinatura HORST */
    unsigned char pk_horst[SPHINCS_N];
    if (!sphincs_horst_verify(pk_horst, D, &sig->horst_sig)) {
        return 0; /* Assinatura HORST inválida */
    }

    /* 3. Sobe a hiper-árvore XMSS */
    unsigned char current_msg[SPHINCS_N];
    memcpy(current_msg, pk_horst, SPHINCS_N);

    uint64_t idx = sig->idx;

    for (int j = 0; j < SPHINCS_D; j++) {
        uint64_t xmss_tree = idx >> ((j + 1) * 5);
        uint32_t xmss_leaf = (idx >> (j * 5)) & 0x1F;

        unsigned char root_rec[SPHINCS_N];
        xmss_verify(root_rec, current_msg, &sig->xmss_sigs[j], pk->PK_seed, SK_seed_val, j, xmss_tree, xmss_leaf);
        memcpy(current_msg, root_rec, SPHINCS_N);
    }

    /* 4. A raiz reconstruída no topo deve coincidir com a chave pública do SPHINCS */
    return (memcmp(current_msg, pk->root, SPHINCS_N) == 0);
}
