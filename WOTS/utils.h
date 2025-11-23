#ifndef UTILS_H
#define UTILS_H

#include "keys.h"

void escreverMensagem(char* caminho, char*mensagem);
void escreverAssinatura(char* caminho, Assinatura* assinatura);
void escreverPkeys(char* caminho, PublicKeys * pKeys, unsigned char* pk_seed, unsigned char* sk_seed);

void lerMensagem(char* caminho, char* mensagem);
void lerAssinatura(char* caminho, Assinatura* assinatura);
void lerPkeys(char* caminho, PublicKeys* pKeys, unsigned char* pk_seed, unsigned char* sk_seed);


#endif // UTILS_H