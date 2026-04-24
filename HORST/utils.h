#ifndef UTILS_H
#define UTILS_H

#include "keys.h"

void salvarMensagem(char* mensagem);
void lerMensagem(char* mensagem);

int salvarSKeys(const char* arquivo, const Keys* keys);
int carregarSKeys(const char* arquivo, Keys* keys);
int salvarPublicKey(const char* arquivo, const PublicKey* pk);
int carregarPublicKey(const char* arquivo, PublicKey* pk);

int salvarAssinatura(const char* arquivo, const Assinatura* assinatura);
int carregarAssinatura(const char* arquivo, Assinatura* assinatura);

#endif
