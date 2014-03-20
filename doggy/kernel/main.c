
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    wjd,szh,zx,tr, 2007
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "memory.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "fork.h"
#include "global.h"
#include "proto.h"

EXTERN TTY_PORT kernelStdout;
EXTERN REGION memKernelReg;

/*======================================================================*
                            doggy_main
 *======================================================================*/
PUBLIC int doggy_main()
{
	PROC_DESC*		p_proc_desc;
	char*		p_proc_stack	= proc_stack + STACK_SIZE_TOTAL;
	//t_16		selector_ldt	= SELECTOR_LDT_FIRST; //初始化时是赋值所有进程表中的LDT选择子
	int		i;
	PROCESS* p_proc;

	// 初始化内存管理模块
	disp_str("Memory init begins......\n");
	STATUS status = memSysLibInit();
	if(status == OK)
	{
		disp_str("Memory init successfully.\n");
	}
	else
	{
		disp_str("Memory init failed.\n");
	}

	/*初始化进程和任务*/
	for (i=0; i<NR_PROCS; i++)
	  {
	    proc_table[i].pid = -1;
	  }
	
	for(i=0;i<NR_SERVICES+NR_INIT_USER_PROCS;i++){
		if (i < NR_SERVICES) {	/* 任务 */
			p_proc_desc		= service_table + i;
			p_proc_desc->privilege	= PRIVILEGE_TASK;
			p_proc_desc->rpl		= RPL_TASK;
			p_proc_desc->eflags		= 0x1202;	/* IF=1, IOPL=1, bit 2 is always 1 */
			p_proc_desc->priority = 15;
			p_proc_desc->ticks	= p_proc_desc->priority;
			p_proc_desc->nr_tty = 0;
			p_proc_desc->ppid = 0;
		}
		else {			/* 用户初始进程 */
			p_proc_desc		= init_user_proc_table + (i - NR_SERVICES);
			p_proc_desc->privilege	= PRIVILEGE_USER;
			p_proc_desc->rpl		= RPL_USER;
			p_proc_desc->eflags		= 0x202;	/* IF=1, bit 2 is always 1 */
			p_proc_desc->priority = 5;
			p_proc_desc->ticks	= p_proc_desc->priority;
			p_proc_desc->nr_tty = 0;
			p_proc_desc->ppid = 0;
		}
		//p_proc_desc->selector_ldt = selector_ldt;
		memcpy(&p_proc_desc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR));
		p_proc_desc->ldts[0].attr1 = DA_C | p_proc_desc->privilege << 5;	/* change the DPL */
		memcpy(&p_proc_desc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR));
		p_proc_desc->ldts[1].attr1 = DA_DRW | p_proc_desc->privilege << 5;/* change the DPL */

		p_proc_desc->p_proc_stack = p_proc_stack;
		
		p_proc = create_process(p_proc_desc);
		p_proc->state = PROC_RUNNING;
		
		p_proc_stack -= p_proc_desc->stacksize;
		//selector_ldt += 1 << 3;
	}
	
	k_reenter	= 0;
	ticks		= 0;

	p_proc_ready	= proc_table;

	kprintf(kernelStdout, "Clock init......\n");
	init_clock();
	/*内存信息调试*/
	//showMemRegStatus(&memKernelReg);
	
	/*进程到现在正式开始运行*/
	kprintf(kernelStdout, "Kernal init successfully.\n");

	//sys_exec("pa", 0, &proc_table[1]);

	kprintf(kernelStdout, "\nWelcome to Doggy.......\n");
	restart();

	while(1){}
}


/**
 * proc_init
 * 所有进程的父进程                             
 */
void proc_init()
{
	while(1){
		//printf("T");
		//milli_delay(800);
	}
}



