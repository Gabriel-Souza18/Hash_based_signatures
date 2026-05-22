#include "horst_tree.h"
#include "../SHA256/sha256.h"
#include <stdlib.h>
#include <string.h>

// Representação em array completo: raiz em 0, filhos em 2*i+1 / 2*i+2
ArvoreHorst* criarArvoreHorst(const unsigned char SKeys[HORST_T][HORST_N]) {
    int num_leaves = HORST_T;
    int num_nodes = 2 * num_leaves - 1;
    int leaf_offset = num_nodes - num_leaves;

    ArvoreHorst* arvore = malloc(sizeof(ArvoreHorst));
    if (!arvore) return NULL;

    arvore->nodes = malloc((size_t)num_nodes * HORST_N);
    if (!arvore->nodes) {
        free(arvore);
        return NULL;
    }
    arvore->num_nodes = num_nodes;
    arvore->leaf_offset = leaf_offset;

    // Preencher folhas: hash das SKeys
    for (int i = 0; i < num_leaves; i++) {
        unsigned char* dest = arvore->nodes + (size_t)(leaf_offset + i) * HORST_N;
        sha256_bytes((unsigned char*)SKeys[i], HORST_N, dest);
    }

    // Construir nós internos bottom-up
    for (int i = leaf_offset - 1; i >= 0; i--) {
        unsigned char* left = arvore->nodes + (size_t)(2*i + 1) * HORST_N;
        unsigned char* right = arvore->nodes + (size_t)(2*i + 2) * HORST_N;
        unsigned char concat[HORST_N * 2];
        memcpy(concat, left, HORST_N);
        memcpy(concat + HORST_N, right, HORST_N);
        unsigned char* out = arvore->nodes + (size_t)i * HORST_N;
        sha256_bytes(concat, HORST_N * 2, out);
    }

    return arvore;
}

void liberarArvoreHorst(ArvoreHorst* arvore) {
    if (!arvore) return;
    if (arvore->nodes) free(arvore->nodes);
    free(arvore);
}

void obterRaizArvoreHorst(const ArvoreHorst* arvore, unsigned char out_root[HORST_N]) {
    if (!arvore || !arvore->nodes) return;
    memcpy(out_root, arvore->nodes, HORST_N);
}

void obterCaminhoAutenticacaoHorst(const ArvoreHorst* arvore, int leaf_index, AuthPath* path) {
    if (!arvore || !arvore->nodes) return;
    if (leaf_index < 0 || leaf_index >= HORST_T) return;

    int node = arvore->leaf_offset + leaf_index;
    for (int nivel = 0; nivel < HORST_H; nivel++) {
        int irmao;
        if (node % 2 == 0) irmao = node - 1; else irmao = node + 1;
        if (irmao < 0 || irmao >= arvore->num_nodes) {
            memset(path->path[nivel], 0, HORST_N);
        } else {
            unsigned char* src = arvore->nodes + (size_t)irmao * HORST_N;
            memcpy(path->path[nivel], src, HORST_N);
        }
        node = (node - 1) / 2;
    }
}
