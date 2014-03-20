#ifndef _FORK_H_
#define _FORK_H_


/**
* 创建一个进程
* @param p_proc_desc 描述一个进程
* @retrun 进程ID
*/
PUBLIC PROCESS* create_process(PROC_DESC* p_proc_desc);

/**
 * 根据进程ID, 找到对应进程表中的PCB项指针
 *
 * @param pid
 *		进程ID
 * @return 对应进程表中的PCB项指针
 */
PUBLIC PROCESS* find_process(int pid);

/**
 * @param command 
 *		软盘下的文件名, "8字节文件名 + 3字节扩展名 + \0"
 *		不足补空格, 如"kernel  bin\0", \0代表空字符
 * @return 进程ID, -1表示失败
 */
PUBLIC int do_exec(char* command, int nouse, PROCESS* pp_proc);

#endif

