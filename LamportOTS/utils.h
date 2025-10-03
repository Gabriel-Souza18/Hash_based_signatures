#ifndef UTILS_H
#define UTILS_H

#include "keys.h"
#include <stdbool.h>

// Funções de leitura
PublicKeys* lerPkeys(char* caminho);
bool* lerMensagem(char* caminho, bool mensagem[256]);
char** lerAssinatura(char* caminho, int *tamanho);

// Funções de escrita
void escreverMensagem(char*caminho, bool* mensagem);
void escreverChavesPublicas(char* caminho, PublicKeys *pKeys);
void escreverAssinatura(char* caminho, char **assinatura, int tamanho);

#endif // UTILS_H