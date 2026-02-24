org 0x7c00
bits 16

jmp short main
nop

bdb_oem:                    db 'MSWIN4.1'
bdb_bytes_per_sector:       dw 512
bdb_sectors_per_cluster:    db 1
bdb_reserved_sectors:       dw 1
bdb_fat_count:              db 2
bdb_dir_entries_count:      dw 0e0h
bdb_total_sectors:          dw 2880
bdb_media_descriptor_type:  db 0f0h   ; NEEDS TO BE CHECKED AT THE END! media>>>>-<<<<(here!)descriptor (?)
bdb_sectors_per_fat:        dw 9
bdb_sectors_per_track:      dw 18
bdb_heads:                  dw 2
bdb_hidden_sectors:         dd 0
bdb_large_sector_count      dd 0

ebr_drive_number:           db 0
                            db 0
ebr_signature:              db 29h
ebr_volume_id:              db 12h,34h,56h,78h
ebr_volume_label:           db 'PIZZOS     '
ebr_system_id:              db 'FAT12   '

main:
    mov ax, 0    ; This moves the value "0" to a trhing on the cpu called a register (like a physical variable) called ax
    mov ds, ax   ; This moves the value that is in ax (0) in ds
    mov es, ax   ; etc
    mov ss, ax   ;

    mov sp, 0x7c00

    mov [ebr_drive_number], dl
    mov ax, 1
    mov cl, 1
    mov bx, 0x7e00
    call disk_read ; This calls a function that is written in C that reads the disk to find the code to run the OS

    mov si, os_boot_msg
    call print

    ; 4  Segments:
    ; 1) Reserved segment (1 sector).
    ; 2) File Allocation Tables (FAT) = bdb_sectors_per_fat * bdb_fat_count = 9 * 2 = 18 sectors.
    ; 3) Root Directory.
    ; 4) Data.

    mov ax, [bdb_sectors_per_fat]
    mov bl, [bdb_fat_count]
    xor bh, bh
    mul bx
    add ax, [bdb_reserved_sectors]  ; LBA Of the root dir.
    push ax

    mov ax, [bdb_dir_entries_count]
    shl ax, 5                       ; = ax * 2^5 = ax*32
    xor dx, dx
    div word [bdb_bytes_per_sector] ; (32 * bdb_dir_entries_count) / bdb_bytes_per_sector

    test dx, dx
    jz rootDirAfter
    inc ax

rootDirAfter:
    mov cl, al
    pop ax
    mov dl, [ebr_drive_number]
    mov bx, buffer
    call disk_read

    xor bx, bx
    mov di, buffer


searchKernel:
    mov si, file_kernel_bin
    mov cx, 11
    push di
    repe cmpsb

    pop di
    je foundKernel

    add di, 32
    inc bx
    cmp bx, [bdb_dir_entries_count]
    jl searchKernel

    jmp kernelNotFound


kernelNotFound:
    mov si, msg_kernel_not_found
    call print

    hlt
    jmp halt

foundKernel:
    mov ax, [di+26]
    mov [kernel_cluster], ax

    mov ax, [bdb_reserved_sectors]
    mov bx, buffer
    mov cl, [bdb_sectors_per_fat]
    mov dl, [ebr_drive_number]

    call disk_read

    mov bx, kernel_load_segment
    mov es, bx
    mov bx, kernel_load_offset

loadKernelLoop:
    mov ax, [kernel_cluster]
    add ax, 31
    mov cl, 1
    mov dl, [ebr_drive_number]

    call disk_read

    add bx, [bdb_bytes_per_sector]

    mov ax, [kernel_cluster]        ; (kernel_cluster * 6) / 2
    mov cx, 3
    mul cx
    mov cx, 2
    div cx

    mov si, buffer
    add si, ax
    mov ax, [ds:si]

    or dx, dx
    jz even

odd:
    shr ax, 4
    jmp nextClusterAfter

even:
    and ax, 0x0fff

nextClusterAfter:
    cmp ax, 0x0ff8
    jae readFinish

    mov [kernel_cluster], ax
    jmp loadKernelLoop

readFinish:
    mov dl, [ebr_drive_number]
    mov ax, kernel_load_segment
    mov ds, ax
    mov es, ax

    jmp kernel_load_segment:kernel_load_offset

    hlt

halt:
    jmp halt


; In : LBA index in ax.
; Out: cx [bits 0-5]: sector number
; Out: cx [bits 6-15]: cylinder
; Out: dh:             head
lba2chs:
    push ax
    push dx

    xor dx, dx
    div word [bdb_sectors_per_track]  ; (LBA % bdb_sectors_per_track) + 1 = sector
    inc dx
    mov cx, dx

    xor dx, dx
    div word [bdb_heads]

    mov dh, dl                        ; head     = (LBA / bdb_sectors_per_track) % bdb_heads
    mov ch, al
    shl ah, 6
    or  cl, ah                        ; cylinder = (LBA / bdb_sectors_per_track) / bdb_heads

    pop ax
    mov dl, al
    pop ax

    ret



disk_read:
    push ax
    push bx
    push cx
    push dx
    push di

    call lba2chs
    mov ah, 2

    mov di, 3 ; Counter

retry:
    stc
    int 13h
    jnc doneRead

    call diskReset
    dec di

    test di, di
    jnz retry

failDiskRead:
    mov si, read_failure
    call print
    hlt
    jmp halt


diskReset:
    pusha
    mov ah, 0
    stc
    int 13h
    jc failDiskRead
    popa
    ret


doneRead:
    pop di
    pop dx
    pop cx
    pop bx
    pop ax

    ret


print:
    push si
    push ax
    push bx

print_loop:
    lodsb
    or al, al
    jz done_print

    mov ah, 0x0e
    mov bh, 0
    int 10h ; BIOS (=Basic I/O System) video interrupt

    jmp print_loop

done_print:
    pop bx
    pop ax
    pop si
    ret

; os_boot_msg:  db 'BIENVENUTO, PIZZA SPAGHETTI OPERATING SYSTEMME!!', 0x0d, 0x0a, 0
os_boot_msg:          db  'Loading PIZZOS...', 0x0d, 0x0a, 0
read_failure:         db  'Failed to read disk!',                             0x0d, 0x0a, 0
file_kernel_bin:      db  'KERNEL  BIN'
msg_kernel_not_found: db  'KERNEL.BIN Not found!'
kernel_cluster:       dw  0

kernel_load_segment   equ 0x2000
kernel_load_offset    equ 0

times 510-($-$$) db 0
dw 0aa55h

buffer: