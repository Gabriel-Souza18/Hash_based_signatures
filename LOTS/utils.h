#ifndef UTILS_H
#define UTILS_H

#include "keys.h"
#include <stdbool.h>
#include <stdint.h>

// Funções de leitura
PublicKeys* lerPkeys(char* caminho);
void lerMensagem(char* caminho, char *mensagem);
uint8_t (*lerAssinatura(char* caminho, int *tamanho))[KEY_SIZE];

// Funções de escrita
void escreverMensagem(char*caminho, char* mensagem);
void escreverPkeys(char* caminho, PublicKeys *pKeys);
void escreverAssinatura(char* caminho, uint8_t assinatura[256][KEY_SIZE], int tamanho);

#endif // UTILS_H