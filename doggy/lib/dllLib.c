#include "dllLib.h"
#include "const.h"

/*********************************************************************
*
* dllCreate - create a doubly linked list descriptor
*
* This routine returns a pointer to an initialized doubly linked list
* descriptor.
*
* RETURNS: Pointer to a doubly linked list descriptor, or NULL if ERROR.
*/
/**
LinkList *dllCreate (void)
{
	LinkList *pList = (LinkList*)malloc(sizeof(LinkList));
	dllInit(pList);

	return pList;
}
//*/

/*********************************************************************
*
* dllInit - initialize doubly linked list descriptor
*
* Initialize the specified list to an empty list.
*
* RETURNS: OK, or ERROR if doubly linked list could not be initialized.
*/
STATUS dllInit(LinkList* pList)
{
	pList->head = NULL;
	pList->tail = NULL;

	return OK;
}

/*******************************************************************************
*
* dllDelete - terminate doubly linked list head free associated memory
*
* Terminate the specified list and deallocated associated memory.
*
* RETURNS: OK, or ERROR if doubly linked list could not be deleted.
*
* ARGSUSED
*/
/**
STATUS dllDelete(LinkList * pList)
{
	free(pList);
	return OK;
}
//*/

/************************************************************************
*
* dllInsert - insert node in list after specified node
*
* This routine inserts the specified node in the specified list.
* The new node is placed following the specified 'previous' node in the list.
* If the specified previous node is NULL, the node is inserted at the head
* of the list.
*/
void dllInsert(LinkList* pList, LinkNode* pNodePre, LinkNode* pNode)
{
	LinkNode* pNodeNext = NULL;

	if(pNodePre == NULL)
	{
		pNodeNext = LL_FIRST(pList);
		pList->head = pNode;		
	}
	else
	{
		pNodeNext = LL_NEXT(pNodePre);
		pNodePre->next = pNode;
	}

	if(pNodeNext == NULL)
	{
		pList->tail = pNode;
	}
	else
	{
		pNodeNext->previous = pNode;
	}

	pNode->next = pNodeNext;
	pNode->previous = pNodePre;
}

/************************************************************************
*
* dllAdd - add node to end of list
*
* This routine adds the specified node to the end of the specified list.
*/

void dllAdd(LinkList* pList, LinkNode* pNode)
{
	LinkNode* pNodePre = LL_PREVIOUS(pNode);

	if(pNodePre == NULL)
	{
		pList->head = pNode;
	}
	
	pList->tail = pNode;
	pNode->previous = pNodePre;
	pNode->next = NULL;
}

/************************************************************************
*
* dllRemove - remove specified node in list
*
* Remove the specified node in the doubly linked list.
*/

void dllRemove(LinkList* pList, LinkNode* pNode)
{
	LinkNode *pNodePre =  LL_PREVIOUS(pNode);
	LinkNode *pNodeNext = LL_NEXT(pNode);
	
	if(pNodeNext == NULL)
	{
		pList->tail = pNodePre;
	}
	else
	{
		pNodeNext->previous = pNodePre;
	}

	if(pNodePre== NULL)
	{
		pList->head = pNodeNext;
	}
	else
	{
		pNodePre->next = pNodeNext;
	}
	
}

/************************************************************************
*
* dllGetFist - get (delete and return) first node from list
*
* This routine gets the first node from the specified list, deletes the node
* from the list, and returns a pointer to the node gotten.
*
* RETURNS: Pointer to the node gotten, or NULL if the list is empty.
*/

LinkNode *dllGetFist(LinkList* pList)
{
	LinkNode* pNode = LL_FIRST(pList);
	dllRemove(pList, pNode);
	
	return pNode;
}

/**************************************************************************
*
* dllCount - report number of nodes in list
*
* This routine returns the number of nodes in the given list.
*
* CAVEAT
* This routine must actually traverse the list to count the nodes.
* If counting is a time critical fuction, consider using lstLib(1) which
* maintains a count field.
*
* RETURNS: Number of nodes in specified list.
*
* SEE ALSO: lstLib(1).
*/

int dllCount(LinkList* pList)
{
	int count;
	LinkNode* pNode = LL_FIRST(pList);

	for(count = 0; pNode != NULL; count++)
	{
		pNode = LL_NEXT(pNode);
	}

	return count;
}

