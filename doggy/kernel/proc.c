
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
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
#include "global.h"
#include "proto.h"

PUBLIC	PROCESS	proc_table[NR_PROCS];

PUBLIC	PROC_DESC	service_table[NR_SERVICES] = {{service_tty, STACK_SIZE_TTY, "tty"}};
PUBLIC	PROC_DESC	init_user_proc_table[NR_USER_PROCS] = {	{proc_init, STACK_SIZE_INIT, "proc_init"}/*,
						{0x100000, STACK_SIZE_INIT, "shell2"},
						{0x100000, STACK_SIZE_INIT, "shell3"}*/};

PUBLIC	char	proc_stack[STACK_SIZE_TOTAL];

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	PROCESS*	p;
	int		greatest_ticks = 0;

	while (!greatest_ticks) {
		for (p=proc_table; p<proc_table+NR_PROCS; p++) {
			/* PROC_RUNNING ¾ÍÐ÷×´Ì¬*/
			if (p->pid != -1 && p->state == PROC_RUNNING && p->ticks > greatest_ticks) {
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}

		if (!greatest_ticks) {
			for (p=proc_table; p<proc_table+NR_PROCS; p++) {
			  if (p->pid != -1 && p->state == PROC_RUNNING)
			    {
			  p->ticks = p->priority;
			    }
			}
		}
	}
}


/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}
