
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
*kernel�����printf��Ҫָ����׼���
*/
PUBLIC int kprintf(int sysPort, const char *fmt, ...)
{
	int i;
	char buf[256];

	va_list arg = (va_list)((char*)(&fmt) + 4);        /* 4 �ǲ��� fmt ��ռ��ջ�еĴ�С */
	i = vsprintf(buf, fmt, arg);

	// ����ֻ��ѡ����disp_str
	buf[i] = '\0';
	if(isInitTty == TRUE)
	{/*tty�Ѿ���ʼ����ɵ����*/
		kernel_write(buf, i, sysPort);
	}
	else
	{// tty����û�г�ʼ�����֮ǰ������kernel_write
		disp_str(buf);
	}

	return i;
}

/**
 * �Ƚ����ַ����Ĵ�С, �ֵ���
 * @return 1 s1>s2; 0 s1=s2; -1 s1<s2
 */
PUBLIC int strcmp(char* s1, char* s2)
{
	int i;
	int r = 0;
	for (i=0; s1[i]!=0 && s1[i]==s2[i]; i++);	/* for�в����κβ���, ";"���Ǳ��ʧ�� */

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
 ��ʼ����������
 *======================================================================*/
PUBLIC void init_descriptor(DESCRIPTOR * p_desc, t_32 base, t_32 limit, t_16 attribute)
{
	p_desc->limit_low		= limit & 0x0FFFF;		// �ν��� 1		(2 �ֽ�)
	p_desc->base_low		= base & 0x0FFFF;		// �λ�ַ 1		(2 �ֽ�)
	p_desc->base_mid		= (base >> 16) & 0x0FF;		// �λ�ַ 2		(1 �ֽ�)
	p_desc->attr1			= attribute & 0xFF;		// ���� 1
	p_desc->limit_high_attr2	= ((limit >> 16) & 0x0F) |
						(attribute >> 8) & 0xF0;// �ν��� 2 + ���� 2
	p_desc->base_high		= (base >> 24) & 0x0FF;		// �λ�ַ 3		(1 �ֽ�)
}

/*======================================================================*
                           desc2phys
 *----------------------------------------------------------------------*
 ȡ�������Ļ�ַ
 *======================================================================*/
PUBLIC t_32 desc2phys(DESCRIPTOR* p_desc)
{
	return (p_desc->base_high << 24) | (p_desc->base_mid << 16) | (p_desc->base_low);
}

/**
 * ��1Mλ�ò����ļ�, ���ؾ��Ե�ַ
 * @param file_name �ļ���, �����ļ���ʽ, ��д, 12���ַ�, ǰ8�ļ���, ��3��չ��
 *		�����\0, �ļ�������չ�����Բ��㲹�ո�
 * @return elf�ļ�Ŀ¼��Ŀ��ַ, ÿ����Ŀ20���ֽ�:
 * 		�ļ���(12), �ļ���С(4), �ļ����ڴ��еľ��Ե�ַ(4)
 */
PUBLIC void* find_floppy_file(char* file_name)
{
	void* base = (void*)0x100000;
	char* pf = (char*)(base+4);		/* ����4�ֽڵ��ļ��ܴ�С*/
	while (*pf != 0 && strcmp(pf, file_name) != 0)
	{
		pf += 20;	/* ÿ���ļ�Ŀ¼20���ֽ� */
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
 * @param file_offset elf�ļ����ڴ��еľ��Ե�ַ
 * @param code_base ������Ĵ������ڴ��еľ��Ե�ַ
 * @param ������Ĵ�����С
 */
 PUBLIC int parse_elf(void* file_offset, void* code_base)
{
	int max_length = 0;
	int length = 0;
	t_16 e_phnum = *((t_32*)(file_offset + 0x2c));	/* program header table�е���Ŀ�� */
	t_32 e_phoff = *((t_32*)(file_offset + 0x1c));	/* program header table��ELF�ļ��е�ƫ��*/
	void* phoff_phy = file_offset + e_phoff;			/* program header table���ڴ��еľ��Ե�ַ */
	t_32 p_filesz;
	t_32 p_offset;
	t_32 p_vaddr;
	
	/*kprintf(kernelStdout,  "e_phnum = %x\n", e_phnum);
	kprintf(kernelStdout,  "e_phoff = %x\n", e_phoff);
	kprintf(kernelStdout,  "phoff_phy = %x\n", phoff_phy);*/

	while (e_phnum-- != 0)	/* ��ȡ���п� */
	{
		if (*((char*)(phoff_phy)) != 0)	/* �ж��Ƿ�Ϊ0, �����Ƿ��ȡ�ö�*/
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
 * ��������ļ���Ϣ
 *
 * @param buf
 *		��õ������ļ���Ϣ����ĵط�
 * @return ���ļ�����
 */
PUBLIC int sys_get_all_files(void* buf, int nouse, PROCESS* p_proc)
{
	void* base = (void*)0x100000;
	char* pf = (char*)(base+4);		/* ����4�ֽڵ��ļ��ܴ�С*/
	int n = -1;		/* �ļ����� */
	
	while (*(pf+(++n)*20) != 0);	/* ÿ���ļ�Ŀ¼20���ֽ� */

	memcpy(buf+desc2phys(&p_proc->ldts[1]), pf, n*20);

	return n;
}

