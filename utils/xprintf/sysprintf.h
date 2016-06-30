#ifndef _SYSPRINTF_H
#define _SYSPRINTF_H
#include "xprintf.h"


void  __xstd_printf(int level, const char * fmt,...);
int __xstd_dump(int level,const char *sczTitle,const void *pciBuf,int nSize);

int strcmp_ex(char const*s1,char const *s2);

#define USER_DEBUG  1
#if USER_DEBUG
//For user debug
//#include "xprintf.h"
#define PRINTF(...) xprintf(__VA_ARGS__)
//#define XPRINTF(...) xprintf(__VA_ARGS__)

#define XPRINTF(__arvs__) __xstd_printf  __arvs__
//#define PRINTF(__arvs__) __xstd_printf __arvs__
#define MEM_DUMP(level,sczTitle,pBuf,nSize) __xstd_dump(level,sczTitle,pBuf,nSize)
#else
#define PRINTF(...)
#define XPRINTF(...)
#define MEM_DUMP(...)
#define PRINT6ADDR_U(addr)
#define PRINTLLADDR_U(addr)
#endif
#endif
