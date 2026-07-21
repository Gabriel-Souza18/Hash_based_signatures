#include "../SHA256/sha256.h"
#include "utils.h"
#include "keys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h> 

bool verificarMSG(const uint8_t msgHash[32], PublicKeys *pKeys, uint8_t assinatura[256][KEY_SIZE]);

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
    
    char mensagemLida[1001];
    lerMensagem(caminhoMsg, mensagemLida);
    
    uint8_t msgLidaHash[SHA256_BYTES_SIZE];
    sha256_bytes(mensagemLida, strlen(mensagemLida), msgLidaHash);
    
    PublicKeys *pKeysVerif = lerPkeys(caminhoPkey);
    if (!pKeysVerif) {
        fprintf(stderr, "Erro ao ler chave pública de %s\n", caminhoPkey);
        return 1;
    }
    
    int tamanhoAssinatura;
    uint8_t (*assinaturaVerif)[KEY_SIZE] = lerAssinatura(caminhoAssinatura, &tamanhoAssinatura);
    if (!assinaturaVerif) {
        fprintf(stderr, "Erro ao ler assinatura de %s\n", caminhoAssinatura);
        freeKeys(pKeysVerif, NULL);
        return 1;
    }
    
    clock_t inicioVerif = clock();
    bool resultado = verificarMSG(msgLidaHash, pKeysVerif, assinaturaVerif);
    clock_t fimVerif = clock();
    double tempoVerif = (double)(fimVerif - inicioVerif) / CLOCKS_PER_SEC;

    printf("Verificação: %s\n", resultado ? "VÁLIDA" : "INVÁLIDA");
    printf("Tempo Verificação: %lfs\n", tempoVerif);
    printf("Total de hashes SHA256: %llu\n", sha256_get_counter());
    
    // Limpeza
    free(assinaturaVerif);
    freeKeys(pKeysVerif, NULL);
    
    return resultado ? 0 : 1;
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
