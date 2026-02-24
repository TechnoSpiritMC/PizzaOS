; org 0x0000
; bits 16
;
; main:
;     mov ax, 0x2000
;     mov ds, ax
;     mov es, ax
;     mov ss, ax
;
;     mov sp, 0xffff
;     mov si, os_boot_msg
;     call print
;
;     hlt
;
; halt:
;     jmp halt
;
; print:
;     push si
;     push ax
;     push bx
;
; print_loop:
;     lodsb
;     or al, al
;     jz done_print
;
;     mov ah, 0x0e
;     mov bh, 0
;     int 10h ; BIOS (Basic I/O System) video interrupt
;
;     jmp print_loop
;
; done_print:
;     pop bx
;     pop ax
;     pop si
;     ret
;
; os_boot_msg: db 'BIENVENUTO, PIZZA SPAGHETTI OPERATING SYSTEMME!!', 0x0d, 0x0a, 0
;
; times 510-($-$$) db 0
; dw 0aa55h


bits 16
section _ENTRY CLASS=CODE

extern __cstart
global entry

entry:
    cli
    mov ax, ds
    mov ss, ax
    mov sp, 0
    mov bp, sp

    sti

    call __cstart

    cli
    hlt