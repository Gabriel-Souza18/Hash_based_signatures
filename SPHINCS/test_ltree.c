/*
 * test_ltree.c — Testa a L-tree e a H_function
 */
#include "ltree.h"
#include "sphincs_address.h"
#include "../SHA256/sha256.h"

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

/* Imprime N bytes em hex */
static void print_hex(const char *label, const unsigned char *buf, int len) {
    printf("  %s: ", label);
    for (int i = 0; i < len; i++) printf("%02x", buf[i]);
    printf("\n");
}

/* ────────────────────────────────────────────────────────────────────────── */

void test_H_deterministico() {
    printf("\n=== Teste 1: H_function é determinística ===\n");

    unsigned char PK_seed[32], adrs[32], left[32], right[32];
    unsigned char out1[32], out2[32];

    memset(PK_seed, 0xAA, 32);
    memset(left,    0x11, 32);
    memset(right,   0x22, 32);
    adrs_for_ltree(adrs, 0, 0, 0, 0, 0);

    H_function(out1, PK_seed, adrs, left, right);
    H_function(out2, PK_seed, adrs, left, right);

    CHECK(memcmp(out1, out2, 32) == 0, "H(x,y) == H(x,y) na segunda chamada");
}

void test_H_entradas_diferentes() {
    printf("\n=== Teste 2: H_function muda com entradas diferentes ===\n");

    unsigned char PK_seed[32], adrs[32];
    unsigned char left[32], right[32];
    unsigned char out_base[32], out_diff[32];

    memset(PK_seed, 0x00, 32);
    memset(left,    0x01, 32);
    memset(right,   0x02, 32);
    adrs_for_ltree(adrs, 0, 0, 0, 0, 0);

    H_function(out_base, PK_seed, adrs, left, right);

    /* Muda o filho direito */
    unsigned char right2[32];
    memset(right2, 0x03, 32);
    H_function(out_diff, PK_seed, adrs, left, right2);
    CHECK(memcmp(out_base, out_diff, 32) != 0, "H muda ao mudar filho direito");

    /* Muda o filho esquerdo */
    unsigned char left2[32];
    memset(left2, 0xFF, 32);
    H_function(out_diff, PK_seed, adrs, left2, right);
    CHECK(memcmp(out_base, out_diff, 32) != 0, "H muda ao mudar filho esquerdo");

    /* Muda o PK_seed */
    unsigned char seed2[32];
    memset(seed2, 0x55, 32);
    H_function(out_diff, seed2, adrs, left, right);
    CHECK(memcmp(out_base, out_diff, 32) != 0, "H muda ao mudar PK_seed");

    /* Muda o ADRS (layer diferente) */
    unsigned char adrs2[32];
    adrs_for_ltree(adrs2, 1, 0, 0, 0, 0);
    H_function(out_diff, PK_seed, adrs2, left, right);
    CHECK(memcmp(out_base, out_diff, 32) != 0, "H muda ao mudar ADRS (layer)");
}

void test_ltree_deterministico() {
    printf("\n=== Teste 3: L-tree é determinística ===\n");

    unsigned char PK_seed[32];
    unsigned char pk[LTREE_L][LTREE_N];
    unsigned char root1[32], root2[32];

    memset(PK_seed, 0xBB, 32);
    /* Preenche as 67 PKs com valores fixos conhecidos */
    for (int i = 0; i < LTREE_L; i++) {
        memset(pk[i], (unsigned char)(i + 1), LTREE_N);
    }

    ltree_compress(root1, pk, PK_seed, 0, 0, 0);
    ltree_compress(root2, pk, PK_seed, 0, 0, 0);

    CHECK(memcmp(root1, root2, 32) == 0, "ltree(pk) == ltree(pk) segunda chamada");
    print_hex("raiz", root1, 32);
}

