/**
 * BENCH_ESP — Benchmark puro de algoritmos de assinatura na ESP32
 *
 * Mede, sem MQTT ou interferência de rede:
 *   • Geração de chaves secretas  (GenSK)
 *   • Geração de chaves públicas  (GenPK)
 *   • Assinatura de mensagem      (Sign)
 *   • Verificação de assinatura   (Verify)
 *
 * Para cada operação registra:
 *   • Tempo de execução (µs)
 *   • Número de chamadas SHA-256
 *   • RAM alocada (bytes consumidos no heap)
 *
 * O ambiente (LOTS, WOTS+, HORS, HORST ou MSS) é selecionado em tempo de
 * compilação via as flags correspondentes definidas no platformio.ini.
 *
 * Saída via Serial a 115200 bps — conecte o monitor serial para coletar
 * os resultados e compará-los com os obtidos no PC.
 */

#include <Arduino.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ─── Pinos para Botão e LEDs ─────────────────────────────────────────────────
const int BUTTON_PIN = 34;
const int LED_RED_PIN = 12;
const int LED_GREEN_PIN = 14;
const int LED_RED_BRIGHTNESS = 32;   // PWM de 0 a 255
const int LED_GREEN_BRIGHTNESS = 64; // PWM de 0 a 255

// ─── Includes condicionais ao algoritmo selecionado ──────────────────────────
extern "C" {
#if defined(BENCH_LOTS)
    #include "keys.h"     // LOTS/keys.h
    #include "sha256.h"   // SHA256/sha256.h
#elif defined(BENCH_WOTS)
    #include "keys.h"     // WOTS/keys.h
    #include "prf.h"      // WOTS/prf.h
    #include "sha256.h"   // SHA256/sha256.h
#elif defined(BENCH_HORS)
    #include "keys.h"     // HORS/keys.h
    #include "sha256.h"   // SHA256/sha256.h
#elif defined(BENCH_HORST)
    #include "keys.h"     // HORST/keys.h
    #include "sha256.h"   // SHA256/sha256.h
#elif defined(BENCH_MSS)
    #include "Arvore.h"   // MSS/Arvore.h
    #include "sha256.h"   // SHA256/sha256.h
#elif defined(BENCH_SPHINCS)
    #include "sphincs.h"  // SPHINCS/sphincs.h
    #include "sha256.h"   // SHA256/sha256.h
#else
    #error "Selecione um algoritmo no platformio.ini"
#endif
}

// ─── Constantes ──────────────────────────────────────────────────────────────
#ifndef BENCH_RUNS
    #define BENCH_RUNS 10       // Número de repetições para cada operação
#endif

static const char *MENSAGEM_TESTE = "Benchmark ESP32 Hash-Based Signatures";

// ─── Estrutura de resultado de uma operação ──────────────────────────────────
struct BenchResult {
    uint32_t           tempo_us;    // Tempo de execução em microssegundos
    unsigned long long num_hashes;  // Chamadas SHA-256 contadas
    uint32_t           ram_bytes;   // Bytes consumidos no heap
};

// ─── Helpers de impressão ─────────────────────────────────────────────────────

static void printSeparador(char c = '-', int n = 60) {
    for (int i = 0; i < n; i++) Serial.print(c);
    Serial.println();
}

static void printResultado(const char *nome, const BenchResult &r) {
    Serial.print("  ");
    Serial.print(nome);
    Serial.print(": tempo=");
    Serial.print(r.tempo_us);
    Serial.print(" us  hashes=");
    // %llu não funciona em todos os cores Arduino; usamos impressão manual
    Serial.print((uint32_t)(r.num_hashes >> 32));   // parte alta (geralmente 0)
    Serial.print((uint32_t)(r.num_hashes & 0xFFFFFFFF));
    Serial.print("  ram=");
    Serial.print(r.ram_bytes);
    Serial.println(" B");
}

