#include <sodium.h>
#include "keys.h"
#include "prf.h"
#include "../SHA256/sha256.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

unsigned char PK_seed[N] = {0};
unsigned char SK_seed[N] = {0};

void initializeSeeds() {
    if (sodium_init() < 0) {
        fprintf(stderr, "Erro ao inicializar libsodium\n");
        exit(EXIT_FAILURE);
    }

    randombytes_buf(PK_seed, N);
    randombytes_buf(SK_seed, N);
}


SecretKeys* mallocSkeys(){
    SecretKeys* k = (SecretKeys*)malloc(sizeof(SecretKeys));
    if (k == NULL) {
        fprintf(stderr, "Erro ao alocar memória para Secret keys\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i <L; i++) {
        for (int j=0 ;j<N; j++){
            k->Sk[i][j] = 0;  // Inicializa com 0, não NULL
        }
    }
    
    return k;
}
PublicKeys* mallocPkeys(){
    PublicKeys* k = (PublicKeys*)malloc(sizeof(PublicKeys));
    if (k == NULL) {
        fprintf(stderr, "Erro ao alocar memória para Public keys\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i <L; i++) {
        for (int j=0 ;j<N; j++){
            k->PK[i][j] = 0;  // Inicializa com 0, não NULL
        }
    }
    return k;
}
Assinatura* mallocAssinatura() {
    Assinatura* a = (Assinatura*)malloc(sizeof(Assinatura));
    if (a == NULL) {
        fprintf(stderr, "Erro ao alocar memória para Assinatura\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < L; i++) {
        for (int j = 0; j < N; j++) {
            a->assinatura[i][j] = 0;
        }
    }
    printf("Memória para assinatura alocada com sucesso\n");
    return a;
}

// Deriva W-1 máscaras pseudo-aleatórias para a chave de índice key_index.
// Deve ser chamada antes de chainFunctionWOTSplus para pré-computar as máscaras.
static void deriveMasksForKey(int key_index, unsigned char masks[W - 1][N]) {
    unsigned char ADRS[32];
    for (int step = 0; step < W - 1; step++) {
        setADRS_WOTS_MASK(ADRS, key_index, step);
        PRF_SHA2(masks[step], PK_seed, SK_seed, ADRS, N);
    }
}

void generateSKeys(SecretKeys* sKeys) {
    unsigned char ADRS[32];
    
    for(int i = 0; i < L; i++) {
        // Configurar ADRS para WOTS_PRF
        setADRS_WOTS_PRF(ADRS, i);
        
        // Gerar chave secreta usando PRF (não mais rand())
        PRF_SHA2((unsigned char*)sKeys->Sk[i], PK_seed, SK_seed, ADRS, N);
    }
}

void generatePKeys(PublicKeys* pKeys, SecretKeys* sKeys) {
    unsigned char ADRS[32];
    
    // Para cada uma das L chaves secretas
    for (int i = 0; i < L; i++) {
        // Configurar ADRS para WOTS_HASH
        setADRS_WOTS_HASH(ADRS, i, 0, 0);
        
        // Pré-calcular máscaras para esta chave (W-1 hashes PRF)
        unsigned char masks[W - 1][N];
        deriveMasksForKey(i, masks);
        
        // Aplicar chain function com W-1 passos (começando do índice 0)
        chainFunctionWOTSplus((unsigned char*)sKeys->Sk[i], W-1,
                             (unsigned char*)pKeys->PK[i], ADRS, 0, masks);
    }
}

void chainFunction(unsigned char* src, int steps, unsigned char* output, unsigned char* ADRS_base, int start_index) {

    unsigned char ADRS[32];
    memcpy(ADRS, ADRS_base, 32);
    
    // Copiar src para output
    memcpy(output, src, N);
    
    for (int i = 0; i < steps; i++) {
        // Atualizar hash do ADRS (start_index + i)
        int hash_idx = start_index + i;
        ADRS[28] = (hash_idx >> 24) & 0xFF;
        ADRS[29] = (hash_idx >> 16) & 0xFF;
        ADRS[30] = (hash_idx >> 8) & 0xFF;
        ADRS[31] = hash_idx & 0xFF;
        
        // Aplicar F
        unsigned char temp[N];
        F_function(temp, PK_seed, ADRS, output);
        memcpy(output, temp, N);
    }
}

// Executa a função de cadeia WOTS+ usando máscaras pré-calculadas.
// 'masks' deve ter W-1 entradas de N bytes (pré-derivadas via deriveMasksForKey).
void chainFunctionWOTSplus(unsigned char* src, int steps, unsigned char* output,
                           unsigned char* ADRS_base, int start_index,
                           const unsigned char masks[W - 1][N]) {
    unsigned char ADRS[32];
    memcpy(ADRS, ADRS_base, 32);

    // Copiar src para output
    memcpy(output, src, N);

    for (int i = 0; i < steps; i++) {
        int hash_idx = start_index + i;

        // Atualizar hash do ADRS (hash_idx)
        ADRS[28] = (hash_idx >> 24) & 0xFF;
        ADRS[29] = (hash_idx >> 16) & 0xFF;
        ADRS[30] = (hash_idx >>  8) & 0xFF;
        ADRS[31] =  hash_idx        & 0xFF;

        unsigned char masked[N];
        for (int j = 0; j < N; j++) {
            masked[j] = output[j] ^ masks[hash_idx][j];
        }

        unsigned char temp[N];
        F_function(temp, PK_seed, ADRS, masked);
        memcpy(output, temp, N);
    }
}

void mensageForBlocks(const unsigned char msgHash[N], int* output) {
    for (int i = 0; i < L1; i++) {
        int byte_index = i / 2;
        int nibble_index = i % 2;
        unsigned char byte = msgHash[byte_index];

        if (nibble_index == 0) {
            output[i] = (byte >> 4) & 0x0F; // Nibble superior
        } else {
            output[i] = byte & 0x0F;        // Nibble inferior
        }
    }
}
void calcularChecksum(const int* message_blocks, int* checksum_blocks){
    int checksum = 0;
    
    for (int i = 0; i < L1; i++) {
        checksum += (W - 1 - message_blocks[i]);
    }

    int temp = checksum;
    for (int i = 0; i < L2; i++) {
        checksum_blocks[i] = temp % W;
        temp = temp / W;

    }

    if (temp > 0) {
        printf("ERRO: Checksum muito grande! Overflow detectado.\n");
    }
}

void assinarMensagem(const unsigned char msgHash[N],  Assinatura* assinatura,
                                SecretKeys* sKeys){    
    int message_blocks[L1];
    int checksum_blocks[L2];
    int b[L];  // mensagem + checksum concatenados
    
    // Converter mensagem para base W
    mensageForBlocks(msgHash, message_blocks);
    
    //  Calcular checksum
    calcularChecksum(message_blocks, checksum_blocks);
    
    for (int i = 0; i < L1; i++) {
        b[i] = message_blocks[i];
    }
    for (int i = 0; i < L2; i++) {
        b[L1 + i] = checksum_blocks[i];
    }

    // Gerar assinatura aplicando chain function
    for (int i = 0; i < L; i++) {
        unsigned char ADRS[32];
        setADRS_WOTS_HASH(ADRS, i, 0, 0);
        
        // Pré-calcular máscaras uma única vez para esta chave
        unsigned char masks[W - 1][N];
        deriveMasksForKey(i, masks);
        
        chainFunctionWOTSplus((unsigned char*)sKeys->Sk[i], b[i],
                             (unsigned char*)assinatura->assinatura[i], ADRS, 0, masks);
    }
}

int verificarMensagem(const unsigned char msgHash[N], Assinatura* assinatura, PublicKeys* pKeys) {
    
    int message_blocks[L1];
    int checksum_blocks[L2];
    int b[L];
    
    mensageForBlocks(msgHash, message_blocks);
    calcularChecksum(message_blocks, checksum_blocks);
    
    for (int i = 0; i < L1; i++) {
        b[i] = message_blocks[i];
    }
    for (int i = 0; i < L2; i++) {
        b[L1 + i] = checksum_blocks[i];
    }
    
    for (int i = 0; i < L; i++) {
        unsigned char computed_pk[N];
        int remaining_steps = W - 1 - b[i];
        
        // Pré-calcular máscaras uma única vez para esta chave
        unsigned char masks[W - 1][N];
        deriveMasksForKey(i, masks);
        
        if (remaining_steps > 0) {
            unsigned char ADRS[32];
            setADRS_WOTS_HASH(ADRS, i, 0, 0);
            chainFunctionWOTSplus(assinatura->assinatura[i], remaining_steps,
                                  computed_pk, ADRS, b[i], masks);
        } else {
            memcpy(computed_pk, assinatura->assinatura[i], N);
        }
        
        if (memcmp(computed_pk, (unsigned char*)pKeys->PK[i], N) != 0) {
            return 0;
        }
    }
    return 1;
}