#include "Arvore.h"
#include "../SHA256/sha256.h"
#include "../WOTS/keys.h"

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
    no->tipo_filho_esq = TIPO_NO;
    no->tipo_filho_dir = TIPO_NO;
    no->nivel = 0;
    memset(no->hash, 0, SHA256_HEX_SIZE);
    return no;
}

Folha * alocarFolha(){
    Folha *folha = (Folha*)malloc(sizeof(Folha));
    if (folha == NULL){
        fprintf(stderr, "ERRO: nao foi possivel alocar Folha\n");
        exit(1);
    }
    
    // Aloca as estruturas WOTS dentro da folha
    folha->Skeys = mallocSkeys();
    folha->Pkeys = mallocPkeys();
    folha->Masks = mallocMasks();
    memset(folha->hash, 0, SHA256_HEX_SIZE);
    
    return folha;
}
AssinaturaMSS * alocarAssinatura(){
    AssinaturaMSS *assinatura = (AssinaturaMSS*)malloc(sizeof(AssinaturaMSS));
    if (assinatura == NULL) {
        fprintf(stderr, "Erro ao alocar memória para a assinatura\n");
        exit(1);
    }
    return assinatura;
}
//Geração
void criarPai(No* pai){
    char concHashs[SHA256_HEX_SIZE * 2 + 1];
    char hash_esq[SHA256_HEX_SIZE];
    char hash_dir[SHA256_HEX_SIZE];
    
    // Pega o hash do filho esquerdo (pode ser No ou Folha)
    if (pai->tipo_filho_esq == TIPO_NO) {
        No* filho = (No*)pai->filho_esq;
        strcpy(hash_esq, filho->hash);
    } else {
        Folha* folha = (Folha*)pai->filho_esq;
        strcpy(hash_esq, folha->hash);
    }
    
    // Pega o hash do filho direito (pode ser No ou Folha)
    if (pai->tipo_filho_dir == TIPO_NO) {
        No* filho = (No*)pai->filho_dir;
        strcpy(hash_dir, filho->hash);
    } else {
        Folha* folha = (Folha*)pai->filho_dir;
        strcpy(hash_dir, folha->hash);
    }
    
    // Concatena e gera hash do pai
    strcpy(concHashs, hash_esq);
    strcat(concHashs, hash_dir); 
    sha256_hex(concHashs, strlen(concHashs), pai->hash);
}

void criarFolhas(Folha *folhas, int quantFolhas){
    for(int i = 0; i < quantFolhas; i++){
        // Gera chaves WOTS para cada folha
        generateMasks(folhas[i].Masks);
        generateSKeys(folhas[i].Skeys);
        generatePKeys(folhas[i].Pkeys, folhas[i].Skeys, folhas[i].Masks);
        
        // Gera hash da chave pública da folha
        char buffer[128];
        sprintf(buffer, "folha_%d", i);
        sha256_hex(buffer, strlen(buffer), folhas[i].hash);
    }
}

// Conecta duas folhas a um nó da camada 1
void conectarFolhasAoNo(No *no, Folha *folha_esq, Folha *folha_dir){
    if (no == NULL) return;
    
    // Define os filhos como folhas
    no->filho_esq = (void*)folha_esq;
    no->filho_dir = (void*)folha_dir;
    
    // Marca o tipo dos filhos
    no->tipo_filho_esq = TIPO_FOLHA;
    no->tipo_filho_dir = TIPO_FOLHA;
    
    // Calcula o hash do nó pai baseado nos hashes das folhas
    char concHashs[SHA256_HEX_SIZE * 2 + 1];
    strcpy(concHashs, folha_esq->hash);
    strcat(concHashs, folha_dir->hash);
    sha256_hex(concHashs, strlen(concHashs), no->hash);
}





//Limpeza 
void liberarFolha(Folha *folha){
    if (folha == NULL) return;
    
    // Libera as estruturas WOTS
    if (folha->Skeys) free(folha->Skeys);
    if (folha->Pkeys) free(folha->Pkeys);
    if (folha->Masks) free(folha->Masks);
    
    free(folha);
}

void liberarNo(No *no){
    if (no == NULL) return;
    free(no);
}

void limparArvore(No *raiz){
    if (raiz == NULL) return;
    
    // Limpa filho esquerdo
    if (raiz->filho_esq != NULL) {
        if (raiz->tipo_filho_esq == TIPO_NO) {
            limparArvore((No*)raiz->filho_esq);
        } else {
            liberarFolha((Folha*)raiz->filho_esq);
        }
    }
    
    // Limpa filho direito
    if (raiz->filho_dir != NULL) {
        if (raiz->tipo_filho_dir == TIPO_NO) {
            limparArvore((No*)raiz->filho_dir);
        } else {
            liberarFolha((Folha*)raiz->filho_dir);
        }
    }
    
    // Libera o nó atual
    free(raiz);
}

