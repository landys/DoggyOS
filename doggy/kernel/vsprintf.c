
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                              vsprintf.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    wjd,szh,zx,tr, 2007
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "string.h"

/*======================================================================*
                               is_alphanumeric
 *======================================================================*/
PUBLIC t_bool is_alphanumeric(char ch)
{
	return ((ch >= ' ') && (ch <= '~'));
}


/*======================================================================*
                               itoa
 *======================================================================*/
PUBLIC char * itoa(char * str, int num)/* ����ǰ��� 0 ������ʾ����, ���� 0000B800 ����ʾ�� B800 */
{
	char *	p = str;
	char	ch;
	int	i;
	t_bool	flag = FALSE;

	*p++ = '0';
	*p++ = 'x';

	if(num == 0){
		*p++ = '0';
	}
	else{	
		for(i=28;i>=0;i-=4){
			ch = (num >> i) & 0xF;
			if(flag || (ch > 0)){
				flag = TRUE;
				ch += '0';
				if(ch > '9'){
					ch += 7;
				}
				*p++ = ch;
			}
		}
	}

	*p = 0;

	return str;
}

/*======================================================================*
/**
*��һ������ת�����ַ���
*/
PUBLIC char * itoc(char * str, int num)
{
	char *p = str;
	char	ch;
	int seq = 1, i;
	
	if(num < 0)
	{
		*p++ = '-';
		num *= -1;
	}

	while(seq <= num + 1)
	{
		seq *= 10;
	}

	for(i = seq /10; i > 0; i = i /10)
	{
		*p++ = '0' + num / i;
		num %= i;
	}
	
	*p = '\0';
	
	return str;
}

/*
 *  Ϊ���õ����˺�����ԭ���ɲο� printf ��ע�Ͳ��֡�
 */

/*======================================================================*
                                vsprintf
 *======================================================================*/
PUBLIC int vsprintf(char *buf, const char *fmt, va_list args)
{
	char*	p;
	char	tmp[256];
	va_list	p_next_arg = args;

	for (p=buf;*fmt;fmt++) {
		if (*fmt != '%') {
			*p++ = *fmt;
			continue;
		}

		fmt++;

		switch (*fmt) {
		case 'x':
			itoa(tmp, *((int*)p_next_arg));
			strcpy(p, tmp);
			p_next_arg += 4;
			p += strlen(tmp);
			break;
		case 'd':
			itoc(tmp, *((int*)p_next_arg));
			strcpy(p, tmp);
			p_next_arg += 4;
			p += strlen(tmp);
			break;
		case 's':
			strcpy(p, *(char**)p_next_arg);
			p += strlen(*(char**)p_next_arg);
			p_next_arg += 4;
			break;
		default:
			break;
		}
	}
	*p = '\0';

	return (p - buf);
}

/*======================================================================*
                                 sprintf
 *======================================================================*/
PUBLIC int sprintf(char *buf, const char *fmt, ...)
{
	va_list arg = (va_list)((char*)(&fmt) + 4);        /* 4 �ǲ��� fmt ��ռ��ջ�еĴ�С */
	return vsprintf(buf, fmt, arg);
}


