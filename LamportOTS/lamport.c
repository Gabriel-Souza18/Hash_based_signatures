#include "sha256.h"
#include "utils.h"
#include "keys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
void assinarMSG(bool* msg, SecretKeys *sKeys, char ** assinatura);
bool verificarMSG(bool* msg, PublicKeys *pKeys, char** assinatura);

void main(){
    int opção;
    printf("1-Assinar mensagem\n2-Verificar Mensagem");
    scanf("%d",&opção );
    
    switch (opção)
    {
    case 1:
        PublicKeys *pKeys = malloc_Pkeys();
        SecretKeys *sKeys = malloc_Skeys();
        generateSecretKeys(sKeys);
        generatePublicKeys(pKeys, sKeys);


        bool msg[256];
        for (int i=0; i<256; i++){
            if( 1){
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
        
      
 
        printKeys(pKeys, sKeys);
        
        escreverAssinatura("assinatura.txt", assinatura, 256);
        escreverPkeys("publicKeys.txt", pKeys);
        escreverMensagem("mensagem.txt", msg);
        freeKeys(pKeys, sKeys);

        for (int i = 0; i < 256; i++) {
            free(assinatura[i]);
        }
        free(assinatura);
        break;
    
    case 2:
        bool mensagem[256];
        lerMensagem("mensagem.txt", mensagem);

        PublicKeys *pKeysVerif = lerPkeys("publicKeys.txt");

        
        int tamanhoAssinatura;
        char** assinaturaVerif = lerAssinatura("assinatura.txt", &tamanhoAssinatura);

        bool resultado = verificarMSG(mensagem, pKeysVerif, assinaturaVerif);
        printf("Verificação: %s\n", resultado ? "VÁLIDA" : "INVÁLIDA");
        
        // Limpeza
        for (int i = 0; i < tamanhoAssinatura; i++) {
            if (assinaturaVerif[i]) free(assinaturaVerif[i]);
        }
        free(assinaturaVerif);
        break;
    default:
        break;
    }
    



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


bool verificarMSG(bool* msg, PublicKeys *pKeys, char** assinatura){
    for (int i = 0; i < 256; i++) {
        char hashAssinatura[SHA256_HEX_SIZE];
        
        sha256_hex(assinatura[i], strlen(assinatura[i]), hashAssinatura);
        
        if (msg[i] == true) {
            // Se bit é 1, deve corresponder a PK1[i]
            if (strcmp(hashAssinatura, pKeys->PK1[i]) != 0) {
                printf("Falha na verificação no bit %d \n", i);
                return false;
            }
        } else {
            // Se bit é 0, deve corresponder a PK0[i]
            if (strcmp(hashAssinatura, pKeys->PK0[i]) != 0) {
                printf("Falha na verificação no bit %d \n", i);
                return false;
            }
        }
    }
    
    printf("Todos os 256 bits verificados com sucesso!\n");
    return true;
}