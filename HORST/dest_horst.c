/*
 * dest_horst.c — Destinatário HORST
 *
 * Lê a mensagem, chave pública e assinatura de arquivos e verifica.
 * Uso: ./dest_horst
 *
 * Arquivos lidos:
 *   - pubkey.bin      (chave pública / raiz: HORST_N bytes)
 *   - assinatura.bin  (assinatura: HORST_K × (HORST_N + HORST_H × HORST_N) bytes)
 *   - mensagem.txt    (mensagem original)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "keys.h"
#include "utils.h"
#include "../SHA256/sha256.h"

#define CAMINHO_PUBKEY     "pubkey.bin"
#define CAMINHO_MENSAGEM   "mensagem.txt"
#define CAMINHO_ASSINATURA "assinatura.bin"

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    sha256_reset_counter();

    printf("=== HORST Destinatario ===\n");

    // Carrega chave pública
    PublicKey pk;
    if (!carregarPublicKey(CAMINHO_PUBKEY, &pk)) {
        fprintf(stderr, "Erro ao carregar chave publica. Execute o remetente primeiro.\n");
        return 1;
    }

    // Carrega assinatura
    Assinatura assinatura;
    if (!carregarAssinatura(CAMINHO_ASSINATURA, &assinatura)) {
        fprintf(stderr, "Erro ao carregar assinatura.\n");
        return 1;
    }

    // Lê mensagem
    char mensagem[1024];
    lerMensagem(mensagem);

    size_t len = strlen(mensagem);
    if (len > 0 && mensagem[len - 1] == '\n') {
        mensagem[len - 1] = '\0';
        len--;
    }

    printf("Mensagem: %s\n", mensagem);
    printf("Tamanho: %zu bytes\n", len);

    // Verifica a assinatura
    clock_t inicio_verify = clock();
    int resultado = verificarAssinatura(mensagem, (int)len, &assinatura, &pk);
    clock_t fim_verify = clock();
    printf("Tempo Verificação: %.6f segundos\n", (double)(fim_verify - inicio_verify) / CLOCKS_PER_SEC);

    if (resultado) {
        printf("Assinatura VALIDA\n");
    } else {
        printf("Assinatura INVALIDA\n");
    }

    printf("Total de hashes SHA256: %llu\n", sha256_get_counter());

    return resultado ? 0 : 1;
}
