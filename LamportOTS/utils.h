#ifndef UTILS_H
#define UTILS_H

#include "keys.h"
#include <stdbool.h>

// Funções de leitura
PublicKeys* lerPkeys(char* caminho);
void lerMensagem(char* caminho, char *mensagem);
char** lerAssinatura(char* caminho, int *tamanho);

// Funções de escrita
void escreverMensagem(char*caminho, char* mensagem);
void escreverPkeys(char* caminho, PublicKeys *pKeys);
void escreverAssinatura(char* caminho, char **assinatura, int tamanho);

#endif // UTILS_H