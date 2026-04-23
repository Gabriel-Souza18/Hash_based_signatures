#include "keys.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "../SHA256/sha256.h"

int rementente();
int destinatario();

int main() {
    uint8_t op=0;
    do{
        printf("Escolha uma opcao:\n1-Rementente\n2-Destinatario\n0-Sair\n");
        if (scanf("%hhd", &op) != 1) {  
            fprintf(stderr, "Erro ao ler opção\n");
            while (getchar() != '\n');  
            continue;
        }
        while (getchar() != '\n'); 

        if(op == 1)rementente();
        else if(op == 2)destinatario();
        
    }while(op !=0);

    return 0;

}
int rementente(){
    sha256_reset_counter();
    char mensagem[1000];
    
    printf("Digite uma mensagem: ");
    if (fgets(mensagem, sizeof(mensagem), stdin) == NULL) {
        fprintf(stderr, "Erro ao ler mensagem\n");
        return 1;
    }

    
    // Remove a quebra de linha se existir
    size_t len = strlen(mensagem);
    if (len > 0 && mensagem[len - 1] == '\n') {
        mensagem[len - 1] = '\0';
        len--;
    }
    
    printf("\nMensagem lida: %s\n", mensagem);
    printf("Tamanho: %zu bytes\n", len);
    
    // Gera as chaves
    Keys keys;
    clock_t inicio_keygen = clock();
    gerarKeys(&keys);
    clock_t fim_keygen = clock();
    printf("Geracao de chaves: %lf s\n", (double)(fim_keygen - inicio_keygen) / CLOCKS_PER_SEC);
    
    // Cria e assina a mensagem
    Assinatura assinatura;
    clock_t inicio_sign = clock();
    assinarMensagem(mensagem, (int)len, &assinatura, keys.SKeys);
    clock_t fim_sign = clock();
    printf("Assinatura: %lf s\n", (double)(fim_sign - inicio_sign) / CLOCKS_PER_SEC);

    
    clock_t inicio_save = clock();
    salvarPkeys((unsigned char*) keys.PKeys);
    salvarAssinatura(&assinatura);
    salvarMensagem(mensagem);
    clock_t fim_save = clock();
    printf("Salvamento em arquivo: %lf s\n", (double)(fim_save - inicio_save) / CLOCKS_PER_SEC);
    
    printf("Total de hashes SHA256: %llu\n", sha256_get_counter());

    return 0;
}
int destinatario(){
    sha256_reset_counter();

    char mensagem[1000];
    clock_t inicio_load = clock();
    lerMensagem(mensagem);

    size_t len = strlen(mensagem);
    if (len > 0 && mensagem[len - 1] == '\n') {
        mensagem[len - 1] = '\0';
        len--;
    }

    Assinatura assinatura;
    lerAssinatura(&assinatura);
    unsigned char PKeys[HORS_T][HORS_N];
    lerPkeys((unsigned char*)PKeys);
    clock_t fim_load = clock();
    printf("Carregamento de arquivos: %lf s\n", (double)(fim_load - inicio_load) / CLOCKS_PER_SEC);

    clock_t inicio_verify = clock();
    int verificação = verificarAssinatura(mensagem,(int)len,&assinatura,PKeys);
    clock_t fim_verify = clock();
    printf("Verificacao: %lf s\n", (double)(fim_verify - inicio_verify) / CLOCKS_PER_SEC);
    
    verificação ? printf("SUCESSO\n") : printf("ERRO\n"); 
    
    printf("Total de hashes SHA256: %llu\n", sha256_get_counter());

    return 0;
}
