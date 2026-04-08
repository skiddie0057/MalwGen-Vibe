#ifndef ANTIVM_H
#define ANTIVM_H

#ifdef _WIN32
#include <windows.h>
#include <intrin.h>

static int check_cpuid_hypervisor(void) {
    int info[4];
    __cpuid(info, 1);
    return (info[2] & (1 << 31)) != 0;
}

static int check_vmware_registry(void) {
    HKEY hKey;
    if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\VBoxGuest", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return 1;
    }
    if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\VMTools", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return 1;
    }
    return 0;
}

static int check_vm_mac_address(void) {
    int adapters[8];
    ULONG adapter_size = sizeof(adapters);
    if(GetAdaptersAddresses(AF_UNSPEC, 0, NULL, (PIP_ADAPTER_ADDRESSES)adapters, &adapter_size) == ERROR_SUCCESS) {
        unsigned char* mac = ((PIP_ADAPTER_ADDRESSES)adapters)->PhysicalAddress;
        if(mac[0] == 0x00 && mac[1] == 0x05 && mac[2] == 0x69) return 1;
        if(mac[0] == 0x00 && mac[1] == 0x0C && mac[2] == 0x29) return 1;
        if(mac[0] == 0x08 && mac[1] == 0x00 && mac[2] == 0x27) return 1;
    }
    return 0;
}

static int check_timing_sandbox(void) {
    volatile unsigned int x = 0;
    unsigned int start = GetTickCount();
    for(int i = 0; i < 1000; i++) x += i;
    unsigned int elapsed = GetTickCount() - start;
    return (elapsed > 100);
}

static int anti_vm_check(void) {
    if(check_cpuid_hypervisor()) return 1;
    if(check_vmware_registry()) return 1;
    if(check_vm_mac_address()) return 1;
    if(check_timing_sandbox()) return 1;
    return 0;
}

#else
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static int check_cpuid_hypervisor(void) {
    unsigned int eax, ebx, ecx, edx;
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1), "c"(0));
    return (ecx & (1 << 31)) != 0;
}

static int check_vm_files(void) {
    struct stat st;
    if(stat("/sys/bus/vmbus/devices", &st) == 0) return 1;
    if(stat("/proc/xen", &st) == 0) return 1;
    if(stat("/usr/bin/VBoxControl", &st) == 0) return 1;
    return 0;
}

static int check_vmware_tools(void) {
    struct stat st;
    if(stat("/usr/bin/vmware-toolbox-cmd", &st) == 0) return 1;
    if(stat("/etc/vmware-tools", &st) == 0) return 1;
    return 0;
}

static int check_timing_sandbox(void) {
    volatile unsigned int x = 0;
    struct timespec ts_start, ts_end;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);
    for(int i = 0; i < 1000; i++) x += i;
    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    long elapsed = (ts_end.tv_sec - ts_start.tv_sec) * 1000 + (ts_end.tv_nsec - ts_start.tv_nsec) / 1000000;
    return (elapsed > 100);
}

static int anti_vm_check(void) {
    if(check_cpuid_hypervisor()) return 1;
    if(check_vm_files()) return 1;
    if(check_vmware_tools()) return 1;
    if(check_timing_sandbox()) return 1;
    return 0;
}

#endif
#endif
