typedef struct {
    char **strings;
    char **hashes;
    int count;
} DicionarioHash;


DicionarioHash* criarDicionario(int tamanho);
void liberarDicionario(DicionarioHash *dict);
DicionarioHash* carregarVetores(char *caminhoArquivo);
