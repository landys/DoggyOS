/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                tty.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef	_TINIX_SHELL_H_
#define	_TINIX_SHELL_H_


#define SHELL_CMD_BYTES	256	/* tty input queue size */

t_32	in_buf[SHELL_CMD_BYTES];


#endif /* _TINIX_SHELL_H_ */
