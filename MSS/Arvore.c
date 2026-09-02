#include "Arvore.h"
#include "../SHA256/sha256.h"
#include "../WOTS/keys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* ─── Utilitários hex ─────────────────────────────────────────────────────── */

void bytes_to_hex(const unsigned char* bytes, char* hex) {
    for (int i = 0; i < MSS_HASH_SIZE; i++) {
        sprintf(hex + i * 2, "%02x", bytes[i]);
    }
    hex[MSS_HASH_SIZE * 2] = '\0';
}

void hex_to_bytes(const char* hex, unsigned char* bytes) {
    for (int i = 0; i < MSS_HASH_SIZE; i++) {
        char buf[3] = { hex[i * 2], hex[i * 2 + 1], '\0' };
        bytes[i] = (unsigned char)strtol(buf, NULL, 16);
    }
}

/* ─── Alocação ────────────────────────────────────────────────────────────── */

No *alocarNo(){
    No *no = (No*)malloc(sizeof(No));
    if (no == NULL) {
        fprintf(stderr, "Erro ao alocar memória para o nó\n");
        exit(1);
    }
    no->filho_esq = NULL;
    no->filho_dir = NULL;
    no->tipo_filho_esq = TIPO_NO;
    no->tipo_filho_dir = TIPO_NO;
    no->nivel = 0;
    memset(no->hash, 0, MSS_HASH_SIZE);
    return no;
}

Folha * alocarFolha(){
    Folha *folha = (Folha*)malloc(sizeof(Folha));
    if (folha == NULL){
        fprintf(stderr, "ERRO: nao foi possivel alocar Folha\n");
        exit(1);
    }
    folha->usada = 0;
    folha->Skeys = mallocSkeys();
    folha->Pkeys = mallocPkeys();
    memset(folha->hash, 0, MSS_HASH_SIZE);
    return folha;
}

AssinaturaMSS * alocarAssinatura(){
    AssinaturaMSS *assinatura = (AssinaturaMSS*)malloc(sizeof(AssinaturaMSS));
    if (assinatura == NULL) {
        fprintf(stderr, "Erro ao alocar memória para a assinatura\n");
        exit(1);
    }
    assinatura->wotsSignature = NULL;
    assinatura->folhaPkeys = mallocPkeys();
    memset(assinatura->leaf_PK_seed, 0, 32);
    memset(assinatura->leaf_SK_seed, 0, 32);
    memset(assinatura->mensagem, 0, 1001);
    return assinatura;
}

void liberarAssinatura(AssinaturaMSS* assinatura) {
    if (assinatura) {
        if (assinatura->wotsSignature) free(assinatura->wotsSignature);
        if (assinatura->folhaPkeys) free(assinatura->folhaPkeys);
        free(assinatura);
    }
}

/* ─── Geração da árvore ───────────────────────────────────────────────────── */

/*
 * criarPai: calcula hash do nó pai como H(hash_esq || hash_dir) sobre bytes brutos.
 * Conforme o artigo: a_{i,j} = H(a_{i-1,2j} || a_{i-1,2j+1})
 */
void criarPai(No* pai){
    unsigned char hash_esq[MSS_HASH_SIZE];
    unsigned char hash_dir[MSS_HASH_SIZE];

    if (pai->tipo_filho_esq == TIPO_NO) {
        memcpy(hash_esq, ((No*)pai->filho_esq)->hash, MSS_HASH_SIZE);
    } else {
        memcpy(hash_esq, ((Folha*)pai->filho_esq)->hash, MSS_HASH_SIZE);
    }

    if (pai->tipo_filho_dir == TIPO_NO) {
        memcpy(hash_dir, ((No*)pai->filho_dir)->hash, MSS_HASH_SIZE);
    } else {
        memcpy(hash_dir, ((Folha*)pai->filho_dir)->hash, MSS_HASH_SIZE);
    }

    // Concatena e aplica SHA-256 sobre bytes brutos
    unsigned char concatenado[MSS_HASH_SIZE * 2];
    memcpy(concatenado,                  hash_esq, MSS_HASH_SIZE);
    memcpy(concatenado + MSS_HASH_SIZE,  hash_dir, MSS_HASH_SIZE);
    sha256_bytes(concatenado, MSS_HASH_SIZE * 2, pai->hash);
}

/*
 * criarFolhas: para cada folha, gera chaves WOTS e computa
 *   hash_folha = H(PK[0] || PK[1] || ... || PK[L-1]) sobre bytes brutos.
 * Conforme o artigo: h_i = H(OTSPKi)
 */
