
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            klib.c
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

EXTERN BOOL isInitTty;
EXTERN TTY_PORT kernelStdout;

/*======================================================================*
                               disp_int
 *======================================================================*/
PUBLIC void disp_int(int input)
{
	char output[16];
	itoa(output, input);
	disp_str(output);
}

/*======================================================================*
                               delay
 *======================================================================*/
PUBLIC void delay(int time)
{
	int i, j, k;
	for(k=0;k<time;k++){
		/*for(i=0;i<10000;i++){	for Virtual PC	*/
		for(i=0;i<10;i++){/*	for Bochs	*/
			for(j=0;j<10000;j++){}
		}
	}
}

/**
*kernel级别的printf，要指定标准输出
*/
PUBLIC int kprintf(int sysPort, const char *fmt, ...)
{
	int i;
	char buf[256];

	va_list arg = (va_list)((char*)(&fmt) + 4);        /* 4 是参数 fmt 所占堆栈中的大小 */
	i = vsprintf(buf, fmt, arg);

	// 这里只能选择用disp_str
	buf[i] = '\0';
	if(isInitTty == TRUE)
	{/*tty已经初始化完成的情况*/
		kernel_write(buf, i, sysPort);
	}
	else
	{// tty任务还没有初始化完成之前不能用kernel_write
		disp_str(buf);
	}

	return i;
}

/**
 * 比较两字符串的大小, 字典序
 * @return 1 s1>s2; 0 s1=s2; -1 s1<s2
 */
PUBLIC int strcmp(char* s1, char* s2)
{
	int i;
	int r = 0;
	for (i=0; s1[i]!=0 && s1[i]==s2[i]; i++);	/* for中不需任何操作, ";"不是编程失误 */

	if (s1[i] < s2[i])
	{
		r = -1;
	}
	else if (s1[i] > s2[i])
	{
		r = 1;
	}

	return r;
}

/*======================================================================*
                           init_descriptor
 *----------------------------------------------------------------------*
 初始化段描述符
 *======================================================================*/
PUBLIC void init_descriptor(DESCRIPTOR * p_desc, t_32 base, t_32 limit, t_16 attribute)
{
	p_desc->limit_low		= limit & 0x0FFFF;		// 段界限 1		(2 字节)
	p_desc->base_low		= base & 0x0FFFF;		// 段基址 1		(2 字节)
	p_desc->base_mid		= (base >> 16) & 0x0FF;		// 段基址 2		(1 字节)
	p_desc->attr1			= attribute & 0xFF;		// 属性 1
	p_desc->limit_high_attr2	= ((limit >> 16) & 0x0F) |
						(attribute >> 8) & 0xF0;// 段界限 2 + 属性 2
	p_desc->base_high		= (base >> 24) & 0x0FF;		// 段基址 3		(1 字节)
}

/*======================================================================*
                           desc2phys
 *----------------------------------------------------------------------*
 取描述符的基址
 *======================================================================*/
PUBLIC t_32 desc2phys(DESCRIPTOR* p_desc)
{
	return (p_desc->base_high << 24) | (p_desc->base_mid << 16) | (p_desc->base_low);
}

/**
 * 在1M位置查找文件, 返回绝对地址
 * @param file_name 文件名, 软盘文件格式, 大写, 12个字符, 前8文件名, 再3扩展名
 *		最后是\0, 文件名和扩展名各自不足补空格
 * @return elf文件目录条目地址, 每个条目20个字节:
 * 		文件名(12), 文件大小(4), 文件在内存中的绝对地址(4)
 */
PUBLIC void* find_floppy_file(char* file_name)
{
	void* base = (void*)0x100000;
	char* pf = (char*)(base+4);		/* 跳过4字节的文件总大小*/
	while (*pf != 0 && strcmp(pf, file_name) != 0)
	{
		pf += 20;	/* 每个文件目录20个字节 */
	}

	if (*pf != 0)
	{
		return pf;
	}
	else
	{
		return 0;
	}
}
	
/**
 * @param file_offset elf文件在内存中的绝对地址
 * @param code_base 解析后的代码在内存中的绝对地址
 * @param 解析后的代码块大小
 */
 PUBLIC int parse_elf(void* file_offset, void* code_base)
{
	int max_length = 0;
	int length = 0;
	t_16 e_phnum = *((t_32*)(file_offset + 0x2c));	/* program header table中的条目数 */
	t_32 e_phoff = *((t_32*)(file_offset + 0x1c));	/* program header table在ELF文件中的偏移*/
	void* phoff_phy = file_offset + e_phoff;			/* program header table在内存中的绝对地址 */
	t_32 p_filesz;
	t_32 p_offset;
	t_32 p_vaddr;
	
	/*kprintf(kernelStdout,  "e_phnum = %x\n", e_phnum);
	kprintf(kernelStdout,  "e_phoff = %x\n", e_phoff);
	kprintf(kernelStdout,  "phoff_phy = %x\n", phoff_phy);*/

	while (e_phnum-- != 0)	/* 读取所有块 */
	{
		if (*((char*)(phoff_phy)) != 0)	/* 判断是否为0, 决定是否读取该段*/
		{
			p_filesz = *((t_32*)(phoff_phy + 0x10));	
			p_offset = *((t_32*)(phoff_phy + 0x4));
			p_vaddr = *((t_32*)(phoff_phy + 0x8));
			/*kprintf(kernelStdout,  "p_filesz = %x\n", p_filesz);
			kprintf(kernelStdout,  "p_offset = %x\n", p_offset);
			kprintf(kernelStdout,  "p_vaddr = %x\n", p_vaddr);*/
			memcpy(code_base+p_vaddr, file_offset+p_offset, p_filesz);
			/*kprintf(kernelStdout,  "code_base+p_vaddr = %x\n", code_base+p_vaddr);
			kprintf(kernelStdout,  "file_offset+p_offset = %x\n", file_offset+p_offset);
			kprintf(kernelStdout,  "first char of code_base+p_vaddr = %x\n", *((t_32*)(code_base+p_vaddr)));
			kprintf(kernelStdout,  "first char of file_offset+p_offset = %x\n", *((t_32*)(file_offset+p_offset)));*/
			length = p_filesz + p_vaddr;
			/*kprintf(kernelStdout,  "length = %x\n", length);
			kprintf(kernelStdout,  "max_length = %x\n", max_length);*/
			if (max_length < length)
			{
				max_length = length;
			}
		}
		phoff_phy += 0x20;
	}

	return max_length;
}

/**
 * 获得所有文件信息
 *
 * @param buf
 *		获得的所有文件信息保存的地方
 * @return 总文件个数
 */
PUBLIC int sys_get_all_files(void* buf, int nouse, PROCESS* p_proc)
{
	void* base = (void*)0x100000;
	char* pf = (char*)(base+4);		/* 跳过4字节的文件总大小*/
	int n = -1;		/* 文件个数 */
	
	while (*(pf+(++n)*20) != 0);	/* 每个文件目录20个字节 */

	memcpy(buf+desc2phys(&p_proc->ldts[1]), pf, n*20);

	return n;
}

