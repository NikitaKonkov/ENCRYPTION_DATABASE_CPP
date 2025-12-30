

// Include necessary headers
#include <iostream>
#include <cstring>
#include <windows.h>
#include <wincrypt.h>
#include <vector>
#include <immintrin.h>
#include <omp.h>  // activate OMP *
#include <chrono> 
#include <mutex>
#include <algorithm>
#include <sstream>
#include <iomanip>


static std::string FINAL_HASH = "";
static std::string SALT_DIGEST = "CHARLIE_PAPA_PAPA_INDIA_SIERRA_GOLF_ALFA_YANKE_BRAVO_ROMEO_OSCAR"; // 64 char salty key use
const static size_t CHUNK_SIZE = 1024 * 1024 * 16; // 16 MB chunks. (prefered range [ 1 - 16 ])
static unsigned long long HASH_ARRAY[4]            // Collision poor set
{
        0x1B0F8642,0x7F3C12ED,0x5E01BC89,0x391542AF
}; 
// AES key test
unsigned char AESKEY0[32] = { // Set 1
    0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
    0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4
};
unsigned char AESKEY1[32] = { // Set 2 
    0x82, 0x45, 0xd6, 0x7e, 0x1a, 0xf3, 0x9c, 0x5b, 0x78, 0x2e, 0xc1, 0x6a, 0xb4, 0x9d, 0x3e, 0x7f,
    0x51, 0xd4, 0xa3, 0x2c, 0x86, 0xf7, 0xe9, 0x1d, 0xbf, 0x4a, 0x0d, 0x53, 0x6e, 0xc2, 0x39, 0x8b
};
unsigned char AESKEY2[32] = { // Set 3
    0x4e, 0xa7, 0xf1, 0x5d, 0x9c, 0x63, 0x28, 0xd1, 0xb6, 0x0a, 0x92, 0x7c, 0xe5, 0x3f, 0x41, 0x74,
    0x2d, 0x36, 0xc8, 0x19, 0xfb, 0x50, 0x8e, 0x94, 0x67, 0x1b, 0x30, 0xd5, 0xa2, 0x4c, 0x0f, 0xe3
};
unsigned char AESKEY3[32] = { // Set 4
    0xc9, 0x7f, 0x5e, 0x3d, 0x2b, 0x81, 0x14, 0xa6, 0x93, 0x40, 0x72, 0x0c, 0xd8, 0x65, 0xf9, 0x1a,
    0x37, 0xb2, 0x4d, 0xe0, 0x6c, 0x8a, 0x29, 0xf5, 0x13, 0x46, 0x7a, 0xd0, 0x5c, 0x91, 0x2f, 0xe8
};


typedef unsigned char BYTE;

// Function to encrypt a long long value using Windows CryptoAPI with AES-256
// The key is first hashed with SHA-256, then used to derive an AES-256 key
static unsigned long long WindowsCryptoAPI_SHA256KeyedAES256(unsigned long long* data, BYTE* key) {
    // Declare handles for cryptographic operations
    HCRYPTPROV hProv = 0;  // Cryptographic provider handle
    HCRYPTHASH hHash = 0;  // Hash object handle
    HCRYPTKEY hKey = 0;    // Cryptographic key handle

    // Allocate buffer for encrypted output (with extra space for potential padding)
    BYTE outputBuffer[64];

    // Initialize the length variable for encrypted data
    DWORD dwEncryptedDataLen = 32;

    // Acquire a handle to the AES cryptographic provider
    if (!CryptAcquireContextW(&hProv, NULL, MS_ENH_RSA_AES_PROV_W, PROV_RSA_AES, 0)) {
        std::cerr << "Error in CryptAcquireContextW: " << GetLastError() << "\n";
        return -1;
    }

    // Create a SHA-256 hash object
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        std::cerr << "Error in CryptCreateHash: " << GetLastError() << "\n";
        CryptReleaseContext(hProv, 0);
        return -1;
    }

    // Hash the encryption key using SHA-256
    if (!CryptHashData(hHash, key, 32, 0)) {
        std::cerr << "Error in CryptHashData: " << GetLastError() << "\n";
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return -1;
    }

    // Derive an AES-256 session key from the hash object
    if (!CryptDeriveKey(hProv, CALG_AES_256, hHash, 0, &hKey)) {
        std::cerr << "Error in CryptDeriveKey: " << GetLastError() << "\n";
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return -1;
    }

    // Copy the input data to the output buffer for in-place encryption
    memcpy(outputBuffer, data, 32);

    // Encrypt the data using the derived AES-256 key
    // Note: This operation modifies the data in-place in the output buffer
    if (!CryptEncrypt(hKey, NULL, TRUE, 0, outputBuffer, &dwEncryptedDataLen, sizeof(outputBuffer))) {
        std::cerr << "Error in CryptEncrypt: " << GetLastError() << "\n";
        CryptDestroyKey(hKey);
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return -1;
    }

    // Clean up: destroy the key, hash object, and release the provider context
    CryptDestroyKey(hKey);
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    // Return the first 8 bytes of encrypted data as a long long
    // WARNING: This may truncate the encrypted data and is not a secure practice
    return *reinterpret_cast<long long*>(outputBuffer);
}



