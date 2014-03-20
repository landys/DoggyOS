#include "memory.h"
#include "dllLib.h"
#include "ktoollib.h"

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

EXTERN TTY_PORT kernelStdout;

/*ϵͳ���ڴ����region*/
REGION memKernelReg;

/*��־�Ƿ��ʼ�����ڴ��*/
BOOL isInitMemLib = FALSE;

/*�Ƿ������windows�µ��Եı�־*/
/*#define DUBUG_MEM 0*/

#ifdef DUBUG_MEM
/*���ڴ�����ĵ�ַ*/
EXTERN int mainMemEnd;
/*���ڴ濪ʼ�ĵ�ַ����Ĭ���ǿ�ʼ��ַ2M,  ע������������
*Ƕ�뵽osʱҪָ��ֵ = 0x200000
*/
EXTERN int mainMemStart;
#else
/*���ֵ�Ǵ�load.asm�õ���*/
extern int mem_size;

int mainMemEnd;
/*���ڴ�����ĵ�ַ*/
int mainMemStart = 0x200000;
#endif

/*forward function*/
PRIVATE STATUS memRegAttach(REGION* pRegion, void* poolStart, unsigned poolSize, unsigned relativeAddress);
PRIVATE BlockHdr* memRegBlockSplit(REGION* pRegionPart, BlockHdr*splitBlockHdr, unsigned long nBytes, unsigned aligened);
PRIVATE REGION* regAlloc();

/**
*���������Ҫ�ǳ�ʼ���ڴ�������,��Ҫ���������ڴ��
*�ռ�.
*@return 
*	��ʼ���ɹ�����OK
*/
PUBLIC STATUS memSysLibInit()
{
	STATUS initStatus;

	if(isInitMemLib == FALSE)
	{
		mainMemEnd = mem_size;

		kprintf(kernelStdout, "Memory begins: %x\nMemory ends: %x\nMemory pool size: %x\n", mainMemStart, mainMemEnd, mainMemEnd - mainMemStart);

		initStatus = memInitReg(&memKernelReg, 
				(void*) mainMemStart, mainMemEnd - mainMemStart, 0x0);

		isInitMemLib = TRUE;
	}	
	
	return (isInitMemLib && initStatus) ? OK : ERROR;
}

/**
*������ʼ��pRegion�ṹ������ĸ������ٰ�poolStart��ָ���
*��СΪpoolSize���ڴ�ռ��ʼ����ǰ������HDR, �м�FREEBLOCK����ʽ
*@param pRegion
*	Ҫ��ʼ�����ڴ����ṹ��
*@param poolStart
*	��������ڴ���ʼ��ַ, Ҫ��֤��MEM_ALIGN_SIZE����
*@param poolSize
*	��������ڴ�Ĵ�С, ����������л�����MEM_ALIGN_SIZE����Ĺ���
*/
PUBLIC STATUS memInitReg(REGION* pRegion, void* poolStart, unsigned poolSize, unsigned relativeAddress)
{
	char* poolEnd = (char*)poolStart + poolSize;
	/*step1: first clear up the pRegion mem*/
	memset(pRegion, 0, sizeof(REGION));

	/*step2: round down the poolSize*/
	poolSize = ROUND_DOWN(poolSize, MEM_ALIGN_SIZE);
	/*�����������ڴ��С����С�ڴ������ڴ��ַû��
	 * ����,�򷵻ش���
	 */
	if(poolSize < (sizeof(FreeBlock)  + 2 * sizeof(BlockHdr))
			|| !IS_ALIGNED(poolStart, MEM_ALIGN_SIZE))
	{
		return ERROR;
	}

	/*step3: init the freeList in the region*/
	dllInit(&(pRegion->freeList));

	/*step 4: attach the mem to region*/
	return memRegAttach(pRegion, (char*)poolStart, poolSize, relativeAddress);
}

