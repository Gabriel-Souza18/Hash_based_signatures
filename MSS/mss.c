#include "Arvore.h"
#include "../SHA256/sha256.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define NUM_FOLHAS 8 // sempre tem que ser 2^n

// Protótipos
No* mssTree(Folha* folhas);
void gerarArvore();
void criarAssinaturaMenu();
void verificarAssinaturaMenu();
void imprimirMenu();

int main(){
    int opcao = 0;
    
    do {
        imprimirMenu();
        scanf("%d", &opcao);
        getchar(); // Limpa buffer
        
        switch(opcao){
            case 1:
                gerarArvore();
                break;
            case 2:
                criarAssinaturaMenu();
                break;
            case 3:
                verificarAssinaturaMenu();
                break;
            case 0:
                printf("Saindo...\n");
                break;
            default:
                printf("Opção inválida!\n");
        }
        
        if (opcao != 0) {
            printf("\nPressione ENTER para continuar...");
            getchar();
        }
        
    } while(opcao != 0);
    
    return 0;
}

void imprimirMenu() {
    printf("\n");
    printf("1 - Gerar Nova Árvore\n");
    printf("2 - Criar Assinatura\n");
    printf("3 - Verificar Assinatura \n");
    printf("0 - Sair\n");
    printf("Escolha uma opção: ");
}

void gerarArvore() {
    printf("\n=== GERANDO NOVA ÁRVORE ===\n");
    
    Folha *folhas = malloc(NUM_FOLHAS * sizeof(Folha));
    if (!folhas) {
        fprintf(stderr, "Erro ao alocar folhas\n");
        return;
    }
    
    // Inicializa os ponteiros de cada folha
    for(int i = 0; i < NUM_FOLHAS; i++){
        folhas[i].Skeys = mallocSkeys();
        folhas[i].Pkeys = mallocPkeys();
        folhas[i].Masks = mallocMasks();
    }
    
    printf("Gerando árvore com %d folhas...\n", NUM_FOLHAS);
    No* raiz = mssTree(folhas);
    
    printf("\nÁrvore gerada com sucesso!\n");
    printf("Hash da Raiz: %s\n", raiz->hash);
    
    // Salva árvore e folhas em formato texto
    escreverArvore("arvore.txt", raiz);
    escreverFolhas("folhas.txt", folhas, NUM_FOLHAS);
    // Salva a chave pública geral (hash da raiz)
    escreverPublicKey("public_key.txt", raiz->hash);
    
    // Imprime estrutura
    imprimirArvore(raiz);
    
    // Libera memória
    limparArvore(raiz);
    for(int i = 0; i < NUM_FOLHAS; i++){
        free(folhas[i].Skeys);
        free(folhas[i].Pkeys);
        free(folhas[i].Masks);
    }
    free(folhas);
}

void criarAssinaturaMenu() {
    printf("\n=== CRIAR ASSINATURA ===\n");
    
    // Aloca folhas
    Folha *folhas = malloc(NUM_FOLHAS * sizeof(Folha));
    if (!folhas) {
        fprintf(stderr, "Erro ao alocar folhas\n");
        return;
    }
    
    // Inicializa ponteiros
    for(int i = 0; i < NUM_FOLHAS; i++){
        folhas[i].Skeys = mallocSkeys();
        folhas[i].Pkeys = mallocPkeys();
        folhas[i].Masks = mallocMasks();
    }
    
    // Carrega os dados das folhas
    int numFolhasLidas;
    lerFolhas("folhas.txt", folhas, &numFolhasLidas);
    
    if (numFolhasLidas != NUM_FOLHAS) {
        printf("Erro: esperado %d folhas, lidas %d. Gere uma árvore primeiro (opção 1).\n", 
               NUM_FOLHAS, numFolhasLidas);
        for(int i = 0; i < NUM_FOLHAS; i++){
            free(folhas[i].Skeys);
            free(folhas[i].Pkeys);
            free(folhas[i].Masks);
        }
        free(folhas);
        return;
    }
    
    // Carrega a árvore (que vai apontar para as folhas já carregadas)
    No* raiz = NULL;
    lerArvore("arvore.txt", folhas, NUM_FOLHAS, &raiz);
    if (raiz == NULL) {
        printf("Erro ao carregar árvore. Gere uma árvore primeiro (opção 1).\n");
        for(int i = 0; i < NUM_FOLHAS; i++){
            free(folhas[i].Skeys);
            free(folhas[i].Pkeys);
            free(folhas[i].Masks);
        }
        free(folhas);
        return;
    }
    
    // Escolhe folha
    printf("\nFolhas disponíveis:\n");
    for(int i = 0; i < NUM_FOLHAS; i++){
        printf("  [%d] Hash: %.16s... %s\n", i, folhas[i].hash, 
               folhas[i].usada ? "(USADA)" : "(Disponível)");
    }
    
    int indiceFolha;
    printf("\nEscolha o índice da folha (0-%d): ", NUM_FOLHAS-1);
    scanf("%d", &indiceFolha);
    
    if (indiceFolha < 0 || indiceFolha >= NUM_FOLHAS) {
        printf("Índice inválido!\n");
        limparArvore(raiz);
        for(int i = 0; i < NUM_FOLHAS; i++){
            free(folhas[i].Skeys);
            free(folhas[i].Pkeys);
            free(folhas[i].Masks);
        }
        free(folhas);
        return;
    }
    
    if (folhas[indiceFolha].usada) {
        printf("Aviso: Esta folha já foi usada!\n");
    }
    
    // Cria assinatura
    AssinaturaMSS* assinatura = alocarAssinatura();
    criarAssinatura(assinatura, raiz, &folhas[indiceFolha], indiceFolha, NUM_FOLHAS);
    
    // Mostra assinatura
    printf("\nAssinatura criada com sucesso!\n");
    
    // Salva assinatura em formato texto
    escreverAssinaturaMSS("assinatura.txt", assinatura);
    
    // Atualiza folhas (marca como usada)
    escreverFolhas("folhas.txt", folhas, NUM_FOLHAS);
    
    // Libera memória
    free(assinatura);
    limparArvore(raiz);
    for(int i = 0; i < NUM_FOLHAS; i++){
        free(folhas[i].Skeys);
        free(folhas[i].Pkeys);
        free(folhas[i].Masks);
    }
    free(folhas);
}

void verificarAssinaturaMenu() {
    printf("\n=== VERIFICAR ASSINATURA ===\n");
    
    // Carrega assinatura do arquivo texto
    AssinaturaMSS* assinatura = alocarAssinatura();
    lerAssinaturaMSS("assinatura.txt", assinatura);
    
    printf("\nAssinatura carregada:\n");
    printf("  Índice da Folha: %d\n", assinatura->indiceFolha);
    printf("  Public Key: %.64s...\n", assinatura->PublicKeysGeral);
    printf("  Tamanho do caminho: %d\n", assinatura->tamanhoCaminho);
    
    // Carrega a chave pública geral do arquivo e atualiza a assinatura
    char publicKey[SHA256_HEX_SIZE];
    lerPublicKey("public_key.txt", publicKey);

    // Verifica (compara com a chave pública lida)
    int resultado = verificarAssinatura(assinatura, publicKey);
    
    printf("\n");
    if (resultado == 1) {
        printf("ASSINATURA VÁLIDA!\n");
    } else {
        printf("ASSINATURA INVÁLIDA!\n");
    }
    
    // assinatura não possui ponteiro para folha mais; apenas libera assinatura
    free(assinatura);
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