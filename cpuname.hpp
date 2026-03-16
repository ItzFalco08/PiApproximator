#pragma once
#include <iostream>
// --- Windows Includes ---
#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #define PLATFORM_NAME "Windows"

// --- Linux/Android Includes ---
#elif defined(__linux__) || defined(__ANDROID__)
    #include <fstream>
    #define PLATFORM_NAME "Linux"

// --- macOS/iOS Includes ---
#elif defined(__APPLE__) || defined(__MACH__)
    #include <sys/types.h>
    #include <sys/sysctl.h>
    #define PLATFORM_NAME "macOS"

#else
    #define PLATFORM_NAME "Unknown OS"
#endif

std::string getCPUModel() {
#if defined(_WIN32) || defined(_WIN64)
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return "Unknown Windows CPU";

    char buffer[256];
    DWORD bufferSize = sizeof(buffer);
    if (RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)buffer, &bufferSize) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return "Unknown Windows CPU";
    }
    RegCloseKey(hKey);
    return std::string(buffer);

#elif defined(__linux__) || defined(__ANDROID__)
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.find("model name") != std::string::npos) {
            size_t pos = line.find(": ");
            if (pos != std::string::npos) return line.substr(pos + 2);
        }
    }
    return "Unknown Linux CPU";

#elif defined(__APPLE__) || defined(__MACH__)
    char buffer[256];
    size_t bufferSize = sizeof(buffer);
    if (sysctlbyname("machdep.cpu.brand_string", &buffer, &bufferSize, NULL, 0) == 0) {
        return std::string(buffer);
    }
    return "Unknown macOS CPU";

#else
    return "Unsupported Platform";
#endif
}