/*
 *  linux/kernel/switch_to.s
 *
 *  (C) 2020  Neolicad
 */

 /*
  * Switches processes using kernel stack.
  */

  KERNEL_STACK = 12

  .global switch_to,first_ret_from_kernel
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
    movl %esp,KERNEL_STACK(%eax)
    movl KERNEL_STACK(%ebx),%esp
  ! Switch LDT
    movl 12(%ebp),%ecx
    lldt %cx
  1: popl %ebp
    ret

/* Return to a fresh process created by fork. */
first_ret_from_kernel:
  popl %eax
  popl %ebp
  popl %edi
  popl %esi
  pop %gs
  popl %ebx
  popl %ecx
  popl %edx
  pop %fs
  pop %es
  pop %ds
  iret

