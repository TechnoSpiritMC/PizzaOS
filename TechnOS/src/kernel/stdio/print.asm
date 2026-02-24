BITS 16

section _TEXT class=CODE


global _x86_div64_32
_x86_div64_32:
    push bp
    mov bp, sp

    push bx

    mov eax, [bp+8]  ; This movest he upper 32 bits.
    mov ecx, [bp+12]
    xor edx, edx
    div ecx

    mov bx, [bp+16]  ; Upper 32 bits of teh quotient.
    mov [bx+4], eax

    mov eax, [bp+4]  ; Lower 32 bits of the divident.
    div ecx

    mov [bx], eax
    mov bx, [bp+18]
    mov [bx], edx

    pop bx
    mov sp, bp
    pop bp
    ret


global _x86_Video_WriteChatTeletype

_x86_Video_WriteChatTeletype:
    push bp
    mov bp, sp

    push bx

    mov ah, 0Eh
    mov al, [bp+4]
    mov bh, [bp+6]

    int 10h

    pop bx
    mov sp, bp
    pop bp

    ret