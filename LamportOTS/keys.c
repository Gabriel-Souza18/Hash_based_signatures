#include "keys.h"
#include "sha256.h"

#include <stdlib.h>
#include <stdio.h>
#include<string.h>



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
    printf("Memoria Alocada com sucesso\n");
    return k;
}
SecretKeys *malloc_Skeys(){
    SecretKeys* k = (SecretKeys*)malloc(sizeof(SecretKeys));
    if (k == NULL) {
        fprintf(stderr, "Erro ao alocar memória para Secret keys\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 256; i++) {
        k->SK0[i] = NULL;
        k->SK1[i] = NULL;
    }
    printf("Memoria Alocada com sucesso\n");
    return k;

}

void generateSecretKeys(SecretKeys *keys) {
        
    for (int i = 0; i < 256; i++) {
        keys->SK0[i] = (char*)malloc(256 * sizeof(char));
        keys->SK1[i] = (char*)malloc(256 * sizeof(char));
    

        for(int j =0; j< 256;j++){
            int bit0 =rand()%2;       
            int bit1 =rand()%2; 
    
            keys->SK0[i][j] =   '0' + bit0;
            keys->SK1[i][j] =   '0' + bit1;
        }

    }
    printf("Chaves Secretas Geradas com sucesso\n");
}

void generatePublicKeys(PublicKeys *Pkeys, SecretKeys*Skeys){
    for (int i =0; i<256; i++){
        Pkeys->PK0[i] = (char*)malloc(SHA256_HEX_SIZE * sizeof(char));        
        sha256_hex(Skeys->SK0[i], strlen(Skeys->SK0[i]), Pkeys->PK0[i]);

        Pkeys->PK1[i] = (char*)malloc(SHA256_HEX_SIZE * sizeof(char));        
        sha256_hex(Skeys->SK1[i], strlen(Skeys->SK1[i]), Pkeys->PK1[i]);
    }
    printf("Chaves publicas geradas com sucesso\n");
}

void printKeys(PublicKeys *Pkeys, SecretKeys*Skeys){
    printf("Chaves Secretas\n");
    for (int i =0; i<256; i++){
        printf("Chave 0 %d: %s\t|",i, Skeys->SK0[i] );
        printf("Chave 1 %d: %s\n",i, Skeys->SK1[i] );
         
    }
    printf("Chaves publicas\n");
    for (int i =0; i<256; i++){
        printf("Chave 0 %d: %s\t|",i, Pkeys->PK0[i] );
        printf("Chave 1 %d: %s\n",i, Pkeys->PK1[i] );
    }
}
void freeKeys(PublicKeys *Pkeys, SecretKeys*Skeys) {
    if (Pkeys == NULL && Skeys == NULL) return;
    
    for (int i = 0; i < 256; i++) {
        if (Skeys->SK0[i] != NULL) {
            free(Skeys->SK0[i]);
        }
        if (Skeys->SK1[i] != NULL) {
            free(Skeys->SK1[i]);
        }
        if (Pkeys->PK0[i] != NULL) {
            free(Pkeys->PK0[i]);
        }
        if (Pkeys->PK1[i] != NULL) {
            free(Pkeys->PK1[i]);
        }
    }
    free(Pkeys);
    free(Skeys);
}