/**
*�������pRegion���ڴ�����أ�������ɰ�poolStart��ָ���
*��СΪpoolSize���ڴ�ռ��ʼ����ǰ������HDR, �м�FREEBLOCK����ʽ
*@param pRegion
*	Ҫ�ҽӵ��ڴ����ṹ��
*@param poolStart
*	��������ڴ���ʼ��ַ, �Ѿ���֤��MEM_ALIGN_SIZE����
*@param poolSize
*	��������ڴ�Ĵ�С,�Ѿ���֤��MEM_ALIGN_SIZE����
*@param relativeAddress
*	�����region��Ҫ����Բ����ĵ�ַ
*@return 
*	OK ������ڳɹ�, ERROR�������ʧ��
*/
PRIVATE STATUS memRegAttach(REGION* pRegion, void* poolStart, unsigned poolSize, unsigned relativeAddress)
{
	char* poolEnd = (char*)poolStart + poolSize;
	unsigned midFreeBlockSize = poolSize - 2 * sizeof(BlockHdr);
	BlockHdr* headHdr;
	BlockHdr* midHdr;
	BlockHdr* tailHdr;
	LinkNode* midFreeBlock;

	/*preStep : init the memory with value 0*/
	memset(poolStart, 0, poolSize);
	
	/*step 1 : init the head HDR*/
	headHdr = (BlockHdr*)poolStart;
	headHdr->pPrevHdr = NULL;
	headHdr->nBytes = sizeof(BlockHdr);
	headHdr->isFree = FALSE;
	
	/*step 2: init the main FreeBlock*/
	/*��ʼ��HDR*/
	midHdr = NEXT_HDR(headHdr);
	midHdr->pPrevHdr = headHdr;
	midHdr->nBytes = midFreeBlockSize;
	midHdr->isFree = TRUE;
	/*��ʼ��FREEBLOCK, ������������б�*/
	midFreeBlock = HDR_TO_NODE(midHdr);
	dllInsert(&pRegion->freeList, (LinkNode*)NULL, midFreeBlock);
	
	/*step 3: init the tail HDR*/
	tailHdr = NEXT_HDR(midHdr);
	tailHdr->pPrevHdr = midHdr;
	tailHdr->nBytes = sizeof(BlockHdr);
	tailHdr->isFree = FALSE;
	
	/*step 4: init the REGION structure*/
	/*�����ϵͳ���ڴ���Ӧ������Դ�������Ҫ����Ϊ0*/
	pRegion->relativeAddress = relativeAddress;
	pRegion->startAddress = (unsigned)poolStart;
	pRegion->totalBytes = poolSize;
	pRegion->minBlockBytes = ROUND_UP(sizeof(FreeBlock) - sizeof(BlockHdr), MEM_ALIGN_SIZE);

	/* statistic information */
	pRegion->curBlocksAllocated = 0;
	pRegion->curBytesAllocated = 0;
	pRegion->cumBlocksAllocated = 0;
	pRegion->cumBytesAllocated = 0;
	return OK;
}

/**
*�������������pRegionPart��ָ����ڴ���������СΪnBytes
*���ڴ�����.ʵ�ʷ���Ĵ�С���ܱ��û�Ҫ��Ĵ�СҪ��,�Ӷ�
*Ҳ�����˷���ʧ�ܵĿ�����.
*@param pRegionPart
*	����ռ��REGIONָ��
*@param nBytes
*	�û�Ҫ�����Ŀռ�Ĵ�С(������HDR�Ĵ�С),
*     �����С���ܻ���Ĭ�϶������
*@return 
*	NULL ����ʧ��, ���򷵻ط���õ����û�
*/
PUBLIC void* memRegAlloc(REGION* pRegionPart, unsigned long nBytes)
{
	return memRegAlignedAlloc(pRegionPart, nBytes, MEM_ALIGN_SIZE);
}

