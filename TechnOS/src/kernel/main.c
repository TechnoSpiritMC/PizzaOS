#include "stdint.h"
#include "stdio.h"
#include "disk/asmDisk.h"

void __cdecl _cstart(){

    uint8_t error;

    puts("BIENVENUTO, ALLA PIZZA SPAGHETTI OPERATINGUA SYSTEMMA!\r\n");
    x86_Disk_Reset(0, &error);

    printf("Disk read status: %d\r\n", error);
}

