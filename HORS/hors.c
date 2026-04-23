#include "keys.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
    iniciar_metricas();
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
    gerarKeys(&keys);
    
    // Cria e assina a mensagem
    Assinatura assinatura;
    assinarMensagem(mensagem, (int)len, &assinatura, keys.SKeys);
    printf("Mensagem assinada!\n");

    
    salvarPkeys((unsigned char*) keys.PKeys);
    salvarAssinatura(&assinatura);
    salvarMensagem(mensagem);
    printar_metricas();

    return 0;
}
int destinatario(){
    iniciar_metricas();

    char mensagem[1000];
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

    int verificação = verificarAssinatura(mensagem,(int)len,&assinatura,PKeys);
    verificação ? printf("SUCESSO\n") : printf("ERRO\n"); 
    
    printar_metricas();

    return 0;
}
