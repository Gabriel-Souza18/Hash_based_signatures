#include "utils.h"
#include "keys.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void escreverMensagem(char* caminho, char*mensagem){
    FILE *arquivo = fopen(caminho, "w");
    if (!arquivo) {
        printf("Erro: não foi possível criar o arquivo %s\n", caminho);
        return;
    }

    // Escreve a mensagem original no arquivo
    fprintf(arquivo, "%s\n", mensagem);
    
    fclose(arquivo);
    printf("Mensagem salva em: %s\n", caminho);
}
void escreverAssinatura(char* caminho, Assinatura* assinatura){
    FILE *arquivo = fopen(caminho, "wb");
    if (!arquivo) {
        printf("Erro: não foi possível criar o arquivo %s\n", caminho);
        return;
    }
    
    // Escreve toda a estrutura da assinatura em formato binário
    fwrite(assinatura, sizeof(Assinatura), 1, arquivo);
    
    fclose(arquivo);
    printf("Assinatura salva em: %s\n", caminho);
}

void escreverPkeys(char* caminho, PublicKeys* pKeys, unsigned char* pk_seed, unsigned char* sk_seed){
    FILE *arquivo = fopen(caminho, "wb");
    if (!arquivo) {
        printf("Erro: não foi possível criar o arquivo %s\n", caminho);
        return;
    }

    // Escreve os seeds (32 bytes cada)
    fwrite(pk_seed, 1, N, arquivo);
    fwrite(sk_seed, 1, N, arquivo);
    
    // Escreve cada chave pública
    fwrite(pKeys, sizeof(PublicKeys), 1, arquivo);
    
    fclose(arquivo);
    printf("Chaves públicas salvas em: %s\n", caminho);
}


void lerMensagem(char* caminho,char *mensagem ){
    FILE *arquivo = fopen(caminho, "r");
    if (!arquivo) {
        printf("Erro: não foi possível abrir o arquivo %s\n", caminho);
        return;
    }

    fseek(arquivo, 0, SEEK_END);
    long tamanho = ftell(arquivo);
    fseek(arquivo, 0, SEEK_SET);
    

    char *contMensagem = malloc(tamanho + 1);
    if (!contMensagem) {
        printf("Erro ao alocar memória para a mensagem\n");
        fclose(arquivo);
        return;
    }
    
    // Lê o conteúdo completo
    size_t bytesLidos = fread(contMensagem, 1, tamanho, arquivo);
    contMensagem[bytesLidos] = '\0';
    
    fclose(arquivo);
    
    // Remove quebra de linha se existir
    contMensagem[strcspn(contMensagem, "\n")] = 0;
    
    // Copia a mensagem original
    strcpy(mensagem, contMensagem);
    
    free(contMensagem);
    printf("Mensagem carregada: %s\n", mensagem);
}
void lerAssinatura(char* caminho, Assinatura *assinatura){
    FILE *arquivo = fopen(caminho, "rb");
    if (!arquivo) {
        printf("Erro: não foi possível abrir o arquivo %s\n", caminho);
        return;
    }
    
    size_t lidos = fread(assinatura, sizeof(Assinatura), 1, arquivo);
    if (lidos != 1) {
        printf("Erro ao ler assinatura\n");
    }
    
    fclose(arquivo);
    printf("Assinatura carregada: %zu bytes\n", sizeof(Assinatura));
}
void lerPkeys(char* caminho, PublicKeys *pKeys, unsigned char* pk_seed, unsigned char* sk_seed){
    FILE *arquivo = fopen(caminho, "rb");
    if (!arquivo) {
        printf("Erro: não foi possível abrir o arquivo %s\n", caminho);
        return;
    }
    
    // Lê PK_SEED e SK_SEED
    if (fread(pk_seed, 1, N, arquivo) != N) {
        printf("Erro ao ler PK_seed\n");
    }
    if (fread(sk_seed, 1, N, arquivo) != N) {
        printf("Erro ao ler SK_seed\n");
    }
    
    // Lê as chaves públicas
    size_t lidos = fread(pKeys, sizeof(PublicKeys), 1, arquivo);
    if (lidos != 1) {
        printf("Erro ao ler chaves públicas\n");
    }
    
    fclose(arquivo);
    printf("Chaves públicas carregadas: %zu bytes\n", sizeof(PublicKeys));
}
