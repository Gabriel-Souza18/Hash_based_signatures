#include "keys.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Função para ler chaves públicas de um arquivo
PublicKeys* lerPkeys(char* caminho){
    FILE *arquivo = fopen(caminho, "r");

    PublicKeys *pKeys = malloc_Pkeys();

    char linha[1024];
    int indice = 0;
    
    while (fgets(linha, sizeof(linha), arquivo) && indice < 256) {

        linha[strcspn(linha, "\n")] = 0;
        
        char *separador = strchr(linha, '|');
        if (!separador) continue;
        
        *separador = '\0';
        char *pk0 = linha;
        char *pk1 = separador + 1;
        
        pKeys->PK0[indice] = malloc(strlen(pk0) + 1);
        if (pKeys->PK0[indice]) {
            strcpy(pKeys->PK0[indice], pk0);
        }
        pKeys->PK1[indice] = malloc(strlen(pk1) + 1);
        if (pKeys->PK1[indice]) {
            strcpy(pKeys->PK1[indice], pk1);
        }
        indice++;
    }
    
    fclose(arquivo);
    printf("Chaves públicas carregadas: %d pares\n", indice);
    return pKeys;
}
void lerMensagem(char* caminho, char *mensagem){
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
void escreverMensagem(char*caminho, char* mensagem){
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

void escreverPkeys(char* caminho, PublicKeys *pKeys){
    FILE *arquivo = fopen(caminho, "w");

    for (int i = 0; i < 256; i++) {
        if (pKeys->PK0[i] && pKeys->PK1[i]) {
            fprintf(arquivo, "%s|%s\n", pKeys->PK0[i], pKeys->PK1[i]);
        }
    }
    
    fclose(arquivo);
    printf("Chaves públicas salvas em: %s\n", caminho);
}

// Função para escrever assinatura em arquivo (formato binário otimizado)
void escreverAssinatura(char* caminho, uint8_t assinatura[256][SECRET_KEY_SIZE], int tamanho){
    FILE *arquivo = fopen(caminho, "wb");  // Modo binário
    if (!arquivo) {
        printf("Erro: não foi possível criar o arquivo %s\n", caminho);
        return;
    }
    
    // Escreve o número de assinaturas
    fwrite(&tamanho, sizeof(int), 1, arquivo);
    
    // Escreve todas as 256 chaves de 32 bytes cada
    for (int i = 0; i < tamanho; i++) {
        fwrite(assinatura[i], sizeof(uint8_t), SECRET_KEY_SIZE, arquivo);
    }
    
    fclose(arquivo);
    printf("Assinatura salva em: %s (formato binário, %d x %d bytes)\n", 
           caminho, tamanho, SECRET_KEY_SIZE);
}

// Função para ler assinatura de arquivo (formato binário)
uint8_t (*lerAssinatura(char* caminho, int *tamanho))[SECRET_KEY_SIZE]{
    FILE *arquivo = fopen(caminho, "rb");  // Modo binário
    if (!arquivo) {
        printf("Erro: não foi possível abrir o arquivo %s\n", caminho);
        return NULL;
    }

    // Lê o número de assinaturas
    int count;
    fread(&count, sizeof(int), 1, arquivo);
    
    // Aloca memória para as assinaturas
    uint8_t (*assinatura)[SECRET_KEY_SIZE] = malloc(count * sizeof(uint8_t[SECRET_KEY_SIZE]));
    if (!assinatura) {
        printf("Erro ao alocar memória para assinatura\n");
        fclose(arquivo);
        return NULL;
    }
    
    // Lê todas as chaves
    for (int i = 0; i < count; i++) {
        fread(assinatura[i], sizeof(uint8_t), SECRET_KEY_SIZE, arquivo);
    }
    
    fclose(arquivo);
    if (tamanho) *tamanho = count;
    printf("Assinatura carregada: %d elementos de %d bytes cada\n", count, SECRET_KEY_SIZE);
    return assinatura;
}