; �ύ1: ȥ���˷�ҳ����->ע�͵���call SetupPaging, ���������ע��

; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               loader.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                    wjd,szh,zx,tr, 2007
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


org  0100h

	jmp	LABEL_START		; Start

; ������ FAT12 ���̵�ͷ, ֮���԰���������Ϊ�����õ��˴��̵�һЩ��Ϣ
%include	"fat12hdr.inc"
%include	"load.inc"
%include	"pm.inc"


; GDT ------------------------------------------------------------------------------------------------------------------------------------------------------------
;                                                �λ�ַ            �ν���     , ����					; �ν������ȿ�Ϊ�ֽڻ�4K, �λ�ַ����ֻ��Ϊ�ֽ�
LABEL_GDT:			Descriptor             0,                    0, 0					; ��������
LABEL_DESC_FLAT_C:		Descriptor             0,              0fffffh, DA_CR  | DA_32 | DA_LIMIT_4K		; 0 ~ 4G, DA_LIMIT_4K���öν�������Ϊ4K
LABEL_DESC_FLAT_RW:		Descriptor             0,              0fffffh, DA_DRW | DA_32 | DA_LIMIT_4K		; 0 ~ 4G
LABEL_DESC_VIDEO:		Descriptor	 0B8000h,               0ffffh, DA_DRW | DA_DPL3			; �Դ��׵�ַ, �ν�������Ϊ�ֽ�
; GDT ------------------------------------------------------------------------------------------------------------------------------------------------------------

GdtLen		equ	$ - LABEL_GDT
GdtPtr		dw	GdtLen - 1				; �ν���
		dd	BaseOfLoaderPhyAddr + LABEL_GDT		; ����ַ ��loader.���ص�λ�ü���LABEL_GDTƪ�Ƶ�ַ

; GDT ѡ���� ----------------------------------------------------------------------------------
SelectorFlatC		equ	LABEL_DESC_FLAT_C	- LABEL_GDT
SelectorFlatRW		equ	LABEL_DESC_FLAT_RW	- LABEL_GDT
SelectorVideo		equ	LABEL_DESC_VIDEO	- LABEL_GDT + SA_RPL3
; GDT ѡ���� ----------------------------------------------------------------------------------


BaseOfStack	equ	0100h


LABEL_START:			; <--- �����￪ʼ *************
	mov	ax, cs
	mov	ds, ax
	mov	es, ax
	mov	ss, ax
	mov	sp, BaseOfStack

	mov	dh, 0			; "Loading  "
	call	DispStrRealMode		; ��ʾ�ַ��� ʵģʽ�µ���ʾ����int10
	
; �õ��ڴ���	��ȡ�ڴ�Ĺ��������92ҳ
	mov	ebx, 0			; ebx = ����ֵ, ��ʼʱ��Ϊ 0
	mov	di, _MemChkBuf		; es:di ָ��һ����ַ��Χ�������ṹ��Address Range Descriptor Structure��
.MemChkLoop:
	mov	eax, 0E820h		; eax = 0000E820h
	mov	ecx, 20			; ecx = ��ַ��Χ�������ṹ�Ĵ�С
	mov	edx, 0534D4150h		; edx = 'SMAP'
	int	15h			; int 15h
	jc	.MemChkFail
	add	di, 20
	inc	dword [_dwMCRNumber]	; dwMCRNumber = ARDS �ĸ���
	cmp	ebx, 0			; ebxΪ0����ʾ�ڴ�Ķ����Ѿ�����
	jne	.MemChkLoop
	jmp	.MemChkOK
.MemChkFail:
	mov	dword [_dwMCRNumber], 0
.MemChkOK:

; �����ں˵��ڴ�
	mov	word [fileName], KernelFileName	; load kernel file, kernel.bin����0x80000��
	mov	word [baseOfFile], BaseOfKernelFile
	mov	word [offsetOfFile], OffsetOfKernelFile
	call	LoadFloppyFile
	cmp	ax, 1
	jz	LABLE_LOAD_KERNEL

	mov	dh, 2			; "No KERNEL."
	call	DispStrRealMode		; ��ʾ�ַ���
	jmp	$			; û���ҵ� KERNEL.BIN, ��ѭ��������
	
LABLE_LOAD_KERNEL:	
;*************************************************************************************
; ���濽��ָ���������ļ����ڴ�1M��, ����100000h���ĸ��ֽڴ����ļ���С, 
; 100004~100400�����ļ�Ŀ¼, ÿ����Ŀ20���ֽ�, 
; Ϊ�ļ���12�ֽ�, �ļ���С4�ֽ�, �ļ�ƫ��(�ڴ��еľ��Ե�ַ)4�ֽ�. 
; 100400����ʼ����ļ�����, ���ļ�16�ֽڶ���, ÿ���ļ���С��64K.
; ע�����, ��ʵģʽ��, ���ȿ���7e00h��, ������100000h, �����ṹһ��,
; Ȼ�����ڱ���ģʽ��ͨ��MoveExeFiles��������1M֮��, ��Ϊʵģʽ�޷�Ѱַ1M֮��Ŀռ�.
;*************************************************************************************
	mov	dword [allExeFileSize], 0	; ���ļ���С��ʼ��Ϊ0
	mov	word [offsetFileName], 4	; ǰ�ĸ��ֽڴ����ļ���С
