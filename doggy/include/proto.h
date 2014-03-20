
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            proto.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    wjd,szh,zx,tr, 2007
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#ifndef _DOGGY_PROTO_H_
#define _DOGGY_PROTO_H_

/* klib.asm */
PUBLIC void	out_byte(t_port port, t_8 value);
PUBLIC t_8	in_byte(t_port port);
PUBLIC void	disable_int();
PUBLIC void	enable_int();
PUBLIC void	disp_str(char * info);
PUBLIC void	disp_color_str(char * info, int color);


/* protect.c */
PUBLIC void	init_prot();
PUBLIC t_32	seg2phys(t_16 seg);
PUBLIC void	disable_irq(int irq);
PUBLIC void	enable_irq(int irq);

/* klib.c */
PUBLIC t_bool	is_alphanumeric(char ch);
PUBLIC void	delay(int time);
PUBLIC char *	itoa(char * str, int num);
PUBLIC int strcmp(char* s1, char* s2);
PUBLIC int kprintf(int sysPort, const char *fmt, ...);
PUBLIC void init_descriptor(DESCRIPTOR * p_desc, t_32 base, t_32 limit, t_16 attribute);
PUBLIC t_32 desc2phys(DESCRIPTOR* p_desc);

/* kernel.asm */
PUBLIC void	restart();

/*main.c*/ 
PUBLIC void     proc_init();
/* PUBLIC void	TestA();
PUBLIC void	TestB();
PUBLIC void	TestC();
*/

/* i8259.c */
PUBLIC void	put_irq_handler(int iIRQ, t_pf_irq_handler handler);
PUBLIC void	spurious_irq(int irq);

/* clock.c */
PUBLIC void	clock_handler(int irq);
/*PUBLIC void	milli_delay(int milli_sec);*/

/* proc.c */
PUBLIC void	schedule();

/* keyboard.c */
PUBLIC void	keyboard_handler(int irq);
PUBLIC void	keyboard_read(TTY* p_tty);

/* tty.c */
PUBLIC void	service_tty();
PUBLIC void	in_process(TTY* p_tty, t_32 key);
PUBLIC void	tty_write(TTY* p_tty, char* buf, int len);

/* console.c */
PUBLIC void	init_screen(TTY* p_tty);
PUBLIC void	out_char(CONSOLE* p_con, char ch);
PUBLIC void	scroll_screen(CONSOLE* p_con, int direction);
PUBLIC t_bool	is_current_console(CONSOLE* p_con);
PUBLIC void console_write(CONSOLE* p_con, char* buf, int len);
PUBLIC void clean_all(CONSOLE* p_con);
PUBLIC void console_write_from_begin(CONSOLE* p_con, char* buf, int len);

/* shell.c */
PUBLIC void put_shell_cmd(t_32* str, int strCnt, CONSOLE* p_con);

/* printf.c 
PUBLIC	int	printf(const char *fmt, ...);*/

/* vsprintf.c */
PUBLIC	int	vsprintf(char *buf, const char *fmt, va_list args);
PUBLIC int sprintf(char *buf, const char *fmt, ...);



/************************************************************************/
/*                        以下是系统调用相关                            */
/************************************************************************/


/*------------*/
/* 系统级部分 */
/*------------*/

/* proc.c */
PUBLIC	int	sys_get_ticks	();
PUBLIC	int	sys_write	(char* buf, int len, PROCESS* p_proc);
/* fork.c */
/**
 * @param command 
 *		软盘下的文件名, "<=8字节文件名[. <=3字节扩展名] + \0"
 *		不足补空格, 如"kernel.bin\0", "zxsh\0", \0代表空字符
 * @param nouse
 * 		no use, 目前未用
 * @param p_proc
 *		当前运行进程PCB
 * @return 进程ID, -1表示失败
 */
 
PUBLIC int sys_exec(char* command, int nouse, PROCESS* p_proc);
/**
 * 进程退出
 *
 * @param code
 *		退出码
 */
 
PUBLIC void sys_exit(int code, int nouse, PROCESS* p_proc);
/**
 * 杀死一个进程
 *
 * @param pid
 *		进程ID
 * @return 0-成功, 1-没有这个进程, 2-没有权限
 */
PUBLIC int sys_kill(int pid, int nouse, PROCESS* p_proc);

/**
 * 获得所有文件信息
 *
 * @param buf
 *		获得的所有文件信息保存的地方
 * @return 总文件个数
 */
PUBLIC int sys_get_all_files(void* buf, int nouse, PROCESS* p_proc);

PUBLIC void* sys_malloc(unsigned nBytes, int nouse, PROCESS* p_proc);
PUBLIC void sys_free(void* freeAddress, int nouse, PROCESS* p_proc);

/* kernel.asm */
PUBLIC	void	sys_call();	/* t_pf_int_handler */
/*------------*/
/* 用户级部分 */
/*------------*/

/* syscall.asm */
/*PUBLIC	int	get_ticks();
PUBLIC	void	write(char* buf, int len);*/

#endif
