#ifndef _SYSCALL_H_
#define _SYSCALL_H_

PUBLIC int get_ticks();
PUBLIC void write(char* buf, int len);
/**
 * @param command 
 *		软盘下的文件名, "<=8字节文件名[. <=3字节扩展名] + \0"
 *		不足补空格, 如"kernel.bin\0", "zxsh\0", \0代表空字符, 以第一个点作为扩展名分隔符
 * @param flag
 * 		前后台运行, 优先级之类的信息, 目前未用
 * @return 进程ID, -1表示失败
 */
PUBLIC int exec(char* command);
/**
 * 进程退出
 *
 * @param code
 *		退出码
 */
PUBLIC void _exit(int code);
/**
 * 杀死一个进程
 *
 * @param pid
 *		进程ID
 * @return 0-成功, 1-没有这个进程, 2-没有权限
 */
PUBLIC int kill(int pid);

/**
 * 获得所有文件信息
 *
 * @param buf
 *		获得的所有文件信息保存的地方
 * @return 总文件个数
 */
PUBLIC int get_all_files(void* buf);

PUBLIC void* malloc(unsigned nBytes);
PUBLIC void free(void* freeAddress);
#endif
