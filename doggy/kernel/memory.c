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

/*系统主内存管理region*/
REGION memKernelReg;

/*标志是否初始化过内存库*/
BOOL isInitMemLib = FALSE;

/*是否可以在windows下调试的标志*/
/*#define DUBUG_MEM 0*/

#ifdef DUBUG_MEM
/*主内存结束的地址*/
EXTERN int mainMemEnd;
/*主内存开始的地址现在默认是开始地址2M,  注意这里在真正
*嵌入到os时要指定值 = 0x200000
*/
EXTERN int mainMemStart;
#else
/*这个值是从load.asm得到的*/
extern int mem_size;

int mainMemEnd;
/*主内存结束的地址*/
int mainMemStart = 0x200000;
#endif

/*forward function*/
PRIVATE STATUS memRegAttach(REGION* pRegion, void* poolStart, unsigned poolSize, unsigned relativeAddress);
PRIVATE BlockHdr* memRegBlockSplit(REGION* pRegionPart, BlockHdr*splitBlockHdr, unsigned long nBytes, unsigned aligened);
PRIVATE REGION* regAlloc();

/**
*这个函数主要是初始化内存管理类库,主要是配置主内存的
*空间.
*@return 
*	初始化成功返回OK
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
*函数初始化pRegion结构体里面的各个域，再把poolStart所指向的
*大小为poolSize的内存空间初始化成前后两个HDR, 中间FREEBLOCK的形式
*@param pRegion
*	要初始化的内存管理结构体
*@param poolStart
*	被管理的内存起始地址, 要保证与MEM_ALIGN_SIZE对齐
*@param poolSize
*	被管理的内存的大小, 在这个函数中会做与MEM_ALIGN_SIZE对齐的工作
*/
PUBLIC STATUS memInitReg(REGION* pRegion, void* poolStart, unsigned poolSize, unsigned relativeAddress)
{
	char* poolEnd = (char*)poolStart + poolSize;
	/*step1: first clear up the pRegion mem*/
	memset(pRegion, 0, sizeof(REGION));

	/*step2: round down the poolSize*/
	poolSize = ROUND_DOWN(poolSize, MEM_ALIGN_SIZE);
	/*如果消减后的内存块小于最小内存块或者内存地址没有
	 * 对齐,则返回错误
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
*将具体的pRegion与内存相挂载，具体完成把poolStart所指向的
*大小为poolSize的内存空间初始化成前后两个HDR, 中间FREEBLOCK的形式
*@param pRegion
*	要挂接的内存管理结构体
*@param poolStart
*	被管理的内存起始地址, 已经保证与MEM_ALIGN_SIZE对齐
*@param poolSize
*	被管理的内存的大小,已经保证与MEM_ALIGN_SIZE对齐
*@param relativeAddress
*	在这个region中要做相对操作的地址
*@return 
*	OK 区域挂在成功, ERROR区域挂载失败
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
	/*初始化HDR*/
	midHdr = NEXT_HDR(headHdr);
	midHdr->pPrevHdr = headHdr;
	midHdr->nBytes = midFreeBlockSize;
	midHdr->isFree = TRUE;
	/*初始化FREEBLOCK, 将它加入空闲列表*/
	midFreeBlock = HDR_TO_NODE(midHdr);
	dllInsert(&pRegion->freeList, (LinkNode*)NULL, midFreeBlock);
	
	/*step 3: init the tail HDR*/
	tailHdr = NEXT_HDR(midHdr);
	tailHdr->pPrevHdr = midHdr;
	tailHdr->nBytes = sizeof(BlockHdr);
	tailHdr->isFree = FALSE;
	
	/*step 4: init the REGION structure*/
	/*如果是系统主内存则不应该做相对处理，所以要设置为0*/
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
*这个函数用来从pRegionPart所指向的内存区域分配大小为nBytes
*的内存区域.实际分配的大小可能比用户要求的大小要大,从而
*也导致了分配失败的可能性.
*@param pRegionPart
*	分配空间的REGION指针
*@param nBytes
*	用户要求分配的空间的大小(不包括HDR的大小),
*     这个大小可能会做默认对齐操作
*@return 
*	NULL 分配失败, 否则返回分配得到的用户
*/
PUBLIC void* memRegAlloc(REGION* pRegionPart, unsigned long nBytes)
{
	return memRegAlignedAlloc(pRegionPart, nBytes, MEM_ALIGN_SIZE);
}

/**
*这个函数用来从pRegionPart所指向的内存区域分配大小为nBytes
*的内存区域.实际分配中会按aligened对末尾地址进行对齐.这个函数
*主要是找到有足够空间的FREEBLOCK,具体的分配是由memRegBlockSplit
*来完成的
*@param pRegionPart
*	分配空间的REGION指针
*@param nBytes
*	用户要求分配的空间的大小
*@param aligened
*	分配的起始地址要对齐的数目
*@return 
*	NULL 分配失败, 否则返回分配得到的用户
*/
PUBLIC void* memRegAlignedAlloc(REGION* pRegionPart, unsigned long nBytes, unsigned aligened)
{
	/*
	*首先nBytes要与MEM_ALIGN_SIZE对齐,这样可以保证BLOCKHDR与前面的块
	*之间没有间隙
	*/
	unsigned long realBytes;		/*实际分配给用户的bytes数*/
	unsigned long totalBytes;		/*系统总共要分配的bytes数,包括了HDR*/
	unsigned long tryTotalBytes;	/*在freelist查找时匹配的bytes数*/
	LinkNode* scanFreeBlock;
	BlockHdr* pFindedBlock;
	BOOL isFind = FALSE;

	/*如果用户要求分配的空间小于最小应该分配的空间,则分配最小空间*/
	if(nBytes < pRegionPart->minBlockBytes)
	{
		nBytes = pRegionPart->minBlockBytes;
	}
	realBytes =  ROUND_UP(nBytes, MEM_ALIGN_SIZE) ;
	totalBytes = realBytes + sizeof(BlockHdr);
	tryTotalBytes = totalBytes + aligened;
	scanFreeBlock = LL_FIRST(&(pRegionPart->freeList));
	
	/*这个while循环主要是找到有足够空间可以分割的freeblock*/
	while(scanFreeBlock)
	{
		BlockHdr *pNewHdr, *pNewNextHdr;
		/*已经到空闲列表尾部的情况*/
		if(scanFreeBlock == NULL)
		{
			break;
		}

		pNewHdr = NODE_TO_HDR(scanFreeBlock);
		if(pNewHdr->nBytes >= tryTotalBytes ||
				(IS_ALIGNED( (unsigned)HDR_TO_BLOCK(NODE_TO_HDR(scanFreeBlock)), aligened) && pNewHdr->nBytes == totalBytes))
		{/*找到了可以分裂的block*/
			pFindedBlock = memRegBlockSplit(pRegionPart, pNewHdr, realBytes, aligened);

			if(pFindedBlock != NULL)
			{
				isFind = TRUE;
				break;
			}
		}

		scanFreeBlock = LL_NEXT(scanFreeBlock);
	}
	/*如果申请成功，返回时要将绝对地址转化成相对地址*/
	return isFind ? PHY_TO_REL_ADD(pRegionPart, HDR_TO_BLOCK(pFindedBlock)) : NULL;
}

/**
* 这个函数实际将一个要分配的block从一个freeblock中分割出去
*并且试图合并它的前后块.此函数的调用者要求保证可以分配
*存在必然性.
*@param pRegionPart
*	分配空间的REGION指针
*@param tryBlockHdr
*	可以分裂的FREEBLOCK的BlockHdr
*@param nBytes
*	试图分配的大小已经与MEM_ALIGN_SIZE对齐,不包括了HDR的大小
*@param aligened
*	分配时要对齐的大小
*@return
*	分配出去的block的起始地址,如果是NULL则没有分配成功
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
*这个函数释放已经释放的内存
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
	/*首先将相对地址转化成绝对地址*/
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

	/*显示管理类的信息*/
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
	
	/*顺序显示block的信息*/
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
	/*显示空闲列表*/
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

