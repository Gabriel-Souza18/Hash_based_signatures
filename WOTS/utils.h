#ifndef UTILS_H
#define UTILS_H

#include "keys.h"

void escreverMensagem(char* caminho, char*mensagem);
void escreverAssinatura(char* caminho, Assinatura* assinatura);
void escreverPkeys(char* caminho, PublicKeys * pKeys);
void escreverMasks(char* caminho, Masks* masks);

void lerMensagem(char* caminho, char* mensagem);
void lerAssinatura(char* caminho, Assinatura* assinatura);
void lerPkeys(char* caminho, PublicKeys* pKeys);
void lerMasks(char* caminho, Masks* masks);

#endif // UTILS_H