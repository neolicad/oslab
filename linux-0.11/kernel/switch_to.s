/*
 *  linux/kernel/switch_to.s
 *
 *  (C) 2020  Neolicad
 */

 /*
  * Switches processes using kernel stack.
  */

KERNEL_STACK = 12
ESP0 = 4

.global switch_to_with_stack,first_ret_from_kernel
switch_to_with_stack:
  pushl %ebp
  movl %esp,%ebp
  movl 8(%ebp),%ebx
  cmpl %ebx,current
  je 1f
# Switch PCB
  movl %ebx,%eax
  xchgl %eax,current
# Rewrite esp0 in tss, which specifies the top of kernel stack to be used when 
# the next schedule happens
  movl tss,%ecx
  addl $4096,%ebx
  movl %ebx,ESP0(%ecx)
# Switch kernel stack
  movl %esp,KERNEL_STACK(%eax)
  movl KERNEL_STACK(%ebx),%esp
# Switch LDT
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

