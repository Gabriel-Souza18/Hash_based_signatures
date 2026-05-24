#include "dicionario.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>

/* Função auxiliar: calcula SHA256 em hex usando OpenSSL */
void sha256_hex(const unsigned char *data, size_t len, char *hex_out) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256(data, len, digest);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hex_out + (i * 2), "%02x", digest[i]);
    }
    hex_out[SHA256_DIGEST_LENGTH * 2] = '\0';
}

int main(void){
    /* Inicializar OpenSSL (boa prática, mesmo que automático em 1.1+) */
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    char * pequeno= "Pequeno.txt" ;
    char * grande= "Grande.txt" ;
    char hex[SHA256_DIGEST_LENGTH * 2 + 1];
    DicionarioHash *dict = carregarVetores(grande);
    if (!dict) {
        printf("Erro ao carregar arquivo\n");
        return 1;
    }

    printf("Carregados %d pares string-hash\n", dict->count);

    for (int i = 0; i < dict->count; i++) {
        printf("Tamanho da String: %ld\n", strlen(dict->strings[i]));
        double inicio = clock();
        sha256_hex((unsigned char*)dict->strings[i], strlen(dict->strings[i]), hex);
        double fim = clock();
        if (strcmp(hex, dict->hashes[i]) == 0) {
            printf("[%d] -> hash'%s' (correto)\n", i, dict->hashes[i]);
        } else {
            printf("[%d] -> hash'%s' (incorreto, calculado: %s)\n", i, dict->hashes[i], hex);
        }
        double tempoTotalSec = ((double)(fim - inicio) / CLOCKS_PER_SEC);      
        double tempoTotalMs = ((double)(fim - inicio) / CLOCKS_PER_SEC) * 1000.0;
        printf("Tempo gasto: %0.10lf ms\n", tempoTotalMs);
        printf("Tempo gasto: %0.10lf Sec\n", tempoTotalSec);
        

    }

    liberarDicionario(dict);
    return 0;
}
