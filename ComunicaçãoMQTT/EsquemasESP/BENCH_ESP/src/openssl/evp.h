#ifndef BENCH_ESP_OPENSSL_EVP_COMPAT_H
#define BENCH_ESP_OPENSSL_EVP_COMPAT_H

#include <stddef.h>
#include <stdint.h>
#include "mbedtls/sha256.h"
#include "mbedtls/sha512.h"

typedef enum {
    BENCH_EVP_SHA256,
    BENCH_EVP_SHA512
} bench_evp_digest_type;

typedef bench_evp_digest_type EVP_MD;

static inline const EVP_MD *EVP_sha256(void) {
    static const EVP_MD type = BENCH_EVP_SHA256;
    return &type;
}

static inline const EVP_MD *EVP_sha512(void) {
    static const EVP_MD type = BENCH_EVP_SHA512;
    return &type;
}

static inline int EVP_Digest(const void *data, size_t data_len,
                             unsigned char *digest, unsigned int *digest_len,
                             const EVP_MD *type, void *impl) {
    (void)impl;
    if (!data || !digest || !digest_len || !type) return 0;

    if (*type == BENCH_EVP_SHA512) {
        if (mbedtls_sha512_ret((const unsigned char *)data, data_len,
                               digest, 0) != 0) {
            return 0;
        }
        *digest_len = 64;
        return 1;
    }

    if (mbedtls_sha256_ret((const unsigned char *)data, data_len,
                           digest, 0) != 0) {
        return 0;
    }
    *digest_len = 32;
    return 1;
}

#endif
