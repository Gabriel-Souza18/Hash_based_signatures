#include <stdio.h>
#include <math.h>

#define W 16
#define N 32 // Tamanho da hash em bytes

// Calculando os valores manualmente para W=16 e N=32:
// L1 = ceil(N * 8 / log2(W)) = ceil(32 * 8 / log2(16)) = ceil(256 / 4) = 64
#define L1 64

// L2 = floor(log2(L1 * (W-1)) / log2(W)) + 1 = floor(log2(64 * 15) / log2(16)) + 1
//    = floor(log2(960) / 4) + 1 = floor(9.9/4) + 1 = floor(2.47) + 1 = 2 + 1 = 3
#define L2 3

#define L (L1 + L2)  // 67

typedef struct 
{
    char Sk[L][N];
}SecretKeys;

typedef struct{
    char PK[L][N];
}PublicKeys;

typedef struct{
    char assinatura[L][N];
}Assinatura;

typedef struct {
    char masks[W-1][N]; 
} Masks;


SecretKeys* mallocSkeys();
PublicKeys* mallocPkeys();
Masks* mallocMasks();

void generateSKeys(SecretKeys* );
void generatePKeys(PublicKeys*, SecretKeys*, Masks* );
void generateMasks(Masks*);

void mensageForBlocks(char*msg,int*output);
void chainFunction(char*src, int steps,Masks* r, char* output);
void calcularChecksum(const int* message_blocks, int* checksum_blocks);

void assinarMensagem(char*msg, Assinatura*, SecretKeys*, Masks*);

int verificarMensagem(char*msg, Assinatura*, Masks*, PublicKeys*);

