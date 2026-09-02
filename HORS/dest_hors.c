/*
 * dest_hors.c — Destinatário HORS
 *
 * Lê a mensagem, chave pública e assinatura de arquivos e verifica.
 * Uso: ./dest_hors
 *
 * Arquivos lidos:
 *   - publicKeys.bin  (chaves públicas: HORS_T × HORS_N bytes)
 *   - assinatura.bin  (assinatura: HORS_K × HORS_N bytes)
 *   - mensagem.txt    (mensagem original)
 */

#include "keys.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../SHA256/sha256.h"

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    sha256_reset_counter();

    printf("=== HORS Destinatario ===\n");

    // Carrega arquivos do remetente
    clock_t inicio_load = clock();

    char mensagem[1000];
    lerMensagem(mensagem);

    size_t len = strlen(mensagem);
    if (len > 0 && mensagem[len - 1] == '\n') {
        mensagem[len - 1] = '\0';
        len--;
    }

    Assinatura assinatura;
    lerAssinatura(&assinatura);

    unsigned char PKeys[HORS_T][HORS_N];
    lerPkeys((unsigned char *)PKeys);

    clock_t fim_load = clock();
    printf("Carregamento de arquivos: %lf s\n", (double)(fim_load - inicio_load) / CLOCKS_PER_SEC);

    printf("Mensagem: %s\n", mensagem);
    printf("Tamanho: %zu bytes\n", len);

    // Verifica a assinatura
    clock_t inicio_verify = clock();
    int resultado = verificarAssinatura(mensagem, (int)len, &assinatura, PKeys);
    clock_t fim_verify = clock();
    printf("Verificacao: %lf s\n", (double)(fim_verify - inicio_verify) / CLOCKS_PER_SEC);

    if (resultado) {
        printf("Assinatura VALIDA\n");
    } else {
        printf("Assinatura INVALIDA\n");
    }

    printf("Total de hashes SHA256: %llu\n", sha256_get_counter());

    return resultado ? 0 : 1;
}
