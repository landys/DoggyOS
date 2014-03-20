
######################
# Makefile for Tinix #
######################


# Entry point of Tinix
# It must be as same as 'KernelEntryPointPhyAddr' in load.inc!!!
ENTRYPOINT	= 0x30400
PAPOINT		= 0
PBPOINT		= 0
PCPOINT		= 0
PDPOINT		= 0

# Offset of entry point in kernel file
# It depends on ENTRYPOINT
ENTRYOFFSET	=   0x400
PAOFFSET	= 0x1000


# Programs, flags, etc.
ASM		= nasm
DASM		= ndisasm
CC		= gcc
LD		= ld
ASMBFLAGS	= -I boot/include/
ASMKFLAGS	= -I include/ -f elf
#CFLAGS		= -I include -c -fno-builtin
#LDFLAGS		= -s -Ttext $(ENTRYPOINT)
CFLAGS		= -I include -c -g -fno-builtin
LDFLAGS		= -s 
LDFLAGS_DEBUG = 
DASMFLAGS	= -u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)
PADASMFLAGS	= -u -o $(PAPOINT) -e $(PAOFFSET)

# This Program
DOGGYBOOT	= boot/boot.bin boot/loader.bin
DOGGYKERNEL	= kernel.bin
DOGGYKERNEL_DEBUG	= kernel.debug
PA	= user/pa
PB	= user/pb
PC	= user/pc
PD	= user/pd
OBJS		= kernel/kernel.o kernel/syscall.o kernel/start.o kernel/main.o\
			kernel/clock.o kernel/i8259.o kernel/global.o kernel/protect.o\
			kernel/proc.o kernel/keyboard.o kernel/tty.o kernel/console.o\
			kernel/shell.o kernel/vsprintf.o kernel/fork.o\
			kernel/memory.o lib/klib.o lib/klibc.o lib/string.o lib/dllLib.o
USEROBJS	= kernel/printf.o lib/util.o user/entry.o
PAOBJS		= user/pa.o $(USEROBJS)  lib/string.o kernel/syscall.o kernel/vsprintf.o
PBOBJS		= user/pb.o $(USEROBJS)  lib/string.o kernel/syscall.o kernel/vsprintf.o
PCOBJS		= user/pc.o $(USEROBJS)  lib/string.o kernel/syscall.o kernel/vsprintf.o
PDOBJS		= user/pd.o $(USEROBJS)  lib/string.o kernel/syscall.o kernel/vsprintf.o

ALLOBJS		= $(OBJS) $(USEROBJS) user/pa.o user/pb.o user/pc.o user/pd.o
DASMOUTPUT	= kernel.bin.asm

# All Phony Targets
.PHONY : everything final image clean realclean disasm all buildimg

# Default starting position
everything : $(DOGGYBOOT) $(DOGGYKERNEL) $(DOGGYKERNEL_DEBUG) $(PA) $(PB) $(PC) $(PD)

all : realclean everything

final : all clean

image : final buildimg

clean :
	rm -f $(ALLOBJS)

realclean :
	rm -f $(ALLOBJS) $(DOGGYBOOT) $(DOGGYKERNEL) $(PA) $(PB) $(PC) $(PD) $(DOGGYKERNEL_DEBUG)

disasm :
	$(DASM) $(DASMFLAGS) $(DOGGYKERNEL) > $(DASMOUTPUT)
padisasm :
	$(DASM) $(PADASMFLAGS) $(PA) > pa.bin.asm

# Write "boot.bin" & "loader.bin" into floppy image "DOGGY.IMG"
# We assume that "DOGGY.IMG" exists in current folder
buildimg :
	mount DOGGY.IMG /mnt/floppy -o loop
	cp -f boot/loader.bin /mnt/floppy/
	cp -f $(DOGGYKERNEL) /mnt/floppy
	cp -f $(PA) /mnt/floppy
	cp -f $(PB) /mnt/floppy
	cp -f $(PC) /mnt/floppy
	cp -f $(PD) /mnt/floppy
	umount  /mnt/floppy

boot/boot.bin : boot/boot.asm boot/include/load.inc boot/include/fat12hdr.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

boot/loader.bin : boot/loader.asm boot/include/load.inc boot/include/fat12hdr.inc boot/include/pm.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

$(DOGGYKERNEL) : $(OBJS)
	$(LD) -Ttext $(ENTRYPOINT) $(LDFLAGS) -o $(DOGGYKERNEL) $(OBJS)
	
