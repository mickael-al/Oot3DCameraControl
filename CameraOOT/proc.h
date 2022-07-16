#ifndef _PROC
#define _PROC

#include <vector>
#include <Windows.h>
#include <Tlhelp32.h>


template<typename T> T RPM(HANDLE hProc,SIZE_T address) {
    T buffer;
    ReadProcessMemory(hProc, (LPCVOID)address, &buffer, sizeof(T), NULL);
    return buffer;
}

template<typename T> void WPM(HANDLE hProc,SIZE_T address, T buffer) {
    WriteProcessMemory(hProc, (LPVOID)address, &buffer, sizeof(buffer), NULL);
}

DWORD GetProcId(const wchar_t* procName);

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);

MODULEENTRY32 GetModuleEntry(DWORD procId, const wchar_t* modName);

uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets);

uintptr_t find_pattern(HANDLE hProc, uint8_t* patern, int size);

#endif