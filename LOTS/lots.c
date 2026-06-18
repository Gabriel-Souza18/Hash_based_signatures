#include "../SHA256/sha256.h"
#include "utils.h"
#include "keys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h> 

void assinarMSG(const uint8_t msgHash[32], SecretKeys *sKeys, uint8_t assinatura[256][KEY_SIZE]);
bool verificarMSG(const uint8_t msgHash[32], PublicKeys *pKeys, uint8_t assinatura[256][KEY_SIZE]);

int main(void){
    int opção;
    printf("1-Gerar mensagem\n2-Verificar Mensagem\n");
    if (scanf("%d", &opção) != 1) {
        return 1;
    }
    
    // Reseta contador global
    sha256_reset_counter();
    
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
        

        char mensagem[1001];
        printf("Digite a mensagem ate 1000 caracteres:\n");
        if (fgets(mensagem, 1000, stdin) == NULL) {
            return 1;
        }
   
        int len = strlen(mensagem);
        if (len > 0 && mensagem[len-1] == '\n') {
            mensagem[len-1] = '\0';
        }

        uint8_t msgHash[SHA256_BYTES_SIZE];
        sha256_bytes(mensagem, strlen(mensagem), msgHash);

        // Assinatura agora é array de bytes (32 bytes por posição)
        uint8_t assinatura[256][KEY_SIZE];

        clock_t inicioAssin = clock();
        assinarMSG(msgHash, sKeys, assinatura);
        clock_t fimAssin = clock();

        printf("Chaves geradas no tempo: \n");
        printf("SecretsKeys: %lfs\n",(double) (fimSkeys-inicioSkeys)/CLOCKS_PER_SEC);
        printf("PublicKeys: %lfs\n", (double)(fimPkeys-inicioPkeys)/CLOCKS_PER_SEC);


        printf("Mensagem Assinada em: %lf s\n",(double) (fimAssin- inicioAssin)/CLOCKS_PER_SEC);
      
        printf("Total de hashes SHA256: %llu\n", sha256_get_counter());

        unsigned long tamanho_assinatura = 256 * KEY_SIZE;
        printf("Tamanho Assinatura: %lu bytes\n", tamanho_assinatura);
        
        unsigned long tamanho_keys = 256 * 2 * KEY_SIZE; 
        unsigned long tamanho_public_keys = 256 * 2 * KEY_SIZE;
        
        printf("Tamanho Secretkeys: %lu bytes (otimizado com máscaras de bits)\n", tamanho_keys);
        printf("Tamanho Publickeys: %lu bytes\n", tamanho_public_keys);

        //printKeys(pKeys, sKeys);
        
        escreverAssinatura("assinatura.txt", assinatura, 256);
        escreverPkeys("publicKeys.txt", pKeys);
        escreverMensagem("mensagem.txt", mensagem);
        freeKeys(pKeys, sKeys);
        break;
    
    case 2:
        char mensagemLida[1000]; // Buffer para a mensagem lida
        lerMensagem("mensagem.txt", mensagemLida);
        uint8_t msgLidaHash[SHA256_BYTES_SIZE];

        sha256_bytes(mensagemLida, strlen(mensagemLida), msgLidaHash);
        PublicKeys *pKeysVerif = lerPkeys("publicKeys.txt");
        if (!pKeysVerif) {
            return 1;
        }

        
        int tamanhoAssinatura;
        uint8_t (*assinaturaVerif)[KEY_SIZE] = lerAssinatura("assinatura.txt", &tamanhoAssinatura);

        bool resultado = verificarMSG(msgLidaHash, pKeysVerif, assinaturaVerif);
        printf("Verificação: %s\n", resultado ? "VÁLIDA" : "INVÁLIDA");
        

        printf("Total de hashes SHA256: %llu\n", sha256_get_counter());
        
        // Limpeza
        free(assinaturaVerif);
        break;
    default:
        break;
    }
    



}

void assinarMSG(const uint8_t msgHash[32], SecretKeys *sKeys, uint8_t assinatura[256][KEY_SIZE]){
    // Para cada bit da mensagem (256 bits no total)
    for (int i = 0; i < 256; i++){
        int byteIndex = i / 8;
        int bitIndex = i % 8;
        
        // Extrai o bit específico (do mais significativo para o menos)
        int bit = (msgHash[byteIndex] >> (7 - bitIndex)) & 1;
        
        // Copia a chave secreta correspondente ao bit (máscara de bits)
        if (bit == 1){
            memcpy(assinatura[i], sKeys->SK1[i], KEY_SIZE);
        } else {
            memcpy(assinatura[i], sKeys->SK0[i], KEY_SIZE);
        }
    }   
}

bool verificarMSG(const uint8_t msgHash[32], PublicKeys *pKeys, uint8_t assinatura[256][KEY_SIZE]){
    for (int i = 0; i < 256; i++) {
        uint8_t hashAssinatura[KEY_SIZE];
        
        // Hash da assinatura (32 bytes)
        sha256_bytes(assinatura[i], KEY_SIZE, hashAssinatura);
        
        int byteIndex = i / 8;
        int bitIndex = i % 8;
        
        int bit = (msgHash[byteIndex] >> (7 - bitIndex)) & 1;
        
        // Verifica se o hash da assinatura corresponde à chave pública correta
        if (bit == 1) {
            if (memcmp(hashAssinatura, pKeys->PK1[i], KEY_SIZE) != 0) {
                printf("Falha na verificação no bit %d (esperado 1)\n", i);
                return false;
            }
        } else {
            if (memcmp(hashAssinatura, pKeys->PK0[i], KEY_SIZE) != 0) {
                printf("Falha na verificação no bit %d (esperado 0)\n", i);
                return false;
            }
        }
    }
    
    printf("✓ Todos os 256 bits verificados com sucesso!\n");
    return true;
}