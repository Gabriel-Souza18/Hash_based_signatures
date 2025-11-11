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
       if (folhaUsada->usada == 1) {
        printf("ERRO: FOLHA JA USADA");
        return;
    }

    strcpy(assinatura->PublicKeysGeral, raiz->hash);
    assinatura->alturaArvore = (int)log2(numFolhas);
    // Armazena apenas a hash da folha usada para evitar dangling pointers
    strcpy(assinatura->hashFolha, folhaUsada->hash);
    assinatura->totalFolhas = numFolhas;
    assinatura->indiceFolha = indice;
    assinatura->tamanhoCaminho =0;
    coletarCaminhoAutenticacao(assinatura, raiz);

    folhaUsada->usada = 1;
}

int verificarAssinatura(AssinaturaMSS* assinatura, char* Pkey){
    char hashAtual[SHA256_HEX_SIZE];
    // Começa com o hash da folha (salvo em assinatura)
    strcpy(hashAtual, assinatura->hashFolha);

    for (int i = 0; i < assinatura->tamanhoCaminho; i++){
        char concatenado[SHA256_HEX_SIZE * 2 + 1];
        // Direcao: 0 = alvo estava à esquerda -> concat = alvo || irmão
        //          1 = alvo estava à direita -> concat = irmão || alvo
        if (assinatura->caminhoDirecao[i] == 0){
            strcpy(concatenado, hashAtual);
            strcat(concatenado, assinatura->caminho[i]);
        } else {
            strcpy(concatenado, assinatura->caminho[i]);
            strcat(concatenado, hashAtual);
        }
        sha256_hex(concatenado, strlen(concatenado), hashAtual);
    }

    if (Pkey != NULL){
        return (strcmp(hashAtual, Pkey) == 0) ? 1 : 0;
    } else {
        return (strcmp(hashAtual, assinatura->PublicKeysGeral) == 0) ? 1 : 0;
    }
}

