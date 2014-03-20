/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    starmark, 2007
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
#include "shell.h"

/*extern data*/
EXTERN PROCESS	proc_table[NR_PROCS];

/* 本文件内函数声明 */
PRIVATE void show_login_user(CONSOLE* p_con_cur);
PRIVATE int shell_cmp(t_32* s1, char* s2, int strCnt);
PRIVATE char* t32_2_char(t_32* s, char* d, int n);
PRIVATE int exec_shell_cmd(char* cmd, CONSOLE* p_con);
PRIVATE void show_all_files(CONSOLE* p_con_cur);
PRIVATE void show_all_files_detail(CONSOLE* p_con_cur);
PRIVATE void show_all_runProcess(CONSOLE* p_con_cur);
PRIVATE void kill_process(CONSOLE* p_con_cur, int pid);

PRIVATE int cur_pid = -1;


/*======================================================================*
                          put_shell_cmd
 *======================================================================*/
PUBLIC void put_shell_cmd(t_32* str, int strCnt, CONSOLE* p_con){
	int i = 0, res = 1;
	t_32* p_head = str;
	char cmd[12];
	
	if(strCnt==0)
		return;

	memset(cmd, 0, 12);
	
	//CONSOLE* p_con = &console_table[0];
	for(; i<strCnt; i++){
		in_buf[i] = *p_head++;
		//out_char(p_con, in_buf[i]);
	}
	
	//now compare the user_input_word with the default command
	if(p_con->state==0){
		if((res = shell_cmp(in_buf,"root", strCnt))==1){
			//do with login work	
			p_con->state=1;
				//out_char(p_con, 'X');
				//strcpy(p_con->name, "root");
			strcpy(p_con->name, "root");
		}else if((res = shell_cmp(in_buf,"doggy", strCnt))==1){
			//do with login work	
			p_con->state=1;
				//out_char(p_con, 'X');
			strcpy(p_con->name, "doggy");
		}else if((res = shell_cmp(in_buf,"beggar", strCnt))==1){
			//do with login work	
			p_con->state=1;
				//out_char(p_con, 'X');
			strcpy(p_con->name, "beggar");
		}else{
			out_char(p_con, '\n');
			console_write_from_begin(p_con, "Not a login user name.", 100);
		}
	}else{
		//justify the shell command
		if((res = shell_cmp(in_buf,"clean", strCnt))==1){
			//do with clean work
			clean_all(p_con);	
		}else if((res = shell_cmp(in_buf,"lu", strCnt))==1){
			//show the user table]
			out_char(p_con, '\n');
			console_write_from_begin(p_con, "root doggy beggar\n", 100);	
		}else if((res = shell_cmp(in_buf,"who", strCnt))==1){
			out_char(p_con, '\n');
			show_login_user(p_con);
		}else if((res = shell_cmp(in_buf,"quit", strCnt))==1){
			p_con->state=0;
		} else if((res = shell_cmp(in_buf,"kill", strCnt))==1){
			kill_process(p_con, cur_pid);
		} else if((res = shell_cmp(in_buf,"ls", strCnt))==1){
			out_char(p_con, '\n');
			show_all_files(p_con);
		}else if((res = shell_cmp(in_buf,"ll", strCnt))==1){
			out_char(p_con, '\n');
			show_all_files_detail(p_con);
		}else if((res = shell_cmp(in_buf,"ps", strCnt))==1){
			out_char(p_con, '\n');
			show_all_runProcess(p_con);
		}else{
			//out_char(p_con, '\n');			
			exec_shell_cmd(t32_2_char(in_buf, cmd, strCnt<11?strCnt:11), p_con);
		}
	}
	
	return;
}

PRIVATE void kill_process(CONSOLE* p_con_cur, int pid)
{
	char buf[100];
	
	if (pid == -1)
	{
		sprintf(buf, "\nNo process is running.\n");
	}
	int flag = kill(pid);
	switch (flag)
	{
	case 0:
		sprintf(buf, "\n%d is killed.\n", pid);	
		break;
	case 1:
		sprintf(buf, "\n%d is not running now.\n", pid);	
		break;
	case 2:
		sprintf(buf, "\nNo right to kill %d.\n", pid);	
		break;
	default:
		sprintf(buf, "\nUnknown error to kill %d!\n", pid);
		break;
	}
	console_write(p_con_cur, buf, 100);
}

PRIVATE void show_login_user(CONSOLE* p_con_cur){
	CONSOLE* p_con;
	int id = 0;
	char buf[100];
	sprintf(buf, "console id     user name\n");
	console_write(p_con_cur, buf, 100);
	
	for (p_con=console_table;p_con<console_table+NR_CONSOLES;p_con++) {
		if(p_con->state==1){
			sprintf(buf, "%d              %s\n", id++, p_con->name);
		}else{
			sprintf(buf, "%d              %s\n", id++, "not login");
		}
		console_write(p_con_cur, buf, 100);
	}	
}

PRIVATE void show_all_files(CONSOLE* p_con_cur){
	FILE_ITEM files[10];
	char buf[100];
	int n = get_all_files((void*)files);
	int i;
	sprintf(buf, "Totally %d files:\n", n);
	console_write(p_con_cur, buf, 100);
	if (n > 0)
	{
		for (i=0; i<n; i++)
		{
			sprintf(buf, "%s   ", files[i].name);
			console_write(p_con_cur, buf, 100);
			if ((i+1) % 5 == 0)
			{
				out_char(p_con_cur, '\n');
			}
		}
	}
	out_char(p_con_cur, '\n');
}


PRIVATE void show_all_files_detail(CONSOLE* p_con_cur){
	FILE_ITEM files[10];
	char buf[100];
	int n = get_all_files((void*)files);
	int i;
	sprintf(buf, "Totally %d files:\n", n);
	console_write(p_con_cur, buf, 100);
	if (n > 0)
	{
		console_write(p_con_cur, "file name     file size     file offset\n", 100);
		for (i=0; i<n; i++)
		{
			sprintf(buf, "%s   %d          %x:\n", files[i].name, files[i].size, files[i].offset);
			console_write(p_con_cur, buf, 100);
		}
	}
}

PRIVATE void show_all_runProcess(CONSOLE* p_con_cur)
{
	char buf[80];
	int i;
	
	console_write(p_con_cur,  "PID     TTY     CMD\n", 100);
	
	for(i = 0; i < NR_PROCS; i++)
	{
		if(proc_table[i].pid != -1)
		{
			sprintf(buf," %d      %d     %s\n", proc_table[i].pid,  proc_table[i].nr_tty, proc_table[i].name);
			console_write(p_con_cur, buf, 100);
		}
	}
}

PRIVATE int exec_shell_cmd(char* cmd, CONSOLE* p_con){
	char buf[100];
	cur_pid = exec(cmd) ;
	if (cur_pid < 0)
	{
		sprintf(buf, "\n%s is not a usable command.", cmd);
		console_write(p_con, buf, 100);
	}
}

PRIVATE int shell_cmp(t_32* s1, char* s2, int strCnt)
{
	char ch;
	int res = 0;
	while(strCnt){
		if((ch=*s1)!=*s2)
		{
			res=1;
			break;
		}
		//out_char(p_con, *s1);
		//out_char(p_con, *s2);
		s1++;
		s2++;
		strCnt--;
	}
	if(res==0)	
		return 1;
	else
		return 0;
}

PRIVATE char* t32_2_char(t_32* s, char* d, int n)
{
	char* r = d;
	while(n--)
	{
		*(r++) = *(s++);
	}
	*r = 0;
	return d;
}

