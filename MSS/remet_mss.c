#include "Arvore.h"
#include "../SHA256/sha256.h"
#include "../WOTS/utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define NUM_FOLHAS 16 // sempre tem que ser 2^n  (H = log2(16) = 4)

// Protótipos
No* mssTree(Folha* folhas);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <caminho_mensagem> <caminho_chave_publica> [caminho_assinatura]\n", argv[0]);
        return 1;
    }

    char *caminhoMsg = argv[1];
    char *caminhoPkey = argv[2];
    char *caminhoAssinatura = (argc >= 4) ? argv[3] : "assinatura.txt";

    // Reseta contador global de SHA256
    sha256_reset_counter();

    printf("\n=== GERANDO NOVA ÁRVORE ===\n");

    Folha *folhas = malloc(NUM_FOLHAS * sizeof(Folha));
    if (!folhas) {
        fprintf(stderr, "Erro ao alocar folhas\n");
        return 1;
    }

    // Inicializa os ponteiros de cada folha
    for(int i = 0; i < NUM_FOLHAS; i++){
        folhas[i].Skeys = mallocSkeys();
        folhas[i].Pkeys = mallocPkeys();
        folhas[i].usada = 0;
    }

    printf("Gerando árvore com %d folhas...\n", NUM_FOLHAS);
    No* raiz = mssTree(folhas);

    printf("\nÁrvore gerada com sucesso!\n");
    // Imprime raiz em hex (bytes brutos)
    char raiz_hex[MSS_HASH_SIZE * 2 + 1];
    bytes_to_hex(raiz->hash, raiz_hex);
    printf("Hash da Raiz: %s\n", raiz_hex);

    // Salva a chave pública geral (hash da raiz)
    escreverPublicKey(caminhoPkey, raiz->hash);

    // Salva árvore e folhas em formato texto
    escreverArvore("arvore.txt", raiz);
    escreverFolhas("folhas.txt", folhas, NUM_FOLHAS);

    // Carrega a mensagem do arquivo
    char mensagem[1001];
    lerMensagem(caminhoMsg, mensagem);

    // Escolhe folha (usa a primeira folha index 0)
    int indiceFolha = 0;

    // Cria assinatura
    AssinaturaMSS* assinatura = alocarAssinatura();
    criarAssinatura(assinatura, raiz, &folhas[indiceFolha], indiceFolha, NUM_FOLHAS, mensagem);

    printf("\nAssinatura criada com sucesso!\n");

    // Salva assinatura em formato texto
    escreverAssinaturaMSS(caminhoAssinatura, assinatura);

    // Atualiza folhas (marca como usada)
    escreverFolhas("folhas.txt", folhas, NUM_FOLHAS);

    printf("Total de hashes SHA256 no remetente: %llu\n", sha256_get_counter());

    // Limpeza de memória
    limparArvore(raiz);
    for(int i = 0; i < NUM_FOLHAS; i++){
        free(folhas[i].Skeys);
        free(folhas[i].Pkeys);
    }
    free(folhas);

    if (assinatura) {
        if (assinatura->wotsSignature) {
            free(assinatura->wotsSignature);
        }
        free(assinatura);
    }

    return 0;
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
