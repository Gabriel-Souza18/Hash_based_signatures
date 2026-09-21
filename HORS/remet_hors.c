/*
 * remet_hors.c — Remetente HORS
 *
 * Gera chaves, assina a mensagem e salva em arquivos.
 * Uso: ./remet_hors <mensagem>
 *
 * Arquivos gerados:
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

extern double hors_tempo_sk;
extern double hors_tempo_pk;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <mensagem>\n", argv[0]);
        return 1;
    }

    char *mensagem = argv[1];
    size_t len = strlen(mensagem);


    printf("=== HORS Remetente ===\n");
    printf("Mensagem: %s\n", mensagem);
    printf("Tamanho: %zu bytes\n", len);
    sha256_reset_counter();
    // Gera as chaves
    Keys keys;
    gerarKeys(&keys);
    printf("Tempo para gerar Chaves Secretas: %lf s\n", hors_tempo_sk);
    printf("Tempo para gerar Chaves Publicas: %lf s\n", hors_tempo_pk);
    printf("Hashes Keygen: %llu\n", sha256_get_counter());
    
    sha256_reset_counter();
    // Assina a mensagem
    Assinatura assinatura;
    clock_t inicio_sign = clock();
    assinarMensagem(mensagem, (int)len, &assinatura, keys.SKeys);
    clock_t fim_sign = clock();
    printf("Tempo para Assinar: %lf s\n",
       (double)(fim_sign - inicio_sign) / CLOCKS_PER_SEC);

    printf("Hashes Assinatura: %llu\n",sha256_get_counter());
    // Salva arquivos para o destinatário
    clock_t inicio_save = clock();
    salvarPkeys((unsigned char *)keys.PKeys);
    salvarAssinatura(&assinatura);
    salvarMensagem(mensagem);
    clock_t fim_save = clock();
    printf("Salvamento em arquivo: %lf s\n", (double)(fim_save - inicio_save) / CLOCKS_PER_SEC);

   
    // Tamanhos
    printf("Tamanho Publickeys: %zu bytes\n", sizeof(keys.PKeys));
    printf("Tamanho Assinatura: %zu bytes\n", sizeof(assinatura));

    return 0;
}
