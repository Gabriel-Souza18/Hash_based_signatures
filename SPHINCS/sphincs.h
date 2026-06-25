#ifndef SPHINCS_HEADER_H
#define SPHINCS_HEADER_H

#include <stdint.h>
#include <stddef.h>
#include "xmss.h"
#include "../HORST/keys.h" /* Para Assinatura e PublicKey do HORST */

/* ── Parâmetros SPHINCS-256 (adaptado para HORST_T=1024) ───────────────── */
#ifndef SPHINCS_N
#define SPHINCS_N       32   /* tamanho hash em bytes */
#endif

#ifndef SPHINCS_D
#define SPHINCS_D       12   /* número de camadas na hiper-árvore */
#endif

#ifndef SPHINCS_H
#define SPHINCS_H       60   /* altura total da hiper-árvore */
#endif

typedef struct {
    unsigned char SK_seed[SPHINCS_N]; /* semente secreta global (SK1) */
    unsigned char SK_prf[SPHINCS_N];  /* semente para randomização e índice (SK2) */
    unsigned char PK_seed[SPHINCS_N]; /* semente pública global (PK_seed) */
} SphincsSecretKey;

typedef struct {
    unsigned char root[SPHINCS_N];    /* raiz da hiper-árvore (camada d-1) */
    unsigned char PK_seed[SPHINCS_N]; /* semente pública global */
} SphincsPublicKey;

typedef struct {
    uint64_t idx;                     /* índice da folha escolhida */
    unsigned char R[SPHINCS_N];       /* randomizador R1 */
    Assinatura horst_sig;             /* assinatura HORST da mensagem */
    XMSSSignature xmss_sigs[SPHINCS_D]; /* D assinaturas XMSS */
} SphincsSignature;

/* ── API pública ────────────────────────────────────────────────────────── */

/*
 * sphincs_keygen — gera chaves pública e secreta
 */
void sphincs_keygen(SphincsPublicKey *pk, SphincsSecretKey *sk);

/*
 * sphincs_sign — assina uma mensagem
 */
void sphincs_sign(SphincsSignature *sig,
                  const unsigned char *msg, size_t msg_len,
                  const SphincsSecretKey *sk);

/*
 * sphincs_verify — verifica uma assinatura. Retorna 1 se válida, 0 se inválida.
 */
int sphincs_verify(const SphincsSignature *sig,
                   const unsigned char *msg, size_t msg_len,
                   const SphincsPublicKey *pk,
                   const unsigned char SK_seed_val[SPHINCS_N]);

#endif /* SPHINCS_HEADER_H */