/**
*�������������pRegionPart��ָ����ڴ���������СΪnBytes
*���ڴ�����.ʵ�ʷ����лᰴaligened��ĩβ��ַ���ж���.�������
*��Ҫ���ҵ����㹻�ռ��FREEBLOCK,����ķ�������memRegBlockSplit
*����ɵ�
*@param pRegionPart
*	����ռ��REGIONָ��
*@param nBytes
*	�û�Ҫ�����Ŀռ�Ĵ�С
*@param aligened
*	�������ʼ��ַҪ�������Ŀ
*@return 
*	NULL ����ʧ��, ���򷵻ط���õ����û�
*/
PUBLIC void* memRegAlignedAlloc(REGION* pRegionPart, unsigned long nBytes, unsigned aligened)
{
	/*
	*����nBytesҪ��MEM_ALIGN_SIZE����,�������Ա�֤BLOCKHDR��ǰ��Ŀ�
	*֮��û�м�϶
	*/
	unsigned long realBytes;		/*ʵ�ʷ�����û���bytes��*/
	unsigned long totalBytes;		/*ϵͳ�ܹ�Ҫ�����bytes��,������HDR*/
	unsigned long tryTotalBytes;	/*��freelist����ʱƥ���bytes��*/
	LinkNode* scanFreeBlock;
	BlockHdr* pFindedBlock;
	BOOL isFind = FALSE;

	/*����û�Ҫ�����Ŀռ�С����СӦ�÷���Ŀռ�,�������С�ռ�*/
	if(nBytes < pRegionPart->minBlockBytes)
	{
		nBytes = pRegionPart->minBlockBytes;
	}
	realBytes =  ROUND_UP(nBytes, MEM_ALIGN_SIZE) ;
	totalBytes = realBytes + sizeof(BlockHdr);
	tryTotalBytes = totalBytes + aligened;
	scanFreeBlock = LL_FIRST(&(pRegionPart->freeList));
	
	/*���whileѭ����Ҫ���ҵ����㹻�ռ���Էָ��freeblock*/
	while(scanFreeBlock)
	{
		BlockHdr *pNewHdr, *pNewNextHdr;
		/*�Ѿ��������б�β�������*/
		if(scanFreeBlock == NULL)
		{
			break;
		}

		pNewHdr = NODE_TO_HDR(scanFreeBlock);
		if(pNewHdr->nBytes >= tryTotalBytes ||
				(IS_ALIGNED( (unsigned)HDR_TO_BLOCK(NODE_TO_HDR(scanFreeBlock)), aligened) && pNewHdr->nBytes == totalBytes))
		{/*�ҵ��˿��Է��ѵ�block*/
			pFindedBlock = memRegBlockSplit(pRegionPart, pNewHdr, realBytes, aligened);

			if(pFindedBlock != NULL)
			{
				isFind = TRUE;
				break;
			}
		}

		scanFreeBlock = LL_NEXT(scanFreeBlock);
	}
	/*�������ɹ�������ʱҪ�����Ե�ַת������Ե�ַ*/
	return isFind ? PHY_TO_REL_ADD(pRegionPart, HDR_TO_BLOCK(pFindedBlock)) : NULL;
}

