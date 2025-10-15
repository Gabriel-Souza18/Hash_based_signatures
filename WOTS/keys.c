#include "keys.h"
#include "../sha256.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


SecretKeys* mallocSkeys(){
    SecretKeys* k = (SecretKeys*)malloc(sizeof(SecretKeys));
    if (k == NULL) {
        fprintf(stderr, "Erro ao alocar memória para Secret keys\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i <L; i++) {
        for (int j=0 ;j<N; j++){
            k->Sk[i][j] = 0;  // Inicializa com 0, não NULL
        }
    }
    printf("Memoria Alocada com sucesso\n");
    return k;
}
PublicKeys* mallocPkeys(){
    PublicKeys* k = (PublicKeys*)malloc(sizeof(PublicKeys));
    if (k == NULL) {
        fprintf(stderr, "Erro ao alocar memória para Public keys\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i <L; i++) {
        for (int j=0 ;j<N; j++){
            k->PK[i][j] = 0;  // Inicializa com 0, não NULL
        }
    }
    printf("Memoria Alocada com sucesso\n");
    return k;
}
Masks* mallocMasks(){
    Masks* m = (Masks*)malloc(sizeof(Masks));
    if (m == NULL) {
        fprintf(stderr, "Erro ao alocar memória para Masks\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i <W-1; i++) {
        for (int j=0 ;j<N; j++){
            m->masks[i][j] = 0;  // Inicializa com 0, não NULL
        }
    }
    printf("Memoria Alocada com sucesso\n");
    return m;
}
Assinatura* mallocAssinatura() {
    Assinatura* a = (Assinatura*)malloc(sizeof(Assinatura));
    if (a == NULL) {
        fprintf(stderr, "Erro ao alocar memória para Assinatura\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < L; i++) {
        for (int j = 0; j < N; j++) {
            a->assinatura[i][j] = 0;
        }
    }
    printf("Memória para assinatura alocada com sucesso\n");
    return a;
}


void generateSKeys(SecretKeys* sKeys ){
        for(int i = 0; i < L; i++) {
            for(int j = 0; j < N; j++) {
                sKeys->Sk[i][j] = rand() % 256;
            }
        }
}

void generatePKeys(PublicKeys* pKeys, SecretKeys* sKeys, Masks* masks){
    printf("Gerando chaves públicas...\n");
    
    // Para cada uma das L chaves secretas
    for (int i = 0; i < L; i++) {
        // Aplica a função de cadeia W-1 vezes (15 vezes para W=16)
        // Isso transforma a chave secreta na chave pública correspondente
        chainFunction(sKeys->Sk[i], W-1, masks, pKeys->PK[i]);
        
        // Debug: mostra progresso a cada 10 chaves
        if ((i + 1) % 10 == 0) {
            printf("Geradas %d/%d chaves públicas\n", i + 1, L);
        }
    }
    
    printf("Todas as %d chaves públicas geradas com sucesso!\n", L);
    
    // Debug: mostra a primeira chave pública
    printf("PK[0]: ");
    for (int j = 0; j < 8; j++) {
        printf("%02x", (unsigned char)pKeys->PK[0][j]);
    }
    printf("...\n");
}
void generateMasks(Masks* masks){
    
    // Gera W-1 máscaras aleatórias
    for (int i = 0; i < W-1; i++) {
        for (int j = 0; j < N; j++) {
            masks->masks[i][j] = rand() % 256; 
        }
        
        //mostra as primeiras máscaras
        if (i < 3) {
            printf("Mask[%d]: ", i);
            for (int k = 0; k < 8; k++) {
                printf("%02x", (unsigned char)masks->masks[i][k]);
            }
            printf("...\n");
        }
    }
    printf("Geradas %d máscaras de %d bytes cada\n", W-1, N);
}

void chainFunction(char*src, int steps, Masks* masks, char* output){
    // Copia a entrada para o output inicialmente
    memcpy(output, src, N);
    
    // Aplica a função de cadeia 'steps' vezes
    for (int i = 0; i < steps; i++) {
        char temp[N];
        
       for (int j = 0; j < N; j++) {
            temp[j] = output[j] ^ masks->masks[i][j];
        }    
        // Aplica SHA256 na concatenação
        char hash_result[SHA256_HEX_SIZE];
        sha256_hex(temp, N * 2, hash_result);
        

        for (int j = 0; j < N && j * 2 < strlen(hash_result); j++) {
            char hex_byte[3] = {hash_result[j*2], hash_result[j*2+1], '\0'};
            output[j] = (char)strtol(hex_byte, NULL, 16);
        }
    }
}
void mensageForBlocks(char* msgHash, int* output) {
    // output é um array de L1 inteiros (64 valores de 0-15)
    
    for (int i = 0; i < L1; i++) {
        // Cada byte contém 2 blocos de 4 bits (nibbles)
        int byte_index = i / 2;           // 2 blocos por byte
        int is_high_nibble = (i % 2) == 0; // Primeiro ou segundo nibble do byte
        
        unsigned char byte = (unsigned char)msgHash[byte_index];
        
        if (is_high_nibble) {
            // Primeiro nibble (4 bits mais significativos)
            output[i] = (byte >> 4) & 0x0F;
        } else {
            // Segundo nibble (4 bits menos significativos)  
            output[i] = byte & 0x0F;
        }
        
        // Debug: mostra os primeiros blocos
        if (i < 5) {
            printf("Bloco[%d] = %d (byte[%d]=%02x)\n", i, output[i], byte_index, byte);
        }
    }
    printf("Convertidos %d bytes em %d blocos de 4 bits\n", N, L1);
}
void calcularChecksum(const int* message_blocks, int* checksum_blocks){
    int checksum = 0;
    
    // Calcula checksum: C = Σ(w-1 - m_i)
    for (int i = 0; i < L1; i++) {
        checksum += (W - 1 - message_blocks[i]);
    }
    
    printf("Checksum total: %d\n", checksum);
    
    // Converte checksum para base W (L2 dígitos)
    int temp = checksum;
    for (int i = 0; i < L2; i++) {
        checksum_blocks[i] = temp % W;
        temp = temp / W;
        
        printf("Checksum bloco[%d] = %d\n", i, checksum_blocks[i]);
    }
    
    // Verifica se não houve overflow
    if (temp > 0) {
        printf("ERRO: Checksum muito grande! Overflow detectado.\n");
    }
}

void assinarMensagem(char*msg,  Assinatura* assinatura,
                                SecretKeys* sKeys, 
                                Masks* masks){
 printf("\n=== INICIANDO ASSINATURA ===\n");
    
    int message_blocks[L1];
    int checksum_blocks[L2];
    int b[L];  // mensagem + checksum concatenados
    
    // 1. Converter mensagem para base W
    mensageForBlocks(msg, message_blocks);
    
    // 2. Calcular checksum
    calcularChecksum(message_blocks, checksum_blocks);
    
    // 3. Concatenar mensagem + checksum
    for (int i = 0; i < L1; i++) {
        b[i] = message_blocks[i];
    }
    for (int i = 0; i < L2; i++) {
        b[L1 + i] = checksum_blocks[i];
    }
    
    printf("Blocos para assinatura (%d total): ", L);
    for (int i = 0; i < L && i < 10; i++) {
        printf("%d ", b[i]);
    }
    if (L > 10) printf("...");
    printf("\n");
    
    // 4. Gerar assinatura aplicando chain function b[i] vezes
    for (int i = 0; i < L; i++) {
        chainFunction(sKeys->Sk[i], b[i], masks, assinatura->assinatura[i]);
        
        // Progresso
        if ((i + 1) % 10 == 0) {
            printf("Assinado %d/%d elementos\n", i + 1, L);
        }
    }
    
    printf("=== ASSINATURA COMPLETADA ===\n");
    
    // Debug: mostra parte da assinatura
    printf("Primeiro elemento da assinatura: ");
    for (int j = 0; j < 8; j++) {
        printf("%02x", (unsigned char)assinatura->assinatura[0][j]);
    }
    printf("...\n");
}

int verificarMensagem(char* msg, Assinatura* assinatura, Masks* masks, PublicKeys* pKeys) {
    printf("\n=== INICIANDO VERIFICAÇÃO ===\n");
    
    int message_blocks[L1];
    int checksum_blocks[L2];
    int b[L];
    
    // 1. Reconstruir b a partir da mensagem
    mensageForBlocks(msg, message_blocks);
    calcularChecksum(message_blocks, checksum_blocks);
    
    for (int i = 0; i < L1; i++) {
        b[i] = message_blocks[i];
    }
    for (int i = 0; i < L2; i++) {
        b[L1 + i] = checksum_blocks[i];
    }
    
    // 2. Para cada elemento, verificar se chain(signature, w-1-b[i]) == pKeys[i]
    for (int i = 0; i < L; i++) {
        char computed_pk[N];
        
        // Aplica chain function (w-1-b[i]) vezes na assinatura
        chainFunction(assinatura->assinatura[i], W-1 - b[i], masks, computed_pk);
        
        // Compara com a chave pública
        if (memcmp(computed_pk, pKeys->PK[i], N) != 0) {
            printf("ERRO: Verificação falhou no elemento %d\n", i);
            printf("Esperado: ");
            for (int j = 0; j < 8; j++) printf("%02x", (unsigned char)pKeys->PK[i][j]);
            printf("\nObtido:   ");
            for (int j = 0; j < 8; j++) printf("%02x", (unsigned char)computed_pk[j]);
            printf("\n");
            return 0; // Falha
        }
        
        if ((i + 1) % 10 == 0) {
            printf("Verificados %d/%d elementos\n", i + 1, L);
        }
    }
    
    printf("=== VERIFICAÇÃO BEM-SUCEDIDA ===\n");
    return 1; // Sucesso
}