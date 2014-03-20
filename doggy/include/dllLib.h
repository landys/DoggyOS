/* dllLib.h - doubly linked list library header */
#include "const.h"

#ifndef	_DOGGY_DLLLIB_H_
#define	_DOGGY_DLLLIB_H_

typedef struct Link_Node
{
	struct Link_Node* next;
	struct Link_Node* previous;
}LinkNode;

typedef struct
{
	LinkNode* head;
	LinkNode* tail;
}LinkList;

#define LL_FIRST(pList) (((LinkList*)(pList))->head)

#define LL_LAST(pList)	(((LinkList*)(pList))->tail)

#define LL_NEXT(pNode)	(((LinkNode*)(pNode))->next)

#define LL_PREVIOUS(pNode)	(((LinkNode*)(pNode))->previous)

#define ISLL_EMPTY(pList)	((((LinkNode*)(pList))->head) == NULL)

EXTERN STATUS dllInit(LinkList* pList);
EXTERN void dllInsert(LinkList* pList, LinkNode* pNodePre, LinkNode* pNode);
EXTERN void dllAdd(LinkList* pList, LinkNode* pNode);
EXTERN void dllRemove(LinkList* pList, LinkNode* pNode);
EXTERN LinkNode *dllGetFist(LinkList* pList);
EXTERN int dllCount(LinkList* pList);

#endif
