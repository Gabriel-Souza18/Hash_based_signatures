#ifndef ARVORE_H
#define ARVORE_H

#define sizeTree 256
#include "../SHA256/sha256.h"

typedef struct no{
    char hash[SHA256_HEX_SIZE];
    struct no* filho_dir;
    struct no* filho_esq;
}No;

typedef struct {
    int quantFolhas;
    No** nos;
}Andar;


// Alocação
No * alocarNo();
Andar* alocarAndar(int quantFolhas);

void criarPai(No *pai);

// Liberação de memória
void liberarNo(No *no);
void liberarAndar(Andar *andar);
void limparArvore(No *raiz);

// I/O
void escreverArvore(char *caminho,No *raiz);
void lerArvore(char* caminho);

// Visualização
void printarArvore(No *raiz);
void printarArvoreNivel(No *raiz, int nivel);
void printarArvoreCompleta(No *raiz);

#endif