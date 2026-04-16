#include "keys.h"
#include "../SHA256/sha256.h"
#include <stdlib.h>
#include <stdio.h>
#include <sodium.h>
#include <math.h>
#include <string.h>

void gerarKeys(Keys* keys){
        if(sodium_init() < 0){
        fprintf(stderr, "Erro: falha ao inicializar libsodium.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < HORS_T; i++) {
        randombytes_buf(keys->SKeys[i], KEY_SIZE);
        sha256_bytes(keys->SKeys[i], KEY_SIZE, keys->PKeys[i]);
    }
    printf("Chaves Geradas\n");
}

void assinarMensagem(const char* msg,int msg_len, 
                    Assinatura* assinatura, 
                    const unsigned char SKeys[HORS_T][HORS_N]){
    unsigned char hash_msg[32];
    sha256_bytes(msg, msg_len, hash_msg);

    int indices[HORS_K];
    int total = selecionarIndices(hash_msg, indices);

    for (int i=0; i<total; i++){
        memcpy(assinatura->assinatura[i], SKeys[indices[i]], KEY_SIZE);
    }
}

int selecionarIndices(unsigned char *hash, int *indices){
    int bits_por_indice = HORS_BITS_PER_INDEX;
    int hash_bits = 256;

    int bit_pos = 0;
    for(int i=0;i< HORS_K; i++){
        int index  = 0;
        
        for(int j = 0; j<bits_por_indice; j++){
            if(bit_pos >= hash_bits) break;

            int byte_index = bit_pos / 8;
            int bit_index = 7-(bit_pos%8);

            int bit = (hash[byte_index] >> bit_index) & 1;
            index = (index << 1) |  bit;

            bit_pos++;
        }
        indices[i]= index;
    }
    return HORS_K;
}


int verificarAssinatura(const char* msg, 
                        const Assinatura* assinatura,
                        const unsigned char PKeys[HORS_T][HORS_N]){
    return 1;
}


void imprimirAssinatura(const Assinatura* assinatura) {
    printf("=== Assinatura HORS ===\n");
    for (int i = 0; i < HORS_K; i++) {
        printf("Componente %d: ", i);
        for (int j = 0; j < HORS_N; j++) {
            printf("%02x", assinatura->assinatura[i][j]);
        }
        printf("\n");
    }
    printf("=======================\n");
}
