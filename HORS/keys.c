#include "keys.h"
#include "../SHA256/sha256.h"
#include <stdlib.h>
#include <stdio.h>
#include <sodium.h>

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

void assinarMensagem(const char* msg, Assinatura* assinatura, const unsigned char SKeys[HORS_T][HORS_N]);


int verificarAssinatura(const char* msg, const Assinatura* assinatura, const unsigned char PKeys[HORS_T][HORS_N]);
