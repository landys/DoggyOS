/*memory.c - the memory manage model header*/

#ifndef _DOGGY_MEMORY_H_
#define _DOGGY_MEMORY_H_

#include "dllLib.h"
#include "const.h"

/*8 byte ����,���ֵ������֤��Block_Hdr��С��һ��*/
#define MEM_ALIGN_SIZE 8

/*�õ�ǰһ��,��һ����block����ʼ��ַ, ����HDR*/
#define NEXT_HDR(pHdr) ((BlockHdr*)(pHdr->nBytes + (char*)pHdr))
#define PREV_HDR(pHdr) ((BlockHdr*)(pHdr->pPrevHdr))
#define IS_HDRLAST(pHdr) ((pHdr->nBytes == sizeof(BlockHdr)) && (pHdr->pPrevHdr != NULL))

/*��Ե�ַ֮����л�*/
#define REL_TO_PHY_ADD(region, address)	((void*)((unsigned)(address) + region->relativeAddress))
#define PHY_TO_REL_ADD(region, address)	((void*)((unsigned)(address) - region->relativeAddress))

/*�û��ռ���HDR֮���໥��ת��*/
#define HDR_TO_BLOCK(pHdr) ((void*)((char*)pHdr + sizeof(BlockHdr)))
#define BLOCK_TO_HDR(pBlock) ((BlockHdr*)((char*)pBlock -sizeof(BlockHdr) ))

/*��FREENODE֮��,HDR��FREE_NODE֮���໥ת��*/
#define HDR_TO_NODE(pHdr) (&((FreeBlock*)pHdr)->freeList)
#define NODE_TO_HDR(pNode) ((BlockHdr*)((char*)pNode - OFFSET(freeList, FreeBlock)))

typedef struct Memory_Region
{
	LinkList freeList;			/* list of free block*/
	unsigned relativeAddress;	/*the relative address, ���صĵ�ַҪ����Դ���*/
	unsigned startAddress;	/* the start address in the region, ʵ��������ڴ濪ʼ��ַ(����HDR)*/
	unsigned totalBytes;		/* total num of bytes in the region*/
	unsigned minBlockBytes;	/* min block size in bytes including hdr*/
	
	/*allocation statistic*/
	unsigned curBlocksAllocated;		/* current # of blocks allocated */
	unsigned curBytesAllocated;		/* current # of bytes allocated */
	unsigned cumBlocksAllocated;		/* cumulative # of blocks allocated */
	unsigned cumBytesAllocated;		/* cumulative # of words allocated */

}REGION;

typedef struct Block_Hdr
{
	struct Block_Hdr* pPrevHdr;
	unsigned	nBytes : 31;			/**/
	unsigned isFree : 1;			/*��ʾ����ڴ���Ƿ��ǿ��п�,true�ǿ��п�*/
}BlockHdr;

typedef struct Free_Block
{
	BlockHdr hdr;
	LinkNode freeList;
}FreeBlock;

PUBLIC STATUS memSysLibInit();
PUBLIC STATUS memInitReg(REGION* pRegion, void* poolStart, unsigned poolSize, unsigned relativeAddress);
PUBLIC void* memRegAlloc(REGION* pRegionPart, unsigned long nBytes);
PUBLIC void* memRegAlignedAlloc(REGION* pRegionPart, unsigned long nBytes, unsigned aligened);
PUBLIC STATUS memRegFree(REGION* pRegionPart, void* freeAddress);
PUBLIC REGION* memCreateRegion(void* poolStart, unsigned poolSize, unsigned relativeAddress);
PUBLIC STATUS memDestroyRegion(REGION* pDesRegion);

#endif
