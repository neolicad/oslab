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
# Rewrite esp0 in tss, which specifies the kernel stack for the current 
# process.
  movl tss,%ecx
  addl $4096,%ebx
  movl %ebx,ESP0(%ecx)
# Switch kernel stack
  movl %esp,KERNEL_STACK(%eax)
  # point ebx to task_struct
  movl current,%ebx
  movl KERNEL_STACK(%ebx),%esp
# Switch LDT
  movl 12(%ebp),%ecx
  lldt %cx
# TODO: %fs will be loaded in ret_from_sys_call, and so it seems unnecessay to 
# load %fs here. However, fork will fail during copy memory with error message 
# "copy_page_tables: already exist" if we don't load %fs here. Figure out why 
# after learning memory.
  movl $0x17,%ecx
  mov %cx,%fs
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