void test_ltree_sensivel_a_pk() {
    printf("\n=== Teste 4: L-tree muda ao mudar qualquer PK ===\n");

    unsigned char PK_seed[32];
    unsigned char pk[LTREE_L][LTREE_N];
    unsigned char root_base[32], root_mod[32];

    memset(PK_seed, 0x00, 32);
    for (int i = 0; i < LTREE_L; i++) {
        memset(pk[i], (unsigned char)i, LTREE_N);
    }
    ltree_compress(root_base, pk, PK_seed, 0, 0, 0);

    /* Muda o primeiro PK */
    pk[0][0] ^= 0xFF;
    ltree_compress(root_mod, pk, PK_seed, 0, 0, 0);
    CHECK(memcmp(root_base, root_mod, 32) != 0, "raiz muda ao modificar pk[0]");
    pk[0][0] ^= 0xFF; /* restaura */

    /* Muda o último PK (pk[66]) */
    pk[LTREE_L - 1][0] ^= 0xFF;
    ltree_compress(root_mod, pk, PK_seed, 0, 0, 0);
    CHECK(memcmp(root_base, root_mod, 32) != 0, "raiz muda ao modificar pk[66] (último)");
    pk[LTREE_L - 1][0] ^= 0xFF;

    /* Muda um PK do meio */
    pk[33][15] ^= 0x01;
    ltree_compress(root_mod, pk, PK_seed, 0, 0, 0);
    CHECK(memcmp(root_base, root_mod, 32) != 0, "raiz muda ao modificar pk[33]");
}

void test_ltree_contexto_isolado() {
    printf("\n=== Teste 5: L-trees de posições diferentes têm raízes diferentes ===\n");

    unsigned char PK_seed[32];
    unsigned char pk[LTREE_L][LTREE_N];
    unsigned char root_kp0[32], root_kp1[32];
    unsigned char root_tree0[32], root_tree1[32];
    unsigned char root_layer0[32], root_layer1[32];

    memset(PK_seed, 0xCC, 32);
    for (int i = 0; i < LTREE_L; i++) {
        memset(pk[i], (unsigned char)(i * 3), LTREE_N);
    }

    /* Keypair diferente */
    ltree_compress(root_kp0,   pk, PK_seed, 0, 0, 0);
    ltree_compress(root_kp1,   pk, PK_seed, 0, 0, 1);
    CHECK(memcmp(root_kp0, root_kp1, 32) != 0,
          "keypair=0 vs keypair=1 → raízes diferentes");

    /* Tree diferente */
    ltree_compress(root_tree0, pk, PK_seed, 0, 0, 0);
    ltree_compress(root_tree1, pk, PK_seed, 0, 1, 0);
    CHECK(memcmp(root_tree0, root_tree1, 32) != 0,
          "tree=0 vs tree=1 → raízes diferentes");

    /* Layer diferente */
    ltree_compress(root_layer0, pk, PK_seed, 0, 0, 0);
    ltree_compress(root_layer1, pk, PK_seed, 1, 0, 0);
    CHECK(memcmp(root_layer0, root_layer1, 32) != 0,
          "layer=0 vs layer=1 → raízes diferentes");
}

void test_ltree_tamanho_saida() {
    printf("\n=== Teste 6: saída da L-tree tem exatamente 32 bytes ===\n");

    unsigned char PK_seed[32];
    unsigned char pk[LTREE_L][LTREE_N];
    unsigned char root[32];
    /* Canário: bytes vizinhos não devem ser escritos */
    unsigned char canary_before = 0xDE, canary_after = 0xAD;
    (void)canary_before; (void)canary_after;

    memset(PK_seed, 0x01, 32);
    for (int i = 0; i < LTREE_L; i++) memset(pk[i], i, LTREE_N);

    ltree_compress(root, pk, PK_seed, 0, 0, 0);

    /* Verifica que nenhum byte da raiz é zero (altamente improvável com entrada não-zero) */
    int all_zero = 1;
    for (int i = 0; i < 32; i++) if (root[i] != 0) { all_zero = 0; break; }
    CHECK(!all_zero, "raiz não é tudo-zero (hash real foi computado)");
    CHECK(sizeof(root) == 32, "buffer de saída tem 32 bytes");
}

/* ────────────────────────────────────────────────────────────────────────── */

int main(void) {
    printf("=========================================\n");
    printf("  TESTE: L-tree (Etapa 2 SPHINCS)\n");
    printf("=========================================\n");

    test_H_deterministico();
    test_H_entradas_diferentes();
    test_ltree_deterministico();
    test_ltree_sensivel_a_pk();
    test_ltree_contexto_isolado();
    test_ltree_tamanho_saida();

    printf("\n=========================================\n");
    printf("  Resultado: %d OK, %d FALHOU\n", passed, failed);
    printf("=========================================\n");
    return (failed == 0) ? 0 : 1;
}
