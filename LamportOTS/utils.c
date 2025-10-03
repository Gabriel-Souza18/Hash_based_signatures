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
bool* lerMensagem(char* caminho, bool mensagem[256]){
    FILE *arquivo = fopen(caminho, "r");

    
    // Determina o tamanho do arquivo
    fseek(arquivo, 0, SEEK_END);
    long tamanho = ftell(arquivo);
    fseek(arquivo, 0, SEEK_SET);
    
    // Aloca memória para a mensagem
    char *contMensagem = malloc(tamanho + 1);
    if (!contMensagem) {
        printf("Erro ao alocar memória para a contMensagem\n");
        fclose(arquivo);
        return NULL;
    }
    
    // Lê o conteúdo completo
    size_t bytesLidos = fread(contMensagem, 1, tamanho, arquivo);
    contMensagem[bytesLidos] = '\0';
    
    fclose(arquivo);


    for (int i =0; i< 256;i++){
        if ( contMensagem[i]=='0'){
            mensagem[i]= true;
            continue;
        }   
        mensagem[i]= false;
    }
}
void escreverMensagem(char*caminho, bool* mensagem){
    FILE *arquivo = fopen(caminho, "w");


    for(int i =0; i< 256; i++){
        if (mensagem[i] ==false){
            fprintf(arquivo,"0" );
            continue;
        }   
        fprintf(arquivo,"1");
    }
    fclose(arquivo);
}

// Função para escrever chaves públicas em arquivo
void escreverChavesPublicas(char* caminho, PublicKeys *pKeys){
    FILE *arquivo = fopen(caminho, "w");

    
    fprintf(arquivo, "# Chaves Públicas Lamport OTS\n");
    fprintf(arquivo, "# Formato: PK0|PK1 (uma por linha)\n\n");
    
    for (int i = 0; i < 256; i++) {
        if (pKeys->PK0[i] && pKeys->PK1[i]) {
            fprintf(arquivo, "%s|%s\n", pKeys->PK0[i], pKeys->PK1[i]);
        }
    }
    
    fclose(arquivo);
    printf("Chaves públicas salvas em: %s\n", caminho);
}

// Função para escrever assinatura em arquivo
void escreverAssinatura(char* caminho, char **assinatura, int tamanho){
    FILE *arquivo = fopen(caminho, "w");
    if (!arquivo) {
        printf("Erro: não foi possível criar o arquivo %s\n", caminho);
        return;
    }
    
    fprintf(arquivo, "# Assinatura Lamport OTS\n");

    
    for (int i = 0; i < tamanho; i++) {
        if (assinatura[i]) {
            fprintf(arquivo, "%d:%s\n", i, assinatura[i]);
        }
    }
    
    fclose(arquivo);
    printf("Assinatura salva em: %s\n", caminho);
}

// Função para ler assinatura de arquivo
char** lerAssinatura(char* caminho, int *tamanho){
    FILE *arquivo = fopen(caminho, "r");

    // Aloca array de ponteiros para assinatura
    char **assinatura = malloc(256 * sizeof(char*));
    if (!assinatura) {
        fclose(arquivo);
        return NULL;
    }
    
    // Inicializa todos os ponteiros como NULL
    for (int i = 0; i < 256; i++) {
        assinatura[i] = NULL;
    }
    
    char linha[1024];
    int count = 0;
    
    while (fgets(linha, sizeof(linha), arquivo)) {
        // Ignora comentários
        if (linha[0] == '#') continue;
        
        // Procura o separador ":"
        char *separador = strchr(linha, ':');
        if (!separador) continue;
        
        // Extrai índice e valor
        int indice = atoi(linha);
        char *valor = separador + 1;
        
        // Remove quebra de linha do valor
        valor[strcspn(valor, "\n")] = 0;
        
        // Aloca e copia o valor
        if (indice >= 0 && indice < 256) {
            assinatura[indice] = malloc(strlen(valor) + 1);
            if (assinatura[indice]) {
                strcpy(assinatura[indice], valor);
                count++;
            }
        }
    }
    
    fclose(arquivo);
    if (tamanho) *tamanho = count;
    printf("Assinatura carregada: %d elementos\n", count);
    return assinatura;
}