// Vectorized hash function using AVX intrinsics
static thread_local unsigned int found_zero = 0; // thread-local for safety in parallel contexts

static void AVX_hash(size_t size, char* arr)
{
    // Use 4 independent accumulators to hide latency and improve ILP
    __m256i vhash0 = _mm256_setzero_si256();
    __m256i vhash1 = _mm256_setzero_si256();
    __m256i vhash2 = _mm256_setzero_si256();
    __m256i vhash3 = _mm256_setzero_si256();

    // Process 128 bytes per iteration (4x unrolled) for better throughput
    size_t i = 0;
    const size_t unroll_limit = size & ~127ULL; // Round down to 128-byte boundary
    
    for (; i < unroll_limit; i += 128) {
        __m256i d0 = _mm256_loadu_si256((__m256i*)(arr + i));
        __m256i d1 = _mm256_loadu_si256((__m256i*)(arr + i + 32));
        __m256i d2 = _mm256_loadu_si256((__m256i*)(arr + i + 64));
        __m256i d3 = _mm256_loadu_si256((__m256i*)(arr + i + 96));
        
        vhash0 = _mm256_xor_si256(vhash0, d0);
        vhash1 = _mm256_xor_si256(vhash1, d1);
        vhash2 = _mm256_xor_si256(vhash2, d2);
        vhash3 = _mm256_xor_si256(vhash3, d3);
    }
    
    // Handle remaining 32-byte chunks
    for (; i < size; i += 32) {
        __m256i data = _mm256_loadu_si256((__m256i*)(arr + i));
        vhash0 = _mm256_xor_si256(vhash0, data);
    }

    // Combine all accumulators
    vhash0 = _mm256_xor_si256(vhash0, vhash1);
    vhash2 = _mm256_xor_si256(vhash2, vhash3);
    __m256i vhash = _mm256_xor_si256(vhash0, vhash2);

    // Store the AVX result into an array
    alignas(32) unsigned long long result[4];
    _mm256_store_si256((__m256i*)result, vhash);

    // Combine the results for AES encryption
    unsigned long long data0[4] = { result[0] * size, result[1], result[2], result[3] + found_zero };
    unsigned long long data1[4] = { result[1] + found_zero, result[2] * size, result[3], result[0] };
    unsigned long long data2[4] = { result[2], result[3] + found_zero, result[0] * size, result[1] };
    unsigned long long data3[4] = { result[3], result[0], result[1] + found_zero, result[2] * size };
    
    if (data0[0] == 0) { found_zero++; }

    // Encrypt in parallel using OpenMP (4 independent operations)
    unsigned long long enc_results[4];
    #pragma omp parallel sections
    {
        #pragma omp section
        { enc_results[0] = WindowsCryptoAPI_SHA256KeyedAES256(data0, AESKEY0); }
        #pragma omp section
        { enc_results[1] = WindowsCryptoAPI_SHA256KeyedAES256(data1, AESKEY1); }
        #pragma omp section
        { enc_results[2] = WindowsCryptoAPI_SHA256KeyedAES256(data2, AESKEY2); }
        #pragma omp section
        { enc_results[3] = WindowsCryptoAPI_SHA256KeyedAES256(data3, AESKEY3); }
    }

    // XOR results into hash array
    HASH_ARRAY[0] ^= enc_results[0];
    HASH_ARRAY[1] ^= enc_results[1];
    HASH_ARRAY[2] ^= enc_results[2];
    HASH_ARRAY[3] ^= enc_results[3];
}

