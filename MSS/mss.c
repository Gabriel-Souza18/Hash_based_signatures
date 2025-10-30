#include "Arvore.h"
#include "../SHA256/sha256.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define NUM_FOLHAS 256 

No* criarTreeMSS(Andar* folhas);
Andar* criarFolhas();

void main(){
    srand(time(NULL));
    
    printf("Criando %d folhas...\n", NUM_FOLHAS);
    Andar* folhas = criarFolhas();
    
    printf("Construindo árvore de Merkle...\n");
    No* raiz = criarTreeMSS(folhas);

    printf("\nÁrvore construída! Imprimindo...\n\n");
    printarArvoreCompleta(raiz);
    
    limparArvore(raiz);
    printf("\nConcluído!\n");
}

Andar* criarFolhas(){
    Andar* andar = alocarAndar(NUM_FOLHAS);
    

    for (int i = 0; i < NUM_FOLHAS; i++){
        char buffer[128];
        sprintf(buffer, "folha_%d_%d", i, rand());
        sha256_hex(buffer, strlen(buffer), andar->nos[i]->hash);
    }
    
    return andar;
}

No* criarTreeMSS(Andar* folhas){
    int faltam = NUM_FOLHAS;
    Andar *andarAtual = folhas;
    
    while (faltam > 1){
        int quantPais = andarAtual->quantFolhas / 2;
        Andar *novoAndar = (Andar*)malloc(sizeof(Andar));
        novoAndar->quantFolhas = quantPais;
        novoAndar->nos = (No**)malloc(sizeof(No*) * quantPais);
        
        int j = 0;
        for (int i = 0; i < andarAtual->quantFolhas; i += 2){
            No* novoNo = alocarNo();
            novoNo->filho_esq = andarAtual->nos[i];
            novoNo->filho_dir = andarAtual->nos[i + 1];
            criarPai(novoNo);
            
            novoAndar->nos[j] = novoNo;
            j++;
        }
        
        free(andarAtual->nos);
        free(andarAtual);
        
        andarAtual = novoAndar;
        faltam = faltam / 2;
    }
    
    No* raiz = andarAtual->nos[0];
    
    free(andarAtual->nos);
    free(andarAtual);
    
    return raiz;
}