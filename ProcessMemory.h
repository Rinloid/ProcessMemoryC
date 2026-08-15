#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct ProcessMemory {
    HANDLE processHandle;
    DWORD processId;
    uintptr_t baseAddress;
    char processName[MAX_PATH];
    bool trustProcess;
    bool opened;
} ProcessMemory;

void ProcessMemory_Init(ProcessMemory* pm, const char* processName, bool trustProcess);
void ProcessMemory_Close(ProcessMemory* pm);
bool ProcessMemory_CheckProcess(ProcessMemory* pm);
bool ProcessMemory_Open(ProcessMemory* pm);

DWORD ProcessMemory_GetProcessId(ProcessMemory* pm);
uintptr_t ProcessMemory_GetBaseAddress(ProcessMemory* pm);

bool ProcessMemory_Read(ProcessMemory* pm, uintptr_t address, void* buffer, SIZE_T size);
bool ProcessMemory_Write(ProcessMemory* pm, uintptr_t address, const void* buffer, SIZE_T size);

bool ProcessMemory_ReadInt8(ProcessMemory* pm, uintptr_t address, int8_t* value);
bool ProcessMemory_ReadUInt8(ProcessMemory* pm, uintptr_t address, uint8_t* value);
bool ProcessMemory_ReadInt16(ProcessMemory* pm, uintptr_t address, int16_t* value);
bool ProcessMemory_ReadUInt16(ProcessMemory* pm, uintptr_t address, uint16_t* value);
bool ProcessMemory_ReadInt32(ProcessMemory* pm, uintptr_t address, int32_t* value);
bool ProcessMemory_ReadUInt32(ProcessMemory* pm, uintptr_t address, uint32_t* value);
bool ProcessMemory_ReadInt64(ProcessMemory* pm, uintptr_t address, int64_t* value);
bool ProcessMemory_ReadUInt64(ProcessMemory* pm, uintptr_t address, uint64_t* value);
bool ProcessMemory_ReadFloat(ProcessMemory* pm, uintptr_t address, float* value);
bool ProcessMemory_ReadDouble(ProcessMemory* pm, uintptr_t address, double* value);

bool ProcessMemory_WriteInt8(ProcessMemory* pm, uintptr_t address, int8_t value);
bool ProcessMemory_WriteUInt8(ProcessMemory* pm, uintptr_t address, uint8_t value);
bool ProcessMemory_WriteInt16(ProcessMemory* pm, uintptr_t address, int16_t value);
bool ProcessMemory_WriteUInt16(ProcessMemory* pm, uintptr_t address, uint16_t value);
bool ProcessMemory_WriteInt32(ProcessMemory* pm, uintptr_t address, int32_t value);
bool ProcessMemory_WriteUInt32(ProcessMemory* pm, uintptr_t address, uint32_t value);
bool ProcessMemory_WriteInt64(ProcessMemory* pm, uintptr_t address, int64_t value);
bool ProcessMemory_WriteUInt64(ProcessMemory* pm, uintptr_t address, uint64_t value);
bool ProcessMemory_WriteFloat(ProcessMemory* pm, uintptr_t address, float value);
bool ProcessMemory_WriteDouble(ProcessMemory* pm, uintptr_t address, double value);

bool ProcessMemory_Traverse(ProcessMemory* pm, uintptr_t address, const int32_t* offsets, size_t offsetCount, uintptr_t* result);
bool ProcessMemory_TraverseInt8(ProcessMemory* pm, uintptr_t address, const int32_t* offsets, size_t offsetCount, int8_t* value);
bool ProcessMemory_TraverseInt16(ProcessMemory* pm, uintptr_t address, const int32_t* offsets, size_t offsetCount, int16_t* value);
bool ProcessMemory_TraverseInt32(ProcessMemory* pm, uintptr_t address, const int32_t* offsets, size_t offsetCount, int32_t* value);
bool ProcessMemory_TraverseInt64(ProcessMemory* pm, uintptr_t address, const int32_t* offsets, size_t offsetCount, int64_t* value);
bool ProcessMemory_TraverseUInt32(ProcessMemory* pm, uintptr_t address, const int32_t* offsets, size_t offsetCount, uint32_t* value);
bool ProcessMemory_TraverseUInt64(ProcessMemory* pm, uintptr_t address, const int32_t* offsets, size_t offsetCount, uint64_t* value);
bool ProcessMemory_TraverseFloat(ProcessMemory* pm, uintptr_t address, const int32_t* offsets, size_t offsetCount, float* value);
bool ProcessMemory_TraverseDouble(ProcessMemory* pm, uintptr_t address, const int32_t* offsets, size_t offsetCount, double* value);