ReadExeToMem:
	mov	bx, word [offsetFileNameStr]
	mov	al, byte [ExeFileName + bx]	; ����ļ���, ���жϵ�һ���ַ��Ƿ�Ϊ0
	cmp	al, 0
	jnz	LABEL_END_READ_RELAY	; ����û��ֱ����jz LABEL_END_READ��ԭ����
					; error: short jump is out of range
	jmp	LABEL_END_READ	; �������ļ�
LABEL_END_READ_RELAY:
	mov	ax, ExeFileName
	add	ax, [offsetFileNameStr]
	mov	word [fileName], ax	; ��ȡ�ļ�

	; �Ȱ�[allExeFileSize] 10h����
	mov	word [baseOfFile], BaseOfExeFile
	mov	eax, dword [allExeFileSize]	; eax=[allExeFileSize]
	add	eax, 0fh		; ��λ������, ��λʱ��һλ
	shr	eax, 4			; eax=([allExeFileSize]+0fh)/10h
	add	word [baseOfFile], ax	; [baseOfFile] = BaseOfExeFile + ([allExeFileSize]+0fh)/10h
	shl	eax, 4			; eax=([allExeFileSize]+0fh)/10h*10h
	mov	dword [allExeFileSize], eax	; [allExeFileSize]=([allExeFileSize]+0fh)/10h*10h
	mov	word [offsetOfFile], OffsetOfExeFile
	call	LoadFloppyFile
	; �ж��Ƿ��ȡ�ɹ�
	cmp	ax, 1
	jnz	LABLE_NOT_LOAD_FILE

	; ���㲢�����ļ��ܴ�С
	mov	eax, dword [allExeFileSize]
	add	eax, dword [exeFileSize]	; �����ļ��ܴ�С
	mov	dword [allExeFileSize], eax	; �����ļ��ܴ�С

	; �����ļ���, �ļ���С, �ļ�ƫ�Ƶ��ڴ���
	; ��ʼ��di, es, ��Щ���ݴ���[es:di]��
	mov	ax, BaseofFileName
	mov	es, ax		; BaseofFileName-->es
	mov	ax, word [offsetFileName]
	mov	di, ax		; [offsetFileName]-->di
	
	; �����ļ������ڴ�BaseofFileName(7e00h)֮��ĳ���ط�
	; ��ʼ��si
	mov	si, [fileName]
	mov	cx, SizeOfFileName	;����
LABEL_COPY_FILE_NAME:
	cmp	cx, 0
	jz	LABEL_END_COPY_FILE_NAME	; end of copy file name
	lodsb				; [ds:si] -> al 
	mov	byte [es:di], al	;[ds:si]-->[es:di]
	inc	di
	dec	cx
	jmp	LABEL_COPY_FILE_NAME
	
LABEL_END_COPY_FILE_NAME:
	; �����ļ���С���ڴ�
	mov	eax, dword [exeFileSize]
	mov	dword [es:di], eax
	add	di, 4

	; �����ļ�ƫ�Ƶ��ڴ�,
	; ([baseOfFile]-BaseOfExeFile) * 10h + ExeContentPhyAddr
	xor	eax, eax
	mov	ax, word [baseOfFile]
	sub	ax, BaseOfExeFile	
	shl	eax, 4	; ����ԭ�ļ������ڴ��ַ�����ƫ��,
	add	eax, ExeContentPhyAddr
	mov	dword [es:di], eax

	; [offsetFileName]ָ����һ�ļ�����
	add	word [offsetFileName], SizeOfFileItem
	
LABLE_NOT_LOAD_FILE:	
	; ȡ���ļ����ַ�������һ���ļ�����λ��
	add	word [offsetFileNameStr], SizeOfFileName
	
	jmp	ReadExeToMem	; �������ļ�
	
LABEL_END_READ:
	; ��ʼ��es, di
	mov	ax, BaseofFileName
	mov	es, ax
	mov	di, 0
	; �������ļ���С, [allExeFileSize]-->[es:di]
	mov	eax, dword [allExeFileSize]
	mov	dword [es:di], eax
	; ���ļ�Ŀ¼������(word [offsetFileName])�ĸ��ֽڶ���0(ʵ����һ��Ҳ����), ��Ϊ�ļ�Ŀ¼������־
	mov	di, word [offsetFileName]
	mov	dword [es:di], 0

	call	KillMotor		; �ر��������

	mov	dh, 1			; "Ready."
	call	DispStrRealMode		; ��ʾ�ַ���
;*****************************************************
; ����׼�����뱣��ģʽ -------------------------------------------

; ���� GDTR
	lgdt	[GdtPtr]

; ���ж�
	cli

; �򿪵�ַ��A20
	in	al, 92h
	or	al, 00000010b
	out	92h, al

; ׼���л�������ģʽ
	mov	eax, cr0
	or	eax, 1
	mov	cr0, eax

; �������뱣��ģʽ
	jmp	dword SelectorFlatC:(BaseOfLoaderPhyAddr+LABEL_PM_START)


