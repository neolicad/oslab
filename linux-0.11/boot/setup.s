INITSEG = 0x7c0
entry _start
_start:
    ! set stack
    mov ax,cs
    mov ss,ax
    mov sp,#0xff00
    ! print "Now we are in SETUP"
    call read_cursor
    mov cx,#25  
    mov bp,#msg_in_setup
    call print_msg
    ! set ds to where bootsect resided
    mov ax,#INITSEG
    mov ds,ax
    ! read cursor position
    mov ah,#0x03
    xor bh,bh
    int 0x10
    mov [0],dx
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
    ! print info
    ! print cursor position
    call read_cursor
    mov cx,#11
    mov bp,#msg_cursor_pos
    call print_msg
    mov ax,#INITSEG
    mov ds,ax
    mov dx,[0]
    push dx
    call print_16
    pop dx
    call print_nl
    ! print memory size
    call read_cursor
    mov cx,#12
    mov bp,#msg_memory_size
    call print_msg
    mov dx,[2]
    push dx
    call print_16
    pop dx
    call read_cursor
    mov cx,#2
    mov bp,#msg_kb
    call print_msg
    call print_nl
    ! print hard disk cycles
    call read_cursor
    mov cx,#5
    mov bp,#msg_cyls
    call print_msg
    mov dx,[4]
    push dx
    call print_16
    pop dx
    call print_nl
    ! print hard disk heads
    call read_cursor
    mov cx,#6
    mov bp,#msg_heads
    call print_msg
    mov dx,[6]
    push dx
    call print_16
    pop dx
    call print_nl
    ! print hard disk sectors
    call read_cursor
    mov cx,#8
    mov bp,#msg_sectors
    call print_msg
    ! sectors info is in the last 2 bytes of hard disk parameter table
    mov dx,[18]
    push dx
    call print_16
    pop dx
    call print_nl
inf_loop:
    jmp inf_loop
read_cursor:
    mov ah,#0x03        
    xor bh,bh
    int 0x10
    ret
print_msg:
    mov bx,#0x0007     
    mov ax,cs
    mov es,ax
    mov ax,#0x1301 
    int 0x10
    ret

print_16: ! print a 16 bit hex
    ! read from stack
    mov bp,sp
    add bp,#2
    mov dx,(bp)
    mov cx,#4
print_digit:
    ! print 4 digit at a time
    rol dx,#4
    mov ax,#0xf
    mov bx,dx
    and bx,ax
    cmp bx,#0x9
    jg greater_than_9
    ! <= 9
    add bx,#0x30
    jmp outp
greater_than_9:
    add bx,#0x37
outp: ! call interrrupt to print a digit    
    mov ax,#0x0e00
    add ax,bx
    mov bx,#0x0007 
    int 0x10
    loop print_digit
    ret
print_nl:
! CR
    mov ax,#0xe0d     
    mov bx,#0x0007
    int 0x10
! LF
    mov al,#0xa     
    mov bx,#0x0007
    int 0x10
    ret
msg_in_setup:
    .byte   13,10                     
    .ascii  "Now we are in SETUP"
    .byte   13,10,13,10
msg_cursor_pos:
    .ascii "Cursor POS:"
msg_memory_size:
    .ascii "Memory size:"
msg_kb:
    .ascii "KB"
msg_cyls:
    .ascii "Cyls:"
msg_heads:
    .ascii "Heads:"
msg_sectors:
    .ascii "Sectors:"
org 2046
    .word 0x0000
