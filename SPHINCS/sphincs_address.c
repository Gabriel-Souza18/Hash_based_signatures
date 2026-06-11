#include "sphincs_address.h"
#include <string.h>
#include <stdio.h>

/* ── helpers internos big-endian ──────────────────────────────────────────── */

static void write_u32(unsigned char *dst, uint32_t v) {
    dst[0] = (v >> 24) & 0xFF;
    dst[1] = (v >> 16) & 0xFF;
    dst[2] = (v >>  8) & 0xFF;
    dst[3] =  v        & 0xFF;
}

static uint32_t read_u32(const unsigned char *src) {
    return ((uint32_t)src[0] << 24) |
           ((uint32_t)src[1] << 16) |
           ((uint32_t)src[2] <<  8) |
            (uint32_t)src[3];
}

/* tree usa os 8 bytes menos significativos dos 12 bytes (bytes 4-15) */
static void write_u64_tree(unsigned char *dst, uint64_t v) {
    /* bytes 4-11 = zeros, bytes 8-15 = tree em big-endian */
    memset(dst, 0, 12);
    dst[ 4] = (v >> 56) & 0xFF;
    dst[ 5] = (v >> 48) & 0xFF;
    dst[ 6] = (v >> 40) & 0xFF;
    dst[ 7] = (v >> 32) & 0xFF;
    dst[ 8] = (v >> 24) & 0xFF;
    dst[ 9] = (v >> 16) & 0xFF;
    dst[10] = (v >>  8) & 0xFF;
    dst[11] =  v        & 0xFF;
}

static uint64_t read_u64_tree(const unsigned char *src) {
    return ((uint64_t)src[ 4] << 56) |
           ((uint64_t)src[ 5] << 48) |
           ((uint64_t)src[ 6] << 40) |
           ((uint64_t)src[ 7] << 32) |
           ((uint64_t)src[ 8] << 24) |
           ((uint64_t)src[ 9] << 16) |
           ((uint64_t)src[10] <<  8) |
            (uint64_t)src[11];
}

/* ── API pública ──────────────────────────────────────────────────────────── */

void adrs_init(unsigned char adrs[32], uint32_t layer, uint64_t tree) {
    memset(adrs, 0, 32);
    write_u32(adrs + 0, layer);
    write_u64_tree(adrs + 4, tree);
}

void adrs_set_type(unsigned char adrs[32], uint32_t type) {
    write_u32(adrs + 16, type);
    /* Zera os 12 bytes de contexto (key pair, chain, hash) */
    memset(adrs + 20, 0, 12);
}

void adrs_set_layer(unsigned char adrs[32], uint32_t layer) {
    write_u32(adrs + 0, layer);
}
uint32_t adrs_get_layer(const unsigned char adrs[32]) {
    return read_u32(adrs + 0);
}

void adrs_set_tree(unsigned char adrs[32], uint64_t tree) {
    write_u64_tree(adrs + 4, tree);
}
uint64_t adrs_get_tree(const unsigned char adrs[32]) {
    return read_u64_tree(adrs + 4);
}

void adrs_set_keypair(unsigned char adrs[32], uint32_t kp) {
    write_u32(adrs + 20, kp);
}
uint32_t adrs_get_keypair(const unsigned char adrs[32]) {
    return read_u32(adrs + 20);
}

void adrs_set_chain(unsigned char adrs[32], uint32_t chain) {
    write_u32(adrs + 24, chain);
}
uint32_t adrs_get_chain(const unsigned char adrs[32]) {
    return read_u32(adrs + 24);
}

void adrs_set_hash(unsigned char adrs[32], uint32_t hash) {
    write_u32(adrs + 28, hash);
}
uint32_t adrs_get_hash(const unsigned char adrs[32]) {
    return read_u32(adrs + 28);
}

/*
 * Compressão ADRS: 32 → 22 bytes  (FIPS 205, Seção 4.2)
 *
 * ADRSc = ADRS[3] || ADRS[8:16] || ADRS[19] || ADRS[20:32]
 *           1 B         8 B           1 B          12 B     = 22 B
 */
void adrs_compress(const unsigned char adrs[32], unsigned char out[22]) {
    out[0] = adrs[3];                /* último byte do layer      */
    memcpy(out +  1, adrs +  8, 8); /* bytes 8-15 do tree        */
    out[9] = adrs[19];              /* último byte do type       */
    memcpy(out + 10, adrs + 20, 12);/* bytes 20-31 (contexto)    */
}

/* ── Construtores de contexto ─────────────────────────────────────────────── */

void adrs_for_wots_prf(unsigned char adrs[32],
                       uint32_t layer, uint64_t tree,
                       uint32_t keypair, uint32_t chain_idx) {
    adrs_init(adrs, layer, tree);
    adrs_set_type(adrs, ADRS_TYPE_WOTS_PRF);
    adrs_set_keypair(adrs, keypair);
    adrs_set_chain(adrs, chain_idx);
    adrs_set_hash(adrs, 0);
}

void adrs_for_wots_hash(unsigned char adrs[32],
                        uint32_t layer, uint64_t tree,
                        uint32_t keypair, uint32_t chain_idx, uint32_t hash_idx) {
    adrs_init(adrs, layer, tree);
    adrs_set_type(adrs, ADRS_TYPE_WOTS_HASH);
    adrs_set_keypair(adrs, keypair);
    adrs_set_chain(adrs, chain_idx);
    adrs_set_hash(adrs, hash_idx);
}

void adrs_for_ltree(unsigned char adrs[32],
                    uint32_t layer, uint64_t tree,
                    uint32_t keypair, uint32_t height, uint32_t index) {
    adrs_init(adrs, layer, tree);
    adrs_set_type(adrs, ADRS_TYPE_WOTS_PK);
    adrs_set_keypair(adrs, keypair);
    adrs_set_chain(adrs, height);
    adrs_set_hash(adrs, index);
}

void adrs_for_tree(unsigned char adrs[32],
                   uint32_t layer, uint64_t tree,
                   uint32_t height, uint32_t index) {
    adrs_init(adrs, layer, tree);
    adrs_set_type(adrs, ADRS_TYPE_TREE);
    adrs_set_keypair(adrs, 0);
    adrs_set_chain(adrs, height);
    adrs_set_hash(adrs, index);
}

void adrs_print(const char *label, const unsigned char adrs[32]) {
    printf("%s:\n", label);
    printf("  layer   = %u\n",   adrs_get_layer(adrs));
    printf("  tree    = %llu\n", (unsigned long long)adrs_get_tree(adrs));
    printf("  type    = %u\n",   read_u32(adrs + 16));
    printf("  keypair = %u\n",   adrs_get_keypair(adrs));
    printf("  chain   = %u\n",   adrs_get_chain(adrs));
    printf("  hash    = %u\n",   adrs_get_hash(adrs));
    printf("  raw     = ");
    for (int i = 0; i < 32; i++) printf("%02x", adrs[i]);
    printf("\n");
}
