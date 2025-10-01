typedef struct
{
    char* SK[256];
    char* PK[256]; 
}Keys;


Keys *malloc_keys();
void generateSecretKeys(Keys *keys);
void  generatePublicKeys(Keys* keys);
void printKeys(Keys* keys);
void freeKeys(Keys *keys) ;