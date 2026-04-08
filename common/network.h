#ifndef NETWORK_H
#define NETWORK_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#define BUFFER_SIZE 4096

#ifdef _WIN32

typedef struct {
    HINTERNET hSession;
    HINTERNET hConnect;
    HINTERNET hRequest;
    int connected;
} NetContext;

static int net_init_win(NetContext *ctx) {
    ctx->hSession = WinHttpOpen(L"Mozilla/5.0 (compatible; Windows NT 10.0; Win64; x64)", 
                                WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                WINHTTP_NO_PROXY_NAME, 
                                WINHTTP_NO_PROXY_BYPASS, 0);
    if(!ctx->hSession) return 0;
    ctx->connected = 0;
    return 1;
}

static int net_connect_win(NetContext *ctx, const char *host, int port, int use_ssl) {
    wchar_t wHost[256];
    mbstowcs(wHost, host, strlen(host) + 1);
    
    ctx->hConnect = WinHttpConnect(ctx->hSession, wHost, (INTERNET_PORT)port, 0);
    if(!ctx->hConnect) return 0;
    
    wchar_t wVerb[] = L"POST";
    wchar_t wPath[] = L"/api/upload";
    
    ctx->hRequest = WinHttpOpenRequest(ctx->hConnect, wVerb, wPath,
                                       NULL, WINHTTP_NO_REFERER,
                                       WINHTTP_DEFAULT_ACCEPT_TYPES,
                                       use_ssl ? WINHTTP_FLAG_SECURE : 0);
    if(!ctx->hRequest) return 0;
    
    ctx->connected = 1;
    return 1;
}

static int net_send_win(NetContext *ctx, const char *data, int len) {
    if(!ctx->connected) return 0;
    
    WinHttpSendRequest(ctx->hRequest,
                       WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                       (LPVOID)data, len, len, 0);
    
    DWORD statusCode = 0;
    DWORD size = sizeof(statusCode);
    WinHttpQueryHeaders(ctx->hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &size, NULL);
    
    return (statusCode == 200);
}

static int net_download_win(NetContext *ctx, char **out_data, int *out_len) {
    if(!ctx->connected) return 0;
    
    WinHttpReceiveResponse(ctx->hRequest, NULL);
    
    DWORD size = 0;
    WinHttpQueryDataAvailable(ctx->hRequest, &size);
    
    if(size == 0) return 0;
    
    *out_data = (char*)malloc(size + 1);
    if(!*out_data) return 0;
    
    DWORD downloaded = 0;
    WinHttpReadData(ctx->hRequest, *out_data, size, &downloaded);
    (*out_data)[downloaded] = '\0';
    *out_len = downloaded;
    
    return 1;
}

static void net_close_win(NetContext *ctx) {
    if(ctx->hRequest) WinHttpCloseHandle(ctx->hRequest);
    if(ctx->hConnect) WinHttpCloseHandle(ctx->hConnect);
    if(ctx->hSession) WinHttpCloseHandle(ctx->hSession);
    ctx->connected = 0;
}

typedef NetContext GlobalNetContext;

#else

typedef struct {
    int sock;
    struct hostent *server;
    int connected;
} NetContext;

static int net_init_unix(NetContext *ctx) {
    ctx->sock = -1;
    ctx->server = NULL;
    ctx->connected = 0;
    return 1;
}

static int net_connect_unix(NetContext *ctx, const char *host, int port) {
    ctx->server = gethostbyname(host);
    if(!ctx->server) return 0;
    
    ctx->sock = socket(AF_INET, SOCK_STREAM, 0);
    if(ctx->sock < 0) return 0;
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, ctx->server->h_addr, ctx->server->h_length);
    serv_addr.sin_port = htons(port);
    
    if(connect(ctx->sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        close(ctx->sock);
        ctx->sock = -1;
        return 0;
    }
    
    ctx->connected = 1;
    return 1;
}

static int net_send_unix(NetContext *ctx, const char *data, int len) {
    if(!ctx->connected) return 0;
    
    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE,
             "POST /api/upload HTTP/1.1\r\n"
             "Host: example.com\r\n"
             "Content-Type: application/octet-stream\r\n"
             "Content-Length: %d\r\n"
             "User-Agent: Mozilla/5.0\r\n"
             "Connection: close\r\n"
             "\r\n", len);
    
    send(ctx->sock, buffer, strlen(buffer), 0);
    int sent = send(ctx->sock, data, len, 0);
    
    char resp[1024];
    recv(ctx->sock, resp, sizeof(resp) - 1, 0);
    
    return (sent == len);
}

static int net_download_unix(NetContext *ctx, char **out_data, int *out_len) {
    if(!ctx->connected) return 0;
    
    char request[512];
    snprintf(request, sizeof(request),
             "GET /api/download HTTP/1.1\r\n"
             "Host: example.com\r\n"
             "User-Agent: Mozilla/5.0\r\n"
             "Connection: close\r\n"
             "\r\n");
    
    send(ctx->sock, request, strlen(request), 0);
    
    char *full_response = (char*)malloc(65536);
    if(!full_response) return 0;
    
    int total = 0;
    char temp[4096];
    int received;
    
    while((received = recv(ctx->sock, temp, sizeof(temp) - 1, 0)) > 0) {
        if(total + received >= 65536) break;
        memcpy(full_response + total, temp, received);
        total += received;
    }
    full_response[total] = '\0';
    
    char *body = strstr(full_response, "\r\n\r\n");
    if(body) {
        body += 4;
        int body_len = total - (body - full_response);
        *out_data = (char*)malloc(body_len + 1);
        if(*out_data) {
            memcpy(*out_data, body, body_len);
            (*out_data)[body_len] = '\0';
            *out_len = body_len;
            free(full_response);
            return 1;
        }
    }
    
    free(full_response);
    return 0;
}

static void net_close_unix(NetContext *ctx) {
    if(ctx->sock >= 0) close(ctx->sock);
    ctx->connected = 0;
}

typedef NetContext GlobalNetContext;

#endif

#endif