;============================================================================
;����
;----------------------------------------------------------------------------
wRootDirSizeForLoop	dw	RootDirSectors	; Root Directory ռ�õ�������
wSectorNo		dw	0		; Ҫ��ȡ��������
bOdd			db	0		; ��������ż��
allExeFileSize		dd	0		; ���п�ִ���ļ��ܴ�С
exeFileSize		dd	0		; ��ִ���ļ���С
offsetFileName		dw 	0		; ��ִ���ļ���ƫ��
fileName		dw	0		; �ļ������Ե�ַ, ��KernelFileName
baseOfFile		dw	0		; �ļ������ص���λ�� ----  �ε�ַ
offsetOfFile		dw	0		; �ļ������ص���λ�� ---- ƫ�Ƶ�ַ

offsetFileNameStr	dw	0		; �ļ����ַ�����(ExeFileName)��ƫ��
;============================================================================
;�ַ���
;----------------------------------------------------------------------------
KernelFileName		db	"KERNEL  BIN", 0	; KERNEL.BIN ֮�ļ���
ExeFileName		db	"PA         ", 0, "PB         ", 0, "PC         ", 0, "PD         ", 0, "PE         ", 0, "PF         ", 0, "PG         ", 0, 0	; ����Ǹ�0�ǽ�����־
SizeOfFileName		equ	0ch	; ÿ���ļ���12���ֽ�
; Ϊ�򻯴���, ����ÿ���ַ����ĳ��Ⱦ�Ϊ MessageLength
MessageLength		equ	9
LoadMessage:		db	"Loading  "
Message1		db	"Ready.   "
Message2		db	"No KERNEL"
;============================================================================

;****************************************************************************
; ������: LoadFloppyFile
;****************************************************************************
; ���л���:
;	ʵģʽ
; ����:
;	�������϶�ȡָ�����ļ���ָ�����ڴ���

LoadFloppyFile:
	; ������ A �̵ĸ�Ŀ¼Ѱ���ļ�
	mov	word [wSectorNo], SectorNoOfRootDirectory	
	xor	ah, ah	; ��
	xor	dl, dl	; �� ������λ
	int	13h	; ��
	
	mov	word [wRootDirSizeForLoop], RootDirSectors	; ��Ŀ¼��������
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
	cmp	word [wRootDirSizeForLoop], 0	; ��
	jz	LABEL_NO_FILE		; �� �жϸ�Ŀ¼���ǲ����Ѿ�����, ��������ʾû���ҵ��ļ�
	dec	word [wRootDirSizeForLoop]	; ��
	mov	ax, [baseOfFile]
	mov	es, ax			; es <- BaseOfKernelFile
	mov	bx, [offsetOfFile]	; bx <- OffsetOfKernelFile	����, es:bx = BaseOfKernelFile:OffsetOfKernelFile = BaseOfKernelFile * 10h + OffsetOfKernelFile
	mov	ax, [wSectorNo]		; ax <- Root Directory �е�ĳ Sector ��
	mov	cl, 1
	call	ReadSector

	mov	si, [fileName]		; ds:si -> "KERNEL  BIN"
	mov	di, [offsetOfFile]	; es:di -> BaseOfKernelFile:???? = BaseOfKernelFile*10h+????
	cld
	mov	dx, 10h
LABEL_SEARCH_FOR_KERNELBIN:
	cmp	dx, 0					; ��
	jz	LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR	; �� ѭ����������, ����Ѿ�������һ�� Sector, ��������һ�� Sector
	dec	dx					; ��
	mov	cx, 11
LABEL_CMP_FILENAME:
	cmp	cx, 0			; ��
	jz	LABEL_FILENAME_FOUND	; �� ѭ����������, ����Ƚ��� 11 ���ַ������, ��ʾ�ҵ�
	dec	cx			; ��
	lodsb				; ds:si -> al
	cmp	al, byte [es:di]	; if al == es:di
	jz	LABEL_GO_ON
	jmp	LABEL_DIFFERENT
LABEL_GO_ON:
	inc	di
	jmp	LABEL_CMP_FILENAME	;	����ѭ��

LABEL_DIFFERENT:
	and	di, 0FFE0h		; else��	��ʱdi��ֵ��֪����ʲô, di &= e0 Ϊ�������� 20h �ı���
	add	di, 20h			;     ��
	mov	si, [fileName]	;     �� di += 20h  ��һ��Ŀ¼��Ŀ
	jmp	LABEL_SEARCH_FOR_KERNELBIN;   ��

LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
	add	word [wSectorNo], 1
	jmp	LABEL_SEARCH_IN_ROOT_DIR_BEGIN

LABEL_NO_FILE:
	mov	ax, 0			; û�ҵ��ļ�
	ret

LABEL_FILENAME_FOUND:			; �ҵ��ļ���������������
	mov	ax, RootDirSectors
	and	di, 0FFF0h		; di -> ��ǰ��Ŀ�Ŀ�ʼ

	push	eax
	mov	eax, [es : di + 01Ch]
	mov	dword [exeFileSize], eax		; �����ļ���С
	pop	eax

	add	di, 01Ah		; di -> �� Sector
	mov	cx, word [es:di]
	push	cx			; ����� Sector �� FAT �е����
	add	cx, ax
	add	cx, DeltaSectorNo	; ��ʱ cl ������ LOADER.BIN ����ʼ������ (�� 0 ��ʼ�������)
	mov	ax, [baseOfFile]	; �����������kernel�ڷŵ�λ��  0x8000:0x0000
	mov	es, ax			; es <- BaseOfKernelFile
	mov	bx, [offsetOfFile]	; bx <- OffsetOfKernelFile	����, es:bx = BaseOfKernelFile:OffsetOfKernelFile = BaseOfKernelFile * 10h + OffsetOfKernelFile
	mov	ax, cx			; ax <- Sector ��

LABEL_GOON_LOADING_FILE:
	push	ax			; ��
	push	bx			; ��
	mov	ah, 0Eh			; �� ÿ��һ���������� "Loading  " �����һ����, �γ�������Ч��:
	mov	al, '.'			; ��
	mov	bl, 0Fh			; �� Loading ......
	int	10h			; ��
	pop	bx			; ��
	pop	ax			; ��

	mov	cl, 1
	call	ReadSector
	pop	ax			; ȡ���� Sector �� FAT �е����
	call	GetFATEntry
	cmp	ax, 0FFFh
	jz	LABEL_FILE_LOADED
	push	ax			; ���� Sector �� FAT �е����
	mov	dx, RootDirSectors
	add	ax, dx
	add	ax, DeltaSectorNo
	add	bx, [BPB_BytsPerSec]
	jmp	LABEL_GOON_LOADING_FILE
LABEL_FILE_LOADED:
	mov	ax, 1
	ret


;----------------------------------------------------------------------------
; ������: DispStrRealMode
;----------------------------------------------------------------------------
; ���л���:
;	ʵģʽ������ģʽ����ʾ�ַ����ɺ��� DispStr ��ɣ�
; ����:
;	��ʾһ���ַ���, ������ʼʱ dh ��Ӧ�����ַ������(0-based)
DispStrRealMode:
	mov	ax, MessageLength
	mul	dh
	add	ax, LoadMessage
	mov	bp, ax			; ��
	mov	ax, ds			; �� ES:BP = ����ַ
	mov	es, ax			; ��
	mov	cx, MessageLength	; CX = ������
	mov	ax, 01301h		; AH = 13,  AL = 01h
	mov	bx, 0007h		; ҳ��Ϊ0(BH = 0) �ڵװ���(BL = 07h)
	mov	dl, 0
	add	dh, 3			; �ӵ� 3 ��������ʾ
	int	10h			; int 10h
	ret
;----------------------------------------------------------------------------
; ������: ReadSector
;----------------------------------------------------------------------------
; ����:
;	�����(Directory Entry �е� Sector ��)Ϊ ax �ĵ� Sector ��ʼ, �� cl �� Sector ���� es:bx ��
ReadSector:
	; -----------------------------------------------------------------------
	; �������������������ڴ����е�λ�� (������ -> �����, ��ʼ����, ��ͷ��)
	; -----------------------------------------------------------------------
	; ��������Ϊ x
	;                           �� ����� = y >> 1
	;       x           �� �� y ��
	; -------------- => ��      �� ��ͷ�� = y & 1
	;  ÿ�ŵ�������     ��
	;                   �� �� z => ��ʼ������ = z + 1
	push	bp
	mov	bp, sp
	sub	esp, 2			; �ٳ������ֽڵĶ�ջ���򱣴�Ҫ����������: byte [bp-2]

	mov	byte [bp-2], cl
	push	bx			; ���� bx
	mov	bl, [BPB_SecPerTrk]	; bl: ����
	div	bl			; y �� al ��, z �� ah ��
	inc	ah			; z ++
	mov	cl, ah			; cl <- ��ʼ������
	mov	dh, al			; dh <- y
	shr	al, 1			; y >> 1 (��ʵ�� y/BPB_NumHeads, ����BPB_NumHeads=2)
	mov	ch, al			; ch <- �����
	and	dh, 1			; dh & 1 = ��ͷ��
	pop	bx			; �ָ� bx
	; ����, "�����, ��ʼ����, ��ͷ��" ȫ���õ� ^^^^^^^^^^^^^^^^^^^^^^^^
	mov	dl, [BS_DrvNum]		; �������� (0 ��ʾ A ��)
.GoOnReading:
	mov	ah, 2			; ��
	mov	al, byte [bp-2]		; �� al ������
	int	13h
	jc	.GoOnReading		; �����ȡ���� CF �ᱻ��Ϊ 1, ��ʱ�Ͳ�ͣ�ض�, ֱ����ȷΪֹ

	add	esp, 2
	pop	bp

	ret

;----------------------------------------------------------------------------
; ������: GetFATEntry
;----------------------------------------------------------------------------
; ����:
;	�ҵ����Ϊ ax �� Sector �� FAT �е���Ŀ, ������� ax ��
;	��Ҫע�����, �м���Ҫ�� FAT �������� es:bx ��, ���Ժ���һ��ʼ������ es �� bx
GetFATEntry:
	push	es
	push	bx
	push	ax
	mov	ax, BaseOfKernelFile	; ��
	sub	ax, 0100h		; �� �� BaseOfKernelFile �������� 4K �ռ����ڴ�� FAT
	mov	es, ax			; ��
	pop	ax
	mov	byte [bOdd], 0
	mov	bx, 3
	mul	bx			; dx:ax = ax * 3
	mov	bx, 2
	div	bx			; dx:ax / 2  ==>  ax <- ��, dx <- ����
	cmp	dx, 0
	jz	LABEL_EVEN
	mov	byte [bOdd], 1
