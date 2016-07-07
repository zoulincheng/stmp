#ifndef _BASICTYPE_H
#define _BASICTYPE_H

/*!< Unsigned integer types  */
typedef unsigned char     u_char;
typedef unsigned short    u_short;
typedef unsigned long     u_long;
typedef unsigned char 	  uint8_t;
typedef char 			  BYTE;
typedef unsigned short	  uint16_t;
typedef unsigned char     U8;
typedef unsigned short    U16;
typedef unsigned char     INT8U;
typedef unsigned short    INT16U;
typedef unsigned long     INT32U;
typedef unsigned char     u8;
typedef unsigned short    u16;

#ifndef NULL
#define NULL  ((void*)0)
#endif

#define CC_ACCESS_NOW(type, variable) (*(volatile type *)&(variable))

#endif