void criarFolhas(Folha *folhas, int quantFolhas){
    clock_t inicio = clock();

    unsigned char base_PK_seed[N];
    unsigned char base_SK_seed[N];

    initializeSeeds();
    memcpy(base_PK_seed, PK_seed, N);
    memcpy(base_SK_seed, SK_seed, N);

    for(int i = 0; i < quantFolhas; i++){
        memcpy(PK_seed, base_PK_seed, N);
        memcpy(SK_seed, base_SK_seed, N);

        // Seed único por folha via XOR com índice
        SK_seed[0] ^= i;
        SK_seed[1] ^= (i >> 8);
        PK_seed[31] ^= i;

        memcpy(folhas[i].leaf_PK_seed, PK_seed, N);
        memcpy(folhas[i].leaf_SK_seed, SK_seed, N);

        generateSKeys(folhas[i].Skeys);
        generatePKeys(folhas[i].Pkeys, folhas[i].Skeys);

        // Concatena todos os L vetores PK em bytes brutos e aplica SHA-256
        unsigned char pk_concat[L * N];
        for(int j = 0; j < L; j++) {
            memcpy(pk_concat + j * N, folhas[i].Pkeys->PK[j], N);
        }
        sha256_bytes(pk_concat, L * N, folhas[i].hash);
    }

    clock_t fim = clock();
    printf("Tempo para gerar Folhas: %.6f segundos\n",
           (double)(fim - inicio) / CLOCKS_PER_SEC);
    fflush(stdout);
}

/* conectarFolhasAoNo: conecta duas folhas a um nó da camada 1 e computa seu hash */
void conectarFolhasAoNo(No *no, Folha *folha_esq, Folha *folha_dir){
    if (no == NULL) return;

    no->filho_esq = (void*)folha_esq;
    no->filho_dir = (void*)folha_dir;
    no->tipo_filho_esq = TIPO_FOLHA;
    no->tipo_filho_dir = TIPO_FOLHA;

    unsigned char concatenado[MSS_HASH_SIZE * 2];
    memcpy(concatenado,                  folha_esq->hash, MSS_HASH_SIZE);
    memcpy(concatenado + MSS_HASH_SIZE,  folha_dir->hash, MSS_HASH_SIZE);
    sha256_bytes(concatenado, MSS_HASH_SIZE * 2, no->hash);
}

/* ─── Assinatura e verificação ────────────────────────────────────────────── */

void criarAssinatura(AssinaturaMSS* assinatura, No* raiz,
                    Folha* folhaUsada, int indice, int numFolhas, char* mensagem){
    clock_t inicio = clock();

    if (folhaUsada->usada == 1) {
        printf("ERRO: FOLHA JA USADA");
        return;
    }

    memcpy(assinatura->PublicKeysGeral, raiz->hash, MSS_HASH_SIZE);
    assinatura->alturaArvore = (int)log2(numFolhas);
    memcpy(assinatura->hashFolha, folhaUsada->hash, MSS_HASH_SIZE);
    assinatura->totalFolhas = numFolhas;
    assinatura->indiceFolha = indice;
    assinatura->tamanhoCaminho = 0;
    coletarCaminhoAutenticacao(assinatura, raiz);

    strncpy(assinatura->mensagem, mensagem, 1000);
    assinatura->mensagem[1000] = '\0';

    memcpy(assinatura->leaf_PK_seed, folhaUsada->leaf_PK_seed, N);
    memcpy(assinatura->leaf_SK_seed, folhaUsada->leaf_SK_seed, N);
    memcpy(PK_seed, folhaUsada->leaf_PK_seed, N);
    memcpy(SK_seed, folhaUsada->leaf_SK_seed, N);

    if (!assinatura->folhaPkeys) assinatura->folhaPkeys = mallocPkeys();
    for (int i = 0; i < L; i++) {
        memcpy(assinatura->folhaPkeys->PK[i], folhaUsada->Pkeys->PK[i], N);
    }

    if (assinatura->wotsSignature) free(assinatura->wotsSignature);
    assinatura->wotsSignature = mallocAssinatura();
    unsigned char msgHash[32];
    sha256_bytes((unsigned char*)mensagem, strlen(mensagem), msgHash);
    assinarMensagem(msgHash, assinatura->wotsSignature, folhaUsada->Skeys);

    folhaUsada->usada = 1;

    clock_t fim = clock();
    printf("Tempo para Assinar: %.6f segundos\n",
           (double)(fim - inicio) / CLOCKS_PER_SEC);
    fflush(stdout);
}

