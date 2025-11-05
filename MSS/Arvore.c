#include "Arvore.h"
#include "../SHA256/sha256.h"
#include "../WOTS/keys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
    folha->usada = 0;// define como nao usada

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

void criarAssinatura(AssinaturaMSS* assinatura, No* raiz, 
                    Folha* folhaUsada,int indice, int numFolhas){
    strcpy(assinatura->PublicKeysGeral, raiz->hash);
    assinatura->alturaArvore = (int)log2(numFolhas);
    assinatura->folhaUsada = folhaUsada;
    assinatura->totalFolhas = numFolhas;
    assinatura->indiceFolha = indice;
    assinatura->tamanhoCaminho =0;
    coletarCaminhoAutenticacao(assinatura, raiz);

    folhaUsada->usada = 1;
}

// Função para coletar o caminho de autenticação
void coletarCaminhoAutenticacao(AssinaturaMSS *assinatura, No* raiz) {
    if (raiz == NULL || assinatura->folhaUsada == NULL) return;
    if (assinatura->indiceFolha == -1) {
        fprintf(stderr, "Erro: Folha não encontrada no array\n");
        return;
    }
    
    assinatura->tamanhoCaminho = 0; 
    
    // Para cada nível, precisamos encontrar o hash do irmão
    coletarCaminhoRecursivo(raiz, assinatura->folhaUsada, 
                            assinatura->caminho,
                            &assinatura->tamanhoCaminho, 
                            assinatura->indiceFolha, 
                            assinatura->totalFolhas);
}
// Função auxiliar recursiva para coletar o caminho
int coletarCaminhoRecursivo(No* no, Folha* folhaAlvo, char caminhoAuth[][SHA256_HEX_SIZE], 
                            int* tamanhoPath, int indiceFolha, int numFolhas) {
    if (no == NULL) return 0;
    
    // Caso base: chegamos no nível das folhas
    if (no->tipo_filho_esq == TIPO_FOLHA && no->tipo_filho_dir == TIPO_FOLHA) {
        Folha* folhaEsq = (Folha*)no->filho_esq;
        Folha* folhaDir = (Folha*)no->filho_dir;
        
        // Verifica qual folha é a alvo e adiciona o hash do irmão
        if (folhaEsq == folhaAlvo) {
            strcpy(caminhoAuth[*tamanhoPath], folhaDir->hash);
            (*tamanhoPath)++;
            return 1; // Encontrou pela esquerda
        } else if (folhaDir == folhaAlvo) {
            strcpy(caminhoAuth[*tamanhoPath], folhaEsq->hash);
            (*tamanhoPath)++;
            return 2; // Encontrou pela direita
        }
        return 0;
    }
    
    // Procura recursivamente nos filhos
    int encontrado = 0;
    
    // Verifica filho esquerdo
    if (no->filho_esq != NULL) {
        if (no->tipo_filho_esq == TIPO_NO) {
            encontrado = coletarCaminhoRecursivo((No*)no->filho_esq, folhaAlvo, 
                                                    caminhoAuth, tamanhoPath, indiceFolha, numFolhas);
        }
    }
    
    // Se não encontrou à esquerda, verifica filho direito
    if (encontrado == 0 && no->filho_dir != NULL) {
        if (no->tipo_filho_dir == TIPO_NO) {
            encontrado = coletarCaminhoRecursivo((No*)no->filho_dir, folhaAlvo, 
                                                   caminhoAuth, tamanhoPath, indiceFolha, numFolhas);
        }
    }
    
    // Se encontrou em algum filho, adiciona o hash do irmão DESTE NÍVEL
    if (encontrado > 0) {
        if (encontrado == 1) {
            // Encontrou à esquerda, adiciona hash da direita
            if (no->tipo_filho_dir == TIPO_NO) {
                strcpy(caminhoAuth[*tamanhoPath], ((No*)no->filho_dir)->hash);
            } else {
                strcpy(caminhoAuth[*tamanhoPath], ((Folha*)no->filho_dir)->hash);
            }
            (*tamanhoPath)++;
        } else if (encontrado == 2) {
            // Encontrou à direita, adiciona hash da esquerda
            if (no->tipo_filho_esq == TIPO_NO) {
                strcpy(caminhoAuth[*tamanhoPath], ((No*)no->filho_esq)->hash);
            } else {
                strcpy(caminhoAuth[*tamanhoPath], ((Folha*)no->filho_esq)->hash);
            }
            (*tamanhoPath)++;
        }
        return encontrado;
    }
    
    return 0;
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
        }
        // NÃO libera folhas aqui - elas são liberadas no main
    }
    
    // Limpa filho direito
    if (raiz->filho_dir != NULL) {
        if (raiz->tipo_filho_dir == TIPO_NO) {
            limparArvore((No*)raiz->filho_dir);
        }
        // NÃO libera folhas aqui - elas são liberadas no main
    }
    
    // Libera o nó atual
    free(raiz);
}

// Imprime a árvore
void imprimirArvoreRecursiva(No* no, int nivel, char* prefixo) {
    if (no == NULL) return;
    
    printf("%s[Nó nivel %d] Hash: %.16s...\n", prefixo, nivel, no->hash);
    
    // Prepara prefixo para os filhos
    char novoPrefixo[256];
    sprintf(novoPrefixo, "%s  ", prefixo);
    
    // Imprime filho esquerdo
    if (no->filho_esq != NULL) {
        if (no->tipo_filho_esq == TIPO_NO) {
            printf("%s  ├─ Esquerda:\n", prefixo);
            imprimirArvoreRecursiva((No*)no->filho_esq, nivel + 1, novoPrefixo);
        } else {
            Folha* folha = (Folha*)no->filho_esq;
            printf("%s  ├─ [Folha ESQ] Hash: %.16s... Usada: %d\n", 
                   prefixo, folha->hash, folha->usada);
        }
    }
    
    // Imprime filho direito
    if (no->filho_dir != NULL) {
        if (no->tipo_filho_dir == TIPO_NO) {
            printf("%s  └─ Direita:\n", prefixo);
            imprimirArvoreRecursiva((No*)no->filho_dir, nivel + 1, novoPrefixo);
        } else {
            Folha* folha = (Folha*)no->filho_dir;
            printf("%s  └─ [Folha DIR] Hash: %.16s... Usada: %d\n", 
                   prefixo, folha->hash, folha->usada);
        }
    }
}

void imprimirArvore(No* raiz) {
    printf("\n========== ÁRVORE MERKLE ==========\n");
    if (raiz == NULL) {
        printf("Árvore vazia!\n");
        return;
    }
    imprimirArvoreRecursiva(raiz, 0, "");
    printf("===================================\n\n");
}