/**
* �������ʵ�ʽ�һ��Ҫ�����block��һ��freeblock�зָ��ȥ
*������ͼ�ϲ�����ǰ���.�˺����ĵ�����Ҫ��֤���Է���
*���ڱ�Ȼ��.
*@param pRegionPart
*	����ռ��REGIONָ��
*@param tryBlockHdr
*	���Է��ѵ�FREEBLOCK��BlockHdr
*@param nBytes
*	��ͼ����Ĵ�С�Ѿ���MEM_ALIGN_SIZE����,��������HDR�Ĵ�С
*@param aligened
*	����ʱҪ����Ĵ�С
*@return
*	�����ȥ��block����ʼ��ַ,�����NULL��û�з���ɹ�
*/
PRIVATE BlockHdr* memRegBlockSplit(REGION* pRegionPart, BlockHdr*splitBlockHdr, unsigned long nBytes, unsigned aligened)
{
	BlockHdr* pNextBlockHdr = NEXT_HDR(splitBlockHdr);
	/*step 1: get the block address exclude the hdr, and init the temp Hdr*/
	unsigned long newBlockAddress = ROUND_DOWN((unsigned)(pNextBlockHdr) - nBytes, aligened);
	unsigned long tailBlockSize, headBlockSize;

	/*step 2: split the new block!*/
	BlockHdr* pNewBlockHdr = BLOCK_TO_HDR(newBlockAddress);
	pNewBlockHdr->isFree = FALSE;
	pNewBlockHdr->nBytes = nBytes + sizeof(BlockHdr);
	pNewBlockHdr->pPrevHdr = splitBlockHdr;

	/*step 3: check if the head block need to and can merge into the new Block*/
	headBlockSize = (unsigned long)pNewBlockHdr - (unsigned long)splitBlockHdr;
	if(headBlockSize < sizeof(BlockHdr) + pRegionPart->minBlockBytes)
	{/*step 3.1: the block size is so small that we have merged it into the new block or failing this spliting*/
		if(IS_ALIGNED((unsigned)HDR_TO_BLOCK(splitBlockHdr),  aligened))
		{/*step3.2: we merge it to the new block!*/
			dllRemove(&pRegionPart->freeList, HDR_TO_NODE(splitBlockHdr));
			pNewBlockHdr = splitBlockHdr;
			pNewBlockHdr->isFree = FALSE;
			pNewBlockHdr->nBytes = nBytes + sizeof(BlockHdr);
		}
		else
		{/*step3.1: it can not split mem from this block*/
			return NULL;
		}
	}
	else
	{/*step 3.4: adjust the head HDR structure*/
		splitBlockHdr->nBytes = (unsigned)pNewBlockHdr - (unsigned)splitBlockHdr;
	}

	/*step 4: check if the tail block need to merge into the new Block*/
	tailBlockSize = (unsigned long)pNextBlockHdr - (unsigned)pNewBlockHdr - nBytes - sizeof(BlockHdr);	
	if(tailBlockSize < sizeof(BlockHdr) + pRegionPart->minBlockBytes)
	{/*step 4.1:if  it need to merge into the new block, it is easy to do*/
		pNewBlockHdr->nBytes += tailBlockSize;
		pNextBlockHdr->pPrevHdr = pNewBlockHdr;
	}
	else
	{
		/*step 4.2: it is large enough to become an independent freeblock, just add it to the free list*/
		/*yeah, yeah~ it is stupid, but what I consider is that the aligened value is some large that it can not 
		* create lots of bit blocks, or aligened is just same to MEM_ALIGN_SIZE!!!^_^
		*/
		LinkNode *tailNode;

		BlockHdr* pTailBlockHdr = NEXT_HDR(pNewBlockHdr);
		pTailBlockHdr->isFree = TRUE;
		pTailBlockHdr->nBytes = tailBlockSize;
		pTailBlockHdr->pPrevHdr = pNewBlockHdr;
		pNextBlockHdr->pPrevHdr = pTailBlockHdr;

		tailNode = HDR_TO_NODE(pTailBlockHdr);
		dllInsert(&pRegionPart->freeList,  NULL, tailNode);
	}

	/* update the statistic information*/
	pRegionPart->cumBlocksAllocated++;
	pRegionPart->cumBytesAllocated += pNewBlockHdr ->nBytes;
	pRegionPart->curBlocksAllocated++;
	pRegionPart->curBytesAllocated += pNewBlockHdr ->nBytes;
	
	return pNewBlockHdr ;
}

