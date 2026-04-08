#ifndef HORS_KEYS_H
#define HORS_KEYS_H

#include <stddef.h>

#define HORS_T 256
#define HORS_K 16
#define HORS_N 32

typedef struct {
    unsigned char SKeys[HORS_T][HORS_N];
    unsigned char PKeys[HORS_T][HORS_N];
} Keys;

typedef struct {
    unsigned char assinatura[HORS_K][HORS_N];
} Assinatura;

void gerarKeys(Keys* keys);
void assinarMensagem(const char* msg, Assinatura* assinatura, const unsigned char SKeys[HORS_T][HORS_N]);
int verificarAssinatura(const char* msg, const Assinatura* assinatura, const unsigned char PKeys[HORS_T][HORS_N]);

#endif
