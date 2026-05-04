#include <stdio.h>
#include <stdlib.h>
#include <sodium.h>
#include <string.h>
#include <math.h>

#include "keys.h"

#define M_PI 3.14159265358979323846
#define LEN_MSG 32

double calcularConflitosEsperados(int num_testes) {

    double numerador = (double)HORS_K * HORS_K * num_testes * (num_testes - 1);
    double conflitos_esperados = numerador / (2.0 * HORS_T);
    return conflitos_esperados;
}

int main(int argc, char *argv[]){
    int TESTES = 25;  // valor padrão

    if(argc > 1){
        TESTES = atoi(argv[1]);
        if(TESTES <= 0){
            fprintf(stderr, "Erro: número de testes deve ser positivo\n");
            fprintf(stderr, "Uso: %s [número_de_testes]\n", argv[0]);
            return 1;
        }
    }
    
    printf(" TESTANDO HORS COM %d ASSINATURAS\n", TESTES);
    if(sodium_init() < 0){
        fprintf(stderr, "Erro: falha ao inicializar libsodium.\n");
        return 1;
    }


    char (*mensagens)[LEN_MSG] = malloc(TESTES * LEN_MSG);
    int (*indices_usados)[HORS_K] = malloc(TESTES * HORS_K * sizeof(int));
    
    if(!mensagens || !indices_usados){
        fprintf(stderr, "Erro ao alocar memória\n");
        return 1;
    }

    for(int i=0; i<TESTES; i++){
        randombytes_buf((unsigned char*)mensagens[i], LEN_MSG);
    }

    Keys keys;
    gerarKeys(&keys);


    for(int i =0; i<TESTES;i++){

        Assinatura *assinatura = malloc(sizeof(Assinatura));

        assinarMensagem(mensagens[i], LEN_MSG, assinatura, keys.SKeys);

        int validacao = verificarAssinatura(mensagens[i], LEN_MSG, assinatura, keys.PKeys);
        if(!validacao) printf("ERRO na assinatura %d\n", i);

        // Extrair os índices usados
        unsigned char hash_msg[32];
        sha256_bytes((unsigned char*)mensagens[i], LEN_MSG, hash_msg);
        selecionarIndices(hash_msg, indices_usados[i]);

        free(assinatura);
        
    }
    printf("Assinou e testou todas assinaturas\n");

    int conflitos_contados = 0;
    int comparacoes_totais = 0;
    
    float conflitos_esperados = calcularConflitosEsperados(TESTES);

    for (int i=0 ; i< TESTES;i++ ){
        for (int j=i+1; j< TESTES; j++){
            for (int k=0 ; k<HORS_K; k++){
                for (int l=0 ; l<HORS_K; l++){
                    comparacoes_totais++;
                    if( indices_usados[i][k] == indices_usados[j][l] ){
                        conflitos_contados++;
                    }
                }
            }
        }
    }
    printf("\nTotal de comparacoes: %d\n", comparacoes_totais);
    printf("Conflitos contados = %d \n", conflitos_contados);
    printf("Esperado: %.2f\n", conflitos_esperados);
    printf("Taxa de conflito: %.2f%%\n", (100.0 * conflitos_contados) / comparacoes_totais);

    // Métrica de segurança: contar índices reutilizados
    int freq_indices[HORS_T] = {0};
    int indices_reutilizados = 0;
    int max_reutilizacao = 0;
    
    for(int i=0; i<TESTES; i++){
        for(int j=0; j<HORS_K; j++){
            int idx = indices_usados[i][j];
            freq_indices[idx]++;
            if(freq_indices[idx] > max_reutilizacao){
                max_reutilizacao = freq_indices[idx];
            }
        }
    }
    
    for(int i=0; i<HORS_T; i++){
        if(freq_indices[i] > 1){
            indices_reutilizados++;
        }
    }
    
    printf("\n ANÁLISE DE SEGURANÇA BASEADA NOS CONFLITOS OBSERVADOS \n");
    printf("Colisoes contadas: %d\n", conflitos_contados);
    printf("Colisoes esperadas (formula): %.2f\n", conflitos_esperados);
    printf("Taxa de colisao observada: %.2f%%\n", (100.0 * conflitos_contados) / comparacoes_totais);
    printf("Indice maximo reutilizado: %d vezes\n", max_reutilizacao);
    printf("Indices comprometidos (freq > 1): %d / %d (%.2f%%)\n", 
           indices_reutilizados, HORS_T, (100.0 * indices_reutilizados) / HORS_T);
    
    
    double sk_comprometidas_percent = (100.0 * indices_reutilizados) / HORS_T;
    double sk_disponivel_percent = 100.0 - sk_comprometidas_percent;
    
    printf("\n ANÁLISE DE SEGURANÇA DA SK \n");
    printf("SK disponivel/segura (nunca usada ou usada 1x): %.2f%%\n", sk_disponivel_percent);
    printf("SK comprometida (usada >1x): %.2f%%\n", sk_comprometidas_percent);
    printf("Número máximo de reutilizações de uma SK: %d vezes\n", max_reutilizacao);
    
    printf("\n STATUS DE SEGURANÇA \n");
    if(sk_comprometidas_percent <= 1.0){
        printf("Status: EXTREMAMENTE SEGURO (%.2f%% de SK comprometida)\n", sk_comprometidas_percent);
    } else if(sk_comprometidas_percent <= 5.0){
        printf("Status: MUITO SEGURO (%.2f%% de SK comprometida)\n", sk_comprometidas_percent);
    } else if(sk_comprometidas_percent <= 15.0){
        printf("Status: RAZOAVELMENTE SEGURO (%.2f%% de SK comprometida)\n", sk_comprometidas_percent);
    } else if(sk_comprometidas_percent <= 30.0){
        printf("Status: POUCO SEGURO (%.2f%% de SK comprometida)\n", sk_comprometidas_percent);
    } else {
        printf("Status: INSEGURO (%.2f%% de SK comprometida)\n", sk_comprometidas_percent);
    }

 
    free(mensagens);
    free(indices_usados);

    return 0;
}