$(DOGGYKERNEL_DEBUG) : $(OBJS)
	$(LD) -Ttext $(ENTRYPOINT) $(LDFLAGS_DEBUG) -o $(DOGGYKERNEL_DEBUG) $(OBJS)

$(PA) : $(PAOBJS)
	$(LD) -Ttext $(PAPOINT) $(LDFLAGS) -o $(PA) $(PAOBJS)

$(PB) : $(PBOBJS)
	$(LD) -Ttext $(PBPOINT) $(LDFLAGS) -o $(PB) $(PBOBJS)

$(PC) : $(PCOBJS)
	$(LD) -Ttext $(PCPOINT) $(LDFLAGS) -o $(PC) $(PCOBJS)

$(PD) : $(PDOBJS)
	$(LD) -Ttext $(PDPOINT) $(LDFLAGS) -o $(PD) $(PDOBJS)
	
kernel/kernel.o : kernel/kernel.asm include/sconst.inc
	$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/syscall.o : kernel/syscall.asm include/sconst.inc
	$(ASM) $(ASMKFLAGS) -o $@ $<

user/entry.o : user/entry.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

user/pa.o : user/pa.c include/unistd.h include/type.h include/const.h \
  include/printf.h include/util.h include/syscall.h
	$(CC) $(CFLAGS) -o $@ $<

user/pb.o : user/pb.c include/unistd.h include/type.h include/const.h \
  include/printf.h include/util.h include/syscall.h
	$(CC) $(CFLAGS) -o $@ $<

user/pc.o : user/pc.c include/unistd.h include/type.h include/const.h \
  include/printf.h include/util.h include/syscall.h
	$(CC) $(CFLAGS) -o $@ $<

user/pd.o : user/pd.c include/unistd.h include/type.h include/const.h \
  include/printf.h include/util.h include/syscall.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/shell.o: kernel/shell.c include/type.h include/const.h include/protect.h \
  include/string.h include/proc.h include/tty.h include/console.h \
  include/global.h include/keyboard.h include/proto.h include/shell.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/start.o: kernel/start.c include/type.h include/const.h include/protect.h include/string.h include/proc.h include/proto.h \
			include/tty.h include/console.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/main.o: kernel/main.c include/type.h include/const.h include/protect.h include/string.h include/proc.h include/proto.h \
			include/tty.h include/console.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/i8259.o: kernel/i8259.c include/type.h include/const.h include/protect.h include/string.h include/proc.h \
			include/tty.h include/console.h include/global.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/global.o: kernel/global.c include/type.h include/const.h include/protect.h include/proc.h \
			include/tty.h include/console.h include/global.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/protect.o: kernel/protect.c include/type.h include/const.h include/protect.h include/proc.h include/proto.h \
			include/tty.h include/console.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/clock.o: kernel/clock.c include/type.h include/const.h include/protect.h include/string.h include/proc.h \
			include/tty.h include/console.h include/global.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/proc.o: kernel/proc.c include/type.h include/const.h include/protect.h include/string.h include/proc.h include/proto.h \
			include/tty.h include/console.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/keyboard.o: kernel/keyboard.c include/type.h include/const.h include/protect.h include/string.h include/proc.h \
			include/tty.h include/console.h include/global.h include/keyboard.h include/keymap.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/tty.o: kernel/tty.c include/type.h include/const.h include/protect.h include/string.h include/proc.h \
			include/tty.h include/console.h include/global.h include/keyboard.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/console.o: kernel/console.c include/type.h include/const.h include/protect.h include/string.h include/proc.h \
			include/tty.h include/console.h include/global.h include/keyboard.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/vsprintf.o: kernel/vsprintf.c include/type.h include/const.h include/string.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/printf.o: kernel/printf.c include/type.h include/const.h include/printf.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/fork.o: kernel/fork.c include/type.h include/const.h include/protect.h include/string.h \
			include/proc.h include/tty.h include/console.h include/fork.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<
kernel/memory.o: kernel/memory.c 
	$(CC) $(CFLAGS) -o $@ $<
	
lib/util.o: lib/util.c include/type.h include/const.h include/util.h include/syscall.h
	$(CC) $(CFLAGS) -o $@ $<
	
lib/klibc.o: lib/klib.c include/type.h include/const.h include/protect.h include/string.h include/proc.h include/proto.h \
			include/tty.h include/console.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

lib/klib.o : lib/klib.asm include/sconst.inc
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/string.o : lib/string.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/dllLib.o : lib/dllLib.c
	$(CC) $(CFLAGS) -o $@ $<