int verificarAssinatura(AssinaturaMSS* assinatura, unsigned char* Pkey, PublicKeys* folhaPkeys){
    PublicKeys* pk_folha = (folhaPkeys != NULL) ? folhaPkeys : assinatura->folhaPkeys;
    if (!pk_folha) {
        fprintf(stderr, "Erro: Chave pública WOTS da folha não fornecida na assinatura\n");
        return 0;
    }

    // Configura os seeds da folha para derivação de máscaras WOTS+
    memcpy(PK_seed, assinatura->leaf_PK_seed, N);
    memcpy(SK_seed, assinatura->leaf_SK_seed, N);

    // 1) Verifica assinatura WOTS da mensagem contra a chave pública WOTS da folha
    unsigned char msgHash[32];
    sha256_bytes((unsigned char*)assinatura->mensagem, strlen(assinatura->mensagem), msgHash);

    int wotsValido = verificarMensagem(msgHash, assinatura->wotsSignature, pk_folha);
    if (!wotsValido) return 0;

    // 2) Computa o hash da folha a partir das chaves públicas WOTS
    // Conforme especificação MSS: folha = H(PK[0] || PK[1] || ... || PK[L-1])
    unsigned char pk_concat[L * N];
    for(int j = 0; j < L; j++) {
        memcpy(pk_concat + j * N, pk_folha->PK[j], N);
    }
    unsigned char hashFolhaComputado[MSS_HASH_SIZE];
    sha256_bytes(pk_concat, L * N, hashFolhaComputado);

    // 3) Reconstrói a raiz via caminho de autenticação subindo da folha computada
    unsigned char hashAtual[MSS_HASH_SIZE];
    memcpy(hashAtual, hashFolhaComputado, MSS_HASH_SIZE);

    for (int i = 0; i < assinatura->tamanhoCaminho; i++){
        unsigned char concatenado[MSS_HASH_SIZE * 2];

        // Direção 0: alvo à esquerda → H(alvo || irmão)
        // Direção 1: alvo à direita  → H(irmão || alvo)
        if (assinatura->caminhoDirecao[i] == 0){
            memcpy(concatenado,                  hashAtual,               MSS_HASH_SIZE);
            memcpy(concatenado + MSS_HASH_SIZE,  assinatura->caminho[i],  MSS_HASH_SIZE);
        } else {
            memcpy(concatenado,                  assinatura->caminho[i],  MSS_HASH_SIZE);
            memcpy(concatenado + MSS_HASH_SIZE,  hashAtual,               MSS_HASH_SIZE);
        }
        sha256_bytes(concatenado, MSS_HASH_SIZE * 2, hashAtual);
    }

    // 4) Compara raiz reconstruída com a chave pública
    const unsigned char* ref = (Pkey != NULL) ? Pkey : assinatura->PublicKeysGeral;
    return (memcmp(hashAtual, ref, MSS_HASH_SIZE) == 0) ? 1 : 0;
}

/* ─── Caminho de autenticação ─────────────────────────────────────────────── */

void coletarCaminhoAutenticacao(AssinaturaMSS *assinatura, No* raiz) {
    if (raiz == NULL) return;
    if (assinatura->indiceFolha == -1) {
        fprintf(stderr, "Erro: Folha não encontrada no array\n");
        return;
    }
    assinatura->tamanhoCaminho = 0;
    coletarCaminhoRecursivo(raiz, assinatura->hashFolha,
                            assinatura->caminho,
                            assinatura->caminhoDirecao,
                            &assinatura->tamanhoCaminho,
                            assinatura->indiceFolha,
                            assinatura->totalFolhas);
}

