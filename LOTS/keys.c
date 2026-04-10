#include "keys.h"
#include "../SHA256/sha256.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sodium.h>

PublicKeys *malloc_Pkeys(){
    PublicKeys* k = (PublicKeys*)malloc(sizeof(PublicKeys));
    if (k == NULL) {
        fprintf(stderr, "Erro ao alocar memória para Public keys\n");
        exit(EXIT_FAILURE);
    }
    printf("Memoria Alocada com sucesso (Public Keys)\n");
    return k;
}

SecretKeys *malloc_Skeys(){
    SecretKeys* k = (SecretKeys*)malloc(sizeof(SecretKeys));
    if (k == NULL) {
        fprintf(stderr, "Erro ao alocar memória para Secret keys\n");
        exit(EXIT_FAILURE);
    }
    printf("Memoria Alocada com sucesso (Secret Keys)\n");
    return k;
}

void generateSecretKeys(SecretKeys *keys) {
    if(sodium_init() < 0){
        fprintf(stderr, "Erro: falha ao inicializar libsodium.\n");
        exit(EXIT_FAILURE);
    }


    // Gera 256 pares de chaves aleatórias de 32 bytes cada
    for (int i = 0; i < 256; i++) {
        randombytes_buf(keys->SK0[i], KEY_SIZE);
        randombytes_buf(keys->SK1[i], KEY_SIZE);
    }
    printf("Chaves Secretas Geradas com sucesso (256 pares de 32 bytes)\n");
}

void generatePublicKeys(PublicKeys *Pkeys, SecretKeys *Skeys){
    for (int i = 0; i < 256; i++){
        // Hash dos 32 bytes da chave secreta
        sha256_bytes(Skeys->SK0[i], KEY_SIZE, Pkeys->PK0[i]);
        sha256_bytes(Skeys->SK1[i], KEY_SIZE, Pkeys->PK1[i]);
    }
    printf("Chaves publicas geradas com sucesso (256 pares de hashes)\n");
}

// Função auxiliar para imprimir chave secreta em hexadecimal
void printSecretKeyBits(uint8_t *key, int keyIndex, int bit) {
    printf("SK%d[%d]: ", bit, keyIndex);
    for (int i = 0; i < KEY_SIZE; i++) {
        printf("%02x", key[i]);
    }
    printf("\n");
}

void printKeys(PublicKeys *Pkeys, SecretKeys *Skeys){
    printf("\n=== CHAVES SECRETAS (primeiras 5) ===\n");
    for (int i = 0; i < 5; i++){
        printSecretKeyBits(Skeys->SK0[i], i, 0);
        printSecretKeyBits(Skeys->SK1[i], i, 1);
        printf("\n");
    }
    
    printf("\n=== CHAVES PÚBLICAS (primeiras 5) ===\n");
    for (int i = 0; i < 5; i++){
        printf("PK0[%d]: ", i);
        for (int j = 0; j < KEY_SIZE; j++) {
            printf("%02x", Pkeys->PK0[i][j]);
        }
        printf("\n");
        printf("PK1[%d]: ", i);
        for (int j = 0; j < KEY_SIZE; j++) {
            printf("%02x", Pkeys->PK1[i][j]);
        }
        printf("\n");
        printf("\n");
    }
    printf("... (mostrando apenas as 5 primeiras de 256)\n");
}
void freeKeys(PublicKeys *Pkeys, SecretKeys *Skeys) {
    if (Pkeys != NULL) {
        sodium_memzero(Pkeys, sizeof(*Pkeys));
        free(Pkeys);
    }
    
    // Secret keys agora são arrays estáticos, só libera a estrutura
    if (Skeys != NULL) {
        sodium_memzero(Skeys, sizeof(*Skeys));
        free(Skeys);
    }
}