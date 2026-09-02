#include "Arvore.h"
#include "../SHA256/sha256.h"
#include "../WOTS/utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define NUM_FOLHAS 16

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <caminho_mensagem> <caminho_chave_publica> [caminho_assinatura]\n", argv[0]);
        return 1;
    }

    char *caminhoMsg = argv[1];
    char *caminhoPkey = argv[2];
    char *caminhoAssinatura = (argc >= 4) ? argv[3] : "assinatura.txt";

    // Reseta contador global de SHA256
    sha256_reset_counter();

    printf("\n=== VERIFICAR ASSINATURA ===\n");

    // Carrega assinatura do arquivo texto
    AssinaturaMSS* assinatura = alocarAssinatura();
    lerAssinaturaMSS(caminhoAssinatura, assinatura);

    printf("\n  Índice da Folha: %d\n", assinatura->indiceFolha);
    printf("  Mensagem: %s\n", assinatura->mensagem);
    
    // Converte root hash para hex para imprimir
    char pk_hex[MSS_HASH_SIZE * 2 + 1];
    bytes_to_hex(assinatura->PublicKeysGeral, pk_hex);
    printf("  Public Key da Assinatura: %.16s...\n", pk_hex);
    printf("  Tamanho do caminho: %d\n", assinatura->tamanhoCaminho);

    // Carrega a mensagem do arquivo para comparar com a que foi assinada
    char mensagemLida[1001];
    lerMensagem(caminhoMsg, mensagemLida);
    
    // Sobrescreve a mensagem lida na assinatura para garantir que estamos validando a mensagem fornecida via argumento
    strncpy(assinatura->mensagem, mensagemLida, 1000);
    assinatura->mensagem[1000] = '\0';

    // Carrega a chave pública geral do arquivo (bytes brutos)
    unsigned char publicKey[MSS_HASH_SIZE];
    if (lerPublicKey(caminhoPkey, publicKey) != 1) {
        fprintf(stderr, "Erro ao ler a chave pública de %s\n", caminhoPkey);
        liberarAssinatura(assinatura);
        return 1;
    }

    // Verifica (compara com a chave pública lida — sem necessidade de folhas.txt)
    clock_t inicio_verify = clock();
    int resultado = verificarAssinatura(assinatura, publicKey, NULL);
    clock_t fim_verify = clock();
    double tempo_verify = (double)(fim_verify - inicio_verify) / CLOCKS_PER_SEC;

    printf("\n");
    if (resultado == 1) {
        printf("Verificação: VÁLIDA\n");
    } else {
        printf("Verificação: INVÁLIDA\n");
    }
    printf("Tempo Verificação: %.6f segundos\n", tempo_verify);
    printf("Total de hashes SHA256 na verificação: %llu\n", sha256_get_counter());

    liberarAssinatura(assinatura);

    return resultado == 1 ? 0 : 1;
}
