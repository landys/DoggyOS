
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     wjd,szh,zx,tr, 2007
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

INT_VECTOR_SYS_CALL	equ	0x80
_NR_get_ticks		equ	0
_NR_write		equ	1
_NR_exec		equ	2
_NR_exit		equ	3
_NR_malloc		equ	4
_NR_free		equ	5
_NR_kill		equ	6
_NR_get_all_files	equ	7

; 导出符号
global	get_ticks
global	write
global	exec
global	_exit
global 	malloc
global	free
global	kill
global	get_all_files

bits 32
[section .text]

; 注意：dx 的值在 save() 中被改变，所以传递参数不能使用 edx！

; ====================================================================================
;                                    get_ticks
; ====================================================================================
get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	ret

; ====================================================================================
;                          void write(char* buf, int len);
; ====================================================================================
write:
	mov	eax, _NR_write
	mov	ebx, [esp + 4]
	mov	ecx, [esp + 8]
	int	INT_VECTOR_SYS_CALL
	ret

; ====================================================================================
;                          int exec(char* command);
; ====================================================================================
exec:
	mov	eax, _NR_exec
	mov	ebx, [esp + 4]
	int	INT_VECTOR_SYS_CALL
	ret
	
; ====================================================================================
;                          void _exit(int code);
; ====================================================================================
_exit:
	mov	eax, _NR_exit
	mov	ebx, [esp + 4]
	int	INT_VECTOR_SYS_CALL
	ret

; ====================================================================================
;                          int kill(int pid);
; ====================================================================================
kill:
	mov	eax, _NR_kill
	mov	ebx, [esp + 4]
	int	INT_VECTOR_SYS_CALL
	ret

; ====================================================================================
;                          int get_all_files(void* buf);
; ====================================================================================
get_all_files:
	mov	eax, _NR_get_all_files
	mov	ebx, [esp + 4]
	int	INT_VECTOR_SYS_CALL
	ret
	
malloc:
	mov	eax, _NR_malloc
	mov	ebx, [esp + 4]
	int	INT_VECTOR_SYS_CALL
	ret
free:
	mov	eax, _NR_free
	mov	ebx, [esp + 4]
	int	INT_VECTOR_SYS_CALL
	ret
	