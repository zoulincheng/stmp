#include <stdarg.h>
#include <ctype.h>


#include "xprintf.h"
#include "basictype.h"

#include "sysprintf.h"

//typedef int *va_list[1];

extern void xvprintf (
	const char*	fmt,	/* Pointer to the format string */
	va_list arp			/* Pointer to arguments */
);



/* Compare memory to memory */
int mem_cmp (const void* dst, const void* src, unsigned int cnt) 
{
	const BYTE *d = (const BYTE *)dst, *s = (const BYTE *)src;
	int r = 0;

	while (cnt-- && (r = *d++ - *s++) == 0) ;
	return r;
}


int strcmp_ex(char const*s1,char const *s2)
{
	char c1,c2;
	
	while( 1)
	{
		c1=toupper(*s1++);
		c2=toupper(*s2++);
		
		if(c1!=c2 || c1 == 0 || c2 == 0)  
			break;
	}

	return ((int)c1 - (int)c2);
}






static char g_nDebufLevel = 12;// level = -1, means to get the level, no change to g_nDebugLevel
char *get_gdbLevel(void)
{
	return &g_nDebufLevel;
}


void  __xstd_printf(int level, const char * fmt,...)
{
    //int rc = 0;
    va_list ap;

    if(((unsigned char)level <= g_nDebufLevel && g_nDebufLevel!=20))
    {
       va_start(ap, fmt);
       xvprintf(fmt, ap);
       va_end(ap);
    }
    //return rc;
}


int __xstd_dump(int level,const char *sczTitle,const void *pciBuf,int nSize)
{
	if(level <= g_nDebufLevel && g_nDebufLevel!=20)
	{
		int i,n;
		const unsigned char*pcBuf = (unsigned char*)pciBuf;
	
		if(sczTitle)
			xprintf("%-5s%-4d:",sczTitle,nSize);
	
		for(i=0;i<nSize;i++)
		{
			unsigned int param = (unsigned int)(pcBuf[i]&0xff);
			xprintf(" %02X",param);
			if((i%16) == 15)
			{
				xprintf(" ;");
				for(n = i&0xffffFFF0; n<=i; n++)
					xprintf("%c",isprint(pcBuf[n])?pcBuf[n]:'.');
				xprintf("\r\n");
				if(sczTitle && (i+1) < nSize)
				{
					xprintf("%-9s:","");
				}
			}
		}

		if(i%16)
		{
			for(n = i; (n%16)!=0 ;n++)
				xprintf("   ");
			xprintf(" ;");
			for(n = i&0xffffFFF0; n<i; n++)
				xprintf("%c",isprint(pcBuf[n])?pcBuf[n]:'.');
			xprintf("\r\n");
		}
		
		if(nSize == 0)
		{
			xprintf("\r\n");
		}

	}
	return 0;
}






