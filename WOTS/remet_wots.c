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
    char *caminhoAssinatura = (argc >= 4) ? argv[3] : "Assinatura.bin";
    
    // Reseta contador global
    sha256_reset_counter();
    
    SecretKeys *sKeys = mallocSkeys();
    PublicKeys *pKeys = mallocPkeys();
    Assinatura *assinatura = mallocAssinatura();
    
    initializeSeeds();
    
    clock_t inicioSkeys = clock();
    generateSKeys(sKeys);
    clock_t fimSkeys = clock();
    
    clock_t inicioPkeys = clock();
    generatePKeys(pKeys, sKeys);
    clock_t fimPkeys = clock();
    
    char mensagem[1001];
    lerMensagem(caminhoMsg, mensagem);
    
    unsigned char msgHash[N];
    sha256_bytes(mensagem, strlen(mensagem), msgHash);
    
    clock_t inicioAssin = clock();
    assinarMensagem(msgHash, assinatura, sKeys);
    clock_t fimAssin = clock();
    
    printf("Chaves geradas no tempo: \n");
    printf("SecretsKeys: %lfs\n", (double)(fimSkeys - inicioSkeys) / CLOCKS_PER_SEC);
    printf("PublicKeys: %lfs\n", (double)(fimPkeys - inicioPkeys) / CLOCKS_PER_SEC);
    
    printf("Mensagem Assinada em: %lf s\n", (double)(fimAssin - inicioAssin) / CLOCKS_PER_SEC);
    
    printf("Total de hashes SHA256: %llu\n", sha256_get_counter());
    
    unsigned long tamanho_assinatura = sizeof(Assinatura);
    printf("Tamanho Assinatura: %lu bytes\n", tamanho_assinatura);
    
    unsigned long tamanho_keys = sizeof(SecretKeys); 
    unsigned long tamanho_public_keys = sizeof(PublicKeys);
    
    printf("Tamanho Secretkeys: %lu bytes\n", tamanho_keys);
    printf("Tamanho Publickeys: %lu bytes\n", tamanho_public_keys);
    
    escreverAssinatura(caminhoAssinatura, assinatura);
    escreverPkeys(caminhoPkey, pKeys, PK_seed, SK_seed);
    
    free(sKeys);
    free(pKeys);
    free(assinatura);
    
    return 0;
}
