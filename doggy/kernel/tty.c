
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
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
#include "keyboard.h"
#include "proto.h"


#define TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)

/*系统要输出的标准控制台端口号*/
TTY_PORT kernelStdout = 0;

/*全局指示符，用来表示tty是否已经初始化成功*/
BOOL isInitTty = FALSE;

/* 本文件内函数声明 */
PRIVATE void	init_tty(TTY* p_tty);
PRIVATE void	tty_do_read(TTY* p_tty);
PRIVATE void	tty_do_write(TTY* p_tty);
PRIVATE void	put_key(TTY* p_tty, t_32 key);
PRIVATE void 	put_cmd_key(TTY* p_tty, t_32 key);


/*======================================================================*
                           service_tty
 *======================================================================*/
PUBLIC void service_tty()
{
	TTY*	p_tty;

	init_keyboard();

	for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
		init_tty(p_tty);
		p_tty->p_console->state = 0;
	}

	select_console(0);

	/*表示tty初始化成功*/
	isInitTty = TRUE;
	
	while (1) {
		for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
			tty_do_read(p_tty);
			tty_do_write(p_tty);
		}
	}
}


/*======================================================================*
                           init_tty
 *======================================================================*/
PRIVATE void init_tty(TTY* p_tty)
{
	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

	p_tty->p_cmdbuf_head=p_tty->cmd_buf;
	p_tty->cmdbuf_count	= 0;

	init_screen(p_tty);
}


/*======================================================================*
                           in_process
 *======================================================================*/
PUBLIC void in_process(TTY* p_tty, t_32 key)
{
	if (!(key & FLAG_EXT)) {
		put_key(p_tty, key);
		put_cmd_key(p_tty, key);
	}
	else {
		int raw_code = key & MASK_RAW;
		switch(raw_code) {
		case ENTER:
			put_key(p_tty, '\f');
			put_cmd_key(p_tty, '\f');
			break;
		case BACKSPACE:
			put_key(p_tty, '\b');
			put_cmd_key(p_tty, '\b');
			break;
		case UP:
			if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {	/* Shift + Up */
				scroll_screen(p_tty->p_console, SCROLL_SCREEN_UP);
			}
			break;
		case DOWN:
			if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {	/* Shift + Down */
				scroll_screen(p_tty->p_console, SCROLL_SCREEN_DOWN);
			}
			break;
		case F1:
		case F2:
		case F3:
		case F4:
		case F5:
		case F6:
		case F7:
		case F8:
		case F9:
		case F10:
		case F11:
		case F12:
			if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {	/* Alt + F1~F12 */
				select_console(raw_code - F1);
			}
			break;
		default:
			break;
		}
	}
}


/*======================================================================*
                              put_key
*======================================================================*/
PRIVATE void put_key(TTY* p_tty, t_32 key)
{
	if (p_tty->inbuf_count < TTY_IN_BYTES) {
		*(p_tty->p_inbuf_head) = key;
		p_tty->p_inbuf_head++;

		if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		p_tty->inbuf_count++;
	}
}

/*======================================================================*
                              put_cmd_key
*======================================================================*/
PRIVATE void put_cmd_key(TTY* p_tty, t_32 key)
{
	t_32* pChar = p_tty->cmd_buf;
	if (p_tty->cmdbuf_count < TTY_IN_BYTES) {
		if(key=='\f'){
			//put the command to shell
			// just for test
			put_shell_cmd(p_tty->cmd_buf, p_tty->cmdbuf_count, p_tty->p_console);
			p_tty->p_cmdbuf_head=p_tty->cmd_buf;
			p_tty->cmdbuf_count = 0;
		}else if(key=='\b'){
			//backspace
			if(p_tty->p_cmdbuf_head>p_tty->cmd_buf){
				p_tty->p_cmdbuf_head--;
				*(p_tty->p_cmdbuf_head)=' ';
				p_tty->cmdbuf_count--;
			}
		}else if(!(key & FLAG_EXT)){
			*(p_tty->p_cmdbuf_head) = key;
			p_tty->p_cmdbuf_head++;
			p_tty->cmdbuf_count++;
		}
		
	}
}


/*======================================================================*
                              tty_do_read
*======================================================================*/
PRIVATE void tty_do_read(TTY* p_tty)
{
	if (is_current_console(p_tty->p_console)) {
		keyboard_read(p_tty);
	}
}


/*======================================================================*
                              tty_do_write
*======================================================================*/
PRIVATE void tty_do_write(TTY* p_tty)
{
	if (p_tty->inbuf_count) {
		char ch = *(p_tty->p_inbuf_tail);
		p_tty->p_inbuf_tail++;
		if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		p_tty->inbuf_count--;

		//disp_int(ch);
		out_char(p_tty->p_console, ch);
	}
}


/*======================================================================*
                              tty_write
*======================================================================*/
PUBLIC void tty_write(TTY* p_tty, char* buf, int len)
{
	char* p = buf;
	int i = len;

	while (i) {
		out_char(p_tty->p_console, *p++);
		i--;
	}
}


/*======================================================================*
                              sys_write
*======================================================================*/
PUBLIC int sys_write(char* buf, int len, PROCESS* p_proc)
{
	tty_write(&tty_table[p_proc->nr_tty], buf+desc2phys(&p_proc->ldts[1]), len);
	return 0;
}

/**
* 系统级的控制台的输出,与系统调用的sys_write相对应
*/
PUBLIC int kernel_write(char* buf, int len, int sysPort)
{
	tty_write(&tty_table[sysPort], buf, len);
	return 0;
}
