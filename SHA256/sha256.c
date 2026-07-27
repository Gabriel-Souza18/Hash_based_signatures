#include "sha256.h"
#include <string.h>
#include <stdio.h>

#ifdef ESP32
#include "mbedtls/sha256.h"
#define SHA256_CTX mbedtls_sha256_context
#else
#include <openssl/sha.h>
#endif

// Contador global de operações SHA256
unsigned long long sha256_counter = 0;

// Funções para gerenciar o contador
void sha256_reset_counter(void) {
    sha256_counter = 0;
}

unsigned long long sha256_get_counter(void) {
    return sha256_counter;
}

/*
 * Compute SHA256 in hexadecimal
 * Output is a hexadecimal string of 64 characters + null terminator
 */
void sha256_hex(const void *src, size_t n_bytes, char *dst_hex65) {
    unsigned char digest[SHA256_BYTES_SIZE];
#ifdef ESP32
    mbedtls_sha256((const unsigned char *)src, n_bytes, digest, 0);
#else
    SHA256((const unsigned char *)src, n_bytes, digest);
#endif
    
    for (int i = 0; i < SHA256_BYTES_SIZE; i++) {
        sprintf(dst_hex65 + (i * 2), "%02x", digest[i]);
    }
    dst_hex65[SHA256_BYTES_SIZE * 2] = '\0';
    
    // Incrementa contador
    sha256_counter++;
}

/*
 * Compute SHA256 in raw bytes
 * Output is 32 bytes (256 bits)
 */
void sha256_bytes(const void *src, size_t n_bytes, void *dst_bytes32) {
#ifdef ESP32
    mbedtls_sha256((const unsigned char *)src, n_bytes, (unsigned char *)dst_bytes32, 0);
#else
    SHA256((const unsigned char *)src, n_bytes, (unsigned char *)dst_bytes32);
#endif
    
    // Incrementa contador
    sha256_counter++;
}

/*
 * Initialize a streaming SHA256 context
 */
void sha256_init(struct sha256 *sha) {
    if (!sha) return;
    
#ifdef ESP32
    SHA256_CTX *ctx = (SHA256_CTX *)sha->buffer;
    mbedtls_sha256_init(ctx);
    mbedtls_sha256_starts(ctx, 0);
#else
    SHA256_CTX *ctx = (SHA256_CTX *)sha->buffer;
    SHA256_Init(ctx);
#endif
}

/*
 * Append data to a streaming SHA256 computation
 */
void sha256_append(struct sha256 *sha, const void *data, size_t n_bytes) {
    if (!sha) return;
    
#ifdef ESP32
    SHA256_CTX *ctx = (SHA256_CTX *)sha->buffer;
    mbedtls_sha256_update(ctx, (const unsigned char *)data, n_bytes);
#else
    SHA256_CTX *ctx = (SHA256_CTX *)sha->buffer;
    SHA256_Update(ctx, (const unsigned char *)data, n_bytes);
#endif
}

/*
 * Finalize and get hexadecimal result
 */
void sha256_finalize_hex(struct sha256 *sha, char *dst_hex65) {
    if (!sha || !dst_hex65) return;
    
    unsigned char digest[SHA256_BYTES_SIZE];
#ifdef ESP32
    SHA256_CTX *ctx = (SHA256_CTX *)sha->buffer;
    mbedtls_sha256_finish(ctx, digest);
    mbedtls_sha256_free(ctx);
#else
    SHA256_CTX *ctx = (SHA256_CTX *)sha->buffer;
    SHA256_Final(digest, ctx);
#endif
    
    for (int i = 0; i < SHA256_BYTES_SIZE; i++) {
        sprintf(dst_hex65 + (i * 2), "%02x", digest[i]);
    }
    dst_hex65[SHA256_BYTES_SIZE * 2] = '\0';
    
    // Incrementa contador
    sha256_counter++;
}

/*
 * Finalize and get raw bytes result
 */
void sha256_finalize_bytes(struct sha256 *sha, void *dst_bytes32) {
    if (!sha || !dst_bytes32) return;
    
#ifdef ESP32
    SHA256_CTX *ctx = (SHA256_CTX *)sha->buffer;
    mbedtls_sha256_finish(ctx, (unsigned char *)dst_bytes32);
    mbedtls_sha256_free(ctx);
#else
    SHA256_CTX *ctx = (SHA256_CTX *)sha->buffer;
    SHA256_Final((unsigned char *)dst_bytes32, ctx);
#endif
    
    // Incrementa contador
    sha256_counter++;
}
