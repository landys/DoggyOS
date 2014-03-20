/**
* fork.c
* ���̴�������
* @author tony
*/
#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h" 
#include "memory.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "fork.h"
#include "global.h"
#include "proto.h"

PRIVATE long last_id = 0;
EXTERN REGION memKernelReg;
EXTERN TTY_PORT kernelStdout;

/**
* ��ȡ���еĽ��̺źͿ��еĽ��̱���
* last_id�ᱻ��Ϊ���еĽ��̺�
* @return ���̱���б����±�
*/
PRIVATE int find_empty_process()
{
	int i;
	while (1)
	{
		if (++last_id <= 0)
		{
			last_id = 1;
		}
		for (i=0; i<NR_PROCS; i++)
		{
			if (proc_table[i].pid == last_id)
			{
				break;
			}
		}
		if (i == NR_PROCS)
		{
			break;
		}
	}
	
	for (i=0; i<NR_PROCS; i++)
	{
		if (proc_table[i].pid == -1)
		{
			return i;
		}
	}
	return -1;
}

/**
* ����һ������
* @param p_proc_desc ����һ������
* @retrun ����ID
*/
PUBLIC PROCESS* create_process(PROC_DESC*  p_proc_desc)
{
	PROCESS* p_proc;
	int it;
	
	//disp_str("before find_empty_process\n");
	it = find_empty_process();
	//disp_int(it);
	//disp_str(" the index\n");
	if (it == -1)
	{
		return 0;
	}
	p_proc = &proc_table[it];

	strcpy(p_proc->name, p_proc_desc->name);	/* name of the process */
	p_proc->pid	= last_id;			/* pid */

	//p_proc->ldt_sel	= p_proc_desc->selector_ldt;
	memcpy(p_proc->ldts, p_proc_desc->ldts, sizeof(DESCRIPTOR)*LDT_SIZE);
	p_proc->regs.cs		= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | p_proc_desc->rpl;
	p_proc->regs.ds		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | p_proc_desc->rpl;
	p_proc->regs.es		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | p_proc_desc->rpl;
	p_proc->regs.fs		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | p_proc_desc->rpl;
	p_proc->regs.ss		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | p_proc_desc->rpl;
	p_proc->regs.gs		= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | p_proc_desc->rpl;
	p_proc->regs.eip	= (t_32)p_proc_desc->initial_eip;
	p_proc->regs.esp	= (t_32)p_proc_desc->p_proc_stack;
	p_proc->regs.eflags	= p_proc_desc->eflags;
	p_proc->ticks	= p_proc_desc->ticks;
	p_proc->priority	= p_proc_desc->priority;
	p_proc->nr_tty	= p_proc_desc->nr_tty;

	p_proc->state		= PROC_STOPPED;

	return p_proc;
}

/**
 * δʵ��
 */
PUBLIC int copy_process()
{
	return 0;
}

/**
 * ���ݽ���ID, �ҵ���Ӧ���̱��е�PCB��ָ��
 *
 * @param pid
 *		����ID
 * @return ��Ӧ���̱��е�PCB��ָ��
 */
PUBLIC PROCESS* find_process(int pid)
{
	int i;
	if (pid < 0)
	{
		return 0;
	}
	
	for (i=0; i<NR_PROCS; i++)
	{
		if (proc_table[i].pid == pid)
		{
			return &proc_table[i];
		}
	}
	return 0;
}

/**
 * @param command 
 *		�����µ��ļ���, "8�ֽ��ļ��� + 3�ֽ���չ�� + \0"
 *		���㲹�ո�, ��"kernel  bin\0", \0������ַ�
 * @return ����ID, -1��ʾʧ��
 */
