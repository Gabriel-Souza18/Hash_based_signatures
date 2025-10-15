#include "utils.h"
#include "keys.h"
#include "sha256.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void main(){
    
    int opcao;
    printf("Digite uma Opção:\n 1-Gerar Mensagem\n2-Verificar Mensagem\n");
    scanf("%d", &opcao);
    srand(clock());
    while(getchar() != '\n');
   
    if (opcao == 1){
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
        
        escreverAssinatura("Assinatura.txt",assinatura);
        escreverMensagem("Mensagem.txt", mensagem);
        escreverPkeys("PublicKeys.txt", pKeys);


        free(sKeys);
        free(pKeys);
        free(masks);
        free(assinatura);


    }else{
        PublicKeys* pKeys = mallocPkeys();
        Masks* masks = mallocMasks();
        Assinatura* assinatura = mallocAssinatura();

        char mensagem[1001];

        lerAssinatura("Assinatura.txt",assinatura);
        lerMensagem("Mensagem.txt", mensagem);
        lerPkeys("PublicKeys.txt", pKeys);

        
        char msgHash[SHA256_HEX_SIZE];
        sha256_hex(mensagem,SHA256_HEX_SIZE, msgHash);
    
        int resultado = verificarMensagem(msgHash, assinatura, masks, pKeys);
        
        if (resultado == 1){
            printf("Sucesso\n");
        }else if (resultado == 0){
            printf("Falhou\n");
        }

        free(pKeys);
        free(masks);
        free(assinatura);

    }

}