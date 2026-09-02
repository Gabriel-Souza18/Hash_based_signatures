#ifndef ARVORE_H
#define ARVORE_H

#define sizeTree 256
#define MSS_HASH_SIZE 32  // SHA-256 em bytes brutos

#include "../SHA256/sha256.h"
#include "../WOTS/keys.h"

typedef struct {
    int usada; //0 = nao, 1 = sim
    SecretKeys* Skeys;
    PublicKeys* Pkeys;
    unsigned char hash[MSS_HASH_SIZE];  // Hash da PK em bytes brutos
    unsigned char leaf_PK_seed[32];     // Seeds específicos desta folha
    unsigned char leaf_SK_seed[32];
}Folha;

typedef enum {
    TIPO_NO,      // Filho é um nó interno
    TIPO_FOLHA    // Filho é uma folha
} TipoFilho;

// Estrutura para nós internos da árvore
typedef struct no{
    unsigned char hash[MSS_HASH_SIZE];  // Hash em bytes brutos

    // Ponteiros void para permitir apontar para No ou Folha
    void* filho_esq;
    void* filho_dir;

    // Flags para identificar o tipo dos filhos
    TipoFilho tipo_filho_esq;
    TipoFilho tipo_filho_dir;

    // Nível do nó na árvore (0 = raiz, aumenta para baixo)
    int nivel;
}No;


typedef struct{
    unsigned char PublicKeysGeral[MSS_HASH_SIZE];  // Raiz da árvore em bytes brutos
    int alturaArvore;
    int totalFolhas;
    // Hash da folha usada em bytes brutos
    unsigned char hashFolha[MSS_HASH_SIZE];
    int indiceFolha;
    int tamanhoCaminho;
    unsigned char caminho[32][MSS_HASH_SIZE];      // Caminho de autenticação em bytes brutos
    // Direções: 0 = alvo estava à esquerda, 1 = alvo estava à direita
    unsigned char caminhoDirecao[32];
    // Mensagem e assinatura WOTS
    char mensagem[1001];
    Assinatura* wotsSignature;
    PublicKeys* folhaPkeys;                        // Chave pública WOTS da folha usada
    unsigned char leaf_PK_seed[32];
    unsigned char leaf_SK_seed[32];
}AssinaturaMSS;

// Alocação
No * alocarNo();
Folha * alocarFolha();
AssinaturaMSS * alocarAssinatura();
void liberarAssinatura(AssinaturaMSS* assinatura);

// Criação
void criarFolhas(Folha *folhas, int quantFolhas);
void criarPai(No *pai);
void criarAssinatura(AssinaturaMSS* assinatura, No* raiz,
                    Folha* folhaUsada,int indice, int numFolhas, char* mensagem);

int verificarAssinatura(AssinaturaMSS*assinatura, unsigned char* Pkey, PublicKeys* folhaPkeys);

// Funcoes auxiliares
void conectarFolhasAoNo(No *no, Folha *folha_esq, Folha *folha_dir);

void coletarCaminhoAutenticacao(AssinaturaMSS *assinatura, No* raiz);
int coletarCaminhoRecursivo(No* no, const unsigned char* folhaAlvoHash,
                            unsigned char caminhoAuth[][MSS_HASH_SIZE],
                            unsigned char direcoes[], int* tamanhoPath,
                            int indiceFolha, int numFolhas);


// Liberação de memória
void liberarFolha(Folha *folha);
void liberarNo(No *no);
void limparArvore(No *raiz);

// Impressão
void imprimirArvore(No* raiz);
void imprimirArvoreRecursiva(No* no, int nivel, char* prefixo);

// I/O
void escreverAssinaturaMSS(char* caminho, AssinaturaMSS *assinatura);
void lerAssinaturaMSS(char* caminho, AssinaturaMSS* assinatura);

void escreverFolhas(char* caminho, Folha* folhas, int numFolhas);
void lerFolhas(char* caminho, Folha* folhas, int* numFolhas);

void escreverArvore(char* caminho, No* raiz);
void lerArvore(char* caminho, Folha* folhas, int numFolhas, No** raiz);

// Escreve/lê a chave pública geral (raiz) em arquivo texto (serializada em hex)
void escreverPublicKey(char* caminho, const unsigned char* publicKey);
int lerPublicKey(char* caminho, unsigned char* outPublicKey);

// Utilitário: converte MSS_HASH_SIZE bytes em string hex (buf deve ter >= 65 bytes)
void bytes_to_hex(const unsigned char* bytes, char* hex);
// Converte string hex (64 chars) em MSS_HASH_SIZE bytes
void hex_to_bytes(const char* hex, unsigned char* bytes);

#endif