/**
*��������ͷ��Ѿ��ͷŵ��ڴ�
*
*@param pRegionPart
*
*@param freeAddress
*
*@return
* 
*/
PUBLIC STATUS memRegFree(REGION* pRegionPart, void* freeAddress)
{
	/*���Ƚ���Ե�ַת���ɾ��Ե�ַ*/
	freeAddress = REL_TO_PHY_ADD(pRegionPart, freeAddress);
	
	BlockHdr* pFreeBlockHdr = BLOCK_TO_HDR(freeAddress);
	BlockHdr* pNextBlockHdr = NEXT_HDR(pFreeBlockHdr);
	BlockHdr* pPreBlockHdr = PREV_HDR(pFreeBlockHdr);
	
	/*some try to avoid the error situation, but not at all*/
	if(!IS_ALIGNED(pFreeBlockHdr, MEM_ALIGN_SIZE) || pFreeBlockHdr->isFree == TRUE)
	{
		return ERROR;
	}

	/*update the statistic information*/
	pRegionPart->curBlocksAllocated--;
	pRegionPart->curBytesAllocated -= pFreeBlockHdr ->nBytes;
	
	pFreeBlockHdr->isFree = TRUE;
	/*step 1: try to merge the next FreeBlock if exist!*/
	if(pNextBlockHdr->isFree == TRUE)
	{
		/*adjust the nnext Block*/
		BlockHdr* pNNextBlockHdr = NEXT_HDR(pNextBlockHdr);
		pNNextBlockHdr->pPrevHdr = pFreeBlockHdr;

		/*remove it from the freeList*/
		dllRemove(&pRegionPart->freeList, HDR_TO_NODE(pNextBlockHdr));

		/*adjust the freeBlock structure*/
		pFreeBlockHdr->nBytes += pNextBlockHdr->nBytes;
	}
	
	/*step 2: try to merge the previous FreeBlock if exist*/
	pNextBlockHdr = NEXT_HDR(pFreeBlockHdr);
	if(pPreBlockHdr->isFree == TRUE)
	{
		pPreBlockHdr->nBytes += pFreeBlockHdr->nBytes;
		pNextBlockHdr->pPrevHdr = pPreBlockHdr;
	}
	else
	{/*step 3: if it can not merge the previous FreeBlock, add itself to the freeList!*/
		dllInsert(&pRegionPart->freeList, NULL, HDR_TO_BLOCK(pFreeBlockHdr));
	}
	
	return OK;
}

PUBLIC REGION* memCreateRegion(void* poolStart, unsigned poolSize, unsigned relativeAddress)
{
	REGION* pUsrRegion;
	void* poolRealStart = (void*)(ROUND_UP(poolStart, MEM_ALIGN_SIZE));
	unsigned poolRealSize = poolSize - ((unsigned)poolRealStart - (unsigned)poolStart);

	/*allocate the region partition*/
	pUsrRegion = regAlloc();
	if(pUsrRegion == NULL)
	{
		return NULL;
	}

	/*init all the region*/
	STATUS status = memInitReg(pUsrRegion, poolRealStart, poolRealSize, relativeAddress);
	if(status != OK)
	{
		return NULL;
	}
	
	return pUsrRegion;
}


PUBLIC STATUS memDestroyRegion(REGION* pDesRegion)
{
	//void* block = (void*)pDesRegion->startAddress;
	STATUS freeBlockStatus = OK, freeRegionStatus;

	//freeBlockStatus = memRegFree(&memKernelReg, block);
	freeRegionStatus = memRegFree(&memKernelReg, pDesRegion);
	if(freeBlockStatus != OK || freeRegionStatus != OK)
	{
		return ERROR;
	}
	
	return OK;
}

PRIVATE REGION* regAlloc()
{
	return (REGION*)memRegAlloc(&memKernelReg, sizeof(REGION));
}

