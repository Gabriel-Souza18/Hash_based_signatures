#ifndef ARVORE_H
#define ARVORE_H

#define sizeTree 256
#include "../SHA256/sha256.h"

typedef struct no{
    char hash[SHA256_HEX_SIZE];
    struct no* filho_dir;
    struct no* filho_esq;
}No;



No * alocarFolha();
No * criarPai(No *filho_esq, No *filho_dir);

void escreverArvore(char *caminho,No *raiz);
void lerArvore(char* caminho);

void limparArvore(No *raiz);

#endif