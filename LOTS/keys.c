#include "keys.h"
#include "../SHA256/sha256.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

PublicKeys *malloc_Pkeys(){
    PublicKeys* k = (PublicKeys*)malloc(sizeof(PublicKeys));
    if (k == NULL) {
        fprintf(stderr, "Erro ao alocar memória para Public keys\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 256; i++) {
        k->PK0[i] = NULL;
        k->PK1[i] = NULL;
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
    // Não precisa inicializar - as chaves já são arrays estáticos
    printf("Memoria Alocada com sucesso (Secret Keys)\n");
    return k;
}

void generateSecretKeys(SecretKeys *keys) {
    // Gera 256 pares de chaves aleatórias de 32 bytes cada
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < SECRET_KEY_SIZE; j++) {
            keys->SK0[i][j] = (uint8_t)(rand() & 0xFF);
            keys->SK1[i][j] = (uint8_t)(rand() & 0xFF);
        }
    }
    printf("Chaves Secretas Geradas com sucesso (256 pares de 32 bytes)\n");
}

void generatePublicKeys(PublicKeys *Pkeys, SecretKeys *Skeys){
    for (int i = 0; i < 256; i++){
        Pkeys->PK0[i] = (char*)malloc(SHA256_HEX_SIZE * sizeof(char));
        // Hash dos 32 bytes da chave secreta
        sha256_hex((char*)Skeys->SK0[i], SECRET_KEY_SIZE, Pkeys->PK0[i]);

        Pkeys->PK1[i] = (char*)malloc(SHA256_HEX_SIZE * sizeof(char));
        sha256_hex((char*)Skeys->SK1[i], SECRET_KEY_SIZE, Pkeys->PK1[i]);
    }
    printf("Chaves publicas geradas com sucesso (256 pares de hashes)\n");
}

// Função auxiliar para imprimir chave secreta em hexadecimal
void printSecretKeyBits(uint8_t *key, int keyIndex, int bit) {
    printf("SK%d[%d]: ", bit, keyIndex);
    for (int i = 0; i < SECRET_KEY_SIZE; i++) {
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
        printf("PK0[%d]: %s\n", i, Pkeys->PK0[i]);
        printf("PK1[%d]: %s\n", i, Pkeys->PK1[i]);
        printf("\n");
    }
    printf("... (mostrando apenas as 5 primeiras de 256)\n");
}
void freeKeys(PublicKeys *Pkeys, SecretKeys *Skeys) {
    if (Pkeys != NULL) {
        for (int i = 0; i < 256; i++) {
            if (Pkeys->PK0[i] != NULL) {
                free(Pkeys->PK0[i]);
            }
            if (Pkeys->PK1[i] != NULL) {
                free(Pkeys->PK1[i]);
            }
        }
        free(Pkeys);
    }
    
    // Secret keys agora são arrays estáticos, só libera a estrutura
    if (Skeys != NULL) {
        free(Skeys);
    }
}