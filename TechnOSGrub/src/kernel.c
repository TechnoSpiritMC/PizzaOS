#include "disk/disk.h"
#include "include/vga.h"
#include "include/gdt.h"
#include "interrupts/idt.h"
#include "stdlib/keyboard.h"
#include "timer/timer.h"
#include "include/console.h"
#include "include/multiboot.h"
#include "memory/kmalloc.h"
#include "memory/memory.h"
#include "stdlib/serial.h"
#include "display/display.h"

#include "stdlib/stdio.h"

void kmain(uint32_t magic, struct multiboot_info* bootInfo);

void kmain(uint32_t magic, struct multiboot_info* bootInfo) {
    Reset();

    serial_init();


    if (bootInfo->flags & 1 << 11) {
        print("Bit 11 is set!");
        serial_printf("Bit 11 is set!\r\n");
    }

    print("Initializing processes..\r\n");
    serial_printf("Initializing processes!\r\n");
    initGdt();
    initIdt();
    initTimer();
    initKeyboard();

    print("Initializing memory..\r\n");
    serial_printf("Initializing memory!\r\n");

    uint32_t mod1 = *(uint32_t*)(bootInfo->mods_addr + 4);
    uint32_t physicalAllocStart = (mod1 + 0xfff) & ~0xfff;
    printf("mod1: %x, pas: %x\n", mod1, physicalAllocStart);
    serial_printf("mod1: %x, pas: %x\r\n", mod1, physicalAllocStart);
    initMemory(bootInfo->mem_upper*1024, physicalAllocStart);

    kmallocInit(0x1000);

    initDisplay(bootInfo);

    if (init_ata()) {
        serial_printf("Every service initialized successfully.\r\n");

        //serial_printf("==================Trying to create a file:=====================\r\n");
        uint32_t lba = create_file("HELP.TXT");
        if (lba == -1) {
            //serial_printf(">>>>>>>>>>>>>>>>>>>> Failed to create file! <<<<<<<<<<<<<<<<<<<<<<<<\r\n");

            goto afterFile;
        }
        //serial_printf("LBA: %x\r\n", lba);
        //serial_printf(">>>>>>>>>>> File created successfully.\r\n");

        //serial_printf("================Trying to write the file:================\r\n");
        const char* data = "This is a test file.";
        write_file_data("HELP.TXT", data, strlen(data));

        char* newData[strlen(data)] = {};

        //serial_printf("===============Trying to read the file:=================\r\n");
        read_file_by_name("HELP.TXT", newData, strlen(data));

        serial_printf(">>>>>>>>>>>>>>>File read successfully.\r\n");
        serial_printf(">>>>>>>>>>>>>>>Data: %s\r\n", newData);



        //serial_printf("==================Trying to create a file:=====================\r\n");
        uint32_t _lba = create_file("MARTIN.TXT");
        if (_lba == -1) {
            //serial_printf(">>>>>>>>>>>>>>>>>>>> Failed to create file! <<<<<<<<<<<<<<<<<<<<<<<<\r\n");

            goto afterFile;
        }
        //serial_printf("LBA: %x\r\n", lba);
        //serial_printf(">>>>>>>>>>> File created successfully.\r\n");

        //serial_printf("================Trying to write the file:================\r\n");
        const char* _data = "Martin, sache que si tu lis ce texte, c'est que tu es une personne très aimable qui regarde ce que ce tdc de denis t'envoie en masse, et c'est ce sur quoi il passe ses nuits au lieu de dormir. Enft, je fais durer ce texte pour vérifier que mon merdier peut écrire sur plusieurs secteurs en même telmps. C'est un test très important car sans ç comment suis-je sencé savoir si mon truc marche ou pas, ru vois?";
        write_file_data("MARTIN.TXT", _data, strlen(_data));

        char* _newData[strlen(_data)] = {};

        //serial_printf("===============Trying to read the file:=================\r\n");
        read_file_by_name("MARTIN.TXT", _newData, strlen(_data));

        serial_printf(">>>>>>>>>>>>>>>File read successfully.\r\n");
        serial_printf(">>>>>>>>>>>>>>>Data: %s\r\n", _newData);

    } else {
        serial_printf("ATA failed to initialize!\r\n");

        for (;;) {
            __asm__ volatile("hlt");
        }
    }

    afterFile:

    serial_printf("Trying to read the file:\r\n");

    const char* data = "This is a test file.";
    char* newData[strlen(data)] = {};

    //serial_printf("===============Trying to read the file:=================\r\n");
    read_file_by_name("HELP.TXT", newData, strlen(data));

    //serial_printf(">>>>>>>>>>>>>>>File read successfully.\r\n");
    //serial_printf(">>>>>>>>>>>>>>>Data: %s\r\n", newData);
    
    testDisplayAndFonts();

#if newConsole
    print("Starting console in 3 seconds...\r\n");
    sleep(3);
    initConsole();
#endif

    for (;;) {
        __asm__ volatile("hlt");
    }
}
