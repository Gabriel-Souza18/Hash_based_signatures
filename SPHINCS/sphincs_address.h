#ifndef SPHINCS_ADDRESS_H
#define SPHINCS_ADDRESS_H

#include <stdint.h>

/*
 * ADRS — endereço de 32 bytes (FIPS 205, Seção 4.2)
 *
 * Layout:
 *   bytes  0- 3 : layer address   (uint32, big-endian)
 *   bytes  4-15 : tree address    (uint96, big-endian — usamos uint64 nos 8 LSBs)
 *   bytes 16-19 : type            (uint32)
 *   bytes 20-23 : key pair addr   (uint32)   ← índice da folha / par de chaves
 *   bytes 24-27 : chain index     (uint32)   ← passo na chain / altura na árvore
 *   bytes 28-31 : hash index      (uint32)   ← posição do hash / índice do nó
 *
 * Tipos definidos no FIPS 205:
 */
#define ADRS_TYPE_WOTS_HASH  0   /* usado na chain function do WOTS+         */
#define ADRS_TYPE_WOTS_PK    1   /* compressão da PK WOTS+ (L-tree)          */
#define ADRS_TYPE_TREE       2   /* nós internos da árvore de Merkle (XMSS)  */
#define ADRS_TYPE_FORS_TREE  3   /* (SPHINCS+/FORS — reservado)              */
#define ADRS_TYPE_FORS_ROOTS 4   /* (SPHINCS+/FORS — reservado)              */
#define ADRS_TYPE_WOTS_PRF   5   /* geração da SK WOTS+ via PRF              */
#define ADRS_TYPE_FORS_PRF   6   /* (SPHINCS+/FORS — reservado)              */

/* Zera o ADRS e define layer + tree */
void adrs_init(unsigned char adrs[32], uint32_t layer, uint64_t tree);

/* Define o tipo do ADRS (zera os 12 bytes de contexto após o tipo) */
void adrs_set_type(unsigned char adrs[32], uint32_t type);

/* Getters / Setters individuais */
void     adrs_set_layer(unsigned char adrs[32], uint32_t layer);
uint32_t adrs_get_layer(const unsigned char adrs[32]);

void     adrs_set_tree(unsigned char adrs[32], uint64_t tree);
uint64_t adrs_get_tree(const unsigned char adrs[32]);

void     adrs_set_keypair(unsigned char adrs[32], uint32_t kp);
uint32_t adrs_get_keypair(const unsigned char adrs[32]);

void     adrs_set_chain(unsigned char adrs[32], uint32_t chain);
uint32_t adrs_get_chain(const unsigned char adrs[32]);

void     adrs_set_hash(unsigned char adrs[32], uint32_t hash);
uint32_t adrs_get_hash(const unsigned char adrs[32]);

/* Compressão ADRS de 32 → 22 bytes (para uso no PRF/F/H) */
void adrs_compress(const unsigned char adrs[32], unsigned char out[22]);

/* Pronto: monta ADRS para contextos específicos do SPHINCS */

/* WOTS+: geração de SK via PRF (type=5) */
void adrs_for_wots_prf(unsigned char adrs[32],
                       uint32_t layer, uint64_t tree,
                       uint32_t keypair, uint32_t chain_idx);

/* WOTS+: chain function (type=0) */
void adrs_for_wots_hash(unsigned char adrs[32],
                        uint32_t layer, uint64_t tree,
                        uint32_t keypair, uint32_t chain_idx, uint32_t hash_idx);

/* L-tree: compressão da PK WOTS+ (type=1) */
void adrs_for_ltree(unsigned char adrs[32],
                    uint32_t layer, uint64_t tree,
                    uint32_t keypair, uint32_t height, uint32_t index);

/* Árvore XMSS: nó interno (type=2) */
void adrs_for_tree(unsigned char adrs[32],
                   uint32_t layer, uint64_t tree,
                   uint32_t height, uint32_t index);

/* Debug: imprime ADRS em hex */
void adrs_print(const char *label, const unsigned char adrs[32]);

#endif /* SPHINCS_ADDRESS_H */
