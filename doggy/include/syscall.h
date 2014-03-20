#ifndef _SYSCALL_H_
#define _SYSCALL_H_

PUBLIC int get_ticks();
PUBLIC void write(char* buf, int len);
/**
 * @param command 
 *		�����µ��ļ���, "<=8�ֽ��ļ���[. <=3�ֽ���չ��] + \0"
 *		���㲹�ո�, ��"kernel.bin\0", "zxsh\0", \0������ַ�, �Ե�һ������Ϊ��չ���ָ���
 * @param flag
 * 		ǰ��̨����, ���ȼ�֮�����Ϣ, Ŀǰδ��
 * @return ����ID, -1��ʾʧ��
 */
PUBLIC int exec(char* command);
/**
 * �����˳�
 *
 * @param code
 *		�˳���
 */
PUBLIC void _exit(int code);
/**
 * ɱ��һ������
 *
 * @param pid
 *		����ID
 * @return 0-�ɹ�, 1-û���������, 2-û��Ȩ��
 */
PUBLIC int kill(int pid);

/**
 * ��������ļ���Ϣ
 *
 * @param buf
 *		��õ������ļ���Ϣ����ĵط�
 * @return ���ļ�����
 */
PUBLIC int get_all_files(void* buf);

PUBLIC void* malloc(unsigned nBytes);
PUBLIC void free(void* freeAddress);
#endif
