#include "Arvore.h"
#include "../SHA256/sha256.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define NUM_FOLHAS 8

No * mssTree(Folha* folhas);

int main(){

    fflush(stdout);  // Força impressão imediata

    // CORRIGIDO: Aloca e inicializa os ponteiros das folhas
    Folha *folhas = malloc(NUM_FOLHAS * sizeof(Folha));
    if (!folhas) {
        fprintf(stderr, "Erro ao alocar folhas\n");
        return 1;
    }
    
    // Inicializa os ponteiros de cada folha
    for(int i = 0; i < NUM_FOLHAS; i++){
        folhas[i].Skeys = mallocSkeys();
        folhas[i].Pkeys = mallocPkeys();
        folhas[i].Masks = mallocMasks();
    }
    

    fflush(stdout);
    
    No * raiz = mssTree(folhas);
    printf("Raiz = %s\n", raiz->hash);
    
    // Libera memória das folhas
    for(int i = 0; i < NUM_FOLHAS; i++){
        free(folhas[i].Skeys);
        free(folhas[i].Pkeys);
        free(folhas[i].Masks);
    }
    free(folhas);
    
    return 0;
}

No* mssTree(Folha* folhas){
    criarFolhas(folhas, NUM_FOLHAS);
    int andar = 1;
    int numNoAndar = NUM_FOLHAS/pow(2,andar);
    No **andarAtual = malloc(sizeof(No*)*numNoAndar);


    for (int i=0 ;i<numNoAndar; i++){
        andarAtual[i] = alocarNo();
    }
    //Conectando folhas a andar 1
    int j=0;
    for(int i=0; i<NUM_FOLHAS; i+=2){
        conectarFolhasAoNo(andarAtual[j],&folhas[i],&folhas[i+1]);
        j++;
    }

    
    while(numNoAndar>=1){
 
        printarAndar(andarAtual, numNoAndar, andar);
        int numNosNovoAndar =numNoAndar/2;

        No **novoAndar = malloc(sizeof(No*) * numNosNovoAndar);

        for (int i=0 ;i<numNoAndar; i++){
            novoAndar[i] = alocarNo();
        }

        j=0;
        for (int i = 0; i<numNoAndar;i++ ){
            novoAndar[j]->filho_esq =  andarAtual[i];
            novoAndar[j]->tipo_filho_esq = TIPO_NO;
            novoAndar[j]->filho_dir =  andarAtual[i+1];
            novoAndar[j]->tipo_filho_dir = TIPO_NO;

            criarPai(novoAndar[j]);
          j++;
        }
        free(andarAtual);
        andarAtual = novoAndar;
        numNoAndar = numNosNovoAndar;
        andar++;
        
    }

    printf("TErminou de gerar Arvore");
    printf("ultimo no: %s", andarAtual[0]->hash );
    limparArvore(andarAtual[0]);
    return andarAtual[0];
}