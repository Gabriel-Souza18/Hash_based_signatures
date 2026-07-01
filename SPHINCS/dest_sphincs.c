#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sodium.h>

#include "sphincs.h"
#include "../SHA256/sha256.h"

static int ler_arquivo(const char *caminho, void *dados, size_t tamanho) {
    FILE *f = fopen(caminho, "rb");
    if (!f) return 0;
    size_t lido = fread(dados, 1, tamanho, f);
    fclose(f);
    return lido == tamanho;
}

static int ler_mensagem(const char *caminho, unsigned char *buf, size_t max_len, size_t *out_len) {
    FILE *f = fopen(caminho, "r");
    if (!f) return 0;
    size_t lido = fread(buf, 1, max_len - 1, f);
    buf[lido] = '\0';
    *out_len = lido;
    fclose(f);
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Uso: %s <caminho_mensagem> <caminho_chave_publica> <caminho_assinatura> <caminho_sk_seed>\n", argv[0]);
        return 1;
    }

    char *caminhoMsg = argv[1];
    char *caminhoPkey = argv[2];
    char *caminhoAssinatura = argv[3];
    char *caminhoSkSeed = argv[4];

    if (sodium_init() < 0) {
        fprintf(stderr, "Erro ao inicializar libsodium\n");
        return 1;
    }

    sha256_reset_counter();

    unsigned char mensagem[4096];
    size_t msg_len = 0;
    if (!ler_mensagem(caminhoMsg, mensagem, sizeof(mensagem), &msg_len)) {
        fprintf(stderr, "Erro ao ler a mensagem de %s\n", caminhoMsg);
        return 1;
    }

    SphincsPublicKey pk;
    if (!ler_arquivo(caminhoPkey, &pk, sizeof(SphincsPublicKey))) {
        fprintf(stderr, "Erro ao ler a chave pública de %s\n", caminhoPkey);
        return 1;
    }

    SphincsSignature sig;
    if (!ler_arquivo(caminhoAssinatura, &sig, sizeof(SphincsSignature))) {
        fprintf(stderr, "Erro ao ler a assinatura de %s\n", caminhoAssinatura);
        return 1;
    }

    unsigned char sk_seed[SPHINCS_N];
    if (!ler_arquivo(caminhoSkSeed, sk_seed, SPHINCS_N)) {
        fprintf(stderr, "Erro ao ler sk_seed de %s\n", caminhoSkSeed);
        return 1;
    }

    clock_t inicio_verif = clock();
    int resultado = sphincs_verify(&sig, mensagem, msg_len, &pk, sk_seed);
    clock_t fim_verif = clock();

    printf("Verificação: %s\n", resultado ? "VÁLIDA" : "INVÁLIDA");
    printf("Tempo Verificação: %lfs\n", (double)(fim_verif - inicio_verif) / CLOCKS_PER_SEC);
    printf("Total de hashes SHA256 na verificação: %llu\n", sha256_get_counter());

    return resultado ? 0 : 1;
}