static void printMedia(const char *nome, BenchResult runs[], int n) {
    uint64_t soma_t = 0;
    uint64_t soma_h = 0;
    uint64_t soma_r = 0;
    for (int i = 0; i < n; i++) {
        soma_t += runs[i].tempo_us;
        soma_h += runs[i].num_hashes;
        soma_r += runs[i].ram_bytes;
    }
    Serial.print("  MEDIA ");
    Serial.print(nome);
    Serial.print(": tempo=");
    Serial.print((uint32_t)(soma_t / n));
    Serial.print(" us  hashes=");
    Serial.print((uint32_t)(soma_h / n));
    Serial.print("  ram=");
    Serial.print((uint32_t)(soma_r / n));
    Serial.println(" B");
}

// ─── Macro de medição ─────────────────────────────────────────────────────────
// Captura heap_antes, zera contador SHA, executa bloco, captura métricas.
#define MEDIR(resultado, bloco)                          \
    do {                                                 \
        uint32_t _h0 = ESP.getFreeHeap();                \
        sha256_reset_counter();                          \
        uint32_t _t0 = micros();                         \
        { bloco }                                        \
        (resultado).tempo_us  = micros() - _t0;         \
        (resultado).num_hashes = sha256_get_counter();   \
        uint32_t _h1 = ESP.getFreeHeap();                \
        (resultado).ram_bytes = (_h0 > _h1)              \
                                  ? (_h0 - _h1) : 0;    \
    } while (0)

// ═══════════════════════════════════════════════════════════════════════════════
//  BENCHMARK LOTS
// ═══════════════════════════════════════════════════════════════════════════════
#if defined(BENCH_LOTS)

