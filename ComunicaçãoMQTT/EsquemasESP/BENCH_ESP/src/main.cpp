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
 * O ambiente (LOTS ou WOTS+) é selecionado em tempo de compilação via
 * as flags -D BENCH_LOTS / -D BENCH_WOTS definidas no platformio.ini.
 *
 * Saída via Serial a 115200 bps — conecte o monitor serial para coletar
 * os resultados e compará-los com os obtidos no PC.
 */

#include <Arduino.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ─── Includes condicionais ao algoritmo selecionado ──────────────────────────
extern "C" {
#if defined(BENCH_LOTS)
    #include "keys.h"     // LOTS/keys.h
    #include "sha256.h"   // SHA256/sha256.h
#elif defined(BENCH_WOTS)
    #include "keys.h"     // WOTS/keys.h
    #include "prf.h"      // WOTS/prf.h
    #include "sha256.h"   // SHA256/sha256.h
#else
    #error "Selecione um algoritmo: -D BENCH_LOTS ou -D BENCH_WOTS no platformio.ini"
#endif
}

// ─── Constantes ──────────────────────────────────────────────────────────────
#ifndef BENCH_RUNS
    #define BENCH_RUNS 5        // Número de repetições para cada operação
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
extern unsigned char PK_seed[N];
extern unsigned char SK_seed[N];

static void inicializar_seeds_deterministico() {
    // Seeds fixas e determinísticas para benchmark reprodutível
    // (evita dependência de libsodium / hardware RNG na ESP32)
    for (int i = 0; i < N; i++) {
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
    Serial.print("  N  = "); Serial.println(N);
    Serial.print("  Heap livre antes do benchmark: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" B");
    printSeparador();

    // Inicializa seeds de forma determinística
    inicializar_seeds_deterministico();

    SecretKeys *sKeys    = nullptr;
    PublicKeys *pKeys    = nullptr;
    Assinatura *assinatura = nullptr;
    uint8_t msgHash[N]  = {0};

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
//  SETUP / LOOP
// ═══════════════════════════════════════════════════════════════════════════════

void setup() {
    // Desabilita detector de Brownout (evita reboot por oscilação de tensão USB)
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    Serial.begin(115200);
    delay(2000); // Aguarda o monitor serial conectar

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
#endif

    Serial.println();
    Serial.println(">>> Benchmark concluido. Reinicie para repetir. <<<");
}

void loop() {
    // Nada a fazer — o benchmark roda uma única vez no setup()
    // Pressione RESET na placa para repetir.
    delay(10000);
}
