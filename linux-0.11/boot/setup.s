INITSEG = 0x7c0
entry _start
_start:
    mov ah,#0x03        
    xor bh,bh
    int 0x10
    mov cx,#25  
    mov bx,#0x0007     
    mov bp,#msg1
    mov ax,cs
    mov es,ax
    mov ax,#0x1301   
    int 0x10
    ! set ds to where bootsect resided
    mov ax,#INITSEG
    mov ds,ax
    ! read cursor position
    mov ah,#0x03
    xor bh,bh
    int 0x10
    mov [0], dx
    ! read memory info
    mov ah,#0x88
    int #0x15
    mov [2],ax
    ! read disk info
    mov ax,#0x0000
    mov ds,ax
    lds si,[4 * 0x41]
    mov ax,#INITSEG
    mov es,ax
    mov di,#0x0004
    mov cx,#0x10
    rep
    movsb
msg1:
    .byte   13,10                     
    .ascii  "Now we are in SETUP"
    .byte   13,10,13,10
org 2046
    .word 0x0000
