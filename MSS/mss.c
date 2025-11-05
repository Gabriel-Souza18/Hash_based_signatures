#include "Arvore.h"
#include "../SHA256/sha256.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define NUM_FOLHAS 8 // sempre tem que ser 2^n

No * mssTree(Folha* folhas);

int main(){

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
    
    // Imprime a estrutura completa da árvore
    imprimirArvore(raiz);

    AssinaturaMSS * assinatura = alocarAssinatura();

    criarAssinatura(assinatura, raiz, &folhas[0],0, NUM_FOLHAS);

    printf("\n=== ASSINATURA MSS ===\n");
    printf("Public Key Geral (Raiz): %s\n", assinatura->PublicKeysGeral);
    printf("Altura da Árvore: %d\n", assinatura->alturaArvore);
    printf("Total de Folhas: %d\n", assinatura->totalFolhas);
    printf("Índice da Folha Usada: %d\n", assinatura->indiceFolha);
    printf("Hash da Folha Usada: %s\n", assinatura->folhaUsada->hash);
    printf("\nCaminho de Autenticação (%d hashes):\n", assinatura->tamanhoCaminho);
    for(int i = 0; i < assinatura->tamanhoCaminho; i++){
        printf("  [%d] %s\n", i, assinatura->caminho[i]);
    }
    printf("======================\n\n");
    
    free(assinatura);

    // limpeza
    for(int i = 0; i < NUM_FOLHAS; i++){
        free(folhas[i].Skeys);
        free(folhas[i].Pkeys);
        free(folhas[i].Masks);
    }
    free(folhas);
    limparArvore(raiz);


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

    
    while(numNoAndar > 1){
 

        int numNosNovoAndar = numNoAndar/2;

        No **novoAndar = malloc(sizeof(No*) * numNosNovoAndar);

        for (int i=0; i<numNosNovoAndar; i++){
            novoAndar[i] = alocarNo();
        }

        j=0;
        for (int i = 0; i<numNoAndar; i+=2){
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

    printf("\nTerminou de gerar Arvore\n");
    printf("ultimo no: %s\n", andarAtual[0]->hash);
    No* raiz = andarAtual[0];
    free(andarAtual);
    return raiz;
}