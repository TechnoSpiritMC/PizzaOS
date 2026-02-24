#include "include/gdt.h"
#include "include/util.h"
#include "include/stdint.h"
#include "stdlib/stdio.h"

#if advancedLogging
#include "include/vga.h"
#endif

extern void gdt_flush(uint32_t);
extern void tss_flush(void);

struct gdt_entry_struct gdt_entries[6];
struct gdt_ptr_struct gdt_ptr;
struct tss_entry_struct tssEntry;

void initGdt() {

#if advancedLogging
    print("Initializing GDT...\r\n");
#endif

    gdt_ptr.limit = (sizeof(struct gdt_entry_struct) * 6) - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;

    setGdtGate(0, 0, 0, 0, 0);                // Null segment, required!
    setGdtGate(1, 0, 0xffffffff, 0x9a, 0xcf); // Kernel code segment.
    setGdtGate(2, 0, 0xffffffff, 0x92, 0xcf); // Kernel data segment.

    setGdtGate(3, 0, 0xffffffff, 0xfa, 0xcf); // User code segment.
    setGdtGate(4, 0, 0xffffffff, 0xf2, 0xcf); // User data segment.
    writeTss(5, 0x10, 0);

#if advancedLogging
    print("GDT initialized. Flushing GDT and TSS...\r\n");
#endif

    gdt_flush((uint32_t)&gdt_ptr);
    tss_flush();
}


void writeTss(uint32_t num, uint32_t ss0, uint32_t esp0) {
    uint32_t base = (uint32_t) &tssEntry;
    uint32_t limit = base + sizeof(tssEntry);

    setGdtGate(num, base, limit, 0xe9, 0x00);
    memset(&tssEntry, 0, sizeof(tssEntry));

    tssEntry.ss0 = ss0;
    tssEntry.esp0 = esp0;

    tssEntry.cs = 0x08 | 0x3;
    tssEntry.ss = tssEntry.ds = tssEntry.es = tssEntry.fs = tssEntry.gs = 0x10 | 0x3;

#if advancedLogging
    print("Wrote TSS.\r\n");
#endif
}


void setGdtGate(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = (base & 0xffff);
    gdt_entries[num].base_middle = (base >> 16) & 0xff;
    gdt_entries[num].base_high   = (base >> 24) & 0xff;

    gdt_entries[num].limit       = (limit & 0xffff);
    gdt_entries[num].flags       = (limit >> 16) & 0x0f;
    gdt_entries[num].flags      |= gran;

    gdt_entries[num].access = access;

#if advancedLogging
    printf("Set GDT Gate (%i)\r\n", num);
#endif
}
