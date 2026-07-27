#ifndef KEYS_H
#define KEYS_H

#include <stdint.h>
#include <stdbool.h>

#define KEY_SIZE 32  // 256 bits = 32 bytes 
typedef struct {
    uint8_t SK0[256][KEY_SIZE];  // 256 chaves de 32 bytes cada
    uint8_t SK1[256][KEY_SIZE];  // 256 chaves de 32 bytes cada
} SecretKeys;

typedef struct{
    uint8_t PK0[256][KEY_SIZE]; 
    uint8_t PK1[256][KEY_SIZE];
} PublicKeys;

SecretKeys *malloc_Skeys();
PublicKeys *malloc_Pkeys();
void generateSecretKeys(SecretKeys *keys);
void generatePublicKeys(PublicKeys *Pkeys, SecretKeys *Skeys);
void assinarMSG(const uint8_t msgHash[32], SecretKeys *sKeys, uint8_t assinatura[256][KEY_SIZE]);
bool verificarMSG(const uint8_t msgHash[32], PublicKeys *pKeys, uint8_t assinatura[256][KEY_SIZE]);

void printKeys(PublicKeys *Pkeys, SecretKeys *Skeys);
void printSecretKeyBits(uint8_t *key, int keyIndex, int bit);
void freeKeys(PublicKeys *Pkeys, SecretKeys *Skeys);
#endif // KEYS_H