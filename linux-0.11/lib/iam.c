/*
 *  linux/lib/iam.c
 *  
 *  (C) 2019  neolicad 
 */


#define __LIBRARY__
#include <unistd.h>

_syscall1(int, iam, const char*, name)

_syscall2(int, whoami, char*, name, unsigned int, size)
