#include "core/Datahash.h"
#include "core/Dataread.h"


int main(){
    char text[] = "Sample text for hashing";
    unsigned long long hashValue = fastHash(text);
    std::cout << "Hash value: " << hashValue << std::endl;

    return 0;
}