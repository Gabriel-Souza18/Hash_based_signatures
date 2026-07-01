#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sodium.h>

#include "sphincs.h"
#include "../SHA256/sha256.h"

static int salvar_arquivo(const char *caminho, const void *dados, size_t tamanho) {
    FILE *f = fopen(caminho, "wb");
    if (!f) return 0;
    size_t escrito = fwrite(dados, 1, tamanho, f);
    fclose(f);
    return escrito == tamanho;
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
    if (argc < 6) {
        fprintf(stderr, "Uso: %s <caminho_mensagem> <caminho_chave_publica> <caminho_assinatura> <caminho_sk_seed> <caminho_chave_secreta>\n", argv[0]);
        return 1;
    }

    char *caminhoMsg = argv[1];
    char *caminhoPkey = argv[2];
    char *caminhoAssinatura = argv[3];
    char *caminhoSkSeed = argv[4];
    char *caminhoSkey = argv[5];

    if (sodium_init() < 0) {
        fprintf(stderr, "Erro ao inicializar libsodium\n");
        return 1;
    }

    sha256_reset_counter();

    SphincsPublicKey pk;
    SphincsSecretKey sk;

    clock_t inicio_keygen = clock();
    sphincs_keygen(&pk, &sk);
    clock_t fim_keygen = clock();

    unsigned char mensagem[4096];
    size_t msg_len = 0;
    if (!ler_mensagem(caminhoMsg, mensagem, sizeof(mensagem), &msg_len)) {
        fprintf(stderr, "Erro ao ler a mensagem de %s\n", caminhoMsg);
        return 1;
    }

    SphincsSignature sig;

    clock_t inicio_sign = clock();
    sphincs_sign(&sig, mensagem, msg_len, &sk);
    clock_t fim_sign = clock();

    printf("Tempo Geração de Chaves: %lfs\n", (double)(fim_keygen - inicio_keygen) / CLOCKS_PER_SEC);
    printf("Tempo Assinatura: %lfs\n", (double)(fim_sign - inicio_sign) / CLOCKS_PER_SEC);
    printf("Total de hashes SHA256 (sign + keygen): %llu\n", sha256_get_counter());
    printf("Tamanho Assinatura: %lu bytes\n", (unsigned long)sizeof(SphincsSignature));
    printf("Tamanho Chave Pública: %lu bytes\n", (unsigned long)sizeof(SphincsPublicKey));
    printf("Tamanho Chave Secreta: %lu bytes\n", (unsigned long)sizeof(SphincsSecretKey));

    if (!salvar_arquivo(caminhoPkey, &pk, sizeof(SphincsPublicKey))) {
        fprintf(stderr, "Erro ao salvar chave pública em %s\n", caminhoPkey);
        return 1;
    }

    if (!salvar_arquivo(caminhoAssinatura, &sig, sizeof(SphincsSignature))) {
        fprintf(stderr, "Erro ao salvar assinatura em %s\n", caminhoAssinatura);
        return 1;
    }

    if (!salvar_arquivo(caminhoSkSeed, sk.SK_seed, SPHINCS_N)) {
        fprintf(stderr, "Erro ao salvar sk_seed em %s\n", caminhoSkSeed);
        return 1;
    }

    if (!salvar_arquivo(caminhoSkey, &sk, sizeof(SphincsSecretKey))) {
        fprintf(stderr, "Erro ao salvar chave secreta em %s\n", caminhoSkey);
        return 1;
    }

    printf("Chaves e Assinatura gravadas em disco com sucesso!\n");
    return 0;
}
