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
    
    char mensagemLida[1001];
    lerMensagem(caminhoMsg, mensagemLida);
    
    unsigned char msgLidaHash[N];
    sha256_bytes(mensagemLida, strlen(mensagemLida), msgLidaHash);
    
    PublicKeys *pKeysVerif = mallocPkeys();
    FILE *fPkey = fopen(caminhoPkey, "rb");
    if (!fPkey) {
        fprintf(stderr, "Erro ao ler chave pública de %s\n", caminhoPkey);
        free(pKeysVerif);
        return 1;
    }
    fclose(fPkey);
    
    lerPkeys(caminhoPkey, pKeysVerif, PK_seed, SK_seed);
    
    Assinatura *assinaturaVerif = mallocAssinatura();
    FILE *fAssin = fopen(caminhoAssinatura, "rb");
    if (!fAssin) {
        fprintf(stderr, "Erro ao ler assinatura de %s\n", caminhoAssinatura);
        free(pKeysVerif);
        free(assinaturaVerif);
        return 1;
    }
    fclose(fAssin);
    
    lerAssinatura(caminhoAssinatura, assinaturaVerif);
    
    int resultado = verificarMensagem(msgLidaHash, assinaturaVerif, pKeysVerif);
    printf("Verificação: %s\n", resultado ? "VÁLIDA" : "INVÁLIDA");
    
    printf("Total de hashes SHA256: %llu\n", sha256_get_counter());
    
    // Limpeza
    free(assinaturaVerif);
    free(pKeysVerif);
    
    return resultado ? 0 : 1;
}
