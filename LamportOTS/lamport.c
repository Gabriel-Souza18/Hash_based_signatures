#include "sha256.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
void assinarMSG(bool* msg, SecretKeys *sKeys, char ** assinatura);
bool verificarMSG(bool*msg, char** PK);

void main(){
    PublicKeys *pKeys = malloc_Pkeys();
    SecretKeys *sKeys = malloc_Skeys();
    generateSecretKeys(sKeys);
    generatePublicKeys(pKeys, sKeys);


    bool msg[256];
    for (int i=0; i<256; i++){
        if( i%2==0){
            msg[i] = true; 
            continue;
        }
        msg[i]= false;
        
    }

    char** assinatura = (char**)malloc(256 * sizeof(char*));
    for (int i = 0; i < 256; i++) {
        assinatura[i] = (char*)malloc(100 * sizeof(char));
    }

    assinarMSG(msg,sKeys, assinatura);

    for (int i =0; i<256;i++){
        printf("%d- %s", i, assinatura[i]);
    }
   // printKeys(pKeys, sKeys);


    freeKeys(pKeys, sKeys);

    for (int i = 0; i < 256; i++) {
        free(assinatura[i]);
    }
    free(assinatura);
}

void assinarMSG(bool* msg, SecretKeys *sKeys, char **assinatura){
    for (int i =0;i<256;i++){

        if (msg[i]== true){
            strcpy(assinatura[i], sKeys->SK1[i]);
            continue;
        }
        strcpy(assinatura[i], sKeys->SK0[i]);
    }   

}


bool verificarMSG(bool*msg, char** PK){
// tem que verificar a assinatura mais a mesagem bate com a tabela de hash
}