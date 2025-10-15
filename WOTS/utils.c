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
    FILE *arquivo = fopen(caminho, "w");
    if (!arquivo) {
        printf("Erro: não foi possível criar o arquivo %s\n", caminho);
        return;
    }
    
    // Escreve cada bloco da assinatura em formato hexadecimal
    for (int i = 0; i < L; i++) {
        for(int j = 0; j < N; j++){
            fprintf(arquivo, "%02x", (unsigned char)assinatura->assinatura[i][j]);
        }
        fprintf(arquivo, "\n");
    }
    
    fclose(arquivo);
    printf("Assinatura salva em: %s\n", caminho);
}
void escreverPkeys(char* caminho, PublicKeys* pKeys){
    FILE *arquivo = fopen(caminho, "w");
    if (!arquivo) {
        printf("Erro: não foi possível criar o arquivo %s\n", caminho);
        return;
    }

    // Escreve cada chave pública em formato hexadecimal
    for (int i = 0; i < L; i++) {
        for(int j = 0; j < N; j++){
            fprintf(arquivo, "%02x", (unsigned char)pKeys->PK[i][j]);
        }
        fprintf(arquivo, "\n");
    }
    
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
    FILE *arquivo = fopen(caminho, "r");
    if (!arquivo) {
        printf("Erro: não foi possível abrir o arquivo %s\n", caminho);
        return;
    }
    
    char linha[N * 2 + 10]; // Buffer para linha hexadecimal (N bytes = N*2 chars hex + extras)
    int i = 0;
    
    while (fgets(linha, sizeof(linha), arquivo) && i < L) {
        // Remove quebra de linha
        linha[strcspn(linha, "\n")] = 0;
        
        // Converte string hexadecimal para bytes
        for (int j = 0; j < N; j++) {
            if (j * 2 + 1 < strlen(linha)) {
                char hex_byte[3] = {linha[j*2], linha[j*2+1], '\0'};
                assinatura->assinatura[i][j] = (char)strtol(hex_byte, NULL, 16);
            } else {
                assinatura->assinatura[i][j] = 0;
            }
        }
        i++;
    }
    
    fclose(arquivo);
    printf("Assinatura carregada: %d blocos de %d bytes cada\n", i, N);
}
void lerPkeys(char* caminho, PublicKeys *pKeys){
    FILE *arquivo = fopen(caminho, "r");
    if (!arquivo) {
        printf("Erro: não foi possível abrir o arquivo %s\n", caminho);
        return;
    }
    
    char linha[N * 2 + 10]; // Buffer para linha hexadecimal
    int i = 0;
    
    while (fgets(linha, sizeof(linha), arquivo) && i < L) {
        // Remove quebra de linha
        linha[strcspn(linha, "\n")] = 0;
        
        // Converte string hexadecimal para bytes
        for (int j = 0; j < N; j++) {
            if (j * 2 + 1 < strlen(linha)) {
                char hex_byte[3] = {linha[j*2], linha[j*2+1], '\0'};
                pKeys->PK[i][j] = (char)strtol(hex_byte, NULL, 16);
            } else {
                pKeys->PK[i][j] = 0;
            }
        }
        i++;
    }
    
    fclose(arquivo);
    printf("Chaves públicas carregadas: %d chaves de %d bytes cada\n", i, N);
}
