#ifndef HORS_KEYS_H
#define HORS_KEYS_H

#include <stddef.h>

// Valor padrão de HORS_T (pode ser sobrescrito via -DHORS_T no makefile)
#ifndef HORS_T
#define HORS_T 1024
#endif

#define HORS_K 16
#define HORS_N 32
#define KEY_SIZE 32

typedef struct {
    unsigned char SKeys[HORS_T][HORS_N];
    unsigned char PKeys[HORS_T][HORS_N];
} Keys;

typedef struct {
    unsigned char assinatura[HORS_K][HORS_N];
} Assinatura;

void gerarKeys(Keys* keys);
int selecionarIndices(unsigned char *hash, int *indices);
void assinarMensagem(const char* msg, Assinatura* assinatura, const unsigned char SKeys[HORS_T][HORS_N]);
int verificarAssinatura(const char* msg, const Assinatura* assinatura, const unsigned char PKeys[HORS_T][HORS_N]);

#endif