int coletarCaminhoRecursivo(No* no, const unsigned char* folhaAlvoHash,
                            unsigned char caminhoAuth[][MSS_HASH_SIZE],
                            unsigned char direcoes[], int* tamanhoPath,
                            int indiceFolha, int numFolhas) {
    if (no == NULL) return 0;

    // Caso base: filhos são folhas
    if (no->tipo_filho_esq == TIPO_FOLHA && no->tipo_filho_dir == TIPO_FOLHA) {
        Folha* folhaEsq = (Folha*)no->filho_esq;
        Folha* folhaDir = (Folha*)no->filho_dir;

        if (memcmp(folhaEsq->hash, folhaAlvoHash, MSS_HASH_SIZE) == 0) {
            memcpy(caminhoAuth[*tamanhoPath], folhaDir->hash, MSS_HASH_SIZE);
            direcoes[*tamanhoPath] = 0;
            (*tamanhoPath)++;
            return 1;
        } else if (memcmp(folhaDir->hash, folhaAlvoHash, MSS_HASH_SIZE) == 0) {
            memcpy(caminhoAuth[*tamanhoPath], folhaEsq->hash, MSS_HASH_SIZE);
            direcoes[*tamanhoPath] = 1;
            (*tamanhoPath)++;
            return 2;
        }
        return 0;
    }

    int encontrado = 0;

    // Verifica filho esquerdo
    if (no->filho_esq != NULL) {
        if (no->tipo_filho_esq == TIPO_NO) {
            int r = coletarCaminhoRecursivo((No*)no->filho_esq, folhaAlvoHash,
                                           caminhoAuth, direcoes, tamanhoPath,
                                           indiceFolha, numFolhas);
            if (r > 0) encontrado = 1;
        } else {
            Folha* f = (Folha*)no->filho_esq;
            if (memcmp(f->hash, folhaAlvoHash, MSS_HASH_SIZE) == 0) encontrado = 1;
        }
    }

    // Verifica filho direito (se não encontrou à esquerda)
    if (encontrado == 0 && no->filho_dir != NULL) {
        if (no->tipo_filho_dir == TIPO_NO) {
            int r = coletarCaminhoRecursivo((No*)no->filho_dir, folhaAlvoHash,
                                           caminhoAuth, direcoes, tamanhoPath,
                                           indiceFolha, numFolhas);
            if (r > 0) encontrado = 2;
        } else {
            Folha* f = (Folha*)no->filho_dir;
            if (memcmp(f->hash, folhaAlvoHash, MSS_HASH_SIZE) == 0) encontrado = 2;
        }
    }

    // Adiciona o hash do irmão deste nível
    if (encontrado > 0) {
        if (encontrado == 1) {
            // Alvo à esquerda → irmão é o direito
            if (no->tipo_filho_dir == TIPO_NO)
                memcpy(caminhoAuth[*tamanhoPath], ((No*)no->filho_dir)->hash,    MSS_HASH_SIZE);
            else
                memcpy(caminhoAuth[*tamanhoPath], ((Folha*)no->filho_dir)->hash, MSS_HASH_SIZE);
            direcoes[*tamanhoPath] = 0;
            (*tamanhoPath)++;
        } else {
            // Alvo à direita → irmão é o esquerdo
            if (no->tipo_filho_esq == TIPO_NO)
                memcpy(caminhoAuth[*tamanhoPath], ((No*)no->filho_esq)->hash,    MSS_HASH_SIZE);
            else
                memcpy(caminhoAuth[*tamanhoPath], ((Folha*)no->filho_esq)->hash, MSS_HASH_SIZE);
            direcoes[*tamanhoPath] = 1;
            (*tamanhoPath)++;
        }
        return encontrado;
    }

    return 0;
}

/* ─── Limpeza ─────────────────────────────────────────────────────────────── */

void liberarFolha(Folha *folha){
    if (folha == NULL) return;
    if (folha->Skeys) free(folha->Skeys);
    if (folha->Pkeys) free(folha->Pkeys);
    free(folha);
}

void liberarNo(No *no){
    if (no == NULL) return;
    free(no);
}

void limparArvore(No *raiz){
    if (raiz == NULL) return;
    if (raiz->filho_esq != NULL && raiz->tipo_filho_esq == TIPO_NO)
        limparArvore((No*)raiz->filho_esq);
    if (raiz->filho_dir != NULL && raiz->tipo_filho_dir == TIPO_NO)
        limparArvore((No*)raiz->filho_dir);
    free(raiz);
}

/* ─── Impressão ───────────────────────────────────────────────────────────── */

void imprimirArvoreRecursiva(No* no, int nivel, char* prefixo) {
    if (no == NULL) return;
    char hex[MSS_HASH_SIZE * 2 + 1];
    bytes_to_hex(no->hash, hex);
    printf("%s[Nó nivel %d] Hash: %.16s...\n", prefixo, nivel, hex);

    char novoPrefixo[256];
    sprintf(novoPrefixo, "%s  ", prefixo);

    if (no->filho_esq != NULL) {
        if (no->tipo_filho_esq == TIPO_NO) {
            printf("%s  ├─ Esquerda:\n", prefixo);
            imprimirArvoreRecursiva((No*)no->filho_esq, nivel + 1, novoPrefixo);
        } else {
            Folha* folha = (Folha*)no->filho_esq;
            char fhex[MSS_HASH_SIZE * 2 + 1];
            bytes_to_hex(folha->hash, fhex);
            printf("%s  ├─ [Folha ESQ] Hash: %.16s... Usada: %d\n",
                   prefixo, fhex, folha->usada);
        }
    }

    if (no->filho_dir != NULL) {
        if (no->tipo_filho_dir == TIPO_NO) {
            printf("%s  └─ Direita:\n", prefixo);
            imprimirArvoreRecursiva((No*)no->filho_dir, nivel + 1, novoPrefixo);
        } else {
            Folha* folha = (Folha*)no->filho_dir;
            char fhex[MSS_HASH_SIZE * 2 + 1];
            bytes_to_hex(folha->hash, fhex);
            printf("%s  └─ [Folha DIR] Hash: %.16s... Usada: %d\n",
                   prefixo, fhex, folha->usada);
        }
    }
}

void imprimirArvore(No* raiz) {
    printf("\n========== ÁRVORE MERKLE ==========\n");
    if (raiz == NULL) { printf("Árvore vazia!\n"); return; }
    imprimirArvoreRecursiva(raiz, 0, "");
    printf("===================================\n\n");
}

/* ─── I/O – Folhas ────────────────────────────────────────────────────────── */