PUBLIC int do_exec(char* command, int nouse, PROCESS* pp_proc)
{
	FILE_ITEM* p_file_item = (FILE_ITEM*)(find_floppy_file(command));
	void* base = 0;
	t_32 code_size = 0;
	t_32 mem_size = 0;
	PROC_DESC proc_desc;
	PROCESS* p_proc;

	//kprintf(kernelStdout,  "p_file_item = %x\n", p_file_item);
	if (p_file_item == 0)
	{
		/* δ�ҵ��ļ� */
		return -1;
	}
	//kprintf(kernelStdout,  "p_file_item->name = %s\n", p_file_item->name);
	//kprintf(kernelStdout,  "p_file_item->offset = %x\n", p_file_item->offset);
	//kprintf(kernelStdout,  "p_file_item->size = %x\n", p_file_item->size);
	
	mem_size = (p_file_item->size<USER_PROCESS_CODE_SIZE_LIMIT ? USER_PROCESS_MEM_SIZE : p_file_item->size + USER_PROCESS_MEM_SIZE-USER_PROCESS_CODE_SIZE_LIMIT);
	/* ��mem_size 4K����һ�� */
	mem_size = ((mem_size + 0xfff)  >> 12) << 12;
	//kprintf(kernelStdout,  "mem_size = %x\n", mem_size);
	//kprintf(kernelStdout,  "limit = %x\n", (mem_size>>12)-1);
	base = memRegAlloc(&memKernelReg, mem_size);
	//kprintf(kernelStdout,  "base = %x\n", (t_32)base);
	code_size = parse_elf(p_file_item->offset, base);
	//kprintf(kernelStdout,  "code_size = %x\n", code_size);
	/********************TODO****************************/
	proc_desc.initial_eip = (t_pf_proc)0;//base;	/*TODO�����Ժ�ɸĽ�*/
	proc_desc.stacksize = STACK_SIZE_USER_PROCESS;
	strcpy(proc_desc.name, command);
	proc_desc.privilege	= PRIVILEGE_USER;
	proc_desc.rpl = RPL_USER;
	proc_desc.eflags = 0x202;	/* IF=1, bit 2 is always 1 */
	proc_desc.priority = 5;
	proc_desc.ticks = proc_desc.priority;
	proc_desc.nr_tty = nr_current_console;
	proc_desc.ppid = pp_proc->pid;
	proc_desc.p_proc_stack = (void*)(mem_size -1);

	init_descriptor(&proc_desc.ldts[0], (t_32)base, (mem_size>>12)-1, DA_32 | DA_C | DA_LIMIT_4K | proc_desc.privilege << 5);
	init_descriptor(&proc_desc.ldts[1], (t_32)base, (mem_size>>12)-1, DA_32 | DA_DRW | DA_LIMIT_4K | proc_desc.privilege << 5);
	//init_descriptor(&proc_desc.ldts[0], 0, 0xfffff, DA_32 | DA_C | DA_LIMIT_4K | proc_desc.privilege << 5);
	//init_descriptor(&proc_desc.ldts[1], 0, 0xfffff, DA_32 | DA_DRW | DA_LIMIT_4K | proc_desc.privilege << 5);
	
	//memcpy(&proc_desc.ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR));
	//proc_desc.ldts[0].attr1 = DA_C | proc_desc.privilege << 5;	/* change the DPL */
	//memcpy(&proc_desc.ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR));
	//proc_desc.ldts[1].attr1 = DA_DRW | proc_desc.privilege << 5;/* change the DPL */
	
	p_proc = create_process(&proc_desc);
	/*
	kprintf(kernelStdout,  "p_proc->pid = %d\n", p_proc->pid);
	kprintf(kernelStdout,  "p_proc->ldt_sel = %d\n", p_proc->ldt_sel);
	kprintf(kernelStdout,  "p_proc->name = %s\n", p_proc->name);
	kprintf(kernelStdout,  "p_proc->ldts[0].limit_low = %x\n", p_proc->ldts[0].limit_low);
	kprintf(kernelStdout,  "p_proc->ldts[0].base_low = %x\n", p_proc->ldts[0].base_low);
	kprintf(kernelStdout,  "p_proc->ldts[0].base_mid = %x\n", p_proc->ldts[0].base_mid);
	kprintf(kernelStdout,  "p_proc->ldts[0].attr1 = %x\n", p_proc->ldts[0].attr1);
	kprintf(kernelStdout,  "p_proc->ldts[0].limit_high_attr2 = %x\n", p_proc->ldts[0].limit_high_attr2);
	kprintf(kernelStdout,  "p_proc->ldts[0].base_high = %x\n", p_proc->ldts[0].base_high);
	kprintf(kernelStdout,  "p_proc->ldts[1].limit_low = %x\n", p_proc->ldts[1].limit_low);
	kprintf(kernelStdout,  "p_proc->ldts[1].base_low = %x\n", p_proc->ldts[1].base_low);
	kprintf(kernelStdout,  "p_proc->ldts[1].base_mid = %x\n", p_proc->ldts[1].base_mid);
	kprintf(kernelStdout,  "p_proc->ldts[1].attr1 = %x\n", p_proc->ldts[1].attr1);
	kprintf(kernelStdout,  "p_proc->ldts[1].limit_high_attr2 = %x\n", p_proc->ldts[1].limit_high_attr2);
	kprintf(kernelStdout,  "p_proc->ldts[1].base_high = %x\n", p_proc->ldts[1].base_high);
	*/
	/*kprintf(kernelStdout,  "proc_table[1].ldts[0].limit_low = %x\n", proc_table[1].ldts[0].limit_low);
	kprintf(kernelStdout,  "proc_table[1].ldts[0].base_low = %x\n", proc_table[1].ldts[0].base_low);
	kprintf(kernelStdout,  "proc_table[1].ldts[0].base_mid = %x\n", proc_table[1].ldts[0].base_mid);
	kprintf(kernelStdout,  "proc_table[1].ldts[0].attr1 = %x\n", proc_table[1].ldts[0].attr1);
	kprintf(kernelStdout,  "proc_table[1].ldts[0].limit_high_attr2 = %x\n", proc_table[1].ldts[0].limit_high_attr2);
	kprintf(kernelStdout,  "proc_table[1].ldts[0].base_high = %x\n", proc_table[1].ldts[0].base_high);
	kprintf(kernelStdout,  "proc_table[1].ldts[1].limit_low = %x\n", proc_table[1].ldts[1].limit_low);
	kprintf(kernelStdout,  "proc_table[1].ldts[1].base_low = %x\n", proc_table[1].ldts[1].base_low);
	kprintf(kernelStdout,  "proc_table[1].ldts[1].base_mid = %x\n", proc_table[1].ldts[1].base_mid);
	kprintf(kernelStdout,  "proc_table[1].ldts[1].attr1 = %x\n", proc_table[1].ldts[1].attr1);
	kprintf(kernelStdout,  "proc_table[1].ldts[1].limit_high_attr2 = %x\n", proc_table[1].ldts[1].limit_high_attr2);
	kprintf(kernelStdout,  "proc_table[1].ldts[1].base_high = %x\n", proc_table[1].ldts[1].base_high);*/
	if (p_proc == 0)
	{
		return -1;
	}
	/******************************************************/
	p_proc ->region =  memCreateRegion(base+code_size, mem_size-code_size-STACK_SIZE_USER_PROCESS, (t_32)(base));
	if (p_proc ->region == 0)
	{
		// TODO
		kprintf(p_proc->nr_tty,  "Process %d create heap region error.\n", p_proc->pid);
	}
	p_proc->base = base;
	p_proc->state = PROC_RUNNING;

	return p_proc->pid;
}

