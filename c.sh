#!/bin/bash
nasm -f elf64 -l $1.lst $1.asm
ld -s -o $1.out $1.o