void escreverFolhas(char* caminho, Folha* folhas, int numFolhas) {
    FILE* arquivo = fopen(caminho, "w");
    if (!arquivo) { fprintf(stderr, "Erro ao abrir %s\n", caminho); return; }

    fprintf(arquivo, "FOLHAS_MSS\n%d\n", numFolhas);

    // Seeds globais
    fprintf(arquivo, "PK_SEED:");
    for(int i = 0; i < N; i++) fprintf(arquivo, "%02x", PK_seed[i]);
    fprintf(arquivo, "\nSK_SEED:");
    for(int i = 0; i < N; i++) fprintf(arquivo, "%02x", SK_seed[i]);
    fprintf(arquivo, "\n---\n");

    for (int i = 0; i < numFolhas; i++) {
        fprintf(arquivo, "FOLHA %d\n", i);
        // Hash da folha serializado em hex
        fprintf(arquivo, "Hash:");
        for(int k = 0; k < MSS_HASH_SIZE; k++) fprintf(arquivo, "%02x", folhas[i].hash[k]);
        fprintf(arquivo, "\n");
        fprintf(arquivo, "Usada: %d\n", folhas[i].usada);

        fprintf(arquivo, "LEAF_PK_SEED:");
        for(int k = 0; k < N; k++) fprintf(arquivo, "%02x", folhas[i].leaf_PK_seed[k]);
        fprintf(arquivo, "\n");
        fprintf(arquivo, "LEAF_SK_SEED:");
        for(int k = 0; k < N; k++) fprintf(arquivo, "%02x", folhas[i].leaf_SK_seed[k]);
        fprintf(arquivo, "\n");

        fprintf(arquivo, "WOTS_SK:\n");
        for(int j = 0; j < L; j++) {
            for(int k = 0; k < N; k++) fprintf(arquivo, "%02x", (unsigned char)folhas[i].Skeys->Sk[j][k]);
            fprintf(arquivo, "\n");
        }

        fprintf(arquivo, "WOTS_PK:\n");
        for(int j = 0; j < L; j++) {
            for(int k = 0; k < N; k++) fprintf(arquivo, "%02x", (unsigned char)folhas[i].Pkeys->PK[j][k]);
            fprintf(arquivo, "\n");
        }
        fprintf(arquivo, "---\n");
    }

    fclose(arquivo);
    printf("Folhas salvas em: %s\n", caminho);
}

void lerFolhas(char* caminho, Folha* folhas, int* numFolhas) {
    FILE* arquivo = fopen(caminho, "r");
    if (!arquivo) {
        fprintf(stderr, "Erro ao abrir %s\n", caminho);
        *numFolhas = 0;
        return;
    }

    char linha[300];
    fgets(linha, sizeof(linha), arquivo); // "FOLHAS_MSS"
    fscanf(arquivo, "%d\n", numFolhas);

    // Seeds globais
    fgets(linha, sizeof(linha), arquivo); // "PK_SEED:..."
    if (strncmp(linha, "PK_SEED:", 8) == 0)
        hex_to_bytes(linha + 8, PK_seed);
    fgets(linha, sizeof(linha), arquivo); // "SK_SEED:..."
    if (strncmp(linha, "SK_SEED:", 8) == 0)
        hex_to_bytes(linha + 8, SK_seed);
    fgets(linha, sizeof(linha), arquivo); // "---"

    for (int i = 0; i < *numFolhas; i++) {
        int indice;
        fscanf(arquivo, "FOLHA %d\n", &indice);

        fgets(linha, sizeof(linha), arquivo); // "Hash:..."
        if (strncmp(linha, "Hash:", 5) == 0)
            hex_to_bytes(linha + 5, folhas[i].hash);

        fscanf(arquivo, "Usada: %d\n", &folhas[i].usada);

        fgets(linha, sizeof(linha), arquivo); // "LEAF_PK_SEED:..."
        if (strncmp(linha, "LEAF_PK_SEED:", 13) == 0)
            hex_to_bytes(linha + 13, folhas[i].leaf_PK_seed);

        fgets(linha, sizeof(linha), arquivo); // "LEAF_SK_SEED:..."
        if (strncmp(linha, "LEAF_SK_SEED:", 13) == 0)
            hex_to_bytes(linha + 13, folhas[i].leaf_SK_seed);

        fgets(linha, sizeof(linha), arquivo); // "WOTS_SK:"
        for(int j = 0; j < L; j++) {
            fgets(linha, sizeof(linha), arquivo);
            for(int k = 0; k < N; k++) {
                char hex[3] = { linha[k*2], linha[k*2+1], '\0' };
                folhas[i].Skeys->Sk[j][k] = (unsigned char)strtol(hex, NULL, 16);
            }
        }

        fgets(linha, sizeof(linha), arquivo); // "WOTS_PK:"
        for(int j = 0; j < L; j++) {
            fgets(linha, sizeof(linha), arquivo);
            for(int k = 0; k < N; k++) {
                char hex[3] = { linha[k*2], linha[k*2+1], '\0' };
                folhas[i].Pkeys->PK[j][k] = (unsigned char)strtol(hex, NULL, 16);
            }
        }
        fgets(linha, sizeof(linha), arquivo); // "---"
    }

    fclose(arquivo);
    printf("Folhas carregadas de: %s (%d folhas)\n", caminho, *numFolhas);
}

