#include "Arvore.h"
#include "../SHA256/sha256.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define NUM_FOLHAS 1024 // sempre tem que ser 2^n  (H = log2(1024) = 10)

// Protótipos
No* mssTree(Folha* folhas);
void gerarArvore();
void criarAssinaturaMenu();
void verificarAssinaturaMenu();
void imprimirMenu();

int main(){
    int opcao = 0;

    while(1) {
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
                return 0;
    
            default:
                printf("Opção inválida!\n");
        }
    }
    
    return 0;
}

void imprimirMenu() {
    printf("\n");
    printf("1- Criar arvore\n");
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
    }
    
    printf("Gerando árvore com %d folhas...\n", NUM_FOLHAS);
    No* raiz = mssTree(folhas);
    
    printf("\nÁrvore gerada com sucesso!\n");
    // Imprime raiz em hex (bytes brutos)
    char raiz_hex[MSS_HASH_SIZE * 2 + 1];
    bytes_to_hex(raiz->hash, raiz_hex);
    printf("Hash da Raiz: %s\n", raiz_hex);

    // Salva árvore e folhas em formato texto
    escreverArvore("arvore.txt", raiz);
    escreverFolhas("folhas.txt", folhas, NUM_FOLHAS);
    // Salva a chave pública geral (hash da raiz)
    escreverPublicKey("public_key.txt", raiz->hash);
    
    // Imprime estrutura
    imprimirArvore(raiz);
    

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
        }
        free(folhas);
        return;
    }
    
    // Carrega a árvore (que vai apontar para as folhas já carregadas)
    No* raiz = NULL;
    lerArvore("arvore.txt", folhas, NUM_FOLHAS, &raiz);
    if (raiz == NULL) {
        printf("Erro ao carregar árvore.\n");
        for(int i = 0; i < NUM_FOLHAS; i++){
            free(folhas[i].Skeys);
            free(folhas[i].Pkeys);
        }
        free(folhas);
        return;
    }
    
    // Escolhe folha
        
    int indiceFolha;

    for(int i = 0; i < NUM_FOLHAS; i++){
        if (folhas[i].usada == 0) {
               printf("\nFolhas que sera usada:%d\n", i);
               indiceFolha = i;
               break;
        }
    }

    char mensagem[1001];
    printf("Digite a mensagem a ser assinada (max 1000 caracteres): ");
    fgets(mensagem, sizeof(mensagem), stdin);
    mensagem[strcspn(mensagem, "\n")] = 0;  // Remove quebra de linha

    // Cria assinatura
    AssinaturaMSS* assinatura = alocarAssinatura();
    criarAssinatura(assinatura, raiz, &folhas[indiceFolha], indiceFolha, NUM_FOLHAS, mensagem);
    
    // Mostra assinatura
    printf("\nAssinatura criada com sucesso!\n");
    
    // Salva assinatura em formato texto
    escreverAssinaturaMSS("assinatura.txt", assinatura);
    
    // Atualiza folhas (marca como usada)
    escreverFolhas("folhas.txt", folhas, NUM_FOLHAS);
    
}

void verificarAssinaturaMenu() {
    printf("\n=== VERIFICAR ASSINATURA ===\n");
    
    // Carrega assinatura do arquivo texto
    AssinaturaMSS* assinatura = alocarAssinatura();
    lerAssinaturaMSS("assinatura.txt", assinatura);
    
    printf("\n  Índice da Folha: %d\n", assinatura->indiceFolha);
    printf("  Mensagem: %s\n", assinatura->mensagem);
    // PublicKeysGeral agora é bytes brutos — converte para hex para imprimir
    char pk_hex[MSS_HASH_SIZE * 2 + 1];
    bytes_to_hex(assinatura->PublicKeysGeral, pk_hex);
    printf("  Public Key: %.16s...\n", pk_hex);
    printf("  Tamanho do caminho: %d\n", assinatura->tamanhoCaminho);
    
    // Carrega a chave pública geral do arquivo (bytes brutos)
    unsigned char publicKey[MSS_HASH_SIZE];
    lerPublicKey("public_key.txt", publicKey);

    // Carrega as folhas para pegar a chave pública WOTS
    Folha *folhas = malloc(NUM_FOLHAS * sizeof(Folha));
    if (!folhas) {
        fprintf(stderr, "Erro ao alocar folhas\n");
        free(assinatura);
        return;
    }
    
    for(int i = 0; i < NUM_FOLHAS; i++){
        folhas[i].Skeys = mallocSkeys();
        folhas[i].Pkeys = mallocPkeys();
    }
    
    int numFolhasLidas;
    lerFolhas("folhas.txt", folhas, &numFolhasLidas);
    
    // Carrega os seeds da folha que assinou
    memcpy(PK_seed, folhas[assinatura->indiceFolha].leaf_PK_seed, 32);
    memcpy(SK_seed, folhas[assinatura->indiceFolha].leaf_SK_seed, 32);
    
    // Verifica (compara com a chave pública lida)
    clock_t inicio_verify = clock();
    int resultado = verificarAssinatura(assinatura, publicKey, folhas[assinatura->indiceFolha].Pkeys);
    clock_t fim_verify = clock();
    double tempo_verify = (double)(fim_verify - inicio_verify) / CLOCKS_PER_SEC;
    
    printf("\n");
    if (resultado == 1) {
        printf("ASSINATURA VÁLIDA!\n");
    } else {
        printf("ASSINATURA INVÁLIDA!\n");
    }
    printf("Tempo Verificação: %.6f segundos\n", tempo_verify);
    

}

No* mssTree(Folha* folhas){
    clock_t inicio_arvore = clock();
    
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

    clock_t fim_arvore = clock();
    double tempo_arvore = (double)(fim_arvore - inicio_arvore) / CLOCKS_PER_SEC;
    printf("Tempo para gerar Árvore: %.6f segundos\n", tempo_arvore);
    fflush(stdout);

    printf("\nTerminou de gerar Arvore\n");
    char ultimo_hex[MSS_HASH_SIZE * 2 + 1];
    bytes_to_hex(andarAtual[0]->hash, ultimo_hex);
    printf("ultimo no: %s\n", ultimo_hex);
    No* raiz = andarAtual[0];
    free(andarAtual);
    return raiz;
}