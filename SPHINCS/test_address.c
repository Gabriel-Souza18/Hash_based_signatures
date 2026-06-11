/*
 * test_address.c — Testa o sistema de endereçamento ADRS do SPHINCS
 */
#include "sphincs_address.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static int passed = 0;
static int failed = 0;

#define CHECK(cond, msg) \
    do { \
        if (cond) { printf("  [OK] %s\n", msg); passed++; } \
        else      { printf("  [FALHOU] %s\n", msg); failed++; } \
    } while(0)

void test_layer_tree() {
    printf("\n=== Teste 1: layer e tree ===\n");
    unsigned char adrs[32];

    adrs_init(adrs, 5, 0xABCD1234ULL);

    CHECK(adrs_get_layer(adrs) == 5,           "layer = 5");
    CHECK(adrs_get_tree(adrs) == 0xABCD1234ULL,"tree = 0xABCD1234");

    /* Muda só a layer, tree deve continuar */
    adrs_set_layer(adrs, 11);
    CHECK(adrs_get_layer(adrs) == 11,           "layer atualizada para 11");
    CHECK(adrs_get_tree(adrs) == 0xABCD1234ULL, "tree intacta após mudar layer");
}

void test_type_reset() {
    printf("\n=== Teste 2: set_type zera contexto ===\n");
    unsigned char adrs[32];
    adrs_init(adrs, 0, 0);

    adrs_set_type(adrs, ADRS_TYPE_WOTS_HASH);
    adrs_set_keypair(adrs, 42);
    adrs_set_chain(adrs, 7);
    adrs_set_hash(adrs, 3);

    /* Ao mudar o tipo, os 12 bytes de contexto devem ser zerados */
    adrs_set_type(adrs, ADRS_TYPE_TREE);
    CHECK(adrs_get_keypair(adrs) == 0, "keypair zerado ao mudar tipo");
    CHECK(adrs_get_chain(adrs)   == 0, "chain zerado ao mudar tipo");
    CHECK(adrs_get_hash(adrs)    == 0, "hash zerado ao mudar tipo");
}

void test_construtores() {
    printf("\n=== Teste 3: construtores de contexto ===\n");
    unsigned char a[32], b[32];

    /* wots_prf: layer=2, tree=7, keypair=3, chain=1 */
    adrs_for_wots_prf(a, 2, 7, 3, 1);
    CHECK(adrs_get_layer(a) == 2,            "wots_prf: layer=2");
    CHECK(adrs_get_tree(a)  == 7,            "wots_prf: tree=7");
    CHECK(adrs_get_keypair(a) == 3,          "wots_prf: keypair=3");
    CHECK(adrs_get_chain(a) == 1,            "wots_prf: chain=1");
    /* type deve ser WOTS_PRF=5 */
    unsigned char type_byte = a[19]; /* último byte do campo type */
    CHECK(type_byte == ADRS_TYPE_WOTS_PRF,   "wots_prf: type=5");

    /* wots_hash: layer=1, tree=0, keypair=10, chain=5, hash=2 */
    adrs_for_wots_hash(b, 1, 0, 10, 5, 2);
    CHECK(adrs_get_chain(b) == 5,            "wots_hash: chain=5");
    CHECK(adrs_get_hash(b)  == 2,            "wots_hash: hash=2");
    CHECK(b[19] == ADRS_TYPE_WOTS_HASH,      "wots_hash: type=0");

    /* ADRSs diferentes devem ser diferentes */
    CHECK(memcmp(a, b, 32) != 0,             "ADRS distintos sao diferentes");
}

void test_compressao() {
    printf("\n=== Teste 4: compressão ADRS (32→22 bytes) ===\n");
    unsigned char adrs[32];
    unsigned char comp[22];
    unsigned char comp2[22];

    adrs_for_wots_hash(adrs, 3, 0xFFFF, 8, 6, 1);
    adrs_compress(adrs, comp);

    /* comp[0] deve ser o byte 3 do ADRS (último byte do layer) */
    CHECK(comp[0] == adrs[3],  "comp[0] = adrs[3] (layer LSB)");
    /* comp[9] deve ser o byte 19 (último byte do type) */
    CHECK(comp[9] == adrs[19], "comp[9] = adrs[19] (type LSB)");
    /* comp[10..21] = adrs[20..31] */
    CHECK(memcmp(comp + 10, adrs + 20, 12) == 0, "comp[10:22] = adrs[20:32]");

    /* Dois ADRSs iguais → compressões iguais */
    unsigned char adrs2[32];
    adrs_for_wots_hash(adrs2, 3, 0xFFFF, 8, 6, 1);
    adrs_compress(adrs2, comp2);
    CHECK(memcmp(comp, comp2, 22) == 0, "ADRS iguais comprimem igual");

    /* Muda uma coisa → compressão diferente */
    adrs_set_hash(adrs2, 99);
    adrs_compress(adrs2, comp2);
    CHECK(memcmp(comp, comp2, 22) != 0, "ADRS diferentes comprimem diferente");
}

void test_separacao_layers() {
    printf("\n=== Teste 5: layers diferentes geram ADRS diferentes ===\n");
    unsigned char a0[32], a1[32], a5[32];

    adrs_for_wots_prf(a0, 0, 0, 0, 0);
    adrs_for_wots_prf(a1, 1, 0, 0, 0);
    adrs_for_wots_prf(a5, 5, 0, 0, 0);

    CHECK(memcmp(a0, a1, 32) != 0, "layer 0 != layer 1");
    CHECK(memcmp(a1, a5, 32) != 0, "layer 1 != layer 5");
    CHECK(memcmp(a0, a5, 32) != 0, "layer 0 != layer 5");
}

int main(void) {
    printf("=========================================\n");
    printf("  TESTE: sphincs_address (Etapa 1 SPHINCS)\n");
    printf("=========================================\n");

    test_layer_tree();
    test_type_reset();
    test_construtores();
    test_compressao();
    test_separacao_layers();

    printf("\n=========================================\n");
    printf("  Resultado: %d OK, %d FALHOU\n", passed, failed);
    printf("=========================================\n");

    /* Imprime exemplo visual de dois ADRSs de layers diferentes */
    printf("\n--- Exemplo visual ---\n");
    unsigned char ex[32];
    adrs_for_wots_hash(ex, 0, 0, 0, 0, 0);
    adrs_print("WOTS_HASH layer=0 tree=0 kp=0 chain=0 hash=0", ex);
    adrs_for_wots_hash(ex, 2, 7, 3, 5, 1);
    adrs_print("WOTS_HASH layer=2 tree=7 kp=3 chain=5 hash=1", ex);
    adrs_for_tree(ex, 1, 0, 3, 2);
    adrs_print("TREE      layer=1 tree=0 height=3 index=2", ex);

    return (failed == 0) ? 0 : 1;
}
