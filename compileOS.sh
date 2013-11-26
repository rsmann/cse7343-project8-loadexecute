# Compile Kernel
bcc -ansi -c -o kernel.o kernel.c
as86 kernel.asm -o kernel_asm.o
ld86 -o kernel -d kernel.o kernel_asm.o

# Compile Shell
#bcc -ansi -c -o shell.o shell.c
#as86 lib.asm -o lib.o
#ld86 -o shell -d shell.o lib.o

dd if=map.img of=floppya.img bs=512 count=1 seek=1 conv=notrunc
dd if=dir.img of=floppya.img bs=512 count=1 seek=2 conv=notrunc
dd if=kernel of=floppya.img bs=512 seek=3 conv=notrunc

# ./loadFile shell
./loadFile message.txt
./loadFile tstprg
./loadFile tstpr2

# bochs -f opsys.bxrc
