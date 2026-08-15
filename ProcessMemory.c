#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ProcessMemory.h"

#define WIN32_LEAN_AND_MEAN
#include <tlhelp32.h>
#include <windows.h>

static DWORD FindProcessId(const char* processName) {
    HANDLE snapshot;
    PROCESSENTRY32 entry;
    DWORD result = 0;

    if (!processName) return 0;

    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE) return 0;

    memset(&entry, 0, sizeof(entry));

    entry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &entry)) {
        do {
            if (_stricmp(entry.szExeFile, processName) == 0) {
                result = entry.th32ProcessID;
                break;
            }

        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);

    return result;
}

static uintptr_t FindMainModuleBase(DWORD processId) {
    HANDLE snapshot;

    MODULEENTRY32 moduleEntry;

    uintptr_t result = 0;

    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);

    if (snapshot == INVALID_HANDLE_VALUE) return 0;

    memset(&moduleEntry, 0, sizeof(moduleEntry));

    moduleEntry.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(snapshot, &moduleEntry)) {
        result = (uintptr_t)moduleEntry.modBaseAddr;
    }

    CloseHandle(snapshot);

    return result;
}

void ProcessMemory_Init(ProcessMemory* pm, const char* processName, bool trustProcess) {
    if (!pm) return;

    memset(pm, 0, sizeof(ProcessMemory));

    pm->processHandle = NULL;
    pm->processId = 0;
    pm->baseAddress = 0;
    pm->trustProcess = trustProcess;
    pm->opened = false;

    if (processName) {
        strncpy_s(pm->processName, sizeof(pm->processName), processName, _TRUNCATE);
    }
}

void ProcessMemory_Close(ProcessMemory* pm) {
    if (!pm) return;

    if (pm->processHandle) {
        CloseHandle(pm->processHandle);

        pm->processHandle = NULL;
    }

    pm->processId = 0;
    pm->baseAddress = 0;
    pm->opened = false;
}

bool ProcessMemory_Open(ProcessMemory* pm) {
    DWORD pid;

    HANDLE process;

    if (!pm) return false;

    if (pm->processName[0] == '\0') return false;

    if (pm->processHandle) {
        CloseHandle(pm->processHandle);

        pm->processHandle = NULL;
    }

    pm->processId = 0;
    pm->baseAddress = 0;

    pid = FindProcessId(pm->processName);

    if (pid == 0) return false;

    process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);

    if (!process) return false;

    pm->processHandle = process;
    pm->processId = pid;
    pm->baseAddress = FindMainModuleBase(pid);
    pm->opened = true;

    return true;
}

bool ProcessMemory_CheckProcess(ProcessMemory* pm) {
    DWORD exitCode;

    if (!pm) return false;

    if (pm->trustProcess && pm->opened && pm->processHandle) {
        return true;
    }

    if (pm->processHandle) {
        if (GetExitCodeProcess(pm->processHandle, &exitCode)) {
            if (exitCode == STILL_ACTIVE) {
                return true;
            }
        }

        CloseHandle(pm->processHandle);

        pm->processHandle = NULL;
        pm->processId = 0;
        pm->baseAddress = 0;
        pm->opened = false;
    }

    if (!ProcessMemory_Open(pm)) return false;

    if (pm->trustProcess) pm->opened = true;

    return true;
}

DWORD ProcessMemory_GetProcessId(ProcessMemory* pm) {
    if (!ProcessMemory_CheckProcess(pm)) return 0;

    return pm->processId;
}

uintptr_t ProcessMemory_GetBaseAddress(ProcessMemory* pm) {
    if (!ProcessMemory_CheckProcess(pm)) return 0;

    if (pm->baseAddress == 0) {
        pm->baseAddress = FindMainModuleBase(pm->processId);
    }

    return pm->baseAddress;
}

bool ProcessMemory_Read(ProcessMemory* pm, uintptr_t address, void* buffer, SIZE_T size) {
    SIZE_T bytesRead = 0;

    if (!pm || !buffer || size == 0) {
        return false;
    }

    if (!ProcessMemory_CheckProcess(pm)) return false;

    return ReadProcessMemory(pm->processHandle, (LPCVOID)address, buffer, size, &bytesRead) && bytesRead == size;
}

