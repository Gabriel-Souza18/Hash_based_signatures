#include "IO.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void escreverMensagemStream(FILE *stream, const char *mensagem) {
    if (stream == NULL || mensagem == NULL) {
        return;
    }
    fprintf(stream, "%s\n", mensagem);
}

void escreverMensagem(const char *caminho, const char *mensagem) {
    if (caminho == NULL || mensagem == NULL) {
        return;
    }
    FILE *arquivo = fopen(caminho, "w");
    if (!arquivo) {
        printf("Erro: não foi possível abrir o arquivo %s para escrita.\n", caminho);
        return;
    }
    escreverMensagemStream(arquivo, mensagem);
    fclose(arquivo);
}

char* lerMensagemStream(FILE *stream) {
    if (stream == NULL) {
        return NULL;
    }

    size_t capacidade = 16; // Tamanho inicial pequeno para demonstrar crescimento dinâmico
    size_t tamanho = 0;
    char *buffer = malloc(capacidade * sizeof(char));
    if (buffer == NULL) {
        printf("Erro: falha de alocação inicial de memória.\n");
        return NULL;
    }

    int c;
    // Lê caractere por caractere da stream até quebra de linha ou fim de arquivo
    while ((c = fgetc(stream)) != EOF && c != '\n' && c != '\r') {
        if (tamanho + 1 >= capacidade) {
            capacidade *= 2; // Dobra a capacidade para amortizar o custo de realocações
            char *novo_buffer = realloc(buffer, capacidade * sizeof(char));
            if (novo_buffer == NULL) {
                printf("Erro: falha de realocação de memória durante a leitura.\n");
                free(buffer);
                return NULL;
            }
            buffer = novo_buffer;
        }
        buffer[tamanho++] = (char)c;
    }

    // Tratamento de quebra de linha CRLF (\r\n) se aplicável
    if (c == '\r') {
        int proximo = fgetc(stream);
        if (proximo != '\n' && proximo != EOF) {
            ungetc(proximo, stream);
        }
    }

    // Se EOF foi atingido sem ler nenhum caractere
    if (tamanho == 0 && c == EOF) {
        free(buffer);
        return NULL;
    }

    // Adiciona o caractere terminador nulo
    buffer[tamanho] = '\0';

    // Realoca para o tamanho exato da mensagem + 1 do terminador nulo,
    // garantindo que não gaste mais nem menos memória do que o necessário.
    char *novo_buffer = realloc(buffer, (tamanho + 1) * sizeof(char));
    if (novo_buffer != NULL) {
        buffer = novo_buffer;
    }

    return buffer;
}

char* lerMensagem(const char *caminho) {
    if (caminho == NULL) {
        return NULL;
    }
    FILE *arquivo = fopen(caminho, "r");
    if (!arquivo) {
        printf("Erro: não foi possível abrir o arquivo %s para leitura.\n", caminho);
        return NULL;
    }

    char *mensagem = lerMensagemStream(arquivo);
    fclose(arquivo);
    return mensagem;
}

void escreverAssinaturaStream(FILE *stream, const uint8_t *assinatura, size_t tamanho) {
    if (stream == NULL || assinatura == NULL || tamanho == 0) {
        return;
    }
    fwrite(assinatura, sizeof(uint8_t), tamanho, stream);
}

void escreverAssinatura(const char *caminho, const uint8_t *assinatura, size_t tamanho) {
    if (caminho == NULL || assinatura == NULL || tamanho == 0) {
        return;
    }
    FILE *arquivo = fopen(caminho, "wb"); // Abre em modo binário
    if (!arquivo) {
        printf("Erro: não foi possível abrir o arquivo %s para escrita de assinatura.\n", caminho);
        return;
    }
    escreverAssinaturaStream(arquivo, assinatura, tamanho);
    fclose(arquivo);
}

uint8_t* lerAssinaturaStream(FILE *stream, size_t *tamanho_lido) {
    if (stream == NULL) {
        if (tamanho_lido) *tamanho_lido = 0;
        return NULL;
    }

    size_t capacidade = 64; // Tamanho inicial pequeno para demonstrar o crescimento dinâmico
    size_t tamanho = 0;
    uint8_t *buffer = malloc(capacidade * sizeof(uint8_t));
    if (buffer == NULL) {
        printf("Erro: falha de alocação inicial para leitura de assinatura.\n");
        if (tamanho_lido) *tamanho_lido = 0;
        return NULL;
    }

    int c;
    // Lê byte por byte até encontrar EOF
    while ((c = fgetc(stream)) != EOF) {
        if (tamanho >= capacidade) {
            capacidade *= 2; // Dobra a capacidade
            uint8_t *novo_buffer = realloc(buffer, capacidade * sizeof(uint8_t));
            if (novo_buffer == NULL) {
                printf("Erro: falha de realocação de memória durante leitura da assinatura.\n");
                free(buffer);
                if (tamanho_lido) *tamanho_lido = 0;
                return NULL;
            }
            buffer = novo_buffer;
        }
        buffer[tamanho++] = (uint8_t)c;
    }

    if (tamanho == 0) {
        free(buffer);
        if (tamanho_lido) *tamanho_lido = 0;
        return NULL;
    }

    // Ajusta o buffer para o tamanho exato da assinatura lida,
    // garantindo que não gaste mais nem menos memória do que o necessário.
    uint8_t *novo_buffer = realloc(buffer, tamanho * sizeof(uint8_t));
    if (novo_buffer != NULL) {
        buffer = novo_buffer;
    }

    if (tamanho_lido) {
        *tamanho_lido = tamanho;
    }

    return buffer;
}

uint8_t* lerAssinatura(const char *caminho, size_t *tamanho_lido) {
    if (caminho == NULL) {
        if (tamanho_lido) *tamanho_lido = 0;
        return NULL;
    }
    FILE *arquivo = fopen(caminho, "rb"); // Abre em modo binário
    if (!arquivo) {
        printf("Erro: não foi possível abrir o arquivo %s para leitura da assinatura.\n", caminho);
        if (tamanho_lido) *tamanho_lido = 0;
        return NULL;
    }

    uint8_t *assinatura = lerAssinaturaStream(arquivo, tamanho_lido);
    fclose(arquivo);
    return assinatura;
}
