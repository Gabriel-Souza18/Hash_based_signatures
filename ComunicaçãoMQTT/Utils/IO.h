#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// --- Funções de Leitura e Escrita de Mensagens (Texto) ---
void escreverMensagem(const char *caminho, const char *mensagem);
void escreverMensagemStream(FILE *stream, const char *mensagem);
char* lerMensagem(const char *caminho);
char* lerMensagemStream(FILE *stream);

// --- Funções de Leitura e Escrita de Assinaturas / Dados Brutos (Binário) ---
void escreverAssinatura(const char *caminho, const uint8_t *assinatura, size_t tamanho);
void escreverAssinaturaStream(FILE *stream, const uint8_t *assinatura, size_t tamanho);
uint8_t* lerAssinatura(const char *caminho, size_t *tamanho_lido);
uint8_t* lerAssinaturaStream(FILE *stream, size_t *tamanho_lido);

// --- Funções de Serialização Genérica (Empacotamento para Comunicação) ---
/**
 * @brief Empacota opcionalmente mensagem, chave pública e assinatura em um único payload binário.
 * 
 * O formato de serialização gerado é:
 * [Tamanho Msg (4B)] + [Msg] + [Tamanho PKey (4B)] + [PKey] + [Tamanho Assinatura (4B)] + [Assinatura]
 * Campos omitidos (ponteiros NULL ou tamanho 0) terão o tamanho gravado como 0 e não ocuparão espaço de dados.
 * 
 * @return uint8_t* Ponteiro para o buffer alocado dinamicamente. O chamador deve liberar com free().
 */
uint8_t* empacotarDados(const char *mensagem, const uint8_t *pkey, size_t tamanho_pkey, const uint8_t *assinatura, size_t tamanho_assinatura, size_t *tamanho_pacote);

/**
 * @brief Desempacota os dados estruturados de um payload binário de comunicação única.
 * 
 * Os ponteiros de retorno (*mensagem, *pkey, *assinatura) serão alocados dinamicamente.
 * Caso algum campo seja inexistente no pacote (tamanho 0), o respectivo retorno será definido como NULL.
 * O chamador deve liberar cada retorno não-nulo com free().
 */
bool desempacotarDados(const uint8_t *pacote, size_t tamanho_pacote, char **mensagem, uint8_t **pkey, size_t *tamanho_pkey, uint8_t **assinatura, size_t *tamanho_assinatura);

#endif // IO_H