LABEL_EVEN:;ż��
	xor	dx, dx			; ���� ax ���� FATEntry �� FAT �е�ƫ����. ���������� FATEntry ���ĸ�������(FATռ�ò�ֹһ������)
	mov	bx, [BPB_BytsPerSec]
	div	bx			; dx:ax / BPB_BytsPerSec  ==>	ax <- ��   (FATEntry ���ڵ���������� FAT ��˵��������)
					;				dx <- ���� (FATEntry �������ڵ�ƫ��)��
	push	dx
	mov	bx, 0			; bx <- 0	����, es:bx = (BaseOfKernelFile - 100):00 = (BaseOfKernelFile - 100) * 10h
	add	ax, SectorNoOfFAT1	; �˾�ִ��֮��� ax ���� FATEntry ���ڵ�������
	mov	cl, 2
	call	ReadSector		; ��ȡ FATEntry ���ڵ�����, һ�ζ�����, �����ڱ߽緢������, ��Ϊһ�� FATEntry ���ܿ�Խ��������
	pop	dx
	add	bx, dx
	mov	ax, [es:bx]
	cmp	byte [bOdd], 1
	jnz	LABEL_EVEN_2
	shr	ax, 4
LABEL_EVEN_2:
	and	ax, 0FFFh

LABEL_GET_FAT_ENRY_OK:

	pop	bx
	pop	es
	ret
;----------------------------------------------------------------------------


;----------------------------------------------------------------------------
; ������: KillMotor
;----------------------------------------------------------------------------
; ����:
;	�ر��������
KillMotor:
	push	dx
	mov	dx, 03F2h
	mov	al, 0
	out	dx, al
	pop	dx
	ret
;----------------------------------------------------------------------------


; �Ӵ��Ժ�Ĵ����ڱ���ģʽ��ִ�� ----------------------------------------------------
; 32 λ�����. ��ʵģʽ���� ---------------------------------------------------------
[SECTION .s32]

ALIGN	32

[BITS	32]

LABEL_PM_START:
	mov	ax, SelectorVideo		; �Դ�ѡ����
	mov	gs, ax
	mov	ax, SelectorFlatRW		; ���ݶ�ѡ����
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	ss, ax
	mov	esp, TopOfStack			; ��ջ�������ݶε�ĩβ

	push	szMemChkTitle
	call	DispStr
	add	esp, 4

	call	DispMemInfo
	
	; ����������ѡ��ص��˷�ҳ���ƣ���Ҫ�ǿ��ǵ�Ҫģ��
	; VxWork�ж��ڴ�Ĺ���ģʽ: �ڱ���ģʽ�£����������������ڴ���й���
	;call	SetupPaging

	;mov	ah, 0Fh				; 0000: �ڵ�    1111: ����
	;mov	al, 'P'
	;mov	[gs:((80 * 0 + 39) * 2)], ax	; ��Ļ�� 0 ��, �� 39 �С�

	; �ƶ�һ�ѿ�ִ���ļ���1M֮��ĵط�
	call	MoveExeFiles
	
	push	BaseOfKernelFilePhyAddr	; ��ELF��ʽkernel.bin�����ڴ��н���������kernel
	call	InitFile
	add	esp, 4

	;jmp	$

	;***************************************************************
	push	dword [dwMemSize]		;����������Ҫ�õ��ڴ�Ĵ�С
	;mov	eax, [dwMemSize]	;����������Ҫ�õ��ڴ�Ĵ�С
	;push	eax
	jmp	SelectorFlatC:KernelEntryPointPhyAddr	; ��ʽ�����ں� *
	;***************************************************************
	; �ڴ濴��ȥ�������ģ�
	;              ��                                    ��
	;              ��                 .                  ��
	;              ��                 .                  ��
	;              ��                 .                  ��
	;              �ǩ�������������������������������������
	;              ����������������������������������������
	;              ��������������Page  Tables��������������
	;              ������������(��С��LOADER����)����������
	;    00101000h ���������������������������������������� PageTblBase
	;              �ǩ�������������������������������������
	;              ����������������������������������������
	;    00100000h ����������Page Directory Table���������� PageDirBase  <- 1M
	;              �ǩ�������������������������������������
	;              ����������������������������������������
	;       F0000h ����������������System ROM��������������
	;              �ǩ�������������������������������������
	;              ����������������������������������������
	;       E0000h ����������Expansion of system ROM ������
	;              �ǩ�������������������������������������
	;              ����������������������������������������
	;       C0000h ��������Reserved for ROM expansion������
	;              �ǩ�������������������������������������
	;              ���������������������������������������� B8000h �� gs
	;       A0000h ��������Display adapter reserved��������
	;              �ǩ�������������������������������������
	;              ����������������������������������������
	;  640k=9FC00h ������extended BIOS data area (EBDA)����
	;              �ǩ�������������������������������������
	;              ����������������������������������������
	;       90000h ����������������LOADER.BIN�������������� somewhere in LOADER �� esp
	;              �ǩ�������������������������������������
	;              ����������������������������������������
	;       80000h ����������������KERNEL.BIN��������������
	;              �ǩ�������������������������������������
	;              ����������������������������������������
	;       30000h ������������������KERNEL���������������� 30400h �� KERNEL ��� (KernelEntryPointPhyAddr)
	;              �ǩ�������������������������������������
	;              ��                                    ��
	;        7E00h ��              F  R  E  E            ��
	;              �ǩ�������������������������������������
	;              ����������������������������������������
	;        7C00h ��������������BOOT  SECTOR��������������
	;              �ǩ�������������������������������������
	;              ��                                    ��
	;         500h ��              F  R  E  E            ��
	;              �ǩ�������������������������������������
	;              ����������������������������������������
	;         400h ����������ROM BIOS parameter area ������
	;              �ǩ�������������������������������������
	;              ���������������������
	;           0h ���������Int  Vectors�������
	;              ���������������������������������������� �� cs, ds, es, fs, ss
	;
	;
	;		����������		����������
	;		���������� Tinixʹ��	���������� ����ʹ�õ��ڴ�
	;		����������		����������
	;		����������		����������
	;		��      �� δʹ�ÿռ�	������ ���Ը��ǵ��ڴ�
	;		����������		����������
	;
	; ע��KERNEL ��λ��ʵ�����Ǻ����ģ�����ͨ��ͬʱ�ı� LOAD.INC �е� KernelEntryPointPhyAddr �� MAKEFILE �в��� -Ttext ��ֵ���ı䡣
	;     ���磬����� KernelEntryPointPhyAddr �� -Ttext ��ֵ����Ϊ 0x400400���� KERNEL �ͻᱻ���ص��ڴ� 0x400000(4M) ��������� 0x400400��
	;




