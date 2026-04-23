#ifndef HORS_UTILS_H
#define HORS_UTILS_H

#include "keys.h"

// Salva e lê mensagens em arquivo de texto
void salvarMensagem(char* mensagem);
void lerMensagem(char* mensagem);

// Salva e lê chaves públicas em arquivo binário
void salvarPkeys(unsigned char *PKeys);
void lerPkeys(unsigned char *PKeys);

// Salva e lê assinaturas em arquivo binário
void salvarAssinatura(Assinatura *assinatura);
void lerAssinatura(Assinatura *assinatura);

void iniciar_metricas();
void printar_metricas();
#endif
