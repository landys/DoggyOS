#ifndef _FORK_H_
#define _FORK_H_


/**
* ����һ������
* @param p_proc_desc ����һ������
* @retrun ����ID
*/
PUBLIC PROCESS* create_process(PROC_DESC* p_proc_desc);

/**
 * ���ݽ���ID, �ҵ���Ӧ���̱��е�PCB��ָ��
 *
 * @param pid
 *		����ID
 * @return ��Ӧ���̱��е�PCB��ָ��
 */
PUBLIC PROCESS* find_process(int pid);

/**
 * @param command 
 *		�����µ��ļ���, "8�ֽ��ļ��� + 3�ֽ���չ�� + \0"
 *		���㲹�ո�, ��"kernel  bin\0", \0������ַ�
 * @return ����ID, -1��ʾʧ��
 */
PUBLIC int do_exec(char* command, int nouse, PROCESS* pp_proc);

#endif