; ------------------------------------------------------------------------
; ��ʾ AL �е�����
; ------------------------------------------------------------------------
DispAL:
	push	ecx
	push	edx
	push	edi

	mov	edi, [dwDispPos]

	mov	ah, 0Fh			; 0000b: �ڵ�    1111b: ����
	mov	dl, al
	shr	al, 4			; ����ʾ��λ
	mov	ecx, 2
.begin:
	and	al, 01111b
	cmp	al, 9			; ��ʾ0-9
	ja	.1
	add	al, '0'
	jmp	.2
.1:					;��ʾ����A-F
	sub	al, 0Ah
	add	al, 'A'
.2:
	mov	[gs:edi], ax
	add	edi, 2

	mov	al, dl
	loop	.begin

	mov	[dwDispPos], edi

	pop	edi
	pop	edx
	pop	ecx

	ret
; DispAL ����-------------------------------------------------------------


; ------------------------------------------------------------------------
; ��ʾһ��������
; ------------------------------------------------------------------------
DispInt:
	mov	eax, [esp + 4]		; 32bit�����ݴӸ�λ����λ��ʾ
	shr	eax, 24
	call	DispAL

	mov	eax, [esp + 4]
	shr	eax, 16
	call	DispAL

	mov	eax, [esp + 4]
	shr	eax, 8
	call	DispAL

	mov	eax, [esp + 4]
	call	DispAL

	mov	ah, 07h			; 0000b: �ڵ�    0111b: ����
	mov	al, 'h'
	push	edi
	mov	edi, [dwDispPos]
	mov	[gs:edi], ax
	add	edi, 4
	mov	[dwDispPos], edi
	pop	edi

	ret
; DispInt ����------------------------------------------------------------

; ------------------------------------------------------------------------
; ��ʾһ���ַ���
; ------------------------------------------------------------------------
DispStr:
	push	ebp
	mov	ebp, esp
	push	ebx
	push	esi
	push	edi

	mov	esi, [ebp + 8]	; pszInfo
	mov	edi, [dwDispPos]
	mov	ah, 0Fh
.1:
	lodsb
	test	al, al			; �ַ����Ƿ��Ѿ�����
	jz	.2	
	cmp	al, 0Ah			; �ǻس���?
	jnz	.3
	push	eax			; ��ʾ�س�
	mov	eax, edi
	mov	bl, 160
	div	bl			; eax���ڴ�ŵ��ǹ�굱ǰ������
	and	eax, 0FFh
	inc	eax			; �򵥵�������1
	mov	bl, 160
	mul	bl
	mov	edi, eax		; ������ʵ�ǵõ�����µ�һ�е���ʼλ��
	pop	eax
	jmp	.1
.3:				; ��ʾ�����ַ�
	mov	[gs:edi], ax
	add	edi, 2
	jmp	.1

.2:
	mov	[dwDispPos], edi	; �������ڹ���λ��

	pop	edi
	pop	esi
	pop	ebx
	pop	ebp
	ret
; DispStr ����------------------------------------------------------------

; ------------------------------------------------------------------------
; ����
; ------------------------------------------------------------------------
DispReturn:
	push	szReturn
	call	DispStr			;printf("\n");
	add	esp, 4			;pop��ջ

	ret
; DispReturn ����---------------------------------------------------------


; ------------------------------------------------------------------------
; �ڴ濽������ memcpy
; ------------------------------------------------------------------------
; pDest* MemCpy(void* es:pDest, void* ds:pSrc, int iSize);
; ------------------------------------------------------------------------
MemCpy:
	push	ebp
	mov	ebp, esp

	push	esi
	push	edi
	push	ecx

	mov	edi, [ebp + 8]	; Destination  call������cs��eip��������ѹջ��������Ҫ��8
	mov	esi, [ebp + 12]	; Source
	mov	ecx, [ebp + 16]	; Counter