/* ─── I/O – Assinatura ────────────────────────────────────────────────────── */

void escreverAssinaturaMSS(char* caminho, AssinaturaMSS* assinatura) {
    FILE* arquivo = fopen(caminho, "w");
    if (!arquivo) { fprintf(stderr, "Erro ao abrir %s\n", caminho); return; }

    fprintf(arquivo, "ASSINATURA_MSS\n---\n");

    // PublicKey (raiz) serializada em hex
    fprintf(arquivo, "PublicKey:");
    for(int i = 0; i < MSS_HASH_SIZE; i++) fprintf(arquivo, "%02x", assinatura->PublicKeysGeral[i]);
    fprintf(arquivo, "\n");

    fprintf(arquivo, "AlturaArvore: %d\n", assinatura->alturaArvore);
    fprintf(arquivo, "TotalFolhas: %d\n",  assinatura->totalFolhas);
    fprintf(arquivo, "IndiceFolha: %d\n",  assinatura->indiceFolha);

    // Hash da folha serializado em hex
    fprintf(arquivo, "HashFolha:");
    for(int i = 0; i < MSS_HASH_SIZE; i++) fprintf(arquivo, "%02x", assinatura->hashFolha[i]);
    fprintf(arquivo, "\n");

    fprintf(arquivo, "TamanhoCaminho: %d\n", assinatura->tamanhoCaminho);
    fprintf(arquivo, "Mensagem: %s\n", assinatura->mensagem);
    fprintf(arquivo, "---\n");

    // Seeds da folha
    fprintf(arquivo, "LEAF_PK_SEED:");
    for(int i = 0; i < 32; i++) fprintf(arquivo, "%02x", assinatura->leaf_PK_seed[i]);
    fprintf(arquivo, "\nLEAF_SK_SEED:");
    for(int i = 0; i < 32; i++) fprintf(arquivo, "%02x", assinatura->leaf_SK_seed[i]);
    fprintf(arquivo, "\n---\n");

    // Chave pública WOTS da folha
    if (assinatura->folhaPkeys) {
        fprintf(arquivo, "WOTS_PK\n");
        for(int i = 0; i < L; i++) {
            for(int j = 0; j < N; j++)
                fprintf(arquivo, "%02x", (unsigned char)assinatura->folhaPkeys->PK[i][j]);
            fprintf(arquivo, "\n");
        }
        fprintf(arquivo, "---\n");
    }

    // Assinatura WOTS
    fprintf(arquivo, "WOTS_SIGNATURE\n");
    for(int i = 0; i < L; i++) {
        for(int j = 0; j < N; j++)
            fprintf(arquivo, "%02x", (unsigned char)assinatura->wotsSignature->assinatura[i][j]);
        fprintf(arquivo, "\n");
    }
    fprintf(arquivo, "---\n");

    // Caminho de autenticação (serializado em hex)
    fprintf(arquivo, "CAMINHO\n");
    for (int i = 0; i < assinatura->tamanhoCaminho; i++) {
        fprintf(arquivo, "%d:", i);
        for(int k = 0; k < MSS_HASH_SIZE; k++) fprintf(arquivo, "%02x", assinatura->caminho[i][k]);
        fprintf(arquivo, "\n");
    }
    fprintf(arquivo, "---\n");

    // Direções
    fprintf(arquivo, "DIRECOES\n");
    for (int i = 0; i < assinatura->tamanhoCaminho; i++)
        fprintf(arquivo, "%d: %d\n", i, assinatura->caminhoDirecao[i]);
    fprintf(arquivo, "---\n");

    fclose(arquivo);
    printf("Assinatura salva em: %s\n", caminho);
}

