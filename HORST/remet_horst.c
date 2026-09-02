/*
 * remet_horst.c — Remetente HORST
 *
 * Gera chaves, assina a mensagem e salva em arquivos.
 * Uso: ./remet_horst <mensagem>
 *
 * Arquivos gerados:
 *   - seckeys.bin     (chaves secretas + raiz: HORST_T × HORST_N + HORST_N bytes)
 *   - pubkey.bin      (chave pública / raiz: HORST_N bytes)
 *   - assinatura.bin  (assinatura: HORST_K × (HORST_N + HORST_H × HORST_N) bytes)
 *   - mensagem.txt    (mensagem original)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sodium.h>
#include <time.h>

#include "keys.h"
#include "utils.h"
#include "../SHA256/sha256.h"

#define CAMINHO_SKEYS      "seckeys.bin"
#define CAMINHO_PUBKEY     "pubkey.bin"
#define CAMINHO_MENSAGEM   "mensagem.txt"
#define CAMINHO_ASSINATURA "assinatura.bin"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <mensagem>\n", argv[0]);
        return 1;
    }

    if (sodium_init() < 0) {
        fprintf(stderr, "Erro ao inicializar libsodium\n");
        return 1;
    }

    sha256_reset_counter();

    char *mensagem = argv[1];
    size_t len = strlen(mensagem);

    printf("=== HORST Remetente ===\n");
    printf("Mensagem: %s\n", mensagem);
    printf("Tamanho: %zu bytes\n", len);

    // Gera as chaves
    Keys keys;
    clock_t inicio_keygen = clock();
    gerarKeys(&keys);
    clock_t fim_keygen = clock();
    printf("Geracao de chaves (total): %lf s\n",
           (double)(fim_keygen - inicio_keygen) / CLOCKS_PER_SEC);

    // Salva chaves secretas
    if (!salvarSKeys(CAMINHO_SKEYS, &keys)) {
        fprintf(stderr, "Erro ao salvar chaves secretas\n");
        return 1;
    }
    printf("Chaves secretas salvas em '%s'\n", CAMINHO_SKEYS);

    // Salva chave pública
    if (!salvarPublicKey(CAMINHO_PUBKEY, &keys.PKey)) {
        fprintf(stderr, "Erro ao salvar chave publica\n");
        return 1;
    }
    printf("Chave publica salva em '%s'\n", CAMINHO_PUBKEY);

    printf("Total de hashes SHA256 (keygen): %llu\n", sha256_get_counter());
    sha256_reset_counter();

    // Assina a mensagem
    Assinatura assinatura;
    clock_t inicio_sign = clock();
    assinarMensagem(mensagem, (int)len, &assinatura, keys.SKeys);
    clock_t fim_sign = clock();
    printf("Assinatura (total): %lf s\n",
           (double)(fim_sign - inicio_sign) / CLOCKS_PER_SEC);

    // Salva assinatura
    if (!salvarAssinatura(CAMINHO_ASSINATURA, &assinatura)) {
        fprintf(stderr, "Erro ao salvar assinatura\n");
        return 1;
    }
    printf("Assinatura salva em '%s'\n", CAMINHO_ASSINATURA);

    // Salva mensagem
    salvarMensagem(mensagem);
    printf("Mensagem salva em '%s'\n", CAMINHO_MENSAGEM);

    printf("Total de hashes SHA256 (assinatura): %llu\n", sha256_get_counter());

    // Tamanhos
    printf("Tamanho Publickey: %zu bytes\n", sizeof(keys.PKey));
    printf("Tamanho Assinatura: %zu bytes\n", sizeof(assinatura));

    return 0;
}
