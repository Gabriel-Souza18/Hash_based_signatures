#include "IO.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Funções de Mensagem (Texto) ---

void escreverMensagemStream(FILE *stream, const char *mensagem) {
    if (stream == NULL || mensagem == NULL) return;
    fprintf(stream, "%s\n", mensagem);
}

void escreverMensagem(const char *caminho, const char *mensagem) {
    if (caminho == NULL || mensagem == NULL) return;
    FILE *arquivo = fopen(caminho, "w");
    if (!arquivo) {
        printf("Erro: não foi possível abrir o arquivo %s para escrita.\n", caminho);
        return;
    }
    escreverMensagemStream(arquivo, mensagem);
    fclose(arquivo);
}

char* lerMensagemStream(FILE *stream) {
    if (stream == NULL) return NULL;

    size_t capacidade = 16;
    size_t tamanho = 0;
    char *buffer = malloc(capacidade * sizeof(char));
    if (buffer == NULL) return NULL;

    int c;
    while ((c = fgetc(stream)) != EOF && c != '\n' && c != '\r') {
        if (tamanho + 1 >= capacidade) {
            capacidade *= 2;
            char *novo_buffer = realloc(buffer, capacidade * sizeof(char));
            if (novo_buffer == NULL) {
                free(buffer);
                return NULL;
            }
            buffer = novo_buffer;
        }
        buffer[tamanho++] = (char)c;
    }

    if (c == '\r') {
        int proximo = fgetc(stream);
        if (proximo != '\n' && proximo != EOF) {
            ungetc(proximo, stream);
        }
    }

    if (tamanho == 0 && c == EOF) {
        free(buffer);
        return NULL;
    }

    buffer[tamanho] = '\0';
    char *novo_buffer = realloc(buffer, (tamanho + 1) * sizeof(char));
    if (novo_buffer != NULL) {
        buffer = novo_buffer;
    }
    return buffer;
}

char* lerMensagem(const char *caminho) {
    if (caminho == NULL) return NULL;
    FILE *arquivo = fopen(caminho, "r");
    if (!arquivo) return NULL;
    char *mensagem = lerMensagemStream(arquivo);
    fclose(arquivo);
    return mensagem;
}

// --- Funções de Assinatura / Dados Brutos (Binário) ---

void escreverAssinaturaStream(FILE *stream, const uint8_t *assinatura, size_t tamanho) {
    if (stream == NULL || assinatura == NULL || tamanho == 0) return;
    fwrite(assinatura, sizeof(uint8_t), tamanho, stream);
}

void escribirAssinatura(const char *caminho, const uint8_t *assinatura, size_t tamanho) {
    if (caminho == NULL || assinatura == NULL || tamanho == 0) return;
    FILE *arquivo = fopen(caminho, "wb");
    if (!arquivo) return;
    escreverAssinaturaStream(arquivo, assinatura, tamanho);
    fclose(arquivo);
}