void lerAssinaturaMSS(char* caminho, AssinaturaMSS* assinatura) {
    FILE* arquivo = fopen(caminho, "r");
    if (!arquivo) { fprintf(stderr, "Erro ao abrir %s\n", caminho); return; }

    char linha[300];
    if (!fgets(linha, sizeof(linha), arquivo)) { fclose(arquivo); return; } // "ASSINATURA_MSS"
    if (!fgets(linha, sizeof(linha), arquivo)) { fclose(arquivo); return; } // "---"

    // PublicKey
    if (fgets(linha, sizeof(linha), arquivo) && strncmp(linha, "PublicKey:", 10) == 0)
        hex_to_bytes(linha + 10, assinatura->PublicKeysGeral);

    if (fscanf(arquivo, "AlturaArvore: %d\n", &assinatura->alturaArvore) != 1) {}
    if (fscanf(arquivo, "TotalFolhas: %d\n",  &assinatura->totalFolhas) != 1) {}
    if (fscanf(arquivo, "IndiceFolha: %d\n",  &assinatura->indiceFolha) != 1) {}

    // HashFolha
    if (fgets(linha, sizeof(linha), arquivo) && strncmp(linha, "HashFolha:", 10) == 0)
        hex_to_bytes(linha + 10, assinatura->hashFolha);

    if (fscanf(arquivo, "TamanhoCaminho: %d\n", &assinatura->tamanhoCaminho) != 1) {}

    if (fgets(linha, sizeof(linha), arquivo) && strncmp(linha, "Mensagem:", 9) == 0) {
        strncpy(assinatura->mensagem, linha + 10, 1000);
        assinatura->mensagem[strcspn(assinatura->mensagem, "\n\r")] = 0;
    }

    // Lê as seções restantes dinamicamente pelo cabeçalho
    while (fgets(linha, sizeof(linha), arquivo)) {
        if (strncmp(linha, "LEAF_PK_SEED:", 13) == 0) {
            hex_to_bytes(linha + 13, assinatura->leaf_PK_seed);
        } else if (strncmp(linha, "LEAF_SK_SEED:", 13) == 0) {
            hex_to_bytes(linha + 13, assinatura->leaf_SK_seed);
        } else if (strncmp(linha, "WOTS_PK", 7) == 0) {
            if (!assinatura->folhaPkeys) assinatura->folhaPkeys = mallocPkeys();
            for(int i = 0; i < L; i++) {
                if (!fgets(linha, sizeof(linha), arquivo)) break;
                for(int j = 0; j < N; j++) {
                    char hex[3] = { linha[j*2], linha[j*2+1], '\0' };
                    assinatura->folhaPkeys->PK[i][j] = (unsigned char)strtol(hex, NULL, 16);
                }
            }
        } else if (strncmp(linha, "WOTS_SIGNATURE", 14) == 0) {
            if (!assinatura->wotsSignature) assinatura->wotsSignature = mallocAssinatura();
            for(int i = 0; i < L; i++) {
                if (!fgets(linha, sizeof(linha), arquivo)) break;
                for(int j = 0; j < N; j++) {
                    char hex[3] = { linha[j*2], linha[j*2+1], '\0' };
                    assinatura->wotsSignature->assinatura[i][j] = (unsigned char)strtol(hex, NULL, 16);
                }
            }
        } else if (strncmp(linha, "CAMINHO", 7) == 0) {
            for (int i = 0; i < assinatura->tamanhoCaminho; i++) {
                if (!fgets(linha, sizeof(linha), arquivo)) break;
                char* ptr = strchr(linha, ':');
                if (ptr) hex_to_bytes(ptr + 1, assinatura->caminho[i]);
            }
        } else if (strncmp(linha, "DIRECOES", 8) == 0) {
            for (int i = 0; i < assinatura->tamanhoCaminho; i++) {
                int indice, direcao;
                if (fscanf(arquivo, "%d: %d\n", &indice, &direcao) == 2) {
                    assinatura->caminhoDirecao[i] = (unsigned char)direcao;
                }
            }
        }
    }

    fclose(arquivo);
    printf("Assinatura carregada de: %s\n", caminho);
}

/* ─── I/O – Chave pública (raiz) ─────────────────────────────────────────── */

void escreverPublicKey(char* caminho, const unsigned char* publicKey) {
    FILE* arquivo = fopen(caminho, "w");
    if (!arquivo) { fprintf(stderr, "Erro ao abrir %s\n", caminho); return; }
    fprintf(arquivo, "PUBLIC_KEY\n");
    for(int i = 0; i < MSS_HASH_SIZE; i++) fprintf(arquivo, "%02x", publicKey[i]);
    fprintf(arquivo, "\n");
    fclose(arquivo);
    printf("Chave pública salva em: %s\n", caminho);
}

int lerPublicKey(char* caminho, unsigned char* outPublicKey) {
    FILE* arquivo = fopen(caminho, "r");
    if (!arquivo) return 0;
    char linha[300];
    if (fgets(linha, sizeof(linha), arquivo) == NULL) { fclose(arquivo); return 0; }
    if (strncmp(linha, "PUBLIC_KEY", 10) == 0) {
        if (fgets(linha, sizeof(linha), arquivo) == NULL) { fclose(arquivo); return 0; }
    }
    linha[strcspn(linha, "\r\n")] = '\0';
    hex_to_bytes(linha, outPublicKey);
    fclose(arquivo);
    return 1;
}

