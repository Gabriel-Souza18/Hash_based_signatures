#include "sha256.h"
#include <openssl/sha.h>
#include <string.h>
#include <stdio.h>

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
 * Compute SHA256 in hexadecimal using OpenSSL
 * Output is a hexadecimal string of 64 characters + null terminator
 */
void sha256_hex(const void *src, size_t n_bytes, char *dst_hex65) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char *)src, n_bytes, digest);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(dst_hex65 + (i * 2), "%02x", digest[i]);
    }
    dst_hex65[SHA256_DIGEST_LENGTH * 2] = '\0';
    
    // Incrementa contador
    sha256_counter++;
}

/*
 * Compute SHA256 in raw bytes using OpenSSL
 * Output is 32 bytes (256 bits)
 */
void sha256_bytes(const void *src, size_t n_bytes, void *dst_bytes32) {
    SHA256((const unsigned char *)src, n_bytes, (unsigned char *)dst_bytes32);
    
    // Incrementa contador
    sha256_counter++;
}

/*
 * Initialize a streaming SHA256 context using OpenSSL
 * Reinterprets the sha256 struct buffer as a SHA256_CTX
 */
void sha256_init(struct sha256 *sha) {
    if (!sha) return;
    
    /* Cast the buffer area to SHA256_CTX for OpenSSL */
    SHA256_CTX *ctx = (SHA256_CTX *)sha->buffer;
    SHA256_Init(ctx);
}

/*
 * Append data to a streaming SHA256 computation
 */
void sha256_append(struct sha256 *sha, const void *data, size_t n_bytes) {
    if (!sha) return;
    
    SHA256_CTX *ctx = (SHA256_CTX *)sha->buffer;
    SHA256_Update(ctx, (const unsigned char *)data, n_bytes);
}

/*
 * Finalize and get hexadecimal result
 */
void sha256_finalize_hex(struct sha256 *sha, char *dst_hex65) {
    if (!sha || !dst_hex65) return;
    
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256_CTX *ctx = (SHA256_CTX *)sha->buffer;
    SHA256_Final(digest, ctx);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(dst_hex65 + (i * 2), "%02x", digest[i]);
    }
    dst_hex65[SHA256_DIGEST_LENGTH * 2] = '\0';
    
    // Incrementa contador
    sha256_counter++;
}

/*
 * Finalize and get raw bytes result
 */
void sha256_finalize_bytes(struct sha256 *sha, void *dst_bytes32) {
    if (!sha || !dst_bytes32) return;
    
    SHA256_CTX *ctx = (SHA256_CTX *)sha->buffer;
    SHA256_Final((unsigned char *)dst_bytes32, ctx);
    
    // Incrementa contador
    sha256_counter++;
}
