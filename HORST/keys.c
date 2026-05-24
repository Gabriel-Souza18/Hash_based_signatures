#include "keys.h"
#include "horst_tree.h"
#include "../SHA256/sha256.h"
#include <stdlib.h>
#include <stdio.h>
#include <sodium.h>
#include <string.h>
#include <time.h>


struct ArvoreHorst* construirArvore(const unsigned char SKeys[HORST_T][HORST_N]) {
    return criarArvoreHorst(SKeys);
}

void obterRaizArvore(const struct ArvoreHorst* raiz, unsigned char *root) {
    if (!raiz) return;
    obterRaizArvoreHorst(raiz, root);
}

void obterCaminhoAutenticacao(const struct ArvoreHorst* raiz, int indice, AuthPath* path) {
    if (!raiz || !path) return;
    for (int i = 0; i < HORST_H; i++) memset(path->path[i], 0, HORST_N);
    obterCaminhoAutenticacaoHorst(raiz, indice, path);
}

int selecionarIndices(unsigned char *hash, int *indices) {
    int bits_por_indice = HORST_BITS_PER_INDEX;
    int hash_bits = 256;
    int bit_pos = 0;
    
    for(int i = 0; i < HORST_K; i++) {
        int index = 0;
        
        for(int j = 0; j < bits_por_indice; j++) {
            if(bit_pos >= hash_bits) break;
            
            int byte_index = bit_pos / 8;
            int bit_index = 7 - (bit_pos % 8);
            
            int bit = (hash[byte_index] >> bit_index) & 1;
            index = (index << 1) | bit;
            
            bit_pos++;
        }
        indices[i] = index;
    }
    
    return HORST_K;
}

void gerarKeys(Keys* keys) {
    clock_t inicioSK = clock();
    if(sodium_init() < 0) {
        fprintf(stderr, "Erro: falha ao inicializar libsodium.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < HORST_T; i++) {
        randombytes_buf(keys->SKeys[i], KEY_SIZE);
    }
    clock_t fimSK  = clock();
    
    clock_t inicioPk = clock();
    // Construir árvore
    struct ArvoreHorst* raiz = construirArvore(keys->SKeys);
    
    // Extrair raiz como chave pública
    obterRaizArvore(raiz, keys->PKey.root);
    clock_t fimPK = clock();
 
    // Liberar árvore
    liberarArvore(raiz);
    
    // Imprimir tempos
    double tempoSK = (double)(fimSK - inicioSK) / CLOCKS_PER_SEC;
    double tempoPK = (double)(fimPK - inicioPk) / CLOCKS_PER_SEC;
    
    printf("Tempo para gerar Chaves Secretas: %.6f segundos\n", tempoSK);
    printf("Tempo para gerar Chave Publica: %.6f segundos\n", tempoPK);
}


void assinarMensagem(const char* msg, int msg_len,
                     Assinatura* assinatura,
                     const unsigned char SKeys[HORST_T][HORST_N]) {
    clock_t inicioAssinatura = clock();
    
    // Reconstruir árvore a partir das chaves secretas na RAM
    struct ArvoreHorst* raiz = construirArvore(SKeys);
    
    unsigned char hash_msg[32];
    sha256_bytes((unsigned char*)msg, msg_len, hash_msg);
    
    // Extrair índices da mensagem
    int indices[HORST_K];
    selecionarIndices(hash_msg, indices);
    
    // Para cada índice, armazenar o segredo e o caminho
    for (int i = 0; i < HORST_K; i++) {
        int idx = indices[i];
        memcpy(assinatura->components[i].sk, SKeys[idx], HORST_N);
        obterCaminhoAutenticacao(raiz, idx, &assinatura->components[i].auth_path);
    }
    
    liberarArvore(raiz);
    
    clock_t fimAssinatura = clock();
    double tempoAssinatura = (double)(fimAssinatura - inicioAssinatura) / CLOCKS_PER_SEC;
    
    printf("Tempo para Assinar: %.6f segundos\n", tempoAssinatura);
}

void reconstruirRaiz(unsigned char folha_hash[HORST_N], 
                     const AuthPath* path,
                     int indice_folha,
                     unsigned char resultado[HORST_N]) {
    unsigned char hash_atual[HORST_N];
    memcpy(hash_atual, folha_hash, HORST_N);
    
    int idx = indice_folha;
    
    // Percorrer o caminho de baixo para cima
    for (int i = 0; i < HORST_H; i++) {
        unsigned char concatenado[HORST_N * 2];
        
        // Se idx é par, este nó está à esquerda
        if (idx % 2 == 0) {
            memcpy(concatenado, hash_atual, HORST_N);
            memcpy(concatenado + HORST_N, path->path[i], HORST_N);
        } else {
            // Se idx é ímpar, este nó está à direita
            memcpy(concatenado, path->path[i], HORST_N);
            memcpy(concatenado + HORST_N, hash_atual, HORST_N);
        }
        
        sha256_bytes(concatenado, HORST_N * 2, hash_atual);
        idx = idx / 2;
    }
    
    memcpy(resultado, hash_atual, HORST_N);
}

int verificarAssinatura(const char* msg, int msg_len,
                        const Assinatura* assinatura,
                        const PublicKey* pk) {
    unsigned char hash_msg[32];
    sha256_bytes((unsigned char*)msg, msg_len, hash_msg);
    
    // Extrair índices
    int indices[HORST_K];
    selecionarIndices(hash_msg, indices);
    
    // Verificar cada componente
    for (int i = 0; i < HORST_K; i++) {
        int idx = indices[i];
        
        // Hash do segredo
        unsigned char folha_hash[HORST_N];
        sha256_bytes(assinatura->components[i].sk, HORST_N, folha_hash);
        
        // Reconstruir raiz usando caminho de autenticação
        unsigned char raiz_reconstruida[HORST_N];
        reconstruirRaiz(folha_hash, &assinatura->components[i].auth_path, idx, raiz_reconstruida);
        
        // Comparar com a raiz da chave pública
        if (memcmp(raiz_reconstruida, pk->root, HORST_N) != 0) {
            return 0;  // Falha na verificação
        }
    }
    
    return 1;  // Sucesso
}

void imprimirChavePublica(const PublicKey* pk) {
    printf("=== Chave Pública ===\n");
    printf("Raiz da Árvore: ");
    for (int i = 0; i < HORST_N; i++) {
        printf("%02x", pk->root[i]);
    }
    printf("\n");
}

void imprimirAssinatura(const Assinatura* assinatura) {
    printf("=== Assinatura ===\n");
    printf("Componentes: %d\n", HORST_K);
    for (int i = 0; i < HORST_K; i++) {
        printf("Componente %d:\n", i);
        printf("  Segredo: ");
        for (int j = 0; j < 16; j++) {
            printf("%02x", assinatura->components[i].sk[j]);
        }
        printf("...\n");
        printf("  Caminho: %d nós\n", HORST_H);
    }
    printf("=======================\n");
}


// Wrapper para liberar árvore (API do módulo HORST)
void liberarArvore(struct ArvoreHorst* no) {
    liberarArvoreHorst(no);
}