/* ─── I/O – Árvore ────────────────────────────────────────────────────────── */

void escreverArvoreRecursivo(FILE* arquivo, No* no, int nivel) {
    if (no == NULL) { fprintf(arquivo, "NULL\n"); return; }

    char hex[MSS_HASH_SIZE * 2 + 1];
    bytes_to_hex(no->hash, hex);
    fprintf(arquivo, "NO %d %s\n", nivel, hex);

    if (no->tipo_filho_esq == TIPO_FOLHA) {
        Folha* folha = (Folha*)no->filho_esq;
        char fhex[MSS_HASH_SIZE * 2 + 1];
        bytes_to_hex(folha->hash, fhex);
        fprintf(arquivo, "FOLHA %s %d\n", fhex, folha->usada);
    } else {
        escreverArvoreRecursivo(arquivo, (No*)no->filho_esq, nivel + 1);
    }

    if (no->tipo_filho_dir == TIPO_FOLHA) {
        Folha* folha = (Folha*)no->filho_dir;
        char fhex[MSS_HASH_SIZE * 2 + 1];
        bytes_to_hex(folha->hash, fhex);
        fprintf(arquivo, "FOLHA %s %d\n", fhex, folha->usada);
    } else {
        escreverArvoreRecursivo(arquivo, (No*)no->filho_dir, nivel + 1);
    }
}

void escreverArvore(char* caminho, No* raiz) {
    FILE* arquivo = fopen(caminho, "w");
    if (!arquivo) { fprintf(stderr, "Erro ao abrir %s\n", caminho); return; }
    fprintf(arquivo, "ARVORE_MSS\n---\n");
    escreverArvoreRecursivo(arquivo, raiz, 0);
    fclose(arquivo);
    printf("Árvore salva em: %s\n", caminho);
}

No* lerArvoreRecursivo(FILE* arquivo, Folha* folhas, int numFolhas) {
    char tipo[20];
    if (fscanf(arquivo, "%s", tipo) != 1) return NULL;

    if (strcmp(tipo, "NULL") == 0) return NULL;

    if (strcmp(tipo, "FOLHA") == 0) {
        char hex[MSS_HASH_SIZE * 2 + 1];
        int usada;
        fscanf(arquivo, "%s %d\n", hex, &usada);

        unsigned char hash[MSS_HASH_SIZE];
        hex_to_bytes(hex, hash);

        for (int i = 0; i < numFolhas; i++) {
            if (memcmp(folhas[i].hash, hash, MSS_HASH_SIZE) == 0) {
                return (No*)&folhas[i];
            }
        }
        fprintf(stderr, "AVISO: Folha com hash %.16s... não encontrada\n", hex);
        return NULL;
    }

    if (strcmp(tipo, "NO") == 0) {
        int nivel;
        char hex[MSS_HASH_SIZE * 2 + 1];
        fscanf(arquivo, "%d %s\n", &nivel, hex);

        No* no = alocarNo();
        hex_to_bytes(hex, no->hash);
        no->nivel = nivel;

        No* filhoEsq = lerArvoreRecursivo(arquivo, folhas, numFolhas);
        if (filhoEsq == NULL) {
            no->filho_esq = NULL;
            no->tipo_filho_esq = TIPO_NO;
        } else if ((char*)filhoEsq >= (char*)folhas &&
                   (char*)filhoEsq <  (char*)(folhas + numFolhas)) {
            no->filho_esq = filhoEsq;
            no->tipo_filho_esq = TIPO_FOLHA;
        } else {
            no->filho_esq = filhoEsq;
            no->tipo_filho_esq = TIPO_NO;
        }

        No* filhoDir = lerArvoreRecursivo(arquivo, folhas, numFolhas);
        if (filhoDir == NULL) {
            no->filho_dir = NULL;
            no->tipo_filho_dir = TIPO_NO;
        } else if ((char*)filhoDir >= (char*)folhas &&
                   (char*)filhoDir <  (char*)(folhas + numFolhas)) {
            no->filho_dir = filhoDir;
            no->tipo_filho_dir = TIPO_FOLHA;
        } else {
            no->filho_dir = filhoDir;
            no->tipo_filho_dir = TIPO_NO;
        }

        return no;
    }

    return NULL;
}

void lerArvore(char* caminho, Folha* folhas, int numFolhas, No** raiz) {
    FILE* arquivo = fopen(caminho, "r");
    if (!arquivo) { fprintf(stderr, "Erro ao abrir %s\n", caminho); *raiz = NULL; return; }
    char linha[300];
    fgets(linha, sizeof(linha), arquivo); // "ARVORE_MSS"
    fgets(linha, sizeof(linha), arquivo); // "---"
    *raiz = lerArvoreRecursivo(arquivo, folhas, numFolhas);
    fclose(arquivo);
    printf("Árvore carregada de: %s\n", caminho);
}
