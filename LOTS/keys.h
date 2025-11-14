#ifndef KEYS_H
#define KEYS_H

#include <stdint.h>

#define SECRET_KEY_SIZE 32  // 256 bits = 32 bytes

// Chaves secretas agora são arrays de bytes (máscaras de bits)
typedef struct {
    uint8_t SK0[256][SECRET_KEY_SIZE];  // 256 chaves de 32 bytes cada
    uint8_t SK1[256][SECRET_KEY_SIZE];  // 256 chaves de 32 bytes cada
} SecretKeys;

// Chaves públicas continuam como strings hex do SHA256
typedef struct{
    char* PK0[256]; 
    char* PK1[256];
} PublicKeys;

SecretKeys *malloc_Skeys();
PublicKeys *malloc_Pkeys();
void generateSecretKeys(SecretKeys *keys);
void generatePublicKeys(PublicKeys *Pkeys, SecretKeys *Skeys);
void printKeys(PublicKeys *Pkeys, SecretKeys *Skeys);
void printSecretKeyBits(uint8_t *key, int keyIndex, int bit);
void freeKeys(PublicKeys *Pkeys, SecretKeys *Skeys);
#endif // KEYS_H