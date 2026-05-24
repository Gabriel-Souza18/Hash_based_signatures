#include "keys.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Função para ler chaves públicas de um arquivo
PublicKeys* lerPkeys(char* caminho){
    FILE *arquivo = fopen(caminho, "rb");
    if (!arquivo) {
        printf("Erro: não foi possível abrir o arquivo %s\n", caminho);
        return NULL;
    }

    PublicKeys *pKeys = malloc_Pkeys();
    if (!pKeys) {
        fclose(arquivo);
        return NULL;
    }

    size_t lidos0 = fread(pKeys->PK0, sizeof(uint8_t), 256 * KEY_SIZE, arquivo);
    size_t lidos1 = fread(pKeys->PK1, sizeof(uint8_t), 256 * KEY_SIZE, arquivo);

    if (lidos0 != 256 * KEY_SIZE || lidos1 != 256 * KEY_SIZE) {
        printf("Erro: arquivo de public keys incompleto\n");
        free(pKeys);
        fclose(arquivo);
        return NULL;
    }
    
    fclose(arquivo);
    printf("Chaves públicas carregadas: 256 pares\n");
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
    if (bytesLidos != (size_t)tamanho) {
        printf("Aviso: leitura parcial da mensagem em %s\n", caminho);
    }
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
    FILE *arquivo = fopen(caminho, "wb");
    if (!arquivo) {
        printf("Erro: não foi possível criar o arquivo %s\n", caminho);
        return;
    }

    fwrite(pKeys->PK0, sizeof(uint8_t), 256 * KEY_SIZE, arquivo);
    fwrite(pKeys->PK1, sizeof(uint8_t), 256 * KEY_SIZE, arquivo);
    
    fclose(arquivo);
    printf("Chaves públicas salvas em: %s\n", caminho);
}

// Funcão para escrever assinatura em arquivo (formato binário)
void escreverAssinatura(char* caminho, uint8_t assinatura[256][KEY_SIZE], int tamanho){
    FILE *arquivo = fopen(caminho, "wb");  // Modo binário
    if (!arquivo) {
        printf("Erro: não foi possível criar o arquivo %s\n", caminho);
        return;
    }
    
    // Escreve todas as 256 chaves de 32 bytes cada (sem cabeçalho)
    for (int i = 0; i < tamanho; i++) {
        fwrite(assinatura[i], sizeof(uint8_t), KEY_SIZE, arquivo);
    }
    
    fclose(arquivo);
    printf("Assinatura salva em: %s (formato binário, %d x %d bytes = %d bytes)\n", 
           caminho, tamanho, KEY_SIZE, tamanho * KEY_SIZE);
}

// Função para ler assinatura de arquivo (formato binário)
uint8_t (*lerAssinatura(char* caminho, int *tamanho))[KEY_SIZE]{
    FILE *arquivo = fopen(caminho, "rb");  // Modo binário
    if (!arquivo) {
        printf("Erro: não foi possível abrir o arquivo %s\n", caminho);
        return NULL;
    }

    // Tamanho fixo: 256 elementos de KEY_SIZE bytes (sem cabeçalho)
    int count = 256;
    
    // Aloca memória para as assinaturas
    uint8_t (*assinatura)[KEY_SIZE] = malloc(count * sizeof(uint8_t[KEY_SIZE]));
    if (!assinatura) {
        printf("Erro ao alocar memória para assinatura\n");
        fclose(arquivo);
        return NULL;
    }
    
    // Lê todas as chaves
    for (int i = 0; i < count; i++) {
        if (fread(assinatura[i], sizeof(uint8_t), KEY_SIZE, arquivo) != KEY_SIZE) {
            printf("Erro: assinatura incompleta no arquivo\n");
            free(assinatura);
            fclose(arquivo);
            return NULL;
        }
    }
    
    fclose(arquivo);
    if (tamanho) *tamanho = count;
    printf("Assinatura carregada: %d elementos de %d bytes cada\n", count, KEY_SIZE);
    return assinatura;
}