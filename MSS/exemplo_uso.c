/*
 * EXEMPLO DE USO DA ÁRVORE MSS COM FOLHAS E NÓS INTERNOS
 * 
 * Estrutura da árvore (256 folhas):
 * 
 * Camada 0 (raiz):           [Raiz]
 *                           /      \
 * Camada 1:            [No1]        [No2]
 *                     /    \        /    \
 * Camada 2 (folhas): F0    F1      F2    F3  ... (256 folhas no total)
 * 
 * - Camadas contam de cima para baixo (0 = raiz)
 * - Nós da camada 1 têm filhos do tipo FOLHA
 * - Nós acima da camada 1 têm filhos do tipo NO
 */

#include "Arvore.h"
#include <stdio.h>
#include <stdlib.h>

// Exemplo com 8 folhas (mais simples para demonstrar)
#define NUM_FOLHAS 8

int main() {
    printf("=== EXEMPLO: Árvore MSS com %d folhas ===\n\n", NUM_FOLHAS);
    
    // 1. Criar array de folhas
    printf("1. Criando %d folhas...\n", NUM_FOLHAS);
    Folha* folhas = (Folha*)malloc(NUM_FOLHAS * sizeof(Folha));
    
    for (int i = 0; i < NUM_FOLHAS; i++) {
        folhas[i].Skeys = mallocSkeys();
        folhas[i].Pkeys = mallocPkeys();
        folhas[i].Masks = mallocMasks();
    }
    
    // Gera as chaves WOTS para todas as folhas
    criarFolhas(folhas, NUM_FOLHAS);
    printf("   ✓ Folhas criadas com chaves WOTS\n\n");
    
    // 2. Criar nós da camada 1 (pais diretos das folhas)
    printf("2. Criando camada 1 (pais das folhas)...\n");
    int num_nos_camada1 = NUM_FOLHAS / 2;  // 4 nós
    No** camada1 = (No**)malloc(num_nos_camada1 * sizeof(No*));
    
    for (int i = 0; i < num_nos_camada1; i++) {
        camada1[i] = alocarNo();
        camada1[i]->nivel = 1;
        
        // Conecta duas folhas a este nó
        Folha* folha_esq = &folhas[i * 2];
        Folha* folha_dir = &folhas[i * 2 + 1];
        
        conectarFolhasAoNo(camada1[i], folha_esq, folha_dir);
        
        printf("   Nó[%d] conectado às Folhas[%d] e [%d]\n", 
               i, i*2, i*2+1);
    }
    printf("   ✓ Camada 1 criada: %d nós\n\n", num_nos_camada1);
    
    // 3. Criar nós da camada 0 (pais dos nós da camada 1)
    printf("3. Criando camada 0 (raiz)...\n");
    int num_nos_camada0 = num_nos_camada1 / 2;  // 2 nós
    No** camada0 = (No**)malloc(num_nos_camada0 * sizeof(No*));
    
    for (int i = 0; i < num_nos_camada0; i++) {
        camada0[i] = alocarNo();
        camada0[i]->nivel = 0;
        
        // Conecta dois nós da camada 1
        camada0[i]->filho_esq = (void*)camada1[i * 2];
        camada0[i]->filho_dir = (void*)camada1[i * 2 + 1];
        camada0[i]->tipo_filho_esq = TIPO_NO;
        camada0[i]->tipo_filho_dir = TIPO_NO;
        
        // Calcula hash do nó
        criarPai(camada0[i]);
        
        printf("   Nó[%d] conectado aos Nós camada1[%d] e [%d]\n", 
               i, i*2, i*2+1);
    }
    printf("   ✓ Camada 0 criada: %d nós\n\n", num_nos_camada0);
    
    // 4. Criar raiz (conecta os nós da camada 0)
    printf("4. Criando raiz...\n");
    No* raiz = alocarNo();
    raiz->nivel = 0;
    raiz->filho_esq = (void*)camada0[0];
    raiz->filho_dir = (void*)camada0[1];
    raiz->tipo_filho_esq = TIPO_NO;
    raiz->tipo_filho_dir = TIPO_NO;
    criarPai(raiz);
    printf("   ✓ Raiz criada\n\n");
    
    // 5. Imprimir árvore
    printf("5. Visualizando árvore:\n");
    printf("   [F] = Folha\n");
    printf("   Sem [F] = Nó interno\n\n");
    printarArvoreCompleta(raiz);
    
    // 6. Exemplo: acessar uma folha específica
    printf("\n6. Exemplo de acesso:\n");
    printf("   Acessando folha 3...\n");
    
    // Folha 3 está no nó camada1[1] (filho direito)
    No* no_pai = camada1[1];
    if (no_pai->tipo_filho_dir == TIPO_FOLHA) {
        Folha* folha3 = (Folha*)no_pai->filho_dir;
        printf("   Hash da folha 3: %.16s...\n", folha3->hash);
        printf("   ✓ Folha acessada com sucesso!\n");
    }
    
    // 7. Limpeza
    printf("\n7. Limpando memória...\n");
    limparArvore(raiz);
    free(camada1);
    free(camada0);
    free(folhas);
    printf("   ✓ Memória liberada\n");
    
    printf("\n=== EXEMPLO CONCLUÍDO ===\n");
    return 0;
}
