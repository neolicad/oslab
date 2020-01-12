/*
 *  linux/kernel/switch_to.s
 *
 *  (C) 2020  Neolicad
 */

 /*
  * Switches processes using kernel stack.
  */

  .global switch_to
  switch_to:
    pushl %ebp
    movl %esp,%ebp
    movl 8(%ebp),%ebx
    cmpl %ebx,current
    je 1f
  ! Switch PCB
    movl %ebx,%eax
    xchgl %eax,current
  ! Rewrite tss
    ! For now I don't think we need this, skip 
  ! Switch kernel stack
    !TODO: implement
  ! Switch LDT
    movl 12(%ebp),%ecx
    lldt %cx
  1: movl %ebp,%esp
    popl %ebp
    ret