// Função para coletar o caminho de autenticação
void coletarCaminhoAutenticacao(AssinaturaMSS *assinatura, No* raiz) {
    if (raiz == NULL) return;
    if (assinatura->indiceFolha == -1) {
        fprintf(stderr, "Erro: Folha não encontrada no array\n");
        return;
    }

    assinatura->tamanhoCaminho = 0;

    // Para cada nível, precisamos encontrar o hash do irmão
    coletarCaminhoRecursivo(raiz, assinatura->hashFolha,
                            assinatura->caminho,
                            assinatura->caminhoDirecao,
                            &assinatura->tamanhoCaminho,
                            assinatura->indiceFolha,
                            assinatura->totalFolhas);
}
// Função auxiliar recursiva para coletar o caminho
int coletarCaminhoRecursivo(No* no, const char* folhaAlvoHash, char caminhoAuth[][SHA256_HEX_SIZE], 
                            unsigned char direcoes[], int* tamanhoPath, int indiceFolha, int numFolhas) {
    if (no == NULL) return 0;
    
    // Caso base: chegamos no nível das folhas
    if (no->tipo_filho_esq == TIPO_FOLHA && no->tipo_filho_dir == TIPO_FOLHA) {
        Folha* folhaEsq = (Folha*)no->filho_esq;
        Folha* folhaDir = (Folha*)no->filho_dir;

        // Verifica qual folha possui o hash alvo e adiciona o hash do irmão
        if (strcmp(folhaEsq->hash, folhaAlvoHash) == 0) {
            strcpy(caminhoAuth[*tamanhoPath], folhaDir->hash);
            // alvo está à esquerda
            direcoes[*tamanhoPath] = 0;
            (*tamanhoPath)++;
            return 1; // Encontrou pela esquerda
        } else if (strcmp(folhaDir->hash, folhaAlvoHash) == 0) {
            strcpy(caminhoAuth[*tamanhoPath], folhaEsq->hash);
            // alvo está à direita
            direcoes[*tamanhoPath] = 1;
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
            int childFound = coletarCaminhoRecursivo((No*)no->filho_esq, folhaAlvoHash,
                                                    caminhoAuth, direcoes,
                                                    tamanhoPath, indiceFolha, numFolhas);
            if (childFound > 0) encontrado = 1; // alvo está na subárvore esquerda
        } else {
            // filho esquerdo é folha: compara hashes
            Folha* f = (Folha*)no->filho_esq;
            if (strcmp(f->hash, folhaAlvoHash) == 0) {
                encontrado = 1;
            }
        }
    }
    
    // Se não encontrou à esquerda, verifica filho direito
    if (encontrado == 0 && no->filho_dir != NULL) {
        if (no->tipo_filho_dir == TIPO_NO) {
            int childFound = coletarCaminhoRecursivo((No*)no->filho_dir, folhaAlvoHash,
                                                    caminhoAuth, direcoes,
                                                    tamanhoPath, indiceFolha, numFolhas);
            if (childFound > 0) encontrado = 2; // alvo está na subárvore direita
        } else {
            // filho direito é folha: compara hashes
            Folha* f = (Folha*)no->filho_dir;
            if (strcmp(f->hash, folhaAlvoHash) == 0) {
                encontrado = 2;
            }
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
            // alvo está à esquerda
            direcoes[*tamanhoPath] = 0;
            (*tamanhoPath)++;
        } else if (encontrado == 2) {
            // Encontrou à direita, adiciona hash da esquerda
            if (no->tipo_filho_esq == TIPO_NO) {
                strcpy(caminhoAuth[*tamanhoPath], ((No*)no->filho_esq)->hash);
            } else {
                strcpy(caminhoAuth[*tamanhoPath], ((Folha*)no->filho_esq)->hash);
            }
            // alvo está à direita
            direcoes[*tamanhoPath] = 1;
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

// ========== FUNÇÕES DE I/O EM TEXTO SIMPLES ==========

// Escrever folhas em formato texto simples
void escreverFolhas(char* caminho, Folha* folhas, int numFolhas) {
    FILE* arquivo = fopen(caminho, "w");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro ao abrir arquivo %s para escrita\n", caminho);
        return;
    }
    
    // Cabeçalho
    fprintf(arquivo, "FOLHAS_MSS\n");
    fprintf(arquivo, "%d\n", numFolhas);
    fprintf(arquivo, "---\n");
    
    // Dados de cada folha
    for (int i = 0; i < numFolhas; i++) {
        fprintf(arquivo, "FOLHA %d\n", i);
        fprintf(arquivo, "Hash: %s\n", folhas[i].hash);
        fprintf(arquivo, "Usada: %d\n", folhas[i].usada);
        fprintf(arquivo, "---\n");
    }
    
    fclose(arquivo);
    printf("Folhas salvas em: %s\n", caminho);
}

// Ler folhas em formato texto simples
void lerFolhas(char* caminho, Folha* folhas, int* numFolhas) {
    FILE* arquivo = fopen(caminho, "r");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro ao abrir arquivo %s para leitura\n", caminho);
        *numFolhas = 0;
        return;
    }
    
    char linha[200];
    
    // Lê cabeçalho
    fgets(linha, sizeof(linha), arquivo); // "FOLHAS_MSS"
    fscanf(arquivo, "%d\n", numFolhas);
    fgets(linha, sizeof(linha), arquivo); // "---"
    
    // Lê dados de cada folha
    for (int i = 0; i < *numFolhas; i++) {
        int indice;
        fscanf(arquivo, "FOLHA %d\n", &indice);
        fscanf(arquivo, "Hash: %s\n", folhas[i].hash);
        fscanf(arquivo, "Usada: %d\n", &folhas[i].usada);
        fgets(linha, sizeof(linha), arquivo); // "---"
    }
    
    fclose(arquivo);
    printf("Folhas carregadas de: %s (%d folhas)\n", caminho, *numFolhas);
}

// Escrever assinatura em formato texto simples
void escreverAssinaturaMSS(char* caminho, AssinaturaMSS* assinatura) {
    FILE* arquivo = fopen(caminho, "w");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro ao abrir arquivo %s para escrita\n", caminho);
        return;
    }
    
    // Cabeçalho
    fprintf(arquivo, "ASSINATURA_MSS\n");
    fprintf(arquivo, "---\n");
    
    // Dados básicos
    fprintf(arquivo, "PublicKey: %s\n", assinatura->PublicKeysGeral);
    fprintf(arquivo, "AlturaArvore: %d\n", assinatura->alturaArvore);
    fprintf(arquivo, "TotalFolhas: %d\n", assinatura->totalFolhas);
    fprintf(arquivo, "IndiceFolha: %d\n", assinatura->indiceFolha);
    fprintf(arquivo, "HashFolha: %s\n", assinatura->hashFolha);
    fprintf(arquivo, "TamanhoCaminho: %d\n", assinatura->tamanhoCaminho);
    fprintf(arquivo, "---\n");
    
    // Caminho de autenticação
    fprintf(arquivo, "CAMINHO\n");
    for (int i = 0; i < assinatura->tamanhoCaminho; i++) {
        fprintf(arquivo, "%d: %s\n", i, assinatura->caminho[i]);
    }
    fprintf(arquivo, "---\n");
    
    // Direções (0=esquerda, 1=direita)
    fprintf(arquivo, "DIRECOES\n");
    for (int i = 0; i < assinatura->tamanhoCaminho; i++) {
        fprintf(arquivo, "%d: %d\n", i, assinatura->caminhoDirecao[i]);
    }
    fprintf(arquivo, "---\n");
    
    fclose(arquivo);
    printf("Assinatura salva em: %s\n", caminho);
}

