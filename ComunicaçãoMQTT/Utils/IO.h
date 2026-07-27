#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <stdint.h>

/**
 * @brief Escreve uma mensagem (texto) em um arquivo.
 * 
 * @param caminho Caminho para o arquivo de destino.
 * @param mensagem Ponteiro para a string com a mensagem.
 */
void escreverMensagem(const char *caminho, const char *mensagem);

/**
 * @brief Escreve uma mensagem em uma stream de saída (ex: stdout, arquivo já aberto).
 * 
 * @param stream Stream de destino (FILE *).
 * @param mensagem Ponteiro para a string com a mensagem.
 */
void escreverMensagemStream(FILE *stream, const char *mensagem);

/**
 * @brief Lê uma mensagem de um arquivo, alocando memória dinamicamente.
 * 
 * O array de retorno terá o tamanho exato da mensagem lida (mais o caractere '\0').
 * Se a mensagem possuir uma quebra de linha ao final, ela será removida.
 * 
 * @param caminho Caminho para o arquivo a ser lido.
 * @return char* Ponteiro para a mensagem alocada dinamicamente, ou NULL em caso de erro.
 *               O chamador é responsável por liberar a memória com free().
 */
char* lerMensagem(const char *caminho);

/**
 * @brief Lê uma mensagem de uma stream de entrada (ex: stdin) até encontrar '\n' ou EOF.
 * 
 * A alocação é dinâmica e utiliza realloc progressivamente para redimensionar 
 * o buffer e um realloc final para ajustar ao tamanho exato.
 * 
 * @param stream Stream de entrada (FILE *).
 * @return char* Ponteiro para a mensagem alocada dinamicamente, ou NULL se nenhum
 *               caractere for lido antes de EOF ou erro.
 *               O chamador é responsável por liberar a memória com free().
 */
char* lerMensagemStream(FILE *stream);

/**
 * @brief Escreve uma assinatura (dados binários) em um arquivo.
 * 
 * @param caminho Caminho para o arquivo de destino.
 * @param assinatura Ponteiro para o buffer contendo a assinatura.
 * @param tamanho Tamanho da assinatura em bytes.
 */
void escreverAssinatura(const char *caminho, const uint8_t *assinatura, size_t tamanho);

/**
 * @brief Escreve uma assinatura em uma stream de saída.
 * 
 * @param stream Stream de destino (FILE *).
 * @param assinatura Ponteiro para o buffer contendo a assinatura.
 * @param tamanho Tamanho da assinatura em bytes.
 */
void escreverAssinaturaStream(FILE *stream, const uint8_t *assinatura, size_t tamanho);

/**
 * @brief Lê uma assinatura (dados binários) de um arquivo.
 * 
 * O buffer retornado terá o tamanho exato da assinatura lida do arquivo.
 * 
 * @param caminho Caminho para o arquivo a ser lido.
 * @param tamanho_lido Ponteiro para retornar o número de bytes lidos.
 * @return uint8_t* Ponteiro para o buffer alocado dinamicamente contendo os bytes lidos,
 *                  ou NULL em caso de erro. O chamador deve liberar a memória com free().
 */
uint8_t* lerAssinatura(const char *caminho, size_t *tamanho_lido);

/**
 * @brief Lê uma assinatura de uma stream até encontrar EOF.
 * 
 * Aloca memória dinamicamente, dobrando a capacidade sempre que necessário,
 * e executa um realloc final para ajustar ao tamanho exato dos bytes lidos.
 * 
 * @param stream Stream de entrada (FILE *).
 * @param tamanho_lido Ponteiro para retornar o número de bytes lidos.
 * @return uint8_t* Ponteiro para o buffer alocado dinamicamente, ou NULL se nada for lido.
 *                  O chamador deve liberar a memória com free().
 */
uint8_t* lerAssinaturaStream(FILE *stream, size_t *tamanho_lido);

#endif // IO_H
