
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                clock.c
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



/*======================================================================*
                           clock_handler
 *======================================================================*/
PUBLIC void clock_handler(int irq)
{
	//disp_str("#");
	ticks++;
	p_proc_ready->ticks--;

	if (k_reenter != 0) {
		//disp_str("!");
		return;
	}

	if (p_proc_ready->ticks > 0) {
		return;


	}

	schedule();
}


/*======================================================================*
                           init_clock
 *======================================================================*/
PUBLIC void init_clock()
{
	/* ��ʼ�� 8253 PIT */
	out_byte(TIMER_MODE, RATE_GENERATOR);
	out_byte(TIMER0, (t_8) (TIMER_FREQ/HZ) );
	out_byte(TIMER0, (t_8) ((TIMER_FREQ/HZ) >> 8));

	put_irq_handler(CLOCK_IRQ, clock_handler);	/* �趨ʱ���жϴ������ */
	enable_irq(CLOCK_IRQ);				/* ��8259A���Խ���ʱ���ж� */
}


