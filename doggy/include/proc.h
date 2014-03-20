
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    wjd 2007
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef _DOGGY_PROC_H_
#define _DOGGY_PROC_H_

typedef struct s_stackframe 
{	/* proc_ptr points here				↑ Low			*/
	t_32	gs;		/* ┓						│			*/
	t_32	fs;		/* ┃						│			*/
	t_32	es;		/* ┃						│			*/
	t_32	ds;		/* ┃						│			*/
	t_32	edi;		/* ┃						│			*/
	t_32	esi;		/* ┣ pushed by save()				│			*/
	t_32	ebp;		/* ┃						│			*/
	t_32	kernel_esp;	/* <- 'popad' will ignore it			│			*/
	t_32	ebx;		/* ┃						↑栈从高地址往低地址增长*/		
	t_32	edx;		/* ┃						│			*/
	t_32	ecx;		/* ┃						│			*/
	t_32	eax;		/* ┛						│			*/
	t_32	retaddr;	/* return address for assembly code save()	│			*/
	t_32	eip;		/*  ┓						│			*/
	t_32	cs;		/*  ┃						│			*/
	t_32	eflags;		/*  ┣ these are pushed by CPU during interrupt	│			*/
	t_32	esp;		/*  ┃						│			*/
	t_32	ss;		/*  ┛						┷High			*/
}STACK_FRAME;


typedef struct s_proc 
{
	STACK_FRAME regs;	/* 进程当前寄存器表 */

	t_16	 ldt_sel;		/* 进程对应的LDT的选择子 */
	DESCRIPTOR ldts[LDT_SIZE];	/* 描述符表 */
	int ticks;		/* 时间片 */
	int priority;	/* 优先级 */
	t_32 pid;		/* 进程ID */
	t_32 ppid;	/* 进程父结点ID */
	char name[16];	/* 进程名 */
	int nr_tty;	/* TTY */
	int state;	/* 进程状态 */
	t_32 cids[16];	/* 子进程ID, 最16个儿子*/
	REGION* region;	/* 内存块*/
	void* base;	/* 进程块的起始绝对地址 */
}PROCESS;


/* 描述一个将要创建的进程 */
typedef struct s_proc_desc 
{
	t_pf_proc	initial_eip;	/* 进程代码段入口 */
	int		stacksize;	/* 进程栈大小 */
	char		name[32];	/* 进程名 */
	t_8		privilege;	/* 特权级 */
	t_8		rpl;			/* 特权属性 */
	int		eflags;		/* 相关属性 */
	t_16		selector_ldt;		/* LDT选择子 */
	void*	p_proc_stack;	/* 堆栈地址 */
	int		ticks;		/* 时间片 */
	int		priority;		/* 优先级 */
	int		nr_tty;		/* TTY */
	t_32		ppid;		/* 父进程ID */
	DESCRIPTOR ldts[LDT_SIZE];	/*  描述符表*/
}PROC_DESC;

/**
 * 1M内存中文件条目结构
 */
typedef struct s_file_item
{
	char name[12];	/* 软盘Fat12中的标准文件名*/
	t_32 size;		/* 文件大小 */
	void* offset;		/* 文件在内存中的绝对偏移 */
} FILE_ITEM;

/* state of process */
#define PROC_RUNNING		1
#define PROC_INTERRUPTIBLE	2
#define PROC_UNINTERRUPTIBLE	3
#define PROC_ZOMBIE		4
#define PROC_STOPPED		5

/* Number of certain processes */
#define NR_SERVICES		1
#define NR_USER_PROCS		16	/* 目前GDT描述符的个数为GDT_SIZE(128)个, 
								其中有5个用于非描述LDT, 这样进程数最多为:
								GDT_SIZE(128)-5-NR_SERVICES个, 这里为122个*/
#define NR_INIT_USER_PROCS	0
#define NR_PROCS	(NR_SERVICES + NR_USER_PROCS) 

#define USER_PROCESS_MEM_SIZE	0x100000	/*1M*/
/*512K,这个不是限制进程代码大小, 而是当它大于这个值时, 分配更大空间给进程*/
#define USER_PROCESS_CODE_SIZE_LIMIT	0x80000	

/* stacks of processes */
#define STACK_SIZE_TTY		0x8000	/*32K*/
#define STACK_SIZE_INIT		0x8000	/*32K*/
#define STACK_SIZE_USER_PROCESS	0x20000	/*128K*/
/*
#define STACK_SIZE_TESTC	0x8000
*/
#define STACK_SIZE_TOTAL	(STACK_SIZE_TTY + \
				STACK_SIZE_INIT * NR_INIT_USER_PROCS)/* + \
STACK_SIZE_USER_PROCESS * (NR_USER_PROCS - NR_INIT_USER_PROCS))*/

#endif
