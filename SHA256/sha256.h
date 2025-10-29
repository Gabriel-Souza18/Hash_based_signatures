#ifndef SHA256_H
#define SHA256_H

#include <stddef.h>
#include <stdint.h>

#define SHA256_HEX_SIZE (64 + 1)
#define SHA256_BYTES_SIZE 32

// Contador global de operações SHA256
extern unsigned long long sha256_counter;

// Funções para gerenciar o contador
void sha256_reset_counter(void);
unsigned long long sha256_get_counter(void);

/*
 * Compute the SHA-256 checksum of a memory region given a pointer and
 * the size of that memory region.
 * The output is a hexadecimal string of 65 characters.
 * The last character will be the null-character.
 */
void sha256_hex(const void *src, size_t n_bytes, char *dst_hex65);

void sha256_bytes(const void *src, size_t n_bytes, void *dst_bytes32);

typedef struct sha256 {
    uint32_t state[8];
    uint8_t buffer[64];
    uint64_t n_bits;
    uint8_t buffer_counter;
} sha256;

/* Functions to compute streaming SHA-256 checksums. */
void sha256_init(struct sha256 *sha);
void sha256_append(struct sha256 *sha, const void *data, size_t n_bytes);
void sha256_finalize_hex(struct sha256 *sha, char *dst_hex65);
void sha256_finalize_bytes(struct sha256 *sha, void *dst_bytes32);

#endif
