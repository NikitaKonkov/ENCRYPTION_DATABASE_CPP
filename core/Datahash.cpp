#include "Datahash.h"
#include "Dataread.h"

unsigned long long fastHash(char* text) {
    unsigned long long hash = 14695981039346656037ULL; // pre salt digest
    while (*text) {
        hash ^= static_cast<unsigned char>(*text++);
        hash *= 1099511628211ULL;
    }
    return hash;
}