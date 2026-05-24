#ifndef HORS_KEYS_H
#define HORS_KEYS_H

#include <stddef.h>
#include <math.h>

// Valor padrão de HORS_T (pode ser sobrescrito via -DHORS_T no makefile)
#ifndef HORS_T
#define HORS_T 1024
#endif


#define HORS_N 32
#define KEY_SIZE 32

#if HORS_T == 256
    #define HORS_BITS_PER_INDEX 8
    #define HORS_K 32 
#elif HORS_T == 512
    #define HORS_BITS_PER_INDEX 9
    #define HORS_K 29  
#elif HORS_T == 1024
    #define HORS_BITS_PER_INDEX 10
    #define HORS_K 26  

#else
    #error "HORS_T deve ser 256, 512, 1024"
#endif

typedef struct {
    unsigned char SKeys[HORS_T][HORS_N];
    unsigned char PKeys[HORS_T][HORS_N];
} Keys;

typedef struct {
    unsigned char assinatura[HORS_K][HORS_N];
} Assinatura;

extern double hors_tempo_sk;
extern double hors_tempo_pk;

void gerarKeys(Keys* keys);
int selecionarIndices(unsigned char *hash, int *indices);
void assinarMensagem(const char* msg,int msg_len, 
                    Assinatura* assinatura, 
                    const unsigned char SKeys[HORS_T][HORS_N]);
int verificarAssinatura(const char* msg, int msg_len,
                        const Assinatura* assinatura, 
                        const unsigned char PKeys[HORS_T][HORS_N]);
void imprimirAssinatura(const Assinatura* assinatura);

#endif