uint8_t* lerAssinaturaStream(FILE *stream, size_t *tamanho_lido) {
    if (stream == NULL) {
        if (tamanho_lido) *tamanho_lido = 0;
        return NULL;
    }

    size_t capacidade = 64;
    size_t tamanho = 0;
    uint8_t *buffer = malloc(capacidade * sizeof(uint8_t));
    if (buffer == NULL) return NULL;

    int c;
    while ((c = fgetc(stream)) != EOF) {
        if (tamanho >= capacidade) {
            capacidade *= 2;
            uint8_t *novo_buffer = realloc(buffer, capacidade * sizeof(uint8_t));
            if (novo_buffer == NULL) {
                free(buffer);
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

    uint8_t *novo_buffer = realloc(buffer, tamanho * sizeof(uint8_t));
    if (novo_buffer != NULL) {
        buffer = novo_buffer;
    }
    if (tamanho_lido) *tamanho_lido = tamanho;
    return buffer;
}

uint8_t* lerAssinatura(const char *caminho, size_t *tamanho_lido) {
    if (caminho == NULL) return NULL;
    FILE *arquivo = fopen(caminho, "rb");
    if (!arquivo) return NULL;
    uint8_t *assinatura = lerAssinaturaStream(arquivo, tamanho_lido);
    fclose(arquivo);
    return assinatura;
}

// --- Funções de Empacotamento Genérico ---

uint8_t* empacotarDados(const char *mensagem, const uint8_t *pkey, size_t tamanho_pkey, const uint8_t *assinatura, size_t tamanho_assinatura, size_t *tamanho_pacote) {
    if (tamanho_pacote == NULL) return NULL;

    size_t tamanho_msg = mensagem ? strlen(mensagem) : 0;
    size_t t_pk = (pkey && tamanho_pkey > 0) ? tamanho_pkey : 0;
    size_t t_as = (assinatura && tamanho_assinatura > 0) ? tamanho_assinatura : 0;

    *tamanho_pacote = sizeof(uint32_t) + tamanho_msg +
                      sizeof(uint32_t) + t_pk +
                      sizeof(uint32_t) + t_as;

    uint8_t *pacote = malloc(*tamanho_pacote);
    if (pacote == NULL) return NULL;

    uint8_t *ptr = pacote;

    // Grava Mensagem
    uint32_t t_msg_32 = (uint32_t)tamanho_msg;
    memcpy(ptr, &t_msg_32, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    if (tamanho_msg > 0) {
        memcpy(ptr, mensagem, tamanho_msg);
        ptr += tamanho_msg;
    }

    // Grava Chave Pública
    uint32_t t_pk_32 = (uint32_t)t_pk;
    memcpy(ptr, &t_pk_32, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    if (t_pk > 0) {
        memcpy(ptr, pkey, t_pk);
        ptr += t_pk;
    }

    // Grava Assinatura
    uint32_t t_as_32 = (uint32_t)t_as;
    memcpy(ptr, &t_as_32, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    if (t_as > 0) {
        memcpy(ptr, assinatura, t_as);
    }

    return pacote;
}

bool desempacotarDados(const uint8_t *pacote, size_t tamanho_pacote, char **mensagem, uint8_t **pkey, size_t *tamanho_pkey, uint8_t **assinatura, size_t *tamanho_assinatura) {
    if (pacote == NULL) return false;

    // Cabeçalho mínimo para guardar os 3 tamanhos
    if (tamanho_pacote < sizeof(uint32_t) * 3) return false;

    const uint8_t *ptr = pacote;

    // 1. Desempacota Mensagem
    uint32_t t_msg;
    memcpy(&t_msg, ptr, sizeof(uint32_t));
    ptr += sizeof(uint32_t);

    if (sizeof(uint32_t) * 3 + t_msg > tamanho_pacote) return false;

    if (mensagem) {
        if (t_msg > 0) {
            *mensagem = malloc(t_msg + 1);
            if (*mensagem == NULL) return false;
            memcpy(*mensagem, ptr, t_msg);
            (*mensagem)[t_msg] = '\0';
        } else {
            *mensagem = NULL;
        }
    }
    ptr += t_msg;

    // 2. Desempacota Chave Pública
    uint32_t t_pk;
    memcpy(&t_pk, ptr, sizeof(uint32_t));
    ptr += sizeof(uint32_t);

    if (sizeof(uint32_t) * 3 + t_msg + t_pk > tamanho_pacote) {
        if (mensagem && *mensagem) { free(*mensagem); *mensagem = NULL; }
        return false;
    }

    if (pkey && tamanho_pkey) {
        if (t_pk > 0) {
            *pkey = malloc(t_pk);
            if (*pkey == NULL) {
                if (mensagem && *mensagem) { free(*mensagem); *mensagem = NULL; }
                return false;
            }
            memcpy(*pkey, ptr, t_pk);
            *tamanho_pkey = t_pk;
        } else {
            *pkey = NULL;
            *tamanho_pkey = 0;
        }
    }
    ptr += t_pk;

    // 3. Desempacota Assinatura
    uint32_t t_as;
    memcpy(&t_as, ptr, sizeof(uint32_t));
    ptr += sizeof(uint32_t);

    if (sizeof(uint32_t) * 3 + t_msg + t_pk + t_as != tamanho_pacote) {
        if (mensagem && *mensagem) { free(*mensagem); *mensagem = NULL; }
        if (pkey && *pkey) { free(*pkey); *pkey = NULL; }
        return false;
    }

    if (assinatura && tamanho_assinatura) {
        if (t_as > 0) {
            *assinatura = malloc(t_as);
            if (*assinatura == NULL) {
                if (mensagem && *mensagem) { free(*mensagem); *mensagem = NULL; }
                if (pkey && *pkey) { free(*pkey); *pkey = NULL; }
                return false;
            }
            memcpy(*assinatura, ptr, t_as);
            *tamanho_assinatura = t_as;
        } else {
            *assinatura = NULL;
            *tamanho_assinatura = 0;
        }
    }

    return true;
}