static void bench_lots() {
    Serial.println();
    printSeparador('=');
    Serial.println("  BENCHMARK: LOTS");
    printSeparador('=');

    // Tamanhos de estrutura
    Serial.print("  sizeof(SecretKeys) = ");
    Serial.print(sizeof(SecretKeys));
    Serial.println(" B");
    Serial.print("  sizeof(PublicKeys)  = ");
    Serial.print(sizeof(PublicKeys));
    Serial.println(" B");
    // Assinatura LOTS: uint8_t[256][KEY_SIZE] = 256 * 32 = 8192 B
    Serial.print("  sizeof(assinatura)  = ");
    Serial.print(256 * 32);
    Serial.println(" B (uint8_t[256][32])");
    Serial.print("  Heap livre antes do benchmark: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" B");
    printSeparador();

    // Aloca estruturas fora do laço de medição para controlar o que é medido
    SecretKeys *sKeys = nullptr;
    PublicKeys *pKeys = nullptr;
    uint8_t (*assinatura)[32] = nullptr;
    uint8_t msgHash[32] = {0};

    // Pré-calcula o hash da mensagem de teste (não conta no benchmark)
    sha256_bytes(MENSAGEM_TESTE, strlen(MENSAGEM_TESTE), msgHash);

    // ── 1. GenSK ──────────────────────────────────────────────────────────────
    Serial.println("[ 1/4 ] Geracao de Chaves Secretas (GenSK)");
    BenchResult res_gensk[BENCH_RUNS];
    for (int r = 0; r < BENCH_RUNS; r++) {
        if (sKeys) { free(sKeys); sKeys = nullptr; }

        MEDIR(res_gensk[r], {
            sKeys = malloc_Skeys();
            generateSecretKeys(sKeys);
        });

        printResultado("run", res_gensk[r]);
        delay(50);
    }
    printMedia("GenSK", res_gensk, BENCH_RUNS);
    printSeparador();

    // ── 2. GenPK ──────────────────────────────────────────────────────────────
    Serial.println("[ 2/4 ] Geracao de Chaves Publicas (GenPK)");
    BenchResult res_genpk[BENCH_RUNS];
    for (int r = 0; r < BENCH_RUNS; r++) {
        if (pKeys) { free(pKeys); pKeys = nullptr; }

        MEDIR(res_genpk[r], {
            pKeys = malloc_Pkeys();
            generatePublicKeys(pKeys, sKeys);
        });

        printResultado("run", res_genpk[r]);
        delay(50);
    }
    printMedia("GenPK", res_genpk, BENCH_RUNS);
    printSeparador();

    // ── 3. Sign ───────────────────────────────────────────────────────────────
    Serial.println("[ 3/4 ] Assinatura (Sign)");
    BenchResult res_sign[BENCH_RUNS];
    for (int r = 0; r < BENCH_RUNS; r++) {
        if (assinatura) { free(assinatura); assinatura = nullptr; }

        MEDIR(res_sign[r], {
            assinatura = (uint8_t (*)[32])malloc(256 * 32);
            assinarMSG(msgHash, sKeys, assinatura);
        });

        printResultado("run", res_sign[r]);
        delay(50);
    }
    printMedia("Sign", res_sign, BENCH_RUNS);
    printSeparador();

    // ── 4. Verify ─────────────────────────────────────────────────────────────
    Serial.println("[ 4/4 ] Verificacao (Verify)");
    BenchResult res_verify[BENCH_RUNS];
    for (int r = 0; r < BENCH_RUNS; r++) {
        bool resultado = false;

        MEDIR(res_verify[r], {
            resultado = verificarMSG(msgHash, pKeys, assinatura);
        });

        printResultado("run", res_verify[r]);
        Serial.print("    resultado=");
        Serial.println(resultado ? "VALIDA" : "INVALIDA");
        delay(50);
    }
    printMedia("Verify", res_verify, BENCH_RUNS);
    printSeparador();

    // Libera
    if (assinatura) free(assinatura);
    if (pKeys)      free(pKeys);
    if (sKeys)      free(sKeys);

    printSeparador('=');
    Serial.print("  Heap livre apos benchmark: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" B");
    printSeparador('=');
}

#endif // BENCH_LOTS

// ═══════════════════════════════════════════════════════════════════════════════
//  BENCHMARK WOTS+
// ═══════════════════════════════════════════════════════════════════════════════
#if defined(BENCH_WOTS)

// Seeds globais usadas pelo WOTS+ (definidas em WOTS/keys.c)
extern unsigned char PK_seed[WOTS_N];
extern unsigned char SK_seed[WOTS_N];

static void inicializar_seeds_deterministico() {
    // Seeds fixas e determinísticas para benchmark reprodutível
    // (evita dependência de libsodium / hardware RNG na ESP32)
    for (int i = 0; i < WOTS_N; i++) {
        PK_seed[i] = (unsigned char)(0xAA ^ i);
        SK_seed[i] = (unsigned char)(0x55 ^ i);
    }
}

static void bench_wots() {
    Serial.println();
    printSeparador('=');
    Serial.println("  BENCHMARK: WOTS+");
    printSeparador('=');

    Serial.print("  sizeof(SecretKeys)  = ");
    Serial.print(sizeof(SecretKeys));
    Serial.println(" B");
    Serial.print("  sizeof(PublicKeys)  = ");
    Serial.print(sizeof(PublicKeys));
    Serial.println(" B");
    Serial.print("  sizeof(Assinatura)  = ");
    Serial.print(sizeof(Assinatura));
    Serial.println(" B");
    Serial.print("  L  = "); Serial.println(L);
    Serial.print("  L1 = "); Serial.println(L1);
    Serial.print("  L2 = "); Serial.println(L2);
    Serial.print("  W  = "); Serial.println(W);
    Serial.print("  N  = "); Serial.println(WOTS_N);
    Serial.print("  Heap livre antes do benchmark: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" B");
    printSeparador();

    // Inicializa seeds de forma determinística
    inicializar_seeds_deterministico();

    SecretKeys *sKeys    = nullptr;
    PublicKeys *pKeys    = nullptr;
    Assinatura *assinatura = nullptr;
    uint8_t msgHash[WOTS_N]  = {0};

    // Pré-calcula hash da mensagem de teste
    sha256_bytes(MENSAGEM_TESTE, strlen(MENSAGEM_TESTE), msgHash);

    // ── 1. GenSK ──────────────────────────────────────────────────────────────
    Serial.println("[ 1/4 ] Geracao de Chaves Secretas (GenSK)");
    BenchResult res_gensk[BENCH_RUNS];
    for (int r = 0; r < BENCH_RUNS; r++) {
        if (sKeys) { free(sKeys); sKeys = nullptr; }

        MEDIR(res_gensk[r], {
            sKeys = mallocSkeys();
            generateSKeys(sKeys);
        });

        printResultado("run", res_gensk[r]);
        delay(50);
    }
    printMedia("GenSK", res_gensk, BENCH_RUNS);
    printSeparador();

    // ── 2. GenPK ──────────────────────────────────────────────────────────────
    Serial.println("[ 2/4 ] Geracao de Chaves Publicas (GenPK)");
    BenchResult res_genpk[BENCH_RUNS];
    for (int r = 0; r < BENCH_RUNS; r++) {
        if (pKeys) { free(pKeys); pKeys = nullptr; }

        MEDIR(res_genpk[r], {
            pKeys = mallocPkeys();
            generatePKeys(pKeys, sKeys);
        });

        printResultado("run", res_genpk[r]);
        delay(50);
    }
    printMedia("GenPK", res_genpk, BENCH_RUNS);
    printSeparador();

    // ── 3. Sign ───────────────────────────────────────────────────────────────
    Serial.println("[ 3/4 ] Assinatura (Sign)");
    BenchResult res_sign[BENCH_RUNS];
    for (int r = 0; r < BENCH_RUNS; r++) {
        if (assinatura) { free(assinatura); assinatura = nullptr; }

        MEDIR(res_sign[r], {
            assinatura = mallocAssinatura();
            assinarMensagem(msgHash, assinatura, sKeys);
        });

        printResultado("run", res_sign[r]);
        delay(50);
    }
    printMedia("Sign", res_sign, BENCH_RUNS);
    printSeparador();

    // ── 4. Verify ─────────────────────────────────────────────────────────────
    Serial.println("[ 4/4 ] Verificacao (Verify)");
    BenchResult res_verify[BENCH_RUNS];
    for (int r = 0; r < BENCH_RUNS; r++) {
        int resultado = 0;

        MEDIR(res_verify[r], {
            resultado = verificarMensagem(msgHash, assinatura, pKeys);
        });

        printResultado("run", res_verify[r]);
        Serial.print("    resultado=");
        Serial.println(resultado ? "VALIDA" : "INVALIDA");
        delay(50);
    }
    printMedia("Verify", res_verify, BENCH_RUNS);
    printSeparador();

    // Libera
    if (assinatura) free(assinatura);
    if (pKeys)      free(pKeys);
    if (sKeys)      free(sKeys);

    printSeparador('=');
    Serial.print("  Heap livre apos benchmark: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" B");
    printSeparador('=');
}

#endif // BENCH_WOTS

// ═══════════════════════════════════════════════════════════════════════════════
//  BENCHMARK HORS
// ═══════════════════════════════════════════════════════════════════════════════
#if defined(BENCH_HORS)

static void bench_hors() {
    Serial.println();
    printSeparador('=');
    Serial.println("  BENCHMARK: HORS");
    printSeparador('=');

    Serial.print("  HORS_T = "); Serial.println(HORS_T);
    Serial.print("  HORS_K = "); Serial.println(HORS_K);
    Serial.print("  HORS_N = "); Serial.println(HORS_N);
    Serial.print("  sizeof(Keys)       = ");
    Serial.print(sizeof(Keys));
    Serial.println(" B");
    Serial.print("  sizeof(Assinatura) = ");
    Serial.print(sizeof(Assinatura));
    Serial.println(" B");
    Serial.print("  Heap livre antes do benchmark: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" B");
    printSeparador();

    Keys *keys = nullptr;
    Assinatura *assinatura = nullptr;

    // O algoritmo HORS gera as chaves secretas e públicas em uma única rotina.
    Serial.println("[ 1/3 ] Geracao de Chaves (GenKeys)");
    BenchResult res_genkeys[BENCH_RUNS];
    for (int r = 0; r < BENCH_RUNS; r++) {
        if (keys) { free(keys); keys = nullptr; }

        MEDIR(res_genkeys[r], {
            keys = (Keys *)malloc(sizeof(Keys));
            if (keys) gerarKeys(keys);
        });

        printResultado("run", res_genkeys[r]);
        delay(50);
    }
    printMedia("GenKeys", res_genkeys, BENCH_RUNS);
    printSeparador();

    Serial.println("[ 2/3 ] Assinatura (Sign)");
    BenchResult res_sign[BENCH_RUNS];
    for (int r = 0; r < BENCH_RUNS; r++) {
        if (assinatura) { free(assinatura); assinatura = nullptr; }

        MEDIR(res_sign[r], {
            assinatura = (Assinatura *)malloc(sizeof(Assinatura));
            if (assinatura) {
                assinarMensagem(MENSAGEM_TESTE, strlen(MENSAGEM_TESTE),
                                assinatura, keys->SKeys);
            }
        });

        printResultado("run", res_sign[r]);
        delay(50);
    }
    printMedia("Sign", res_sign, BENCH_RUNS);
    printSeparador();

    Serial.println("[ 3/3 ] Verificacao (Verify)");
    BenchResult res_verify[BENCH_RUNS];
    for (int r = 0; r < BENCH_RUNS; r++) {
        int resultado = 0;

        MEDIR(res_verify[r], {
            if (assinatura && keys) {
                resultado = verificarAssinatura(MENSAGEM_TESTE,
                                                strlen(MENSAGEM_TESTE),
                                                assinatura, keys->PKeys);
            }
        });

        printResultado("run", res_verify[r]);
        Serial.print("    resultado=");
        Serial.println(resultado ? "VALIDA" : "INVALIDA");
        delay(50);
    }
    printMedia("Verify", res_verify, BENCH_RUNS);
    printSeparador();

    if (assinatura) free(assinatura);
    if (keys) free(keys);

    printSeparador('=');
    Serial.print("  Heap livre apos benchmark: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" B");
    printSeparador('=');
}

#endif // BENCH_HORS

// ═══════════════════════════════════════════════════════════════════════════════
//  BENCHMARK HORST
// ═══════════════════════════════════════════════════════════════════════════════
#if defined(BENCH_HORST)

static void bench_horst() {
    Serial.println();
    printSeparador('=');
    Serial.println("  BENCHMARK: HORST");
    printSeparador('=');

    Serial.print("  HORST_T = "); Serial.println(HORST_T);
    Serial.print("  HORST_K = "); Serial.println(HORST_K);
    Serial.print("  HORST_H = "); Serial.println(HORST_H);
    Serial.print("  HORST_N = "); Serial.println(HORST_N);
    Serial.print("  sizeof(Keys)       = ");
    Serial.print(sizeof(Keys));
    Serial.println(" B");
    Serial.print("  sizeof(Assinatura) = ");
    Serial.print(sizeof(Assinatura));
    Serial.println(" B");
    Serial.print("  Heap livre antes do benchmark: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" B");
    printSeparador();

    Keys *keys = nullptr;
    Assinatura *assinatura = nullptr;

    Serial.println("[ 1/3 ] Geracao de Chaves (GenKeys)");
    BenchResult res_genkeys[BENCH_RUNS];
    for (int r = 0; r < BENCH_RUNS; r++) {
        if (keys) { free(keys); keys = nullptr; }

        MEDIR(res_genkeys[r], {
            keys = (Keys *)malloc(sizeof(Keys));
            if (keys) gerarKeys(keys);
        });

        printResultado("run", res_genkeys[r]);
        delay(50);
    }
    printMedia("GenKeys", res_genkeys, BENCH_RUNS);
    printSeparador();

    Serial.println("[ 2/3 ] Assinatura (Sign)");
    BenchResult res_sign[BENCH_RUNS];
    for (int r = 0; r < BENCH_RUNS; r++) {
        if (assinatura) { free(assinatura); assinatura = nullptr; }

        MEDIR(res_sign[r], {
            assinatura = (Assinatura *)malloc(sizeof(Assinatura));
            if (assinatura) {
                assinarMensagem(MENSAGEM_TESTE, strlen(MENSAGEM_TESTE),
                                assinatura, keys->SKeys);
            }
        });

        printResultado("run", res_sign[r]);
        delay(50);
    }
    printMedia("Sign", res_sign, BENCH_RUNS);
    printSeparador();

    Serial.println("[ 3/3 ] Verificacao (Verify)");
    BenchResult res_verify[BENCH_RUNS];
    for (int r = 0; r < BENCH_RUNS; r++) {
        int resultado = 0;

        MEDIR(res_verify[r], {
            if (assinatura && keys) {
                resultado = verificarAssinatura(MENSAGEM_TESTE,
                                                strlen(MENSAGEM_TESTE),
                                                assinatura, &keys->PKey);
            }
        });

        printResultado("run", res_verify[r]);
        Serial.print("    resultado=");
        Serial.println(resultado ? "VALIDA" : "INVALIDA");
        delay(50);
    }
    printMedia("Verify", res_verify, BENCH_RUNS);
    printSeparador();

    if (assinatura) free(assinatura);
    if (keys) free(keys);

    printSeparador('=');
    Serial.print("  Heap livre apos benchmark: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" B");
    printSeparador('=');
}

#endif // BENCH_HORST

// ═══════════════════════════════════════════════════════════════════════════════
//  BENCHMARK MSS
// ═══════════════════════════════════════════════════════════════════════════════
#if defined(BENCH_MSS)

#define MSS_BENCH_LEAVES 16

extern "C" No *mssBenchmarkTree(Folha *folhas);

static void liberar_contexto_mss(No *raiz, Folha *folhas) {
    if (raiz) limparArvore(raiz);
    if (folhas) {
        for (int i = 0; i < MSS_BENCH_LEAVES; i++) {
            free(folhas[i].Skeys);
            free(folhas[i].Pkeys);
        }
        free(folhas);
    }
}

static void bench_mss() {
    Serial.println();
    printSeparador('=');
    Serial.println("  BENCHMARK: MSS");
    printSeparador('=');

    Serial.print("  Folhas MSS = "); Serial.println(MSS_BENCH_LEAVES);
    Serial.print("  sizeof(Folha) = "); Serial.print(sizeof(Folha)); Serial.println(" B");
    Serial.print("  sizeof(AssinaturaMSS) = ");
    Serial.print(sizeof(AssinaturaMSS));
    Serial.println(" B");
    Serial.print("  Heap livre antes do benchmark: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" B");
    printSeparador();

    Folha *folhas = nullptr;
    No *raiz = nullptr;
    AssinaturaMSS *assinatura = nullptr;

    Serial.println("[ 1/3 ] Geracao de Chaves e Arvore (GenKeys)");
    BenchResult res_genkeys[BENCH_RUNS];
    for (int r = 0; r < BENCH_RUNS; r++) {
        liberar_contexto_mss(raiz, folhas);
        raiz = nullptr;
        folhas = nullptr;

        MEDIR(res_genkeys[r], {
            folhas = (Folha *)calloc(MSS_BENCH_LEAVES, sizeof(Folha));
            if (folhas) {
                for (int i = 0; i < MSS_BENCH_LEAVES; i++) {
                    folhas[i].Skeys = mallocSkeys();
                    folhas[i].Pkeys = mallocPkeys();
                }
                criarFolhas(folhas, MSS_BENCH_LEAVES);
                raiz = mssBenchmarkTree(folhas);
            }
        });

        printResultado("run", res_genkeys[r]);
        delay(50);
    }
    printMedia("GenKeys", res_genkeys, BENCH_RUNS);
    printSeparador();

    Serial.println("[ 2/3 ] Assinatura (Sign)");
    BenchResult res_sign[BENCH_RUNS];
    for (int r = 0; r < BENCH_RUNS; r++) {
        if (assinatura) {
            liberarAssinatura(assinatura);
            assinatura = nullptr;
        }
        if (folhas) folhas[0].usada = 0;

        MEDIR(res_sign[r], {
            assinatura = alocarAssinatura();
            if (assinatura && raiz && folhas) {
                criarAssinatura(assinatura, raiz, &folhas[0], 0,
                                MSS_BENCH_LEAVES,
                                (char *)MENSAGEM_TESTE);
            }
        });

        printResultado("run", res_sign[r]);
        delay(50);
    }
    printMedia("Sign", res_sign, BENCH_RUNS);
    printSeparador();

    Serial.println("[ 3/3 ] Verificacao (Verify)");
    BenchResult res_verify[BENCH_RUNS];
    for (int r = 0; r < BENCH_RUNS; r++) {
        int resultado = 0;

        MEDIR(res_verify[r], {
            if (assinatura && raiz) {
                resultado = verificarAssinatura(assinatura, raiz->hash, NULL);
            }
        });

        printResultado("run", res_verify[r]);
        Serial.print("    resultado=");
        Serial.println(resultado ? "VALIDA" : "INVALIDA");
        delay(50);
    }
    printMedia("Verify", res_verify, BENCH_RUNS);
    printSeparador();

    if (assinatura) liberarAssinatura(assinatura);
    liberar_contexto_mss(raiz, folhas);

    printSeparador('=');
    Serial.print("  Heap livre apos benchmark: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" B");
    printSeparador('=');
}

#endif // BENCH_MSS

// ═══════════════════════════════════════════════════════════════════════════════
//  BENCHMARK SPHINCS
// ═══════════════════════════════════════════════════════════════════════════════
#if defined(BENCH_SPHINCS)

static void bench_sphincs() {
    Serial.println();
    printSeparador('=');
    Serial.println("  BENCHMARK: SPHINCS");
    printSeparador('=');

    Serial.print("  SPHINCS_N = "); Serial.println(SPHINCS_N);
    Serial.print("  SPHINCS_D = "); Serial.println(SPHINCS_D);
    Serial.print("  SPHINCS_H = "); Serial.println(SPHINCS_H);
    Serial.print("  HORST_T   = "); Serial.println(HORST_T);
    Serial.print("  sizeof(SphincsSecretKey) = ");
    Serial.print(sizeof(SphincsSecretKey));
    Serial.println(" B");
    Serial.print("  sizeof(SphincsPublicKey) = ");
    Serial.print(sizeof(SphincsPublicKey));
    Serial.println(" B");
    Serial.print("  sizeof(SphincsSignature) = ");
    Serial.print(sizeof(SphincsSignature));
    Serial.println(" B");
    Serial.print("  Heap livre antes do benchmark: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" B");
    printSeparador();

    SphincsSecretKey *sk = (SphincsSecretKey *)malloc(sizeof(SphincsSecretKey));
    SphincsPublicKey *pk = (SphincsPublicKey *)malloc(sizeof(SphincsPublicKey));
    SphincsSignature *sig = (SphincsSignature *)malloc(sizeof(SphincsSignature));
    if (!sk || !pk || !sig) {
        Serial.println("[SPHINCS][ERRO] Falha ao alocar estruturas.");
        free(sk);
        free(pk);
        free(sig);
        return;
    }

    Serial.println("[ 1/3 ] Geracao de Chaves (Keygen)");
    BenchResult res_keygen[BENCH_RUNS];
    for (int r = 0; r < BENCH_RUNS; r++) {
        MEDIR(res_keygen[r], {
            sphincs_keygen(pk, sk);
        });

        printResultado("run", res_keygen[r]);
        delay(50);
    }
    printMedia("Keygen", res_keygen, BENCH_RUNS);
    printSeparador();

    Serial.println("[ 2/3 ] Assinatura (Sign)");
    BenchResult res_sign[BENCH_RUNS];
    for (int r = 0; r < BENCH_RUNS; r++) {
        MEDIR(res_sign[r], {
            sphincs_sign(sig,
                         (const unsigned char *)MENSAGEM_TESTE,
                         strlen(MENSAGEM_TESTE), sk);
        });

        printResultado("run", res_sign[r]);
        delay(50);
    }
    printMedia("Sign", res_sign, BENCH_RUNS);
    printSeparador();

    Serial.println("[ 3/3 ] Verificacao (Verify)");
    BenchResult res_verify[BENCH_RUNS];
    for (int r = 0; r < BENCH_RUNS; r++) {
        int resultado = 0;

        MEDIR(res_verify[r], {
            resultado = sphincs_verify(
                sig,
                (const unsigned char *)MENSAGEM_TESTE,
                strlen(MENSAGEM_TESTE),
                pk,
                sk->SK_seed);
        });

        printResultado("run", res_verify[r]);
        Serial.print("    resultado=");
        Serial.println(resultado ? "VALIDA" : "INVALIDA");
        delay(50);
    }
    printMedia("Verify", res_verify, BENCH_RUNS);
    printSeparador();

    free(sig);
    free(pk);
    free(sk);

    printSeparador('=');
    Serial.print("  Heap livre apos benchmark: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" B");
    printSeparador('=');
}

#endif // BENCH_SPHINCS

// ═══════════════════════════════════════════════════════════════════════════════
//  FUNÇÃO DE BENCHMARK
// ═══════════════════════════════════════════════════════════════════════════════
void runBenchmark() {
    Serial.println();
    printSeparador('*');
    Serial.println("*  BENCH_ESP — Benchmark de assinaturas hash na ESP32   *");
    printSeparador('*');
    Serial.print("  Chip:      "); Serial.println(ESP.getChipModel());
    Serial.print("  CPU freq:  "); Serial.print(ESP.getCpuFreqMHz()); Serial.println(" MHz");
    Serial.print("  Heap tot:  "); Serial.print(ESP.getHeapSize());   Serial.println(" B");
    Serial.print("  Heap free: "); Serial.print(ESP.getFreeHeap());   Serial.println(" B");
    Serial.print("  Runs:      "); Serial.println(BENCH_RUNS);

#if defined(BENCH_LOTS)
    Serial.println("  Algoritmo: LOTS");
    bench_lots();
#elif defined(BENCH_WOTS)
    Serial.println("  Algoritmo: WOTS+");
    bench_wots();
#elif defined(BENCH_HORS)
    Serial.println("  Algoritmo: HORS");
    bench_hors();
#elif defined(BENCH_HORST)
    Serial.println("  Algoritmo: HORST");
    bench_horst();
#elif defined(BENCH_MSS)
    Serial.println("  Algoritmo: MSS");
    bench_mss();
#elif defined(BENCH_SPHINCS)
    Serial.println("  Algoritmo: SPHINCS");
    bench_sphincs();
#endif

    Serial.println();
    Serial.println(">>> Benchmark concluido. Reinicie para repetir. <<<");
}

// ═══════════════════════════════════════════════════════════════════════════════
//  SETUP / LOOP
// ═══════════════════════════════════════════════════════════════════════════════

void setup() {
    // Desabilita detector de Brownout (evita reboot por oscilação de tensão USB)
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    Serial.begin(115200);
    delay(200); // Aguarda a inicializacao da Serial

    // GPIO34 é somente entrada e não possui pull-up/pull-down interno.
    // Use um resistor externo de 10 kΩ entre GPIO34 e GND.
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);

    analogWrite(LED_RED_PIN, LED_RED_BRIGHTNESS); // Aguardando o botao
    analogWrite(LED_GREEN_PIN, 0);

    Serial.println("Pressione o botão para iniciar o benchmark...");
}

void loop() {
    if (digitalRead(BUTTON_PIN) == HIGH) {
        // Botão pressionado, iniciar benchmark
        Serial.println("Botão pressionado! Iniciando benchmark...");
        analogWrite(LED_RED_PIN, 0);                  // Desliga LED vermelho
        analogWrite(LED_GREEN_PIN, LED_GREEN_BRIGHTNESS); // Benchmark rodando

        runBenchmark();

        analogWrite(LED_RED_PIN, LED_RED_BRIGHTNESS); // Concluido/ocioso
        analogWrite(LED_GREEN_PIN, 0);
        Serial.println("Benchmark concluído. Aguardando novo botão...");

        // Debounce e espera o botão ser solto
        while (digitalRead(BUTTON_PIN) == HIGH) {
            delay(10);
        }
        delay(50); // Pequeno atraso para evitar multiplos disparos
    }
    delay(20); // Pequeno atraso para evitar busy-waiting
}