.1:
	cmp	ecx, 0		; �жϼ�����
	jz	.2		; ������Ϊ��ʱ����

	mov	al, [ds:esi]		; ��
	inc	esi			; ��
					; �� ���ֽ��ƶ�
	mov	byte [es:edi], al	; ��
	inc	edi			; ��

	dec	ecx		; ��������һ
	jmp	.1		; ѭ��
.2:
	mov	eax, [ebp + 8]	; ����ֵ

	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp

	ret			; ��������������
; MemCpy ����-------------------------------------------------------------




; ��ʾ�ڴ���Ϣ --------------------------------------------------------------
DispMemInfo:
	push	esi
	push	edi
	push	ecx

	mov	esi, MemChkBuf
	mov	ecx, [dwMCRNumber]	;for(int i=0;i<[MCRNumber];i++) // ÿ�εõ�һ��ARDS(Address Range Descriptor Structure)�ṹ
.loop:					;{
	mov	edx, 5			;	for(int j=0;j<5;j++)	// ÿ�εõ�һ��ARDS�еĳ�Ա����5����Ա
	mov	edi, ARDStruct		;	{			// ������ʾ��BaseAddrLow��BaseAddrHigh��LengthLow��LengthHigh��Type
.1:					;
	push	dword [esi]		;
	call	DispInt			;		DispInt(MemChkBuf[j*4]); // ��ʾһ����Ա
	pop	eax			;
	stosd				;		ARDStruct[j*4] = MemChkBuf[j*4];
	add	esi, 4			;
	dec	edx			;
	cmp	edx, 0			;
	jnz	.1			;	}
	call	DispReturn		;	printf("\n");
	cmp	dword [dwType], 1	;	if(Type == AddressRangeMemory) // AddressRangeMemory : 1, AddressRangeReserved : 2
	jne	.2			;	{
	mov	eax, [dwBaseAddrLow]	;
	add	eax, [dwLengthLow]	;
	cmp	eax, [dwMemSize]	;		if(BaseAddrLow + LengthLow > MemSize)
	jb	.2			;
	mov	[dwMemSize], eax	;	���ﱣ�����ڴ���ܴ�С			MemSize = BaseAddrLow + LengthLow;
.2:					;	}
	loop	.loop			;}
					;
	call	DispReturn		;printf("\n");
	push	szRAMSize		;
	call	DispStr			;printf("RAM size:");
	add	esp, 4			;
					;
	push	dword [dwMemSize]	;
	call	DispInt			;DispInt(MemSize);
	add	esp, 4			;

	pop	ecx
	pop	edi
	pop	esi
	ret
; ---------------------------------------------------------------------------

; ������ҳ���� --------------------------------------------------------------
SetupPaging:
	; �����ڴ��С����Ӧ��ʼ������PDE�Լ�����ҳ��
	xor	edx, edx
	mov	eax, [dwMemSize]
	mov	ebx, 400000h	; 400000h = 4M = 4096 * 1024, һ��ҳ���Ӧ���ڴ��С
	div	ebx
	mov	ecx, eax	; ��ʱ ecx Ϊҳ��ĸ�����Ҳ�� PDE Ӧ�õĸ���
	test	edx, edx
	jz	.no_remainder
	inc	ecx		; ���������Ϊ 0 ��������һ��ҳ��
.no_remainder:
	push	ecx		; �ݴ�ҳ�����

	; Ϊ�򻯴���, �������Ե�ַ��Ӧ��ȵ������ַ. ���Ҳ������ڴ�ն�.

	; ���ȳ�ʼ��ҳĿ¼
	mov	ax, SelectorFlatRW
	mov	es, ax
	mov	edi, PageDirBase	; �˶��׵�ַΪ PageDirBase
	xor	eax, eax
	mov	eax, PageTblBase | PG_P  | PG_USU | PG_RWW
.1:
	stosd
	add	eax, 4096		; Ϊ�˼�, ����ҳ�����ڴ�����������.
	loop	.1

	; �ٳ�ʼ������ҳ��
	pop	eax			; ҳ�����
	mov	ebx, 1024		; ÿ��ҳ�� 1024 �� PTE
	mul	ebx
	mov	ecx, eax		; PTE���� = ҳ����� * 1024
	mov	edi, PageTblBase	; �˶��׵�ַΪ PageTblBase
	xor	eax, eax
	mov	eax, PG_P  | PG_USU | PG_RWW
.2:
	stosd
	add	eax, 4096		; ÿһҳָ�� 4K �Ŀռ�
	loop	.2

	mov	eax, PageDirBase
	mov	cr3, eax
	mov	eax, cr0
	or	eax, 80000000h
	mov	cr0, eax
	jmp	short .3
.3:
	nop

	ret
; ��ҳ����������� ----------------------------------------------------------

