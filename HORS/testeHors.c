#include <stdio.h>
#include <stdlib.h>
#include <sodium.h>
#include <string.h>
#include <math.h>

#include "keys.h"

#define TESTES 21
#define LEN_MSG 32

// vou testar o hors com multiplas assinatras e testar o conflito de componentes
double calcularConflitosEsperados() {
    // E[C] = k × n×(n-1) / (2×t)
    // Onde:
    //   k = HORS_K (componentes por assinatura)
    //   n = TESTES (número de assinaturas)
    //   t = HORS_T (total de chaves)
    
    double numerador = (double)HORS_K * TESTES * (TESTES - 1);
    double conflitos_esperados = numerador / (2.0 * HORS_T);
    return conflitos_esperados;
}

int main(){

    char mensagens[TESTES][LEN_MSG];
    if(sodium_init() < 0){
        fprintf(stderr, "Erro: falha ao inicializar libsodium.\n");
        return 1;
    }

    for(int i=0; i<TESTES; i++){
        randombytes_buf((unsigned char*)mensagens[i], LEN_MSG);
        for(int j = 0; j < 8; j++) {
            printf("%02x", mensagens[i][j]);
        }
        printf("... (truncado)\n");
    }

    unsigned char componentes_usados [TESTES][HORS_K][HORS_N];

    Keys keys;
    gerarKeys(&keys);

    printf("Gerou keys\n");
    for(int i =0; i<TESTES;i++){

        Assinatura *assinatura = malloc(sizeof(Assinatura));

        assinarMensagem(mensagens[i], LEN_MSG, assinatura, keys.SKeys);

        int validacao = verificarAssinatura(mensagens[i], LEN_MSG, assinatura, keys.PKeys);
        validacao? printf("OK\n"): printf("ERRO\n");

        
        for (int j=0;j<HORS_K;j++){
            memcpy(componentes_usados[i][j], assinatura->assinatura[j], HORS_N );
        }

        free(assinatura);
        
    }
    printf("Assinou e Testou todas assinaturas\n");

    int conflitos_contados = 0;
    int comparacoes_totais = 0;
    
    //P(colisão) = 1 - e^(-(n×k)² / (2×t))
    float conflitos_esperados= calcularConflitosEsperados();

    printf("\n=== Comparando assinaturas ===\n");
    for (int i=0 ; i< TESTES;i++ ){
        for (int j=i+1; j< TESTES; j++){
            for (int k=0 ; k<HORS_K; k++){
                comparacoes_totais++;
                if( memcmp(componentes_usados[i][k], componentes_usados[j][k], HORS_N)==0 ){
                    conflitos_contados++;
                    // Debug: mostra os primeiros 5 conflitos
                    if (conflitos_contados <= 5) {
                        printf("Conflito %d: Assinatura[%d][%d] == Assinatura[%d][%d]\n", 
                               conflitos_contados, i, k, j, k);
                    }
                }
            }
        }
    }
    printf("\nTotal de comparações: %d\n", comparacoes_totais);
    printf("Conflitos contados = %d \n", conflitos_contados);
    printf("Esperado: %.2f\n", conflitos_esperados);
    printf("Taxa de conflito: %.2f%%\n", (100.0 * conflitos_contados) / comparacoes_totais);

    return 0;
}
