
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               load.inc
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     wjd,szh,zx,tr, 2007
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


BaseOfLoader		equ	 09000h	; LOADER.BIN 被加载到的位置 ----  段地址
OffsetOfLoader		equ	  0100h	; LOADER.BIN 被加载到的位置 ---- 偏移地址

BaseOfLoaderPhyAddr	equ	BaseOfLoader * 10h	; LOADER.BIN 被加载到的位置 ---- 物理地址 (= BaseOfLoader * 10h)


BaseOfKernelFile	equ	 08000h	; KERNEL.BIN 被加载到的位置 ----  段地址
OffsetOfKernelFile	equ	     0h	; KERNEL.BIN 被加载到的位置 ---- 偏移地址

BaseOfKernelFilePhyAddr	equ	BaseOfKernelFile * 10h
KernelEntryPointPhyAddr	equ	030400h	; 注意：1、必须与 MAKEFILE 中参数 -Ttext 的值相等!!
					;       2、这是个地址而非仅仅是个偏移，如果 -Ttext 的值为 0x400400，则它的值也应该是 0x400400。

BaseofFileName		equ	07e0h	; 目录区, 初始文件名保存的内存地址, 7e0h*10h=7e00h
BaseofFileNamePhyAddr	equ	BaseofFileName * 10h
SizeOfFileItem		equ	14h	; 20个字节
SizeOfAllFileItems	equ	400h	; 目录区总大小
BaseOfExeFile		equ	 0820h	; Exe 被加载到的位置 ----  段地址, 820h*10h=8200h
BaseOfExeFilePhyAddr	equ	BaseOfExeFile * 10h
OffsetOfExeFile		equ	     0h	; Exe 被加载到的位置 ---- 偏移地址
ExeEntryPointPhyAddr	equ	0100000h	; 所有EXE文件在保护模式下重新被拷贝的地方
ExeContentPhyAddr	equ	0100400h	; 所有EXE文件内容在保护模式下重新被拷贝的地方


;BaseOfPaFile		equ	 04000h	; PA 被加载到的位置 ----  段地址
;OffsetOfPaFile		equ	     0h	; PA 被加载到的位置 ---- 偏移地址

;BaseOfPaFilePhyAddr	equ	BaseOfPaFile * 10h
;PaEntryPointPhyAddr	equ	0150000h

;BaseOfZxshFile		equ	 07E0h	; PA 被加载到的位置 ----  段地址
;OffsetOfZxshFile		equ	     0h	; PA 被加载到的位置 ---- 偏移地址

;BaseOfZxshFilePhyAddr	equ	BaseOfZxshFile * 10h
;ZxshEntryPointPhyAddr	equ	0100000h

PageDirBase		equ	200000h	; 页目录开始地址:		2M
PageTblBase		equ	201000h	; 页表开始地址:			2M + 4K