// Ler assinatura em formato texto simples
void lerAssinaturaMSS(char* caminho, AssinaturaMSS* assinatura) {
    FILE* arquivo = fopen(caminho, "r");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro ao abrir arquivo %s para leitura\n", caminho);
        return;
    }
    
    char linha[200];
    
    // Lê cabeçalho
    fgets(linha, sizeof(linha), arquivo); // "ASSINATURA_MSS"
    fgets(linha, sizeof(linha), arquivo); 
    
    // Lê dados básicos
    fscanf(arquivo, "PublicKey: %s\n", assinatura->PublicKeysGeral);
    fscanf(arquivo, "AlturaArvore: %d\n", &assinatura->alturaArvore);
    fscanf(arquivo, "TotalFolhas: %d\n", &assinatura->totalFolhas);
    fscanf(arquivo, "IndiceFolha: %d\n", &assinatura->indiceFolha);
    fscanf(arquivo, "HashFolha: %s\n", assinatura->hashFolha);
    fscanf(arquivo, "TamanhoCaminho: %d\n", &assinatura->tamanhoCaminho);
    fgets(linha, sizeof(linha), arquivo); 
    
    // Lê caminho de autenticação
    fgets(linha, sizeof(linha), arquivo); // "CAMINHO"
    for (int i = 0; i < assinatura->tamanhoCaminho; i++) {
        int indice;
        fscanf(arquivo, "%d: %s\n", &indice, assinatura->caminho[i]);
    }
    fgets(linha, sizeof(linha), arquivo); 
    
    // Lê direções
    fgets(linha, sizeof(linha), arquivo); // "DIRECOES"
    for (int i = 0; i < assinatura->tamanhoCaminho; i++) {
        int indice, direcao;
        fscanf(arquivo, "%d: %d\n", &indice, &direcao);
        assinatura->caminhoDirecao[i] = (unsigned char)direcao;
    }
    
    fclose(arquivo);
    printf("Assinatura carregada de: %s\n", caminho);
}

// Escreve a chave pública geral (hash da raiz) em arquivo texto simples
void escreverPublicKey(char* caminho, const char* publicKey) {
    FILE* arquivo = fopen(caminho, "w");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro ao abrir arquivo %s para escrita\n", caminho);
        return;
    }
    fprintf(arquivo, "PUBLIC_KEY\n");
    fprintf(arquivo, "%s\n", publicKey);
    fclose(arquivo);
    printf("Chave pública salva em: %s\n", caminho);
}

// Lê a chave pública geral de um arquivo texto simples. Retorna 1 em sucesso, 0 em falha.
int lerPublicKey(char* caminho, char* outPublicKey) {
    FILE* arquivo = fopen(caminho, "r");
    if (arquivo == NULL) {
        return 0;
    }
    char linha[200];
    if (fgets(linha, sizeof(linha), arquivo) == NULL) { fclose(arquivo); return 0; }
    // pode ser que a primeira linha seja "PUBLIC_KEY"; se for, lê a próxima
    if (strncmp(linha, "PUBLIC_KEY", 10) == 0) {
        if (fgets(linha, sizeof(linha), arquivo) == NULL) { fclose(arquivo); return 0; }
    }
    // remove nova linha
    linha[strcspn(linha, "\r\n")] = '\0';
    strncpy(outPublicKey, linha, SHA256_HEX_SIZE);
    outPublicKey[SHA256_HEX_SIZE-1] = '\0';
    fclose(arquivo);
    return 1;
}

