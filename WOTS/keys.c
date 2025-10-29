#include "keys.h"
#include "../SHA256/sha256.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


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
    printf("Memoria Alocada com sucesso\n");
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
    printf("Memoria Alocada com sucesso\n");
    return k;
}
Masks* mallocMasks(){
    Masks* m = (Masks*)malloc(sizeof(Masks));
    if (m == NULL) {
        fprintf(stderr, "Erro ao alocar memória para Masks\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i <W-1; i++) {
        for (int j=0 ;j<N; j++){
            m->masks[i][j] = 0;  // Inicializa com 0, não NULL
        }
    }
    printf("Memoria Alocada com sucesso\n");
    return m;
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


void generateSKeys(SecretKeys* sKeys ){
        for(int i = 0; i < L; i++) {
            for(int j = 0; j < N; j++) {
                sKeys->Sk[i][j] = rand() % 256;
            }
        }
}

void generatePKeys(PublicKeys* pKeys, SecretKeys* sKeys, Masks* masks){
    // Para cada uma das L chaves secretas
    for (int i = 0; i < L; i++) {
      chainFunction(sKeys->Sk[i], W-1, masks, pKeys->PK[i]);
    }

}
void generateMasks(Masks* masks){
    
    // Gera W-1 máscaras aleatórias
    for (int i = 0; i < W-1; i++) {
        for (int j = 0; j < N; j++) {
            masks->masks[i][j] = rand() % 256; 
        }

    }
}

void chainFunction(char*src, int steps, Masks* masks, char* output){
    memcpy(output, src, N);
    
    for (int i = 0; i < steps; i++) {
        char temp[N];
        
        // XOR com a máscara atual
        for (int j = 0; j < N; j++) {
            temp[j] = output[j] ^ masks->masks[i][j];
        }
        
 
        char hash_result[SHA256_HEX_SIZE];
        sha256_hex(temp, N, hash_result);
     
        for (int j = 0; j < N && j * 2 < SHA256_HEX_SIZE; j++) {
            char hex_byte[3] = {hash_result[j*2], hash_result[j*2+1], '\0'};
            output[j] = (char)strtol(hex_byte, NULL, 16);
        }
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
                                SecretKeys* sKeys, 
                                Masks* masks){    
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
        chainFunction(sKeys->Sk[i], b[i], masks, assinatura->assinatura[i]);
    }

}

int verificarMensagem(char* msg, Assinatura* assinatura, Masks* masks, PublicKeys* pKeys) {
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
        char computed_pk[N];
        int remaining_steps = W - 1 - b[i];
        
        if (remaining_steps > 0) {
            Masks partial_masks;
            for (int j = 0; j < remaining_steps; j++) {
                int mask_index = b[i] + j;  // Começa da máscara b[i], não da 0
                if (mask_index < W - 1) {
                    memcpy(partial_masks.masks[j], masks->masks[mask_index], N);
                }
            }
            chainFunction(assinatura->assinatura[i], remaining_steps, &partial_masks, computed_pk);
        } else {
            memcpy(computed_pk, assinatura->assinatura[i], N);
        }
        
        // Comparar com a chave pública
        if (memcmp(computed_pk, pKeys->PK[i], N) != 0) {
            printf("ERRO: Verificação falhou no elemento %d (b[%d]=%d, steps=%d)\n", 
                   i, i, b[i], W-1-b[i]);
            return 0;
        }

    }
    
    printf("=== VERIFICAÇÃO BEM-SUCEDIDA ===\n");
    return 1;
}