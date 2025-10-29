#include "Arvore.h"
#include "../SHA256/sha256.h"

#include <stdlib.h>
#include <stdio.h>

void main(){
    No* filho1 = alocarFolha();
    No* filho2 = alocarFolha();

    filho1->filho_esq = NULL;
    filho2->filho_esq = NULL;

    filho1->filho_dir = NULL;
    filho2->filho_dir = NULL;

    sha256_hex("1", 1, filho1->hash);
    sha256_hex("2", 1, filho2->hash);


    No* pai = criarPai(filho1,filho2);
    printf("Hashs filhos:\n1- %s\n2- %s\n",filho1->hash, filho2->hash );
    printf("Hash pai: %s\n", pai->hash);

    free(filho1);
    free(filho2);
    free(pai);
}