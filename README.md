# Compiler_translator
So, this is my language that i called puton which is actually looks like python, but it is compilable!   
The idea is that my program compiles => python can execute it   
Currently there is only int32 data type and no classes   
make to create puton.out   
puton.out (input filename) (option1) (option2) ...   
options:
* -h - show help
* -S - make assembler code instead of binary code
* -o (filename) - output filename
* -ast-dump - make graphviz dump of ast tree
* -Oreg - optimization - use registers most frequent used variables (but they will be saved to stack when other function is called)

c.sh - to compile .asm files with nasm
