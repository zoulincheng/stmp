#ifndef __CHAIN_H
#define __CHAIN_H


//#define NULL 0
#define TIMER_SLOT             200
#define RATIO_OF_TIMESTAMP     (1000/TIMER_SLOT)

typedef enum _FINDDATA{
  F_TIMESTAMP,
  F_TASK,
  F_POINT
}FINDDATA;

typedef struct _NODE
{  
  unsigned int timestamp;
  unsigned int task;
  struct _NODE* node_next;
}NODE;

typedef u16 EVENT;

extern NODE *head;

u8 start_timerEx( NODE* p, unsigned int timestamp_value, EVENT task_value );
u8 delete_timerEx( NODE* p );
NODE* add_node(NODE* p, unsigned int timestamp_value, u16 task_value);
ErrorStatus delete_node(NODE* pDelete);
void delete_all_node(void);
unsigned char insert_node(NODE* pInsert, unsigned int timestamp_value, u16 task_value, unsigned char place);
NODE* find_node(NODE* pFind, FINDDATA type, unsigned int value);

#endif