// Escrever árvore em formato texto (pré-ordem)
void escreverArvoreRecursivo(FILE* arquivo, No* no, int nivel) {
    if (no == NULL) {
        fprintf(arquivo, "NULL\n");
        return;
    }
    
    // Escreve nó atual
    fprintf(arquivo, "NO %d %s\n", nivel, no->hash);
    
    // Escreve filhos
    if (no->tipo_filho_esq == TIPO_FOLHA) {
        Folha* folha = (Folha*)no->filho_esq;
        fprintf(arquivo, "FOLHA %s %d\n", folha->hash, folha->usada);
    } else {
        escreverArvoreRecursivo(arquivo, (No*)no->filho_esq, nivel + 1);
    }
    
    if (no->tipo_filho_dir == TIPO_FOLHA) {
        Folha* folha = (Folha*)no->filho_dir;
        fprintf(arquivo, "FOLHA %s %d\n", folha->hash, folha->usada);
    } else {
        escreverArvoreRecursivo(arquivo, (No*)no->filho_dir, nivel + 1);
    }
}

void escreverArvore(char* caminho, No* raiz) {
    FILE* arquivo = fopen(caminho, "w");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro ao abrir arquivo %s para escrita\n", caminho);
        return;
    }
    
    fprintf(arquivo, "ARVORE_MSS\n");
    fprintf(arquivo, "---\n");
    
    escreverArvoreRecursivo(arquivo, raiz, 0);
    
    fclose(arquivo);
    printf("Árvore salva em: %s\n", caminho);
}

// Ler árvore em formato texto (pré-ordem)
No* lerArvoreRecursivo(FILE* arquivo, Folha* folhas, int numFolhas) {
    char tipo[20];
    
    if (fscanf(arquivo, "%s", tipo) != 1) {
        return NULL;
    }
    
    if (strcmp(tipo, "NULL") == 0) {
        return NULL;
    }
    
    if (strcmp(tipo, "FOLHA") == 0) {
        // Lê hash e flag usada
        char hash[SHA256_HEX_SIZE];
        int usada;
        fscanf(arquivo, "%s %d\n", hash, &usada);
        
        // Encontra a folha correspondente no array
        for (int i = 0; i < numFolhas; i++) {
            if (strcmp(folhas[i].hash, hash) == 0) {
                return (No*)&folhas[i]; // Retorna ponteiro para folha
            }
        }
        
        fprintf(stderr, "AVISO: Folha com hash %.16s... não encontrada\n", hash);
        return NULL;
    }
    
    if (strcmp(tipo, "NO") == 0) {
        // Lê nível e hash
        int nivel;
        char hash[SHA256_HEX_SIZE];
        fscanf(arquivo, "%d %s\n", &nivel, hash);
        
        // Cria nó
        No* no = alocarNo();
        strcpy(no->hash, hash);
        no->nivel = nivel;
        
        // Lê filhos
        No* filhoEsq = lerArvoreRecursivo(arquivo, folhas, numFolhas);
        if (filhoEsq == NULL) {
            no->filho_esq = NULL;
            no->tipo_filho_esq = TIPO_NO;
        } else if (filhoEsq == (No*)folhas || 
                   (filhoEsq >= (No*)folhas && filhoEsq < (No*)(folhas + numFolhas))) {
            // É uma folha
            no->filho_esq = filhoEsq;
            no->tipo_filho_esq = TIPO_FOLHA;
        } else {
            // É um nó
            no->filho_esq = filhoEsq;
            no->tipo_filho_esq = TIPO_NO;
        }
        
        No* filhoDir = lerArvoreRecursivo(arquivo, folhas, numFolhas);
        if (filhoDir == NULL) {
            no->filho_dir = NULL;
            no->tipo_filho_dir = TIPO_NO;
        } else if (filhoDir == (No*)folhas || 
                   (filhoDir >= (No*)folhas && filhoDir < (No*)(folhas + numFolhas))) {
            // É uma folha
            no->filho_dir = filhoDir;
            no->tipo_filho_dir = TIPO_FOLHA;
        } else {
            // É um nó
            no->filho_dir = filhoDir;
            no->tipo_filho_dir = TIPO_NO;
        }
        
        return no;
    }
    
    return NULL;
}

void lerArvore(char* caminho, Folha* folhas, int numFolhas, No** raiz) {
    FILE* arquivo = fopen(caminho, "r");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro ao abrir arquivo %s para leitura\n", caminho);
        *raiz = NULL;
        return;
    }
    
    char linha[200];
    fgets(linha, sizeof(linha), arquivo); // "ARVORE_MSS"
    fgets(linha, sizeof(linha), arquivo); // "---"
    
    *raiz = lerArvoreRecursivo(arquivo, folhas, numFolhas);
    
    fclose(arquivo);
    printf("Árvore carregada de: %s\n", caminho);
}

