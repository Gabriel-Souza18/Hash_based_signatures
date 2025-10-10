#include "keys.h"



void escreverMensagem(char* caminho, char*mensagem);
void escreverAssinatura(char* caminho, Assinatura* assinatura);
void escreverPkeys(char* caminho, PublicKeys pKeys);

char* lerMensagem(char* caminho);
Assinatura* lerAssinatura(char* caminho);
PublicKeys lerPkeys(char* caminho);