PUBLIC void showMemRegStatus(REGION* pRegion)
{
	REGION* pDugRegion = (pRegion == NULL ? &memKernelReg : pRegion);
	BlockHdr* scanHdr;
	LinkNode* scanFreeNode = LL_FIRST(pDugRegion);
	int i, freeBlockNum = 0;

	/*��ʾ���������Ϣ*/
	/*
	kprintf(kernelStdout,  "\nThe memory manager information is as follows:\n");
	kprintf(kernelStdout,  "dugRegion->startAddress = %x\n", pDugRegion->startAddress);
	kprintf(kernelStdout,  "dugRegion->totalBytes = %x\n", pDugRegion->totalBytes);
	kprintf(kernelStdout,  "dugRegion->minBlockBytes = %x\n", pDugRegion->minBlockBytes);
	kprintf(kernelStdout,  "dugRegion->curBlocksAllocated = %x\n", pDugRegion->curBlocksAllocated);
	kprintf(kernelStdout,  "dugRegion->curBytesAllocated = %x\n", pDugRegion->curBytesAllocated);
	kprintf(kernelStdout,  "dugRegion->cumBlocksAllocated = %x\n", pDugRegion->cumBlocksAllocated);
	kprintf(kernelStdout,  "dugRegion->cumBytesAllocated = %x\n", pDugRegion->cumBytesAllocated);
	//*/
	
	/*˳����ʾblock����Ϣ*/
	/*
	kprintf(kernelStdout,  "\nNow show the sequence block!\n");
	for(scanHdr = (BlockHdr*)(pDugRegion->startAddress), i = 1; ; scanHdr = NEXT_HDR(scanHdr), i++)
	{
		kprintf(kernelStdout,  "BLOCK %d{\n", i);
		kprintf(kernelStdout,  "BLOCK start address: %x\n", (unsigned)scanHdr);
		kprintf(kernelStdout,  "BLOCK bytes size: %x\n", scanHdr->nBytes);
		kprintf(kernelStdout,  "BLOCK previous block address: %x\n", (unsigned)scanHdr->pPrevHdr);
		kprintf(kernelStdout,  "BLOCK next block address:%x\n", (unsigned)scanHdr + scanHdr->nBytes);
		if(scanHdr->isFree)
		{
			kprintf(kernelStdout,  "This block is free!\n");
		}
		kprintf(kernelStdout,  "}\n\n");

		if(IS_HDRLAST(scanHdr))
		{
			break;
		}
	}
	//*/
	/*��ʾ�����б�*/
	//*
	kprintf(kernelStdout,  "\nNow show the free BLock List!\n");
	for(i = 1; scanFreeNode != NULL; scanFreeNode = LL_NEXT(scanFreeNode), i++)
	{
		scanHdr = NODE_TO_HDR(scanFreeNode);

		kprintf(kernelStdout,  "FREE BLOCK %d{\n", i);
		kprintf(kernelStdout,  "FREE BLOCK start address(including HDR): %x\n", (unsigned)scanHdr);
		kprintf(kernelStdout,  "FREE BLOCK start address(excluding HDR): %x\n", (unsigned)HDR_TO_NODE(scanHdr));
		kprintf(kernelStdout,  "FREE BLOCK bytes size: 0x%x\n", scanHdr->nBytes);
		kprintf(kernelStdout,  "FREE BLOCK next free block address: %x\n", (unsigned)LL_NEXT(scanFreeNode));
		kprintf(kernelStdout,  "FREE BLOCK previous free block address: %x\n", (unsigned)LL_PREVIOUS(scanFreeNode));
		kprintf(kernelStdout,  "}\n\n");
	}
	//*/
}


PUBLIC void* sys_malloc(unsigned nBytes, int nouse, PROCESS* p_proc)
{
	
	void* address;
	char* str;
	
	address = memRegAlloc(p_proc->region, nBytes);
	kprintf(p_proc->nr_tty, "process %d malloc %d bytes now, the address is %x!\n", p_proc->pid, nBytes, address);
	
	return address;
}

PUBLIC void sys_free(void* freeAddress, int nouse, PROCESS* p_proc)
{
	kprintf(p_proc->nr_tty, "process %d free memory now, the address is %x bytes!\n", p_proc->pid, freeAddress);

	memRegFree(p_proc->region, freeAddress);
}

