#include "utils.h"
#include "sha256.h"

#include <stdlib.h>
#include <stdio.h>
#include<string.h>



Keys* malloc_keys() {
    Keys* k = (Keys*)malloc(sizeof(Keys));
    if (k == NULL) {
        fprintf(stderr, "Erro ao alocar memória para keys\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 256; i++) {
        k->SK[i] = NULL;
        k->PK[i] = NULL;
    }
    return k;
}

void generateSecretKeys(Keys *keys) {
    for (int i = 0; i < 256; i++) {
        int key = rand() % 1000; 
        
        keys->SK[i] = (char*)malloc(10 * sizeof(char));
        sprintf(keys->SK[i], "%d", key);
    }
    printf("Chaves Secretas Geradas com sucesso\n");
}

void generatePublicKeys(Keys *keys){

    for (int i =0; i<256; i++){
        keys->PK[i] = (char*)malloc(SHA256_HEX_SIZE * sizeof(char));        
        sha256_hex(keys->SK[i], strlen(keys->SK[i]), keys->PK[i]);
    }
    printf("Chaves publicas geradas com sucesso\n");
}

void printKeys(Keys *keys){
    printf("Chaves Secretas\n");
    for (int i =0; i<256; i++){
        printf("Chave %d: %s\n",i, keys->SK[i] );
    }
    printf("Chaves publicas\n");
    for (int i =0; i<256; i++){
        printf("Chave %d: %s\n",i, keys->PK[i] );
    }
}
void freeKeys(Keys *keys) {
    if (keys == NULL) return;
    
    for (int i = 0; i < 256; i++) {
        if (keys->SK[i] != NULL) {
            free(keys->SK[i]);
        }
        if (keys->PK[i] != NULL) {
            free(keys->PK[i]);
        }
    }
    free(keys);
}