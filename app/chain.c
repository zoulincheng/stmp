#include "stm8l15x.h"
#include "basictype.h"
#include "chain.h"

NODE *head = NULL;

u8 start_timerEx( NODE* p, unsigned int timestamp_value, EVENT task_value )
{
	NODE* newTimer = NULL;
	if(timestamp_value == 0)
	{
		set_event(task_value);//TODO
	}
	else
	{
		newTimer = add_node( p, timestamp_value, task_value );
	}
	return ( (newTimer != NULL) ? SUCCESS : ERROR );
}

u8 delete_timerEx( NODE* p )
{
	u8 state;
	RTC_WakeUpCmd(DISABLE);
	//Is_rtc_settable = TRUE;
	state = delete_node( p );
	return ( state );
}

NODE* add_node(NODE* p, unsigned int timestamp_value, EVENT task_value)
{
	NODE* pNode;
	if(p == NULL )
	return NULL;
	if(head == NULL)
	{
		head = p;
		head->timestamp = timestamp_value;
		head->task = task_value;
		head->node_next = NULL;
	}
	else
	{
		pNode = head;
		while( pNode->node_next != NULL || ( pNode == p ) )
		{
			if (pNode == p)
			{
				p->task = task_value;
				p->timestamp = timestamp_value;
				return pNode;
			}
			pNode = pNode->node_next;
		}
		pNode->node_next = p;
		pNode = p;
		pNode->task = task_value;
		pNode->timestamp = timestamp_value;
		pNode->node_next = NULL;
	}
	return pNode;
}

ErrorStatus delete_node(NODE* pDelete)
{
	NODE* pNode_front;
	NODE* pNode_present;

	if(head == NULL || pDelete == NULL)
	return ERROR;

	pNode_present = head;
	pNode_front = pNode_present;
	while((pNode_present != pDelete) && (pNode_present != NULL))
	{
		pNode_front = pNode_present;
		pNode_present = pNode_present->node_next;
	}
	if(pNode_present == pDelete)
	{
		if(pDelete == head)
		{
			head = head->node_next;
		}
		else
		{
			pNode_front->node_next = pNode_present->node_next;
		}
		pNode_present ->node_next = NULL;
		return SUCCESS;
	}
	return ERROR;
}

void delete_all_node(void)
{
	//disableInterrupts();
	head = NULL;
	//enableInterrupts();

}

unsigned char insert_node(NODE* pInsert, unsigned int timestamp_value, EVENT task_value, unsigned char place)
{
	NODE* pNode_front;
	NODE* pNode_present;
	unsigned int i = 0;
	if(pInsert == NULL )
		return ERROR;
	if((pNode_present = find_node(pInsert, F_POINT, 0)) != NULL)
	{
		delete_node(pNode_present);
	}

	if(head == NULL)
	{
		add_node(pInsert, timestamp_value, task_value);
		return i;
	}

	pNode_present = head;
	pNode_front = pNode_present;
	for(i = 0; i < place; i++)
	{
		pNode_front = pNode_present;
		pNode_present = pNode_present->node_next;
		if(pNode_present == NULL)
		{
			break;
		}
	}
	pInsert->node_next = pNode_present;
	pInsert->timestamp = timestamp_value;
	pInsert->task = task_value;
	if(place != 0)
	{
		pNode_front->node_next = pInsert;
	}
	else
	{
		head = pInsert;
	}
	return i;
}

NODE* find_node(NODE* pFind, FINDDATA type, unsigned int value)
{
	NODE* pNode;
	if(pFind == NULL )
		return NULL;
	pNode = head;
	while(pNode != NULL)
	{
		switch (type)
		{
			case F_TIMESTAMP:
				if(pNode->timestamp == value)
				{
					return pNode;
				}
				break;

			case F_TASK:
				if(pNode->task == value)
				{
					return pNode;
				}
				break;

			case F_POINT:
				if(pNode == pFind)
				{
					return pNode;
				}
				break;
			default:
				break;
		}
		pNode = pNode->node_next;
	}
	return NULL;
}



