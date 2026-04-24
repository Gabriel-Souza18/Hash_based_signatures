#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sodium.h>
#include "keys.h"
#include "utils.h"
#include "../SHA256/sha256.h"

#define CAMINHO_SKEYS "seckeys.bin"
#define CAMINHO_PUBKEY "pubkey.bin"
#define CAMINHO_MENSAGEM "mensagem.txt"
#define CAMINHO_ASSINATURA "assinatura.bin"

int gerarChaves();
int assinarMsg();
int verificarMsg();
int menu();

int main() {
    if(sodium_init() < 0) {
        fprintf(stderr, "Erro ao inicializar libsodium\n");
        return 1;
    }

    int opcao;
    while(1) {
        opcao = menu();
        
        switch(opcao) {
            case 1:
                gerarChaves();
                break;
            case 2:
                assinarMsg();
                break;
            case 3:
                verificarMsg();
                break;
            case 0:
                printf("Saindo...\n");
                return 0;
            default:
                printf("Opção inválida!\n");
        }
    }

    return 0;
}

int menu() {
    int opcao;
    printf("\n____ HORST - Menu Principal ____\n");
    printf("1.Gerar novas chaves\n");
    printf("2.Assinar mensagem\n");
    printf("3.Verificar assinatura\n");
    printf("0.Sair\n");
    printf("Escolha: ");
    if(scanf("%d", &opcao) != 1) opcao = -1;
    getchar();  
    return opcao;
}

int gerarChaves() {
    printf("\n____ Gerando Novas Chaves ____\n");
    
    Keys keys;
    gerarKeys(&keys);
    
    // Salvar chaves secretas
    if (salvarSKeys(CAMINHO_SKEYS, &keys)) {
        printf("Chaves secretas salvas em '%s'\n", CAMINHO_SKEYS);
    } else {
        printf("Erro ao salvar chaves secretas\n");
        return 0;
    }
    
    // Salvar chave pública
    if (salvarPublicKey(CAMINHO_PUBKEY, &keys.PKey)) {
        printf("Chave pública salva em '%s'\n", CAMINHO_PUBKEY);
    } else {
        printf("Erro ao salvar chave pública\n");
        return 0;
    }
    
    printf("Chaves geradas com sucesso!\n");
    return 1;
}

int assinarMsg() {
    printf("\n____ Assinando Mensagem ____\n");
    
    // Carregar chaves secretas
    Keys keys;
    if (!carregarSKeys(CAMINHO_SKEYS, &keys)) {
        printf("Erro ao carregar chaves. Execute a opção 1 primeiro.\n");
        return 0;
    }
    
    // Ler mensagem
    printf("Digite a mensagem (máx 1024 caracteres): ");
    char mensagem[1024];
    if(!fgets(mensagem, sizeof(mensagem), stdin)) {
        printf("Erro ao ler mensagem.\n");
        return 0;
    }
    
    // Remover newline se existir
    size_t len = strlen(mensagem);
    if(len > 0 && mensagem[len-1] == '\n') {
        mensagem[len-1] = '\0';
        len--;
    }
    
    // Reconstruir árvore a partir das chaves secretas
    printf("Reconstruindo árvore de Merkle...\n");
    MerkleNode* raiz = construirArvore(keys.SKeys, 0, HORST_T - 1);
    
    // Assinar mensagem
    Assinatura assinatura;
    assinarMensagem(mensagem, len, &assinatura, keys.SKeys, raiz);
    
    // Salvar assinatura
    if (salvarAssinatura(CAMINHO_ASSINATURA, &assinatura)) {
        printf("Assinatura salva em '%s'\n", CAMINHO_ASSINATURA);
    } else {
        printf("Erro ao salvar assinatura\n");
        liberarArvore(raiz);
        return 0;
    }
    
    // Salvar mensagem
    FILE* f = fopen(CAMINHO_MENSAGEM, "w");
    if(f) {
        fprintf(f, "%s", mensagem);
        fclose(f);
        printf("Mensagem salva em '%s'\n", CAMINHO_MENSAGEM);
    }
    
    liberarArvore(raiz);
    printf("Mensagem assinada com sucesso!\n");
    return 1;
}

int verificarMsg() {
    printf("\n____ Verificando Assinatura ____\n");
    
    // Carregar chave pública
    PublicKey pk;
    if (!carregarPublicKey(CAMINHO_PUBKEY, &pk)) {
        printf("Erro ao carregar chave pública. Chaves não foram geradas.\n");
        return 0;
    }
    
    // Carregar assinatura
    Assinatura assinatura;
    if (!carregarAssinatura(CAMINHO_ASSINATURA, &assinatura)) {
        printf("Erro ao carregar assinatura. Nenhuma mensagem foi assinada ainda.\n");
        return 0;
    }
    
    // Ler mensagem
    FILE* f = fopen(CAMINHO_MENSAGEM, "r");
    if(!f) {
        printf("Erro ao abrir arquivo de mensagem.\n");
        return 0;
    }
    
    char mensagem[1024];
    if(!fgets(mensagem, sizeof(mensagem), f)) {
        printf("Erro ao ler mensagem.\n");
        fclose(f);
        return 0;
    }
    fclose(f);
    
    // Remover newline se existir
    size_t len = strlen(mensagem);
    if(len > 0 && mensagem[len-1] == '\n') {
        mensagem[len-1] = '\0';
        len--;
    }
    
    // Verificar assinatura
    int resultado = verificarAssinatura(mensagem, len, &assinatura, &pk);
    
    if(resultado) {
        printf("ASSINATURA VÁLIDA!\n");
        printf("Mensagem: %s\n", mensagem);
    } else {
        printf("ASSINATURA INVÁLIDA!\n");
    }
    
    return resultado;
}

