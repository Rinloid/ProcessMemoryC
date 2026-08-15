#include <stdio.h>
#include "ProcessMemory.h"

int main() {
    ProcessMemory pm;
	ProcessMemory_Init(&pm, "example.exe", false);

    if (!ProcessMemory_Open(&pm)) {
		printf("Process Not Found!\n");
		return 0;
	}

    int32_t address = 0x123456;
    int32_t offsets[] = {0x123, 0x456, 0x789};
    int32_t value = 0;

    /* Memory Reading */
    ProcessMemory_ReadInt32(&pm, address, &value);
    printf("%d\n", value);

    /* Memory Writing */
    ProcessMemory_WriteInt32(&pm, address, 1234);

    /* Pointer Chain Traversal */
    ProcessMemory_TraverseInt32(&pm, ProcessMemory_GetBaseAddress(&pm) + address, offsets, ARRAYSIZE(offsets), &value);
    printf("%d\n", value);

    ProcessMemory_Close(&pm);

    return 0;
}
