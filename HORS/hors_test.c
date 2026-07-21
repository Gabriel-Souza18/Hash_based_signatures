#include "keys.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "../SHA256/sha256.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <mensagem>\n", argv[0]);
        return 1;
    }

    char *mensagem = argv[1];
    size_t len = strlen(mensagem);

    sha256_reset_counter();

    printf("Mensagem: %s\n", mensagem);
    printf("Tamanho: %zu bytes\n", len);

    // Gera as chaves
    Keys keys;
    gerarKeys(&keys);
    printf("Tempo para gerar Chaves Secretas: %lf s\n", hors_tempo_sk);
    printf("Tempo para gerar Chaves Publicas: %lf s\n", hors_tempo_pk);

    // Assina a mensagem
    Assinatura assinatura;
    clock_t inicio_sign = clock();
    assinarMensagem(mensagem, len, &assinatura, keys.SKeys);
    clock_t fim_sign = clock();
    double tempo_sign = (double)(fim_sign - inicio_sign) / CLOCKS_PER_SEC;
    printf("Tempo para Assinar: %lf s\n", tempo_sign);

    // Verifica a assinatura
    clock_t inicio_verify = clock();
    int resultado = verificarAssinatura(mensagem, len, &assinatura, keys.PKeys);
    clock_t fim_verify = clock();
    double tempo_verify = (double)(fim_verify - inicio_verify) / CLOCKS_PER_SEC;

    printf("Verificacao: %s\n", resultado ? "VALIDA" : "INVALIDA");
    printf("Tempo Verificação: %lfs\n", tempo_verify);

    // Informações sobre hashes
    unsigned long long hashes = sha256_get_counter();
    printf("Total de hashes SHA256: %llu\n", hashes);

    // Tamanhos
    printf("Tamanho Secretkeys: %zu\n", sizeof(keys.SKeys));
    printf("Tamanho Publickeys: %zu\n", sizeof(keys.PKeys));
    printf("Tamanho Assinatura: %zu\n", sizeof(assinatura));

    return 0;
}
