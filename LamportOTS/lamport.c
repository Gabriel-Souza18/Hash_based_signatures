#include "sha256.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



void main(){
    Keys *keys = malloc_keys();
    generateSecretKeys(keys);
    generatePublicKeys(keys);

    
    printKeys(keys);
    freeKeys(keys);
}

