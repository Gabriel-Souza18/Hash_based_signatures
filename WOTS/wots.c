#include "utils.h"
#include "keys.h"
#include "../SHA256/sha256.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// MUDAR void main() para int main()
int main(){
    
    int opcao;
    printf("Digite uma Opção:\n1-Gerar Mensagem\n2-Verificar Mensagem\n");
    // Adicionar verificação do scanf para evitar warning
    if (scanf("%d", &opcao) != 1) {
        printf("Erro na leitura da opção\n");
        return 1;
    }
    
    // Reseta contador global
    sha256_reset_counter();
    
    srand(clock());
    while(getchar() != '\n');
   
    if (opcao == 1){
        SecretKeys* sKeys = mallocSkeys();
        PublicKeys* pKeys = mallocPkeys();
        Assinatura* assinatura = mallocAssinatura();

        initializeSeeds();

        clock_t inicioSk= clock();
        generateSKeys(sKeys);
        clock_t fimSk = clock();

        clock_t inicioPk = clock();
        generatePKeys(pKeys, sKeys);
        clock_t fimPk = clock();

        char mensagem[1001];
        printf("Digite sua mensagem:\n");
        // Adicionar verificação do fgets
        if (fgets(mensagem, 1000, stdin) == NULL) {
            printf("Erro na leitura da mensagem\n");
            return 1;
        }
        
        mensagem[strcspn(mensagem, "\n")] = 0;

        char msgHash[SHA256_HEX_SIZE];
        sha256_hex(mensagem, strlen(mensagem), msgHash); 

        clock_t inicioAssinatura = clock();
        assinarMensagem(msgHash,assinatura, sKeys);
        clock_t fimAssinatura = clock();

        escreverAssinatura("Assinatura.txt",assinatura);
        escreverMensagem("Mensagem.txt", mensagem);
        escreverPkeys("PublicKeys.txt", pKeys, PK_seed, SK_seed);

        printf("Tempo para gerar Chaves Secretas: %lfs\n", 
            (double)(fimSk-inicioSk)/CLOCKS_PER_SEC);

        printf("Tempo para gerar Chaves Public: %lfs\n", 
            (double)(fimPk-inicioPk)/CLOCKS_PER_SEC);

        printf("Tempo para Assinar: %lfs\n", 
            (double)(fimAssinatura-inicioAssinatura)/CLOCKS_PER_SEC);
        
        printf("Total de hashes SHA256: %llu\n", sha256_get_counter());
    
        // Calcula o tamanho real da assinatura WOTS
        unsigned long tamanho_assinatura = L * N; // 67 * 32 = 2.144 bytes
        printf("Tamanho Assinatura: %lu bytes\n", tamanho_assinatura);

        printf("Tamanho Secretkeys: %ld bytes\n", sizeof(SecretKeys));
        printf("Tamanho Publickeys: %ld bytes\n", sizeof(PublicKeys));

        free(sKeys);
        free(pKeys);
        free(assinatura);
    } else if (opcao==2){
        PublicKeys* pKeys = mallocPkeys();
        Assinatura* assinatura = mallocAssinatura();

        char mensagem[1001];

        lerAssinatura("Assinatura.txt",assinatura);
        lerMensagem("Mensagem.txt", mensagem);
        lerPkeys("PublicKeys.txt", pKeys, PK_seed, SK_seed);

        
        char msgHash[SHA256_HEX_SIZE];
        sha256_hex(mensagem, strlen(mensagem), msgHash);  
        int resultado = verificarMensagem(msgHash, assinatura, pKeys);
        
        printf("Verificação: %s\n", resultado ? "VÁLIDA" : "INVÁLIDA");

        printf("Total de hashes SHA256: %llu\n", sha256_get_counter());

        free(pKeys);
        free(assinatura);

    } else if (opcao ==3){
        SecretKeys* sKeys = mallocSkeys();
        PublicKeys* pKeys = mallocPkeys();

        Assinatura* assinatura = mallocAssinatura();


        initializeSeeds();

        generateSKeys(sKeys);
        generatePKeys(pKeys, sKeys);

        char mensagem[1001];
        printf("Digite a Mensagem: \n");
        // Adicionar verificação do fgets
        if (fgets(mensagem, 1000, stdin) == NULL) {
            printf("Erro na leitura da mensagem\n");
            return 1;
        }

        char msgHash[SHA256_HEX_SIZE];
        sha256_hex(mensagem, SHA256_HEX_SIZE, msgHash);

        assinarMensagem(msgHash,assinatura, sKeys);
        
        int resultado = verificarMensagem(msgHash, assinatura, pKeys);
    
        if (resultado == 1){
            printf("Sucesso\n");
        } else if (resultado == 0){
            printf("Falhou\n");
        }
    }

    return 0;  // ADICIONAR return 0 no final do main
}