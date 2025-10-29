#include "Arvore.h"
#include "../SHA256/sha256.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

No *alocarFolha(){
    No *no = (No*)malloc(sizeof(No));
    if (no == NULL) {
        fprintf(stderr, "Erro ao alocar memória para o nó\n");
        exit(1);
    }
    return no;
}

No * criarPai(No *filho_esq, No *filho_dir){
    No* pai = alocarFolha();
    char concHashs[SHA256_HEX_SIZE * 2 + 1];
    
    strcpy(concHashs, filho_esq->hash);
    strcat(concHashs, filho_dir->hash); 

    sha256_hex(concHashs, strlen(concHashs), pai->hash);

    pai->filho_esq = filho_esq;
    pai->filho_dir = filho_dir;

    return pai;
}

void escreverArvore(char *caminho,No* raiz);
void lerArvore(char* caminho);

void limparArvore(No *raiz);