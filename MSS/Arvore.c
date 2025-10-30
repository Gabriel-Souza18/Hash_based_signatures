#include "Arvore.h"
#include "../SHA256/sha256.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

No *alocarNo(){
    No *no = (No*)malloc(sizeof(No));
    if (no == NULL) {
        fprintf(stderr, "Erro ao alocar memória para o nó\n");
        exit(1);
    }
    no->filho_esq = NULL;
    no->filho_dir = NULL;
    return no;
}

Andar* alocarAndar(int quantFolhas){
    Andar* andar = (Andar*)malloc(sizeof(Andar));
    if (andar == NULL) {
        fprintf(stderr, "Erro ao alocar memória para o andar\n");
        exit(1);
    }
    
    andar->quantFolhas = quantFolhas;
    andar->nos = (No**)malloc(sizeof(No*) * quantFolhas);
    if (andar->nos == NULL) {
        fprintf(stderr, "Erro ao alocar memória para os nós do andar\n");
        free(andar);
        exit(1);
    }
    
    // Aloca cada nó do andar
    for (int i = 0; i < quantFolhas; i++) {
        andar->nos[i] = alocarNo();
    }
    
    return andar;
}

void criarPai(No* pai){
    char concHashs[SHA256_HEX_SIZE * 2 + 1];
    
    strcpy(concHashs, pai->filho_esq->hash);
    strcat(concHashs, pai->filho_dir->hash); 

    sha256_hex(concHashs, strlen(concHashs), pai->hash);
}

void liberarNo(No *no){
    if (no == NULL) return;
    free(no);
}

void liberarAndar(Andar *andar){
    if (andar == NULL) return;
    
    // Libera cada nó do andar
    if (andar->nos != NULL) {
        for (int i = 0; i < andar->quantFolhas; i++) {
            liberarNo(andar->nos[i]);
        }
        free(andar->nos);
    }
    
    free(andar);
}

void limparArvore(No *raiz){
    if (raiz == NULL) return;
    
    // Recursivamente limpa os filhos
    limparArvore(raiz->filho_esq);
    limparArvore(raiz->filho_dir);
    
    // Libera o nó atual
    free(raiz);
}

// Função auxiliar para calcular altura da árvore
int alturaArvore(No *raiz){
    if (raiz == NULL) return 0;
    
    int alturaEsq = alturaArvore(raiz->filho_esq);
    int alturaDir = alturaArvore(raiz->filho_dir);
    
    return 1 + (alturaEsq > alturaDir ? alturaEsq : alturaDir);
}

// Função auxiliar para contar nós em um nível
int contarNosNivel(No *raiz, int nivel){
    if (raiz == NULL) return 0;
    if (nivel == 0) return 1;
    
    return contarNosNivel(raiz->filho_esq, nivel - 1) + 
           contarNosNivel(raiz->filho_dir, nivel - 1);
}

// Função auxiliar para printar um nível específico
void printarNivel(No *raiz, int nivel, int espacamento){
    if (raiz == NULL){
        for (int i = 0; i < espacamento; i++) printf(" ");
        printf("        ");
        for (int i = 0; i < espacamento; i++) printf(" ");
        return;
    }
    
    if (nivel == 0){
        for (int i = 0; i < espacamento; i++) printf(" ");
        printf("%.8s", raiz->hash);
        for (int i = 0; i < espacamento; i++) printf(" ");
    } else {
        printarNivel(raiz->filho_esq, nivel - 1, espacamento / 2);
        printarNivel(raiz->filho_dir, nivel - 1, espacamento / 2);
    }
}

// Printar árvore de forma simples (percurso in-order)
void printarArvore(No *raiz){
    if (raiz == NULL) return;
    
    printarArvore(raiz->filho_esq);
    printf("Hash: %s\n", raiz->hash);
    printarArvore(raiz->filho_dir);
}

// Printar árvore mostrando níveis
void printarArvoreNivel(No *raiz, int nivel){
    if (raiz == NULL){
        printf("Árvore vazia\n");
        return;
    }
    
    int altura = alturaArvore(raiz);
    
    if (nivel < 0 || nivel >= altura){
        printf("Nível inválido. Altura da árvore: %d (níveis 0-%d)\n", 
               altura, altura - 1);
        return;
    }
    
    printf("=== Nível %d ===\n", nivel);
    int nosNivel = contarNosNivel(raiz, nivel);
    printf("Número de nós: %d\n", nosNivel);
    
    printarNivel(raiz, nivel, 0);
    printf("\n");
}

// Printar árvore completa de forma hierárquica
void printarArvoreCompleta(No *raiz){
    if (raiz == NULL){
        printf("Árvore vazia\n");
        return;
    }
    
    int altura = alturaArvore(raiz);
    printf("===================Altura: %d níveis===================\n", altura);
    
    for (int i = 0; i < altura; i++){
        int nosNivel = contarNosNivel(raiz, i);
        int espacamento = (1 << (altura - i - 1));
        
        printf("Nível %d (%d nós):\n", i, nosNivel);
        printarNivel(raiz, i, espacamento);
        printf("\n\n");
    }
    
    printf("Raiz (completa): %s\n", raiz->hash);
}

void escreverArvore(char *caminho,No* raiz);
void lerArvore(char* caminho);