bool ProcessMemory_Write(ProcessMemory* pm, uintptr_t address, const void* buffer, SIZE_T size) {
    SIZE_T bytesWritten = 0;

    if (!pm || !buffer || size == 0) {
        return false;
    }

    if (!ProcessMemory_CheckProcess(pm)) return false;

    return WriteProcessMemory(pm->processHandle, (LPVOID)address, buffer, size, &bytesWritten) && bytesWritten == size;
}

#define DEFINE_READ_FUNCTION(type, name) \
bool ProcessMemory_Read##name(ProcessMemory* pm, uintptr_t address, type* value) { \
    return ProcessMemory_Read(pm, address, value, sizeof(type)); \
}

DEFINE_READ_FUNCTION(int8_t, Int8)
DEFINE_READ_FUNCTION(uint8_t, UInt8)
DEFINE_READ_FUNCTION(int16_t, Int16)
DEFINE_READ_FUNCTION(uint16_t, UInt16)
DEFINE_READ_FUNCTION(int32_t, Int32)
DEFINE_READ_FUNCTION(uint32_t, UInt32)
DEFINE_READ_FUNCTION(int64_t, Int64)
DEFINE_READ_FUNCTION(uint64_t, UInt64)
DEFINE_READ_FUNCTION(float, Float)
DEFINE_READ_FUNCTION(double, Double)

#undef DEFINE_READ_FUNCTION

#define DEFINE_WRITE_FUNCTION(type, name)                                          \
bool ProcessMemory_Write##name(ProcessMemory* pm, uintptr_t address, type value) { \
    return ProcessMemory_Write(pm, address, &value, sizeof(type));                 \
}

DEFINE_WRITE_FUNCTION(int8_t, Int8)
DEFINE_WRITE_FUNCTION(uint8_t, UInt8)
DEFINE_WRITE_FUNCTION(int16_t, Int16)
DEFINE_WRITE_FUNCTION(uint16_t, UInt16)
DEFINE_WRITE_FUNCTION(int32_t, Int32)
DEFINE_WRITE_FUNCTION(uint32_t, UInt32)
DEFINE_WRITE_FUNCTION(int64_t, Int64)
DEFINE_WRITE_FUNCTION(uint64_t, UInt64)
DEFINE_WRITE_FUNCTION(float, Float)
DEFINE_WRITE_FUNCTION(double, Double)

#undef DEFINE_WRITE_FUNCTION

bool ProcessMemory_Traverse(ProcessMemory* pm, uintptr_t address, const int32_t* offsets, size_t offsetCount, uintptr_t* result) {
    size_t i;

    uintptr_t current;

    if (!result) return false;

    *result = 0;

    if (!ProcessMemory_CheckProcess(pm)) return false;

    if (address == 0) return false;

    current = address;

    for (i = 0; i < offsetCount; i++) {
        uintptr_t pointer;

        if (!ProcessMemory_Read(pm, current, &pointer, sizeof(pointer))) {
            return false;
        }

        if (pointer == 0) return false;

        current = pointer + (intptr_t)offsets[i];
    }

    *result = current;

    return true;
}

#define DEFINE_TRAVERSE_READ(type, name)                                                                                               \
    bool ProcessMemory_Traverse##name(ProcessMemory* pm, uintptr_t address, const int32_t* offsets, size_t offsetCount, type* value) { \
        uintptr_t result;                                                                                                              \
                                                                                                                                       \
        if (!ProcessMemory_Traverse(pm, address, offsets, offsetCount, &result)) {                                                     \
            return false;                                                                                                              \
        }                                                                                                                              \
                                                                                                                                       \
        return ProcessMemory_Read##name(pm, result, value);                                                                            \
    }

DEFINE_TRAVERSE_READ(int8_t, Int8)
DEFINE_TRAVERSE_READ(int16_t, Int16)
DEFINE_TRAVERSE_READ(int32_t, Int32)
DEFINE_TRAVERSE_READ(int64_t, Int64)
DEFINE_TRAVERSE_READ(uint32_t, UInt32)
DEFINE_TRAVERSE_READ(uint64_t, UInt64)
DEFINE_TRAVERSE_READ(float, Float)
DEFINE_TRAVERSE_READ(double, Double)

#undef DEFINE_TRAVERSE_READ