#ifndef ARVORE_H
#define ARVORE_H

#define sizeTree 256
#include "../SHA256/sha256.h"
#include "../WOTS/keys.h"
typedef struct {
    int usada; //0 = nao, 1 = sim
    SecretKeys* Skeys;
    PublicKeys* Pkeys;
    Masks* Masks;
    char hash[SHA256_HEX_SIZE];  // Hash da chave pública
}Folha;
typedef enum {
    TIPO_NO,      // Filho é um nó interno
    TIPO_FOLHA    // Filho é uma folha
} TipoFilho;

// Estrutura para nós internos da árvore
typedef struct no{
    char hash[SHA256_HEX_SIZE];
    
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
    char PublicKeysGeral[SHA256_HEX_SIZE];
    int alturaArvore;
    int totalFolhas;
    // Armazenar apenas a hash da folha usada (evita dangling pointer ao liberar a árvore)
    char hashFolha[SHA256_HEX_SIZE];
    int indiceFolha;
    int tamanhoCaminho;
    char caminho[32][SHA256_HEX_SIZE];
    // Direções: 0 = alvo estava à esquerda (concat: alvo||irmão), 1 = alvo estava à direita (concat: irmão||alvo)
    unsigned char caminhoDirecao[32];
    
}AssinaturaMSS;

// Alocação
No * alocarNo();
Folha * alocarFolha();
AssinaturaMSS * alocarAssinatura();

// Criação
void criarFolhas(Folha *folhas, int quantFolhas);
void criarPai(No *pai);
void criarAssinatura(AssinaturaMSS* assinatura, No* raiz, 
                    Folha* folhaUsada,int indice, int numFolhas);

int verificarAssinatura(AssinaturaMSS*assinatura, char* Pkey);

// Funcoes auxiliares
void conectarFolhasAoNo(No *no, Folha *folha_esq, Folha *folha_dir);

void coletarCaminhoAutenticacao(AssinaturaMSS *assinatura, No* raiz);
int coletarCaminhoRecursivo(No* no, const char* folhaAlvoHash, char caminhoAuth[][SHA256_HEX_SIZE], 
                            unsigned char direcoes[], int* tamanhoPath, int indiceFolha, int numFolhas);


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

// Escreve a chave pública geral (hash da raiz) em arquivo texto
void escreverPublicKey(char* caminho, const char* publicKey);
// Lê a chave pública geral de um arquivo texto (retorna 1 em sucesso, 0 em falha)
int lerPublicKey(char* caminho, char* outPublicKey);


#endif