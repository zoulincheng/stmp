#ifndef _BASICTYPE_H
#define _BASICTYPE_H

/*!< Unsigned integer types  */
typedef unsigned char     u_char;
typedef unsigned short    u_short;
typedef unsigned long     u_long;
typedef unsigned char 	  uint8_t;
typedef char 			  BYTE;
typedef unsigned short	  uint16_t;

#define CC_ACCESS_NOW(type, variable) (*(volatile type *)&(variable))

#endif
