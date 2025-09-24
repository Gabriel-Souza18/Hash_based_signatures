#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char **strings;
    char **hashes;
    int count;
} DicionarioHash;

DicionarioHash* criarDicionario(int tamanho) {
    DicionarioHash *dict = malloc(sizeof(DicionarioHash));
    if (!dict) return NULL;
    
    dict->strings = malloc(tamanho * sizeof(char*));
    dict->hashes = malloc(tamanho * sizeof(char*));
    dict->count = 0;
    
    if (!dict->strings || !dict->hashes) {
        free(dict->strings);
        free(dict->hashes);
        free(dict);
        return NULL;
    }
    return dict;
}

void liberarDicionario(DicionarioHash *dict) {
    if (!dict) return;
    for (int i = 0; i < dict->count; i++) {
        free(dict->strings[i]);
        free(dict->hashes[i]);
    }
    free(dict->strings);
    free(dict->hashes);
    free(dict);
}

// Função para ler linha de tamanho dinâmico
char* lerLinhaDinamica(FILE *arquivo) {
    size_t tamanho = 256;
    size_t posicao = 0;
    char *buffer = malloc(tamanho);
    if (!buffer) return NULL;
    
    int c;
    while ((c = fgetc(arquivo)) != EOF && c != '\n') {
        if (posicao >= tamanho - 1) {
            tamanho *= 2;
            char *temp = realloc(buffer, tamanho);
            if (!temp) {
                free(buffer);
                return NULL;
            }
            buffer = temp;
        }
        buffer[posicao++] = c;
    }
    
    if (posicao == 0 && c == EOF) {
        free(buffer);
        return NULL;
    }
    
    buffer[posicao] = '\0';
    return buffer;
}

DicionarioHash* carregarVetores(char *caminhoArquivo) {
    FILE *arquivo = fopen(caminhoArquivo, "r");
    if (!arquivo) return NULL;
    
    DicionarioHash *dict = criarDicionario(100); 
    if (!dict) {
        fclose(arquivo);
        return NULL;
    }
    
    char *linha;
    while ((linha = lerLinhaDinamica(arquivo)) && dict->count < 100) {
        char *sep = strchr(linha, '-');
        if (!sep) {
            free(linha);
            continue;
        }
        
        *sep = '\0';
        char *string = linha;
        char *hash = sep + 1;
        
        char *copy = hash; // Tem que tirar os espaços em branco das hashs
        for (char *src = hash; *src; src++) {
            if (*src != ' ') {
                *copy++ = *src;
            }
        }
        *copy = '\0';
        
        // Aloca e copia
        dict->strings[dict->count] = malloc(strlen(string) + 1);
        dict->hashes[dict->count] = malloc(strlen(hash) + 1);
        
        if (dict->strings[dict->count] && dict->hashes[dict->count]) {
            strcpy(dict->strings[dict->count], string);
            strcpy(dict->hashes[dict->count], hash);
            dict->count++;
        }
        
        free(linha);
    }
    
    fclose(arquivo);
    return dict;
}
