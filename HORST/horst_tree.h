#ifndef HORST_TREE_H
#define HORST_TREE_H

#include "keys.h"

// Definição da árvore HORST em português: ArvoreHorst
struct ArvoreHorst {
    unsigned char *nodes; // array contíguo: num_nodes * HORST_N
    int num_nodes;
    int leaf_offset; // índice do primeiro nó folha
};

typedef struct ArvoreHorst ArvoreHorst;

// API em Português
ArvoreHorst* criarArvoreHorst(const unsigned char SKeys[HORST_T][HORST_N]);
void liberarArvoreHorst(ArvoreHorst* arvore);
void obterRaizArvoreHorst(const ArvoreHorst* arvore, unsigned char out_root[HORST_N]);
void obterCaminhoAutenticacaoHorst(const ArvoreHorst* arvore, int leaf_index, AuthPath* path);



#endif
