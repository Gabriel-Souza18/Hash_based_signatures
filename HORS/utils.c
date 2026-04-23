#include "utils.h"
#include "keys.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../SHA256/sha256.h"

void salvarMensagem(char* mensagem) {
    FILE *f = fopen("mensagem.txt", "w");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo mensagem.txt para escrita\n");
        return;
    }
    
    fprintf(f, "%s", mensagem);
    fclose(f);
}

// Lê a mensagem do arquivo de texto
void lerMensagem(char* mensagem) {
    FILE *f = fopen("mensagem.txt", "r");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo mensagem.txt para leitura\n");
        return;
    }
    
    if (fgets(mensagem, 256, f) == NULL) {
        fprintf(stderr, "Erro ao ler mensagem\n");
        fclose(f);
        return;
    }
    
    fclose(f);

}

void salvarPkeys(unsigned char *PKeys) {
    FILE *f = fopen("publicKeys.bin", "wb");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo publicKeys.bin para escrita\n");
        return;
    }
    
    // Salva todas as chaves públicas (HORS_T × HORS_N bytes)
    size_t escritos = fwrite(PKeys, HORS_N, HORS_T, f);
    if (escritos != HORS_T) {
        fprintf(stderr, "Erro ao escrever chaves públicas. Escritos: %zu/%d\n", escritos, HORS_T);
        fclose(f);
        return;
    }
    
    fclose(f);
}

// Lê as chaves públicas do arquivo binário
void lerPkeys(unsigned char *PKeys) {
    FILE *f = fopen("publicKeys.bin", "rb");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo publicKeys.bin para leitura\n");
        return;
    }
    
    // Lê todas as chaves públicas (HORS_T × HORS_N bytes)
    size_t lidos = fread(PKeys, HORS_N, HORS_T, f);
    if (lidos != HORS_T) {
        fprintf(stderr, "Erro ao ler chaves públicas. Lidos: %zu/%d\n", lidos, HORS_T);
        fclose(f);
        return;
    }
    
    fclose(f);
}

// Salva a assinatura em arquivo binário
void salvarAssinatura(Assinatura *assinatura) {
    FILE *f = fopen("assinatura.bin", "wb");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo assinatura.bin para escrita\n");
        return;
    }
    
    // Salva toda a assinatura (HORS_K × HORS_N bytes)
    size_t escritos = fwrite(assinatura->assinatura, HORS_N, HORS_K, f);
    if (escritos != HORS_K) {
        fprintf(stderr, "Erro ao escrever assinatura. Escritos: %zu/%d\n", escritos, HORS_K);
        fclose(f);
        return;
    }
    
    fclose(f);
}

// Lê a assinatura do arquivo binário
void lerAssinatura(Assinatura *assinatura) {
    FILE *f = fopen("assinatura.bin", "rb");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo assinatura.bin para leitura\n");
        return;
    }
    
    // Lê toda a assinatura (HORS_K × HORS_N bytes)
    size_t lidos = fread(assinatura->assinatura, HORS_N, HORS_K, f);
    if (lidos != HORS_K) {
        fprintf(stderr, "Erro ao ler assinatura. Lidos: %zu/%d\n", lidos, HORS_K);
        fclose(f);
        return;
    }
    
    fclose(f);

}

void iniciar_metricas(){
    sha256_reset_counter();
}
void printar_metricas(){
    printf("Total de hashes SHA256: %llu\n", sha256_get_counter());
}
