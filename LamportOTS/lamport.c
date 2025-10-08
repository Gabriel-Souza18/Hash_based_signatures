#include "sha256.h"
#include "utils.h"
#include "keys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h> 

void assinarMSG(char* msgHash, SecretKeys *sKeys, char ** assinatura);
bool verificarMSG(char* msgHash, PublicKeys *pKeys, char** assinatura);

void main(){
    int opção;
    printf("1-gerar mensagem\n2-Verificar Mensagem");
    scanf("%d",&opção );

    srand(clock());
    while(getchar() != '\n');
   

    switch (opção)
    {
    case 1:
        PublicKeys *pKeys = malloc_Pkeys();
        SecretKeys *sKeys = malloc_Skeys();
        clock_t inicioSkeys= clock();
        generateSecretKeys(sKeys);
        clock_t fimSkeys = clock();
        clock_t inicioPkeys = clock();
        generatePublicKeys(pKeys, sKeys);
        clock_t fimPkeys = clock();
        
        printf("Chaves geradas no tempo: \n");
        printf("SecretsKeys: %lfs\n",(double) (fimSkeys-inicioSkeys)/CLOCKS_PER_SEC);
        printf("PublicKeys: %lfs\n", (double)(fimPkeys-inicioPkeys)/CLOCKS_PER_SEC);


        char mensagem[1001];
        printf("Digite a mensagem ate 1000 caracteres:\n");
        fgets(mensagem,1000,stdin);
   
        int len = strlen(mensagem);
        if (len > 0 && mensagem[len-1] == '\n') {
            mensagem[len-1] = '\0';
        }

        char msgHash[SHA256_HEX_SIZE];
        sha256_hex(mensagem,strlen(mensagem), msgHash);

        char** assinatura = (char**)malloc(256 * sizeof(char*));
        for (int i = 0; i < 256; i++) {
            assinatura[i] = (char*)malloc(256 * sizeof(char));
        }

        clock_t inicioAssin = clock();
        assinarMSG(msgHash,sKeys, assinatura);
        clock_t fimAssin = clock();

        printf("Mensagem Assinada em: %lf s\n",(double) (fimAssin- inicioAssin)/CLOCKS_PER_SEC);
      
 
        //printKeys(pKeys, sKeys);
        
        escreverAssinatura("assinatura.txt", assinatura, 256);
        escreverPkeys("publicKeys.txt", pKeys);
        escreverMensagem("mensagem.txt", mensagem);
        freeKeys(pKeys, sKeys);

        for (int i = 0; i < 256; i++) {
            free(assinatura[i]);
        }
        free(assinatura);
        break;
    
    case 2:
        char mensagemLida[1000]; // Buffer para a mensagem lida
        lerMensagem("mensagem.txt", mensagemLida);
        char msgLidaHash[SHA256_HEX_SIZE];

        sha256_hex(mensagemLida, strlen(mensagemLida), msgLidaHash);
        PublicKeys *pKeysVerif = lerPkeys("publicKeys.txt");

        
        int tamanhoAssinatura;
        char** assinaturaVerif = lerAssinatura("assinatura.txt", &tamanhoAssinatura);

        bool resultado = verificarMSG(msgLidaHash, pKeysVerif, assinaturaVerif);
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

void assinarMSG(char* msgHash, SecretKeys *sKeys, char **assinatura){
    int hashLen = strlen(msgHash); // 64 caracteres para SHA256
    
    for (int i = 0; i < 256; i++){
        // Calcula qual caractere hex e qual bit dentro dele
        int charIndex = i / 4;  // Cada caractere hex representa 4 bits
        int bitIndex = i % 4;   // Posição do bit dentro do caractere hex
        
        // Converte caractere hex para valor numérico
        char hexChar = msgHash[charIndex];
        int hexValue;
        if (hexChar >= '0' && hexChar <= '9') {
            hexValue = hexChar - '0';
        } else if (hexChar >= 'a' && hexChar <= 'f') {
            hexValue = hexChar - 'a' + 10;
        } else if (hexChar >= 'A' && hexChar <= 'F') {
            hexValue = hexChar - 'A' + 10;
        }
        
        // Extrai o bit específico (do mais significativo para o menos)
        int bit = (hexValue >> (3 - bitIndex)) & 1;
        
        if (bit == 1){
            strcpy(assinatura[i], sKeys->SK1[i]);
        } else {
            strcpy(assinatura[i], sKeys->SK0[i]);
        }
    }   
}

bool verificarMSG(char* msgHash, PublicKeys *pKeys, char** assinatura){
    for (int i = 0; i < 256; i++) {
        char hashAssinatura[SHA256_HEX_SIZE];
        
        sha256_hex(assinatura[i], strlen(assinatura[i]), hashAssinatura);
        
        // Calcula qual bit do hash original
        int charIndex = i / 4;
        int bitIndex = i % 4;
        
        char hexChar = msgHash[charIndex];
        int hexValue;
        if (hexChar >= '0' && hexChar <= '9') {
            hexValue = hexChar - '0';
        } else if (hexChar >= 'a' && hexChar <= 'f') {
            hexValue = hexChar - 'a' + 10;
        } else if (hexChar >= 'A' && hexChar <= 'F') {
            hexValue = hexChar - 'A' + 10;
        }
        
        int bit = (hexValue >> (3 - bitIndex)) & 1;
        
        if (bit == 1) {
            if (strcmp(hashAssinatura, pKeys->PK1[i]) != 0) {
                printf("Falha na verificação no bit %d\n", i);
                return false;
            }
        } else {
            if (strcmp(hashAssinatura, pKeys->PK0[i]) != 0) {
                printf("Falha na verificação no bit %d\n", i);
                return false;
            }
        }
    }
    
    printf("Todos os 256 bits verificados com sucesso!\n");
    return true;
}