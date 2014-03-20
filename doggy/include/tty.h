
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                tty.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    wjd,szh,zx,tr, 2007
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef	_TINIX_TTY_H_
#define	_TINIX_TTY_H_


#define TTY_IN_BYTES	256	/* tty input queue size */

/*要输出信息的控制台端口号*/
#define TTY_PORT int

struct s_tty;
struct s_console;

/* TTY */
typedef struct s_tty
{
	t_32	in_buf[TTY_IN_BYTES];	/* TTY 输入缓冲区 */
	t_32*	p_inbuf_head;		/* 指向缓冲区中下一个空闲位置 */
	t_32*	p_inbuf_tail;		/* 指向键盘任务应处理的键值 */
	int	inbuf_count;		/* 缓冲区中已经填充了多少 */
	t_32	cmd_buf[TTY_IN_BYTES];		/* tty command buff space */
	t_32*	p_cmdbuf_head;					/*	point to the next node */
	int	cmdbuf_count;					/* num of the character */

	struct s_console *	p_console;
}TTY;


#endif /* _TINIX_TTY_H_ */
