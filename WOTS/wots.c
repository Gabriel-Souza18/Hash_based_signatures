#include "utils.h"
#include "keys.h"
#include "../SHA256/sha256.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


void main(){
    
    int opcao;
    printf("Digite uma Opção:\n1-Gerar Mensagem\n2-Verificar Mensagem\n");
    scanf("%d", &opcao);
    
    // Reseta contador global
    sha256_reset_counter();
    
    srand(clock());
    while(getchar() != '\n');
   
    if (opcao == 1){
        SecretKeys* sKeys = mallocSkeys();
        PublicKeys* pKeys = mallocPkeys();
        Masks* masks = mallocMasks();
        Assinatura* assinatura = mallocAssinatura();

        clock_t inicioSk= clock();
        generateSKeys(sKeys);
        clock_t fimSk = clock();
        clock_t inicioMasks = clock();
        generateMasks(masks);
        clock_t fimMasks = clock();
        clock_t inicioPk = clock();
        generatePKeys(pKeys, sKeys, masks);
        clock_t fimPk = clock();

        char mensagem[1001];
        printf("Digite sua mensagem:\n");
        fgets (mensagem, 1000, stdin);
        

        mensagem[strcspn(mensagem, "\n")] = 0;

        char msgHash[SHA256_HEX_SIZE];
        sha256_hex(mensagem, strlen(mensagem), msgHash); 

        clock_t inicioAssinatura = clock();
        assinarMensagem(msgHash,assinatura, sKeys, masks);
        clock_t fimAssinatura = clock();

        escreverAssinatura("Assinatura.txt",assinatura);
        escreverMensagem("Mensagem.txt", mensagem);
        escreverPkeys("PublicKeys.txt", pKeys);
        escreverMasks("Masks.txt", masks);  



        printf("Tempo para gerar Chaves Secretas: %lfs\n", 
            (double)(fimSk-inicioSk)/CLOCKS_PER_SEC);

        printf("Tempo para gerar Chaves Public: %lfs\n", 
            (double)(fimPk-inicioPk)/CLOCKS_PER_SEC);

        printf("Tempo para gerar Masks: %lfs\n", 
            (double)(fimMasks-inicioMasks)/CLOCKS_PER_SEC);

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
        free(masks);
        free(assinatura);
        }else if (opcao==2){
        PublicKeys* pKeys = mallocPkeys();
        Masks* masks = mallocMasks();
        Assinatura* assinatura = mallocAssinatura();

        char mensagem[1001];

        lerAssinatura("Assinatura.txt",assinatura);
        lerMensagem("Mensagem.txt", mensagem);
        lerPkeys("PublicKeys.txt", pKeys);
        lerMasks("Masks.txt", masks);

        
        char msgHash[SHA256_HEX_SIZE];
        sha256_hex(mensagem, strlen(mensagem), msgHash);  
        int resultado = verificarMensagem(msgHash, assinatura, masks, pKeys);
        
        printf("Verificação: %s\n", resultado ? "VÁLIDA" : "INVÁLIDA");


        printf("Total de hashes SHA256: %llu\n", sha256_get_counter());

        free(pKeys);
        free(masks);
        free(assinatura);

    }else if (opcao ==3){
         SecretKeys* sKeys = mallocSkeys();
        PublicKeys* pKeys = mallocPkeys();
        Masks* masks = mallocMasks();
        Assinatura* assinatura = mallocAssinatura();


        generateSKeys(sKeys);
        generateMasks(masks);
        generatePKeys(pKeys, sKeys, masks);

        char mensagem[1001];
        printf("Digite a Mensagem: \n");
        fgets (mensagem, 1000, stdin);

        char msgHash[SHA256_HEX_SIZE];
        sha256_hex(mensagem,SHA256_HEX_SIZE, msgHash);

        assinarMensagem(msgHash,assinatura, sKeys, masks);
        

        int resultado =verificarMensagem(msgHash, assinatura, masks, pKeys);
    
                
        if (resultado == 1){
            printf("Sucesso\n");
        }else if (resultado == 0){
            printf("Falhou\n");
        }

    }

}