// Função auxiliar para calcular altura da árvore
int alturaArvore(No *raiz){
    if (raiz == NULL) return 0;
    
    int alturaEsq = 0;
    int alturaDir = 0;
    
    // Calcula altura do filho esquerdo
    if (raiz->filho_esq != NULL) {
        if (raiz->tipo_filho_esq == TIPO_NO) {
            alturaEsq = alturaArvore((No*)raiz->filho_esq);
        } else {
            alturaEsq = 1; // Folhas têm altura 1
        }
    }
    
    // Calcula altura do filho direito
    if (raiz->filho_dir != NULL) {
        if (raiz->tipo_filho_dir == TIPO_NO) {
            alturaDir = alturaArvore((No*)raiz->filho_dir);
        } else {
            alturaDir = 1; // Folhas têm altura 1
        }
    }
    
    return 1 + (alturaEsq > alturaDir ? alturaEsq : alturaDir);
}

// Função auxiliar para contar nós em um nível
int contarNosNivel(No *raiz, int nivel){
    if (raiz == NULL) return 0;
    if (nivel == 0) return 1;
    
    int count = 0;
    
    // Conta no filho esquerdo
    if (raiz->filho_esq != NULL) {
        if (raiz->tipo_filho_esq == TIPO_NO) {
            count += contarNosNivel((No*)raiz->filho_esq, nivel - 1);
        } else if (nivel == 1) {
            count += 1; // Folha no próximo nível
        }
    }
    
    // Conta no filho direito
    if (raiz->filho_dir != NULL) {
        if (raiz->tipo_filho_dir == TIPO_NO) {
            count += contarNosNivel((No*)raiz->filho_dir, nivel - 1);
        } else if (nivel == 1) {
            count += 1; // Folha no próximo nível
        }
    }
    
    return count;
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
    } else if (nivel == 1) {
        // Imprime folhas se existirem
        if (raiz->filho_esq != NULL) {
            for (int i = 0; i < espacamento; i++) printf(" ");
            if (raiz->tipo_filho_esq == TIPO_FOLHA) {
                Folha* folha = (Folha*)raiz->filho_esq;
                printf("[F]%.6s", folha->hash);
            } else {
                No* no = (No*)raiz->filho_esq;
                printf("%.8s", no->hash);
            }
            for (int i = 0; i < espacamento; i++) printf(" ");
        }
        
        if (raiz->filho_dir != NULL) {
            for (int i = 0; i < espacamento; i++) printf(" ");
            if (raiz->tipo_filho_dir == TIPO_FOLHA) {
                Folha* folha = (Folha*)raiz->filho_dir;
                printf("[F]%.6s", folha->hash);
            } else {
                No* no = (No*)raiz->filho_dir;
                printf("%.8s", no->hash);
            }
            for (int i = 0; i < espacamento; i++) printf(" ");
        }
    } else {
        // Continua recursão para nós internos
        if (raiz->filho_esq != NULL && raiz->tipo_filho_esq == TIPO_NO) {
            printarNivel((No*)raiz->filho_esq, nivel - 1, espacamento / 2);
        }
        if (raiz->filho_dir != NULL && raiz->tipo_filho_dir == TIPO_NO) {
            printarNivel((No*)raiz->filho_dir, nivel - 1, espacamento / 2);
        }
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

// Printar um andar específico (array de nós)
void printarAndar(No **andar, int numNos, int numeroAndar){
    if (andar == NULL || numNos == 0) {
        printf("Andar %d: vazio\n", numeroAndar);
        return;
    }
    
    printf("╔══════════════════════════════════════════════════════════════════════╗\n");
    printf("║  ANDAR %d - %d nós                                                   \n", numeroAndar, numNos);
    printf("╠══════════════════════════════════════════════════════════════════════╣\n");
    
    for (int i = 0; i < numNos; i++) {
        if (andar[i] == NULL) {
            printf("║  Nó[%2d]: NULL                                                      ║\n", i);
            continue;
        }
        
        // Mostra hash do nó (primeiros 16 caracteres)
        printf("║  Nó[%2d]: %.16s... ", i, andar[i]->hash);
        
        // Mostra informações dos filhos
        if (andar[i]->filho_esq != NULL || andar[i]->filho_dir != NULL) {
            printf("│ Filhos: ");
            
            // Filho esquerdo
            if (andar[i]->filho_esq != NULL) {
                if (andar[i]->tipo_filho_esq == TIPO_FOLHA) {
                    Folha* folha = (Folha*)andar[i]->filho_esq;
                    printf("E:[F]%.6s ", folha->hash);
                } else {
                    No* no = (No*)andar[i]->filho_esq;
                    printf("E:%.6s ", no->hash);
                }
            } else {
                printf("E:NULL ");
            }
            
            // Filho direito
            if (andar[i]->filho_dir != NULL) {
                if (andar[i]->tipo_filho_dir == TIPO_FOLHA) {
                    Folha* folha = (Folha*)andar[i]->filho_dir;
                    printf("D:[F]%.6s", folha->hash);
                } else {
                    No* no = (No*)andar[i]->filho_dir;
                    printf("D:%.6s", no->hash);
                }
            } else {
                printf("D:NULL");
            }
        }
        
        printf("\n");
    }
    
    printf("╚══════════════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void escreverArvore(char *caminho,No* raiz);
void lerArvore(char* caminho);