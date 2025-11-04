#ifndef ARVORE_H
#define ARVORE_H

#define sizeTree 256
#include "../SHA256/sha256.h"
#include "../WOTS/keys.h"

/*
 * ESTRUTURA DA ÁRVORE MERKLE (MSS)
 * 
 * Camadas são numeradas de cima para baixo (0 = raiz):
 * 
 * Camada 0:              [Raiz]
 *                       /      \
 * Camada 1:       [No1]          [No2]
 *                /    \          /    \
 * Camada 2:  [Folha] [Folha] [Folha] [Folha]
 * 
 * - Nós da CAMADA 1 têm filhos do tipo FOLHA (tipo_filho = TIPO_FOLHA)
 * - Nós ACIMA da camada 1 têm filhos do tipo NO (tipo_filho = TIPO_NO)
 * - Folhas contêm chaves WOTS (SecretKeys, PublicKeys, Masks)
 * - Nós internos contêm apenas hashes
 * 
 * Para 256 folhas:
 * - Camada 2 (última): 256 folhas
 * - Camada 1: 128 nós (cada um com 2 folhas)
 * - Camada 0: raiz
 */

// Estrutura para folhas (nós terminais com chaves WOTS)
typedef struct {
    SecretKeys* Skeys;
    PublicKeys* Pkeys;
    Masks* Masks;
    char hash[SHA256_HEX_SIZE];  // Hash da chave pública
}Folha;

// Enum para identificar o tipo de filho
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
    char* caminho;
    No* raiz;
}AssinaturaMSS;

// Alocação
No * alocarNo();
Folha * alocarFolha();
AssinaturaMSS * alocarAssinatura();

// Criação
void criarFolhas(Folha *folhas, int quantFolhas);
void criarPai(No *pai);

// Função auxiliar para conectar folhas aos nós da camada 1
void conectarFolhasAoNo(No *no, Folha *folha_esq, Folha *folha_dir);


// Liberação de memória
void liberarFolha(Folha *folha);
void liberarNo(No *no);
void limparArvore(No *raiz);

// I/O
void escreverArvore(char *caminho,No *raiz);
void lerArvore(char* caminho);

// Visualização
void printarArvore(No *raiz);
void printarArvoreNivel(No *raiz, int nivel);
void printarArvoreCompleta(No *raiz);
void printarAndar(No **andar, int numNos, int numeroAndar);


//chaves
No* gerarNo(char* SeedMaster);
AssinaturaMSS gerarChavesMSS();
No* gerarArvoreMss(AssinaturaMSS);
#endif