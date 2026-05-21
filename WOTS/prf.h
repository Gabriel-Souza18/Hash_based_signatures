#ifndef PRF_H
#define PRF_H

#include <stdint.h>

#define W 16
#define N 32

// Compressão de ADRS (32 bytes → 22 bytes)
void compressADRS(const unsigned char* ADRS_32bytes, unsigned char* ADRS_22bytes);

// PRF para parâmetros SHA2
void PRF_SHA2(unsigned char* output, const unsigned char* PK_seed, 
              const unsigned char* SK_seed, const unsigned char* ADRS_32bytes, int n);

// Configuração de ADRS para diferentes tipos
void setADRS_WOTS_PRF(unsigned char* ADRS, int key_index);
void setADRS_WOTS_HASH(unsigned char* ADRS, int key_index, int chain_index, int hash_index);
void setADRS_WOTS_MASK(unsigned char* ADRS, int key_index, int step_index);

// Função F para chain function
void F_function(unsigned char* output, const unsigned char* PK_seed, 
                const unsigned char* ADRS_32bytes, const unsigned char* input);

#endif