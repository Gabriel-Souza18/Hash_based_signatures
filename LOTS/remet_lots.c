#include "../SHA256/sha256.h"
#include "utils.h"
#include "keys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h> 


int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <caminho_mensagem> <caminho_chave_publica> [caminho_assinatura]\n", argv[0]);
        return 1;
    }
    
    char *caminhoMsg = argv[1];
    char *caminhoPkey = argv[2];
    char *caminhoAssinatura = (argc >= 4) ? argv[3] : "assinatura.txt";
    
    // Reseta contador global
    sha256_reset_counter();
    
    PublicKeys *pKeys = malloc_Pkeys();
    SecretKeys *sKeys = malloc_Skeys();
    
    clock_t inicioSkeys = clock();
    generateSecretKeys(sKeys);
    clock_t fimSkeys = clock();
    
    clock_t inicioPkeys = clock();
    generatePublicKeys(pKeys, sKeys);
    clock_t fimPkeys = clock();
    
    char mensagem[1001];
    lerMensagem(caminhoMsg, mensagem);
    
    uint8_t msgHash[SHA256_BYTES_SIZE];
    sha256_bytes(mensagem, strlen(mensagem), msgHash);
    
    uint8_t assinatura[256][KEY_SIZE];
    
    clock_t inicioAssin = clock();
    assinarMSG(msgHash, sKeys, assinatura);
    clock_t fimAssin = clock();
    
    printf("Chaves geradas no tempo: \n");
    printf("SecretsKeys: %lfs\n", (double)(fimSkeys - inicioSkeys) / CLOCKS_PER_SEC);
    printf("PublicKeys: %lfs\n", (double)(fimPkeys - inicioPkeys) / CLOCKS_PER_SEC);
    
    printf("Mensagem Assinada em: %lf s\n", (double)(fimAssin - inicioAssin) / CLOCKS_PER_SEC);
    
    printf("Total de hashes SHA256: %llu\n", sha256_get_counter());
    
    unsigned long tamanho_assinatura = 256 * KEY_SIZE;
    printf("Tamanho Assinatura: %lu bytes\n", tamanho_assinatura);
    
    unsigned long tamanho_keys = 256 * 2 * KEY_SIZE; 
    unsigned long tamanho_public_keys = 256 * 2 * KEY_SIZE;
    
    printf("Tamanho Secretkeys: %lu bytes (otimizado com máscaras de bits)\n", tamanho_keys);
    printf("Tamanho Publickeys: %lu bytes\n", tamanho_public_keys);
    
    escreverAssinatura(caminhoAssinatura, assinatura, 256);
    escreverPkeys(caminhoPkey, pKeys);
    
    freeKeys(pKeys, sKeys);
    
    return 0;
}