/**
 * Сдת��д
 */
PRIVATE char toUpper(char c)
{
	char uc = c;
	if (c <= 'z' && c >='a')
	{
		uc = c - 32;
	}
	return uc;
}

/**
 * @param command 
 *		�����µ��ļ���, "<=8�ֽ��ļ���[. <=3�ֽ���չ��] + \0"
 *		���㲹�ո�, ��"kernel.bin\0", "zxsh\0", \0������ַ�, ȫ����д
 * @param flag
 * 		no use, Ŀǰδ��
 * @param p_proc
 *		��ǰ���н���PCB
 * @return ����ID, -1��ʾʧ��
 */
PUBLIC int sys_exec(char* command, int nouse, PROCESS* pp_proc)
{
	/* ��ת���ļ���Ϊ���̿���ʶ�����ʽ, ��:
	�����µ��ļ���, "8�ֽ��ļ��� + 3�ֽ���չ�� + \0"
 	���㲹�ո�, ��"kernel  bin\0", \0������ַ�*/

	int i, j;
	char fc[12];	/* ��ʽ��֮���command */

	memset(fc, ' ', 11);
	fc[11] = 0;
	for (i=0; i<8 && command[i]!=0 && command[i]!='.'; i++)
	{
		fc[i] = toUpper(command[i]);
	}
	if (command[i] == '.')
	{
		for (j=8, i++; j<11 && command[i]!=0; j++, i++)
		{
			fc[j] = toUpper(command[i]);
		}
	}

	do_exec(fc, nouse, pp_proc);
}

/**
 * �����˳�
 *
 * @param code
 *		�˳���
 */
PUBLIC void sys_exit(int code, int nouse, PROCESS* p_proc)
{
	int pid = p_proc->pid;
	p_proc->state = PROC_ZOMBIE;
	p_proc->pid = -1;
	if (memDestroyRegion(p_proc->region) != OK)
	{
		kprintf(p_proc->nr_tty, "Release process %d memory error.", p_proc->pid);
	}
	memRegFree(&memKernelReg, p_proc->base);
	
	schedule();
}

/**
 * ɱ��һ������
 *
 * @param pid
 *		����ID
 * @return 0-�ɹ�, 1-û���������, 2-û��Ȩ��
 */
PUBLIC int sys_kill(int pid, int nouse, PROCESS* p_proc)
{
	PROCESS* p;
	p = find_process(pid);
	if (p == 0)
	{
		return 1;
	}
	
	/* ĿǰTTY���丸�����йر��������̵�Ȩ�� */
	if (p_proc->pid != p->ppid && strcmp(p_proc->name, "tty") != 0)
	{
		return 2;
	}
	
	/* ���ܽ���TTY���� */
	if (strcmp(p->name, "tty") == 0)
	{
		return 2;
	}
	sys_exit(0, 0, p);
	
	return 0;
}


