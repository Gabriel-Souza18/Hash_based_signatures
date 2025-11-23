#include "keys.h"
#include "prf.h"
#include "../SHA256/sha256.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

unsigned char PK_seed[N] = {0};
unsigned char SK_seed[N] = {0};

void initializeSeeds() {
    // Gera seeds aleatórios mudar a funcçao rand
    srand(clock());
    for(int i = 0; i < N; i++) {
        PK_seed[i] = rand() % 256;
        SK_seed[i] = rand() % 256;
    }
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
        setADRS_WOTS_HASH(ADRS, i, 0, 0); // chain_index e hash_index serão atualizados na chain function
        
        // Aplicar chain function com W-1 passos (começando do índice 0)
        chainFunction((unsigned char*)sKeys->Sk[i], W-1, (unsigned char*)pKeys->PK[i], ADRS, 0);
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
void mensageForBlocks(char* msgHash, int* output) {
    for (int i = 0; i < L1; i++) {
        int byte_index = i / 2;
        int nibble_index = i % 2;
        unsigned char byte = (unsigned char)msgHash[byte_index];
        
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

void assinarMensagem(char*msg,  Assinatura* assinatura,
                                SecretKeys* sKeys){    
    int message_blocks[L1];
    int checksum_blocks[L2];
    int b[L];  // mensagem + checksum concatenados
    
    // Converter mensagem para base W
    mensageForBlocks(msg, message_blocks);
    
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
        chainFunction((unsigned char*)sKeys->Sk[i], b[i], (unsigned char*)assinatura->assinatura[i], ADRS, 0);
    }

}

int verificarMensagem(char* msg, Assinatura* assinatura, PublicKeys* pKeys) {
    printf("\n=== INICIANDO VERIFICAÇÃO ===\n");
    
    int message_blocks[L1];
    int checksum_blocks[L2];
    int b[L];
    
    mensageForBlocks(msg, message_blocks);
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
        
        if (remaining_steps > 0) {
            unsigned char ADRS[32];
            setADRS_WOTS_HASH(ADRS, i, 0, 0);
            // Continuar da posição b[i] até W-1
            chainFunction(assinatura->assinatura[i], remaining_steps, computed_pk, ADRS, b[i]);
        } else {
            memcpy(computed_pk, assinatura->assinatura[i], N);
        }
        
        // Comparar com a chave pública
        if (memcmp(computed_pk, (unsigned char*)pKeys->PK[i], N) != 0) {
            printf("ERRO: Verificação falhou no elemento %d\n", i);
            return 0;
        }

    }
    
    printf("=== VERIFICAÇÃO BEM-SUCEDIDA ===\n");
    return 1;
}