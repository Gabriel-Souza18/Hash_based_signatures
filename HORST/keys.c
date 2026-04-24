#include "keys.h"
#include "../SHA256/sha256.h"
#include <stdlib.h>
#include <stdio.h>
#include <sodium.h>
#include <string.h>



MerkleNode* criarNo(int eh_folha, int indice) {
    MerkleNode* no = malloc(sizeof(MerkleNode));
    if (!no) {
        fprintf(stderr, "Erro ao alocar nó\n");
        return NULL;
    }
    no->eh_folha = eh_folha;
    no->indice_folha = indice;
    no->esq = NULL;
    no->dir = NULL;
    memset(no->hash, 0, HORST_N);
    return no;
}

void liberarArvore(MerkleNode* no) {
    if (!no) return;
    if (no->esq) liberarArvore(no->esq);
    if (no->dir) liberarArvore(no->dir);
    free(no);
}


void calcularHashNo(MerkleNode* no) {
    if (!no) return;
    
    // Se é folha, o hash já foi calculado (F(sk_i))
    if (no->eh_folha) return;
    
    // Concatenar hashes dos filhos
    unsigned char concatenado[HORST_N * 2];
    memcpy(concatenado, no->esq->hash, HORST_N);
    memcpy(concatenado + HORST_N, no->dir->hash, HORST_N);
    
    // Hash da concatenação
    sha256_bytes(concatenado, HORST_N * 2, no->hash);
}


MerkleNode* construirArvore(unsigned char SKeys[HORST_T][HORST_N], int inicio, int fim) {
    if (inicio == fim) {
        MerkleNode* folha = criarNo(1, inicio);
        sha256_bytes(SKeys[inicio], HORST_N, folha->hash);
        return folha;
    }
    
    // Caso recursivo: dividir em duas subárvores
    int meio = (inicio + fim) / 2;
    
    MerkleNode* no = criarNo(0, -1);
    no->esq = construirArvore(SKeys, inicio, meio);
    no->dir = construirArvore(SKeys, meio + 1, fim);
    
    calcularHashNo(no);
    
    return no;
}

/**
 * Obter a raiz da árvore
 */
void obterRaizArvore(MerkleNode* raiz, unsigned char *root) {
    if (raiz) {
        memcpy(root, raiz->hash, HORST_N);
    }
}


int _temFolha(MerkleNode* no, int indice) {
    if (!no) return 0;
    if (no->eh_folha) return no->indice_folha == indice;
    
    return _temFolha(no->esq, indice) || _temFolha(no->dir, indice);
}

void _obterCaminhoRecursivo(MerkleNode* no, int indice, AuthPath* path, int profundidade) {
    if (!no || profundidade >= HORST_TAU) return;
    
    // Se é uma folha, retornar sem fazer nada
    if (no->eh_folha) {
        return;
    }
    
    // Verificar se a folha está à esquerda ou direita
    int temFolhaEsq = _temFolha(no->esq, indice);
    int temFolhaDir = _temFolha(no->dir, indice);
    
    // Uma das duas subárvores deve conter a folha
    if (temFolhaEsq) {
        // Folha está à esquerda, armazenar hash do nó direito
        if (no->dir) {
            memcpy(path->path[profundidade], no->dir->hash, HORST_N);
        }
        _obterCaminhoRecursivo(no->esq, indice, path, profundidade + 1);
    } else if (temFolhaDir) {
        // Folha está à direita, armazenar hash do nó esquerdo
        if (no->esq) {
            memcpy(path->path[profundidade], no->esq->hash, HORST_N);
        }
        _obterCaminhoRecursivo(no->dir, indice, path, profundidade + 1);
    }
}

/**
 * Wrapper para obter caminho de autenticação
 */
void obterCaminhoAutenticacao(MerkleNode* raiz, int indice, AuthPath* path) {
    AuthPath temp_path;
    memset(temp_path.path, 0, HORST_TAU * HORST_N);
    memset(path->path, 0, HORST_TAU * HORST_N);
    
    _obterCaminhoRecursivo(raiz, indice, &temp_path, 0);
    
    // Encontrar o número de elementos no caminho
    int num_elementos = 0;
    for (int i = 0; i < HORST_TAU; i++) {
        int is_zero = 1;
        for (int j = 0; j < HORST_N; j++) {
            if (temp_path.path[i][j] != 0) {
                is_zero = 0;
                break;
            }
        }
        if (!is_zero) num_elementos++;
    }
    
    // Reverter a ordem
    for (int i = 0; i < num_elementos; i++) {
        memcpy(path->path[i], temp_path.path[num_elementos - 1 - i], HORST_N);
    }
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
    if(sodium_init() < 0) {
        fprintf(stderr, "Erro: falha ao inicializar libsodium.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < HORST_T; i++) {
        randombytes_buf(keys->SKeys[i], KEY_SIZE);
    }
    
    printf("Construindo árvore de Merkle com altura %d...\n", HORST_TAU);
    
    // Construir árvore
    MerkleNode* raiz = construirArvore(keys->SKeys, 0, HORST_T - 1);
    
    // Extrair raiz como chave pública
    obterRaizArvore(raiz, keys->PKey.root);
    
    printf("Chaves HORST Geradas\n");
    printf("Raiz: ");
    for (int i = 0; i < 16; i++) printf("%02x", keys->PKey.root[i]);
    printf("...\n");
    
    // Liberar árvore
    liberarArvore(raiz);
}


void assinarMensagem(const char* msg, int msg_len,
                     Assinatura* assinatura,
                     const unsigned char SKeys[HORST_T][HORST_N],
                     MerkleNode* raiz) {
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
}

void reconstruirRaiz(unsigned char folha_hash[HORST_N], 
                     const AuthPath* path,
                     int indice_folha,
                     unsigned char resultado[HORST_N]) {
    unsigned char hash_atual[HORST_N];
    memcpy(hash_atual, folha_hash, HORST_N);
    
    int idx = indice_folha;
    
    // Percorrer o caminho de baixo para cima
    for (int i = 0; i < HORST_TAU; i++) {
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
        printf("  Caminho: %d nós\n", HORST_TAU);
    }
    printf("=======================\n");
}



