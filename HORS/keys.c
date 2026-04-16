#include "keys.h"
#include "../SHA256/sha256.h"
#include <stdlib.h>
#include <stdio.h>
#include <sodium.h>
#include <math.h>

void gerarKeys(Keys* keys){
        if(sodium_init() < 0){
        fprintf(stderr, "Erro: falha ao inicializar libsodium.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 256; i++) {
        randombytes_buf(keys->SKeys[i], KEY_SIZE);
        sha256_bytes(keys->PKeys[i], KEY_SIZE, keys->SKeys[i]);
    }
    printf("Chaves Geradas\n");

}

void assinarMensagem(const char* msg, Assinatura* assinatura, const unsigned char SKeys[HORS_T][HORS_N]){}

int selecionarIndices(unsigned char *hash, int *indices){
    int bits_por_indice = (int)log2(HORS_T);
    int total_bits = HORS_K * bits_por_indice;

    int bit_pos = 0;
    for(int i=0;i< HORS_K; i++){
        int index  = 0;
        
        for(int j = 0; j<bits_por_indice; j++){
            int byte_index = bit_pos / 8;
            int bit_index = 7-(bit_pos%8);

            int bit = (hash[byte_index] >> bit_index) & 1;
            index = (index << 1) |  bit;

            bit_pos++;
        }
        indices[i]= index;
    }
    return total_bits;
}


int verificarAssinatura(const char* msg, const Assinatura* assinatura, const unsigned char PKeys[HORS_T][HORS_N]){}
