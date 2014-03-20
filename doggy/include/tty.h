
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                tty.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    wjd,szh,zx,tr, 2007
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef	_TINIX_TTY_H_
#define	_TINIX_TTY_H_


#define TTY_IN_BYTES	256	/* tty input queue size */

/*Ҫ�����Ϣ�Ŀ���̨�˿ں�*/
#define TTY_PORT int

struct s_tty;
struct s_console;

/* TTY */
typedef struct s_tty
{
	t_32	in_buf[TTY_IN_BYTES];	/* TTY ���뻺���� */
	t_32*	p_inbuf_head;		/* ָ�򻺳�������һ������λ�� */
	t_32*	p_inbuf_tail;		/* ָ���������Ӧ����ļ�ֵ */
	int	inbuf_count;		/* ���������Ѿ�����˶��� */
	t_32	cmd_buf[TTY_IN_BYTES];		/* tty command buff space */
	t_32*	p_cmdbuf_head;					/*	point to the next node */
	int	cmdbuf_count;					/* num of the character */

	struct s_console *	p_console;
}TTY;


#endif /* _TINIX_TTY_H_ */
