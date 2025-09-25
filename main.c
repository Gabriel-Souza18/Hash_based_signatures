#include "sha256.h"
#include "dicionario.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void){
    char hex[SHA256_HEX_SIZE];
    // Carrega dicionário do arquivo
    DicionarioHash *dict = carregarVetores("Arquivo.txt");
    if (!dict) {
        printf("Erro ao carregar arquivo\n");
        return 1;
    }

    printf("Carregados %d pares string-hash\n", dict->count);

    for (int i = 0; i < dict->count; i++) {
        printf("Tamanho da String: %ld\n", strlen(dict->strings[i]));
        double inicioInMs = (clock()/CLOCKS_PER_SEC)*1e9;
        sha256_hex(dict->strings[i], strlen(dict->strings[i]), hex);
        double fimInMs = (clock()/CLOCKS_PER_SEC)*1e9;
        if (strcmp(hex, dict->hashes[i]) == 0) {
            printf("[%d] -> hash'%s' (correto)\n", i, dict->hashes[i]);
        } else {
            printf("[%d] -> hash'%s' (incorreto, calculado: %s)\n", i, dict->hashes[i], hex);
        }
        double tempoTotal = fimInMs - inicioInMs;
        printf("Tempo gasto: %0.10lf ms\n", tempoTotal);

    }
    printf("%s", dict->strings[4]);

    liberarDicionario(dict);
    return 0;
}
