#ifndef HORST_KEYS_H
#define HORST_KEYS_H

#include <stddef.h>
#include <math.h>

// Parâmetros HORST
#ifndef HORST_T
#define HORST_T 1024
#endif

#define HORST_N 32          // Tamanho do hash (SHA256)
#define KEY_SIZE 32

// Cálculo de k e altura da árvore baseado em t
#if HORST_T == 256
    #define HORST_BITS_PER_INDEX 8
    #define HORST_K 32
    #define HORST_H 8      // altura da árvore: log2(256) = 8
#elif HORST_T == 512
    #define HORST_BITS_PER_INDEX 9
    #define HORST_K 29
    #define HORST_H 9      // altura da árvore: log2(512) = 9
#elif HORST_T == 1024
    #define HORST_BITS_PER_INDEX 10
    #define HORST_K 26
    #define HORST_H 10     // altura da árvore: log2(1024) = 10
#else
    #error "HORST_T deve ser 256, 512, ou 1024"
#endif

typedef struct {
    unsigned char root[HORST_N];  // Raiz da árvore
} PublicKey;

// caminho de autenticação
typedef struct {
    unsigned char path[HORST_H][HORST_N];  // Nós irmãos até a raiz
} AuthPath;

//componente da assinatura (secreto + caminho)
typedef struct {
    unsigned char sk[HORST_N];     
    AuthPath auth_path;               // Caminho de autenticação até raiz
} SignatureComponent;

//assinatura completa
typedef struct {
    SignatureComponent components[HORST_K];  // k componentes
} Assinatura;

// Estrutura para as chaves (secretas e públicas)
typedef struct {
    unsigned char SKeys[HORST_T][HORST_N];  
    PublicKey PKey;                     
} Keys;

// Estrutura de árvore HORST definida em `horst_tree.h` (implementação separada)
struct HorstTree;

void gerarKeys(Keys* keys);
struct ArvoreHorst* construirArvore(const unsigned char SKeys[HORST_T][HORST_N]);
void obterRaizArvore(const struct ArvoreHorst* raiz, unsigned char *root);
void obterCaminhoAutenticacao(const struct ArvoreHorst* raiz, int indice, AuthPath* path);

int selecionarIndices(unsigned char *hash, int *indices);

void assinarMensagem(const char* msg, int msg_len,
                     Assinatura* assinatura,
                     const unsigned char SKeys[HORST_T][HORST_N],
                     const struct ArvoreHorst* raiz);

int verificarAssinatura(const char* msg, int msg_len,
                        const Assinatura* assinatura,
                        const PublicKey* pk);

void liberarArvore(struct ArvoreHorst* no);

void imprimirChavePublica(const PublicKey* pk);
void imprimirAssinatura(const Assinatura* assinatura);

#endif
