
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    wjd 2007
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef _DOGGY_PROC_H_
#define _DOGGY_PROC_H_

typedef struct s_stackframe 
{	/* proc_ptr points here				�� Low			*/
	t_32	gs;		/* ��						��			*/
	t_32	fs;		/* ��						��			*/
	t_32	es;		/* ��						��			*/
	t_32	ds;		/* ��						��			*/
	t_32	edi;		/* ��						��			*/
	t_32	esi;		/* �� pushed by save()				��			*/
	t_32	ebp;		/* ��						��			*/
	t_32	kernel_esp;	/* <- 'popad' will ignore it			��			*/
	t_32	ebx;		/* ��						��ջ�Ӹߵ�ַ���͵�ַ����*/		
	t_32	edx;		/* ��						��			*/
	t_32	ecx;		/* ��						��			*/
	t_32	eax;		/* ��						��			*/
	t_32	retaddr;	/* return address for assembly code save()	��			*/
	t_32	eip;		/*  ��						��			*/
	t_32	cs;		/*  ��						��			*/
	t_32	eflags;		/*  �� these are pushed by CPU during interrupt	��			*/
	t_32	esp;		/*  ��						��			*/
	t_32	ss;		/*  ��						��High			*/
}STACK_FRAME;


typedef struct s_proc 
{
	STACK_FRAME regs;	/* ���̵�ǰ�Ĵ����� */

	t_16	 ldt_sel;		/* ���̶�Ӧ��LDT��ѡ���� */
	DESCRIPTOR ldts[LDT_SIZE];	/* �������� */
	int ticks;		/* ʱ��Ƭ */
	int priority;	/* ���ȼ� */
	t_32 pid;		/* ����ID */
	t_32 ppid;	/* ���̸����ID */
	char name[16];	/* ������ */
	int nr_tty;	/* TTY */
	int state;	/* ����״̬ */
	t_32 cids[16];	/* �ӽ���ID, ��16������*/
	REGION* region;	/* �ڴ��*/
	void* base;	/* ���̿����ʼ���Ե�ַ */
}PROCESS;


/* ����һ����Ҫ�����Ľ��� */
typedef struct s_proc_desc 
{
	t_pf_proc	initial_eip;	/* ���̴������� */
	int		stacksize;	/* ����ջ��С */
	char		name[32];	/* ������ */
	t_8		privilege;	/* ��Ȩ�� */
	t_8		rpl;			/* ��Ȩ���� */
	int		eflags;		/* ������� */
	t_16		selector_ldt;		/* LDTѡ���� */
	void*	p_proc_stack;	/* ��ջ��ַ */
	int		ticks;		/* ʱ��Ƭ */
	int		priority;		/* ���ȼ� */
	int		nr_tty;		/* TTY */
	t_32		ppid;		/* ������ID */
	DESCRIPTOR ldts[LDT_SIZE];	/*  ��������*/
}PROC_DESC;

/**
 * 1M�ڴ����ļ���Ŀ�ṹ
 */
typedef struct s_file_item
{
	char name[12];	/* ����Fat12�еı�׼�ļ���*/
	t_32 size;		/* �ļ���С */
	void* offset;		/* �ļ����ڴ��еľ���ƫ�� */
} FILE_ITEM;

/* state of process */
#define PROC_RUNNING		1
#define PROC_INTERRUPTIBLE	2
#define PROC_UNINTERRUPTIBLE	3
#define PROC_ZOMBIE		4
#define PROC_STOPPED		5

/* Number of certain processes */
#define NR_SERVICES		1
#define NR_USER_PROCS		16	/* ĿǰGDT�������ĸ���ΪGDT_SIZE(128)��, 
								������5�����ڷ�����LDT, �������������Ϊ:
								GDT_SIZE(128)-5-NR_SERVICES��, ����Ϊ122��*/
#define NR_INIT_USER_PROCS	0
#define NR_PROCS	(NR_SERVICES + NR_USER_PROCS) 

#define USER_PROCESS_MEM_SIZE	0x100000	/*1M*/
/*512K,����������ƽ��̴����С, ���ǵ����������ֵʱ, �������ռ������*/
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