; MoveExeFiles ----------------------------------------------------------------------
; �ƶ���ִ���ļ���1M֮��ĵط�
; BaseofFileName(7e00h)��ʼ400h�ֽڵ�ExeEntryPointPhyAddr(100000h), 
; BaseOfExeFile(8200h)��ʼ���ļ���С�ֽڵ�100400h��, �ܴ�С����7e00ǰ�ĸ��ֽڴ�
; �����ܵ���˵, BaseofFileName(7e00h)-->ExeEntryPointPhyAddr(100000h), �ܿ�����400h+���ļ���С�ֽ�
MoveExeFiles:
	mov	eax, dword [BaseofFileNamePhyAddr] ; �����ļ��ܴ�С-->eax
	add	eax, SizeOfAllFileItems		; Ŀ¼����С400h+���ļ���С�ֽ�-->eax
	push	eax				; �������ֽ���ѹջ
	mov	eax, BaseofFileNamePhyAddr	; ����Դ��ʼ��ַ-->eax	
	push	eax				; ����Դ��ʼ��ַѹջ
	mov	eax, ExeEntryPointPhyAddr	; ����Ŀ�ĵ���ʼ��ַ-->eax	
	push	eax				; ����Ŀ�ĵ���ʼ��ַѹջ
	call	MemCpy		; ִ�п���
	add	esp, 12		; �ָ���ջ

	ret


; InitFile ---------------------------------------------------------------------------------
; �� ĳ��elf�ļ� �����ݾ�����������ŵ��µ�λ�ã�����elf���ļ���ʽ��p150
; @param elf�ļ����ڵ��ڴ���ʼ��ַ, ����ģʽ������
; --------------------------------------------------------------------------------------------
InitFile:	; ����ÿһ�� Program Header������ Program Header �е���Ϣ��ȷ����ʲô�Ž��ڴ棬�ŵ�ʲôλ�ã��Լ��Ŷ��١�
	push	ebp
	push	ebx
	mov	ebp, esp
	mov	ebx, [ebp + 12]
	
	xor	esi, esi
	mov	cx, word [ebx + 2Ch]; �� ecx <- pELFHdr->e_phnum  program header table�е���Ŀ����
	movzx	ecx, cx					; ��
	mov	esi, [ebx + 1Ch]	; esi <- pELFHdr->e_phoff	program header table���ļ��е�ƫ��
	add	esi, ebx		; esi <- OffsetOfKernel + pELFHdr->e_phoff ָ�������elf�ļ���program header table
.Begin:
	mov	eax, [esi + 0]
	cmp	eax, 0				; PT_NULL
	jz	.NoAction
	push	dword [esi + 010h]		; size	��
	mov	eax, [esi + 04h]		;	��
	add	eax, ebx	;	�� ::memcpy(	(void*)(pPHdr->p_vaddr),
	push	eax				; src	��		uchCode + pPHdr->p_offset,
	push	dword [esi + 08h]		; dst	��		pPHdr->p_filesz;
	call	MemCpy				;	��
	add	esp, 12				;	������������ջ
.NoAction:
	add	esi, 020h			; esi += pELFHdr->e_phentsize 	program header table��ÿһ����Ŀ�Ĵ�С
	dec	ecx
	jnz	.Begin

	pop	ebx
	pop	ebp
	ret
; InitFile ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

; SECTION .data1 ֮��ʼ ---------------------------------------------------------------------------------------------
[SECTION .data1]

ALIGN	32

LABEL_DATA:
; ʵģʽ��ʹ����Щ����
; �ַ���
_szMemChkTitle:			db	"BaseAddrL BaseAddrH LengthLow LengthHigh   Type", 0Ah, 0
_szRAMSize:			db	"RAM size:", 0
_szReturn:			db	0Ah, 0
;; ����
_dwMCRNumber:			dd	0	; Memory Check Result
_dwDispPos:			dd	(80 * 6 + 0) * 2	; ��Ļ�� 6 ��, �� 0 �С�
_dwMemSize:			dd	0	; !!!!!!!!!!!!ע�����ﱣ�����ڴ�Ĵ�С�����ڴ��ʼ��Ҫ�õ�
_ARDStruct:			; Address Range Descriptor Structure
	_dwBaseAddrLow:		dd	0
	_dwBaseAddrHigh:	dd	0
	_dwLengthLow:		dd	0
	_dwLengthHigh:		dd	0
	_dwType:		dd	0
_MemChkBuf:	times	256	db	0
;
;; ����ģʽ��ʹ����Щ����
szMemChkTitle		equ	BaseOfLoaderPhyAddr + _szMemChkTitle
szRAMSize		equ	BaseOfLoaderPhyAddr + _szRAMSize
szReturn		equ	BaseOfLoaderPhyAddr + _szReturn
dwDispPos		equ	BaseOfLoaderPhyAddr + _dwDispPos
dwMemSize		equ	BaseOfLoaderPhyAddr + _dwMemSize
dwMCRNumber		equ	BaseOfLoaderPhyAddr + _dwMCRNumber
ARDStruct		equ	BaseOfLoaderPhyAddr + _ARDStruct
	dwBaseAddrLow	equ	BaseOfLoaderPhyAddr + _dwBaseAddrLow
	dwBaseAddrHigh	equ	BaseOfLoaderPhyAddr + _dwBaseAddrHigh
	dwLengthLow	equ	BaseOfLoaderPhyAddr + _dwLengthLow
	dwLengthHigh	equ	BaseOfLoaderPhyAddr + _dwLengthHigh
	dwType		equ	BaseOfLoaderPhyAddr + _dwType
MemChkBuf		equ	BaseOfLoaderPhyAddr + _MemChkBuf


; ��ջ�������ݶε�ĩβ
StackSpace:	times	1000h	db	0
TopOfStack	equ	BaseOfLoaderPhyAddr + $	; ջ��
; SECTION .data1 ֮���� ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

