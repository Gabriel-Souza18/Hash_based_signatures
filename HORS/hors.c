#include "keys.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    char mensagem[256];
    
    printf("Digite uma mensagem: ");
    if (fgets(mensagem, sizeof(mensagem), stdin) == NULL) {
        fprintf(stderr, "Erro ao ler mensagem\n");
        return EXIT_FAILURE;
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
    
    // Imprime a assinatura
    imprimirAssinatura(&assinatura);
    

    // TESTE VERIFICAÇÂO

    int verificação = verificarAssinatura(mensagem,(int)len,&assinatura,keys.PKeys);
    verificação ? printf("SUCESSO\n") : printf("ERRO\n"); //é pra passar nesse


    mensagem[0]= ' ';
    int verificação2 = verificarAssinatura(mensagem,(int)len,&assinatura,keys.PKeys);
    verificação2 ? printf("SUCESSO\n") : printf("ERRO\n"); //nao é pra passar nesse

    return EXIT_SUCCESS;
}
