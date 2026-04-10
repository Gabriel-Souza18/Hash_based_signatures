#include <stdio.h>
#include <string.h>
#include <sodium.h>

int main(void) {
    if (sodium_init() < 0) {
        fprintf(stderr, "Erro: falha ao inicializar libsodium.\n");
        return 1;
    }

    unsigned char random_bytes[32];
    unsigned char hash[crypto_hash_sha256_BYTES];
    const char *msg = "teste-libsodium-ubuntu22";

    randombytes_buf(random_bytes, sizeof random_bytes);
    crypto_hash_sha256(hash, (const unsigned char *)msg, strlen(msg));

    printf("libsodium OK\n");
    printf("versao: %s\n", sodium_version_string());

    printf("random[0..7]: ");
    for (int i = 0; i < 8; i++) {
        printf("%02x", random_bytes[i]);
    }
    printf("\n");

    printf("sha256(msg)[0..7]: ");
    for (int i = 0; i < 8; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");

    return 0;
}