// Basic cipher to salt the final hash
static std::string substitution_cipher(const std::string& input, const std::string& key) {
    std::string result = input;
    for (int c = 0; c < 64 && c < input.length(); ++c) {
        int k = key[c % key.length()] % 64;
        std::swap(result[c], result[k]);
    }
    return result;
}


// Convert ULL into 16 byte hex
static std::string ull_to_hex16(unsigned long long value) {
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(16) << std::hex << value;
    return ss.str();
}


// Generate the hash by given file
static std::string hash_file(const wchar_t* filename) {
    std::vector<unsigned long long> local_chunk_hashes;
    std::mutex hash_mutex; // Mutex for thread-safe access to local_chunk_hashes

    // Open the file using Windows API
    HANDLE hFile = CreateFileW(
        filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    // If the file cannot be open
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening file" << "\n";
    }

    // Get the file size using Windows API
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        std::cerr << "Error getting file size" << "\n";
        CloseHandle(hFile);
    }

    // Parallel loop to read and hash chunks of the file
    #pragma omp parallel // Silly "C6993" error but it works
    {
        std::vector<char> buffer(CHUNK_SIZE);
    #pragma omp for schedule(dynamic)
        for (LONGLONG offset = 0; offset < fileSize.QuadPart; offset += CHUNK_SIZE) {
            DWORD bytesToRead = static_cast<DWORD>(min(static_cast<LONGLONG>(CHUNK_SIZE), fileSize.QuadPart - offset));

            // Set up overlapped structure for asynchronous file reading
            OVERLAPPED overlapped = { 0 };
            overlapped.Offset = static_cast<DWORD>(offset);
            overlapped.OffsetHigh = static_cast<DWORD>(offset >> 32);

            // Read a chunk of the file
            DWORD bytesRead;
            if (!ReadFile(hFile, buffer.data(), bytesToRead, &bytesRead, &overlapped)) {
                if (GetLastError() != ERROR_IO_PENDING) {
                    std::cerr << "Error reading file" << "\n";
                    continue;
                }
                // Wait for the asynchronous read to complete
                if (!GetOverlappedResult(hFile, &overlapped, &bytesRead, TRUE)) {
                    std::cerr << "Error getting overlapped result" << "\n";
                    continue;
                }
            }
            else {
                std::lock_guard<std::mutex> guard(hash_mutex); // Critical section for thread-safe access
                AVX_hash(bytesRead, buffer.data()); // Hash the chunk using only the actual bytes read
            }
        }
    }

    // Close the file handle
    CloseHandle(hFile);

    // Combine all chunk hashes into a final hash
    for (auto hash : HASH_ARRAY) FINAL_HASH += ull_to_hex16(hash);

    // Cypher salt with hash
    SALT_DIGEST = substitution_cipher(SALT_DIGEST, FINAL_HASH);

    // Cypher final hash with salt
    FINAL_HASH = substitution_cipher(FINAL_HASH, SALT_DIGEST);

    return FINAL_HASH;
}


int main() {
    // Start the timer
    auto start = std::chrono::high_resolution_clock::now(); 


    // 100 GB test_file
    static const wchar_t* filename = L"C:/LARGE.txt";

    // Generate a hash
    std::string hash = hash_file(filename);
    std::cout << hash << "\n";


    // Stop the timer and calculate the duration
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);

    // Print the execution time
    std::cout << duration.count() << " seconds" << "\n";

    
    return 0;
}
