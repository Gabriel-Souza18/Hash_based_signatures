#ifndef KEYS_H
#define KEYS_H
#include <stdio.h>
#include <math.h>

#define W 16
#define N 32

// Cálculos corretos baseados no FIPS 205
#define L1 64//(int)ceil((8 * N) / log2(W))  // = 64
#define L2 3 //(int)(floor(log2(L1 * (W-1)) / log2(W)) + 1)  // = 3
#define L (L1 + L2)  // = 67

typedef struct {
    unsigned char Sk[L][N];
} SecretKeys;

typedef struct {
    unsigned char PK[L][N];
} PublicKeys;

typedef struct {
    unsigned char assinatura[L][N];
} Assinatura;

// Seeds (adicionar)
extern unsigned char PK_seed[N];
extern unsigned char SK_seed[N];

void initializeSeeds(void);

SecretKeys* mallocSkeys();
PublicKeys* mallocPkeys();
Assinatura* mallocAssinatura();

void generateSKeys(SecretKeys*);
void generatePKeys(PublicKeys*, SecretKeys*);

void mensageForBlocks(char* msgHash, int* output);
void chainFunction(unsigned char* src, int steps, unsigned char* output, unsigned char* ADRS_base, int start_index);
void calcularChecksum(const int* message_blocks, int* checksum_blocks);

void assinarMensagem(char* msg, Assinatura*, SecretKeys*);
int verificarMensagem(char* msg, Assinatura*, PublicKeys*);

#endif