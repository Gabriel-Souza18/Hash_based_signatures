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

int salvarSKeys(const char* arquivo, const Keys* keys) {
    FILE* f = fopen(arquivo, "wb");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo %s para escrita\n", arquivo);
        return 0;
    }
    
    if (fwrite(keys->SKeys, sizeof(keys->SKeys), 1, f) != 1) {
        fprintf(stderr, "Erro ao escrever chaves secretas\n");
        fclose(f);
        return 0;
    }
    
    if (fwrite(keys->PKey.root, HORST_N, 1, f) != 1) {
        fprintf(stderr, "Erro ao escrever chave pública\n");
        fclose(f);
        return 0;
    }
    
    fclose(f);
    return 1;
}

int carregarSKeys(const char* arquivo, Keys* keys) {
    FILE* f = fopen(arquivo, "rb");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo %s para leitura\n", arquivo);
        return 0;
    }
    
    if (fread(keys->SKeys, sizeof(keys->SKeys), 1, f) != 1) {
        fprintf(stderr, "Erro ao ler chaves secretas\n");
        fclose(f);
        return 0;
    }
    
    if (fread(keys->PKey.root, HORST_N, 1, f) != 1) {
        fprintf(stderr, "Erro ao ler chave pública\n");
        fclose(f);
        return 0;
    }
    
    fclose(f);
    return 1;
}

int salvarPublicKey(const char* arquivo, const PublicKey* pk) {
    FILE* f = fopen(arquivo, "wb");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo %s para escrita\n", arquivo);
        return 0;
    }
    
    if (fwrite(pk->root, HORST_N, 1, f) != 1) {
        fprintf(stderr, "Erro ao escrever chave pública\n");
        fclose(f);
        return 0;
    }
    
    fclose(f);
    return 1;
}

int carregarPublicKey(const char* arquivo, PublicKey* pk) {
    FILE* f = fopen(arquivo, "rb");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo %s para leitura\n", arquivo);
        return 0;
    }
    
    if (fread(pk->root, HORST_N, 1, f) != 1) {
        fprintf(stderr, "Erro ao ler chave pública\n");
        fclose(f);
        return 0;
    }
    
    fclose(f);
    return 1;
}

int salvarAssinatura(const char* arquivo, const Assinatura* assinatura) {
    FILE* f = fopen(arquivo, "wb");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo %s para escrita\n", arquivo);
        return 0;
    }
    
    for (int i = 0; i < HORST_K; i++) {
        if (fwrite(assinatura->components[i].sk, HORST_N, 1, f) != 1) {
            fprintf(stderr, "Erro ao escrever segredo do componente %d\n", i);
            fclose(f);
            return 0;
        }
        
        if (fwrite(assinatura->components[i].auth_path.path, HORST_TAU * HORST_N, 1, f) != 1) {
            fprintf(stderr, "Erro ao escrever caminho do componente %d\n", i);
            fclose(f);
            return 0;
        }
    }
    
    fclose(f);
    return 1;
}

int carregarAssinatura(const char* arquivo, Assinatura* assinatura) {
    FILE* f = fopen(arquivo, "rb");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo %s para leitura\n", arquivo);
        return 0;
    }
    
    for (int i = 0; i < HORST_K; i++) {
        if (fread(assinatura->components[i].sk, HORST_N, 1, f) != 1) {
            fprintf(stderr, "Erro ao ler segredo do componente %d\n", i);
            fclose(f);
            return 0;
        }
        
        if (fread(assinatura->components[i].auth_path.path, HORST_TAU * HORST_N, 1, f) != 1) {
            fprintf(stderr, "Erro ao ler caminho do componente %d\n", i);
            fclose(f);
            return 0;
        }
    }
    
    fclose(f);
    return 1;
}
