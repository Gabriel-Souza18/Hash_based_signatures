
typedef struct {
    char* SK0[256];
    char* SK1[256];
}SecretKeys;

typedef struct{
    char* PK0[256]; 
    char* PK1[256];
}PublicKeys;

SecretKeys *malloc_Skeys();
PublicKeys *malloc_Pkeys();
void generateSecretKeys(SecretKeys *keys);
void  generatePublicKeys(PublicKeys *Pkeys, SecretKeys*Skeys) ;
void printKeys(PublicKeys *Pkeys, SecretKeys*Skeys) ;
void freeKeys(PublicKeys *Pkeys, SecretKeys*Skeys) ;
