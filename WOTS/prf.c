#include "prf.h"
#include "sha256.h"
#include <string.h>

void compressADRS(const unsigned char* ADRS_32bytes, unsigned char* ADRS_22bytes) {
    /*
     * Converte ADRS de 32 bytes para 22 bytes conforme Figura 18 do FIPS 205
     * ADRSᶜ = ADRS[3] || ADRS[8:16] || ADRS[19] || ADRS[20:32]
     */
    
    // Layer address: 1 byte (último byte dos 4 originais)
    ADRS_22bytes[0] = ADRS_32bytes[3];
    
    // Tree address: 8 bytes (bytes 8-15 dos 12 originais)
    memcpy(ADRS_22bytes + 1, ADRS_32bytes + 8, 8);
    
    // Type: 1 byte (último byte dos 4 originais)
    ADRS_22bytes[9] = ADRS_32bytes[19];
    
    // Restante: 12 bytes (bytes 20-31)
    memcpy(ADRS_22bytes + 10, ADRS_32bytes + 20, 12);
}

void PRF_SHA2(unsigned char* output, const unsigned char* PK_seed, 
              const unsigned char* SK_seed, const unsigned char* ADRS_32bytes, int n) {
    /*
     * PRF para parâmetros SHA2 conforme FIPS 205 Seção 11.2.1
     * PRF(PK.seed, SK.seed, ADRS) = 
     *   Trunc_n(SHA-256(PK.seed || toByte(0,64-n) || ADRSᶜ || SK.seed))
     * 
     * Para n=32 (nosso caso): 64-n = 32 bytes de zeros
     */
    
    unsigned char input[32 + 32 + 22 + 32]; // PK.seed(32) + zeros(32) + ADRSᶜ(22) + SK.seed(32)
    int pos = 0;
    
    // 1. PK.seed (n bytes = 32)
    memcpy(input + pos, PK_seed, n);
    pos += n;
    
    // 2. Zeros (64 - n = 32 bytes)
    memset(input + pos, 0, 64 - n);
    pos += (64 - n);
    
    // 3. ADRS comprimido (22 bytes)
    unsigned char ADRS_compressed[22];
    compressADRS(ADRS_32bytes, ADRS_compressed);
    memcpy(input + pos, ADRS_compressed, 22);
    pos += 22;
    
    // 4. SK.seed (n bytes = 32)
    memcpy(input + pos, SK_seed, n);
    pos += n;
    
    // Calcular SHA-256
    unsigned char hash[SHA256_BYTES_SIZE];
    sha256_bytes(input, pos, hash);
    
    // Truncar para n bytes (no nosso caso n=32, então não precisa truncar)
    memcpy(output, hash, n);
}

void setADRS_WOTS_PRF(unsigned char* ADRS, int key_index) {
    /*
     * Configura ADRS para WOTS_PRF (type = 5)
     * Conforme Figura 8 do FIPS 205
     */
    memset(ADRS, 0, 32);
    
    // Layer address (4 bytes) - assumindo 0 para WOTS
    ADRS[3] = 0; // Último byte dos 4
    
    // Tree address (12 bytes) - assumindo 0
    // Já está zerado pelo memset
    
    // Type (4 bytes) = 5 (WOTS_PRF)
    ADRS[19] = 5; // Último byte dos 4
    
    // Key pair address (4 bytes)
    ADRS[20] = (key_index >> 24) & 0xFF;
    ADRS[21] = (key_index >> 16) & 0xFF;
    ADRS[22] = (key_index >> 8) & 0xFF;
    ADRS[23] = key_index & 0xFF;
    
    // Chain address (4 bytes) - será configurado depois
    // Hash address (4 bytes) = 0 (para PRF)
}

void setADRS_WOTS_HASH(unsigned char* ADRS, int key_index, int chain_index, int hash_index) {
    /*
     * Configura ADRS para WOTS_HASH (type = 0)
     * Conforme Figura 3 do FIPS 205
     */
    memset(ADRS, 0, 32);
    
    // Layer address (4 bytes) - assumindo 0
    ADRS[3] = 0;
    
    // Tree address (12 bytes) - assumindo 0
    
    // Type (4 bytes) = 0 (WOTS_HASH)
    ADRS[19] = 0;
    
    // Key pair address (4 bytes)
    ADRS[20] = (key_index >> 24) & 0xFF;
    ADRS[21] = (key_index >> 16) & 0xFF;
    ADRS[22] = (key_index >> 8) & 0xFF;
    ADRS[23] = key_index & 0xFF;
    
    // Chain address (4 bytes)
    ADRS[24] = (chain_index >> 24) & 0xFF;
    ADRS[25] = (chain_index >> 16) & 0xFF;
    ADRS[26] = (chain_index >> 8) & 0xFF;
    ADRS[27] = chain_index & 0xFF;
    
    // Hash address (4 bytes)
    ADRS[28] = (hash_index >> 24) & 0xFF;
    ADRS[29] = (hash_index >> 16) & 0xFF;
    ADRS[30] = (hash_index >> 8) & 0xFF;
    ADRS[31] = hash_index & 0xFF;
}
void F_function(unsigned char* output, const unsigned char* PK_seed, 
                const unsigned char* ADRS_32bytes, const unsigned char* input) {
    /*
     * Implementação da função F para SHA2
     * F(PK.seed, ADRS, M1) = Trunc_n(SHA-256(PK.seed || toByte(0,64-n) || ADRSᶜ || M1))
     */
    
    unsigned char buffer[32 + 32 + 22 + 32]; // PK.seed + zeros + ADRSᶜ + input
    int pos = 0;
    
    // PK.seed
    memcpy(buffer + pos, PK_seed, N);
    pos += N;
    
    // Zeros (64 - N = 32)
    memset(buffer + pos, 0, 64 - N);
    pos += (64 - N);
    
    // ADRS comprimido
    unsigned char ADRS_compressed[22];
    compressADRS(ADRS_32bytes, ADRS_compressed);
    memcpy(buffer + pos, ADRS_compressed, 22);
    pos += 22;
    
    // Input (M1)
    memcpy(buffer + pos, input, N);
    pos += N;
    
    // Calcular SHA-256
    unsigned char hash[SHA256_BYTES_SIZE];
    sha256_bytes(buffer, pos, hash);
    
    memcpy(output, hash, N);
}
