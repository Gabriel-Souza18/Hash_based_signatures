#include "../SHA256/sha256.h"
#include "dicionario.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void){
    char * pequeno= "TesteHash/Pequeno.txt" ;
    char * grande= "TesteHash/Grande.txt" ;
    char hex[SHA256_HEX_SIZE];
    DicionarioHash *dict = carregarVetores(grande);
    if (!dict) {
        printf("Erro ao carregar arquivo\n");
        return 1;
    }

    printf("Carregados %d pares string-hash\n", dict->count);

    for (int i = 0; i < dict->count; i++) {
        printf("Tamanho da String: %ld\n", strlen(dict->strings[i]));
        double inicio = clock();
        sha256_hex(dict->strings[i], strlen(dict->strings[i]), hex);
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
