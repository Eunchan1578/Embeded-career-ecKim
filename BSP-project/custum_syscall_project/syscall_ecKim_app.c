#include <stdio.h>
#include <unistd.h>
#include <asm-generic/unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#pragma GCC diagnostic ignored "-Wunused-result"
void printText(long val);
long g_Key;
long g_prevKey = 0;
long firstKey;
int main(int argc,char* argv[])
{
	firstKey = strtoul(argv[1], NULL, 16);
	g_Key = syscall(__NR_ecKimsyscall, firstKey);
	g_prevKey=firstKey;
	printText(firstKey);
	while(1)
	{
		if(g_Key==0b10000000)
			return 0;
		if((g_Key != 0)&&(g_Key != g_prevKey))
		{
			g_prevKey=g_Key;
			printText(g_Key);
		}
		g_Key = syscall(__NR_ecKimsyscall, g_prevKey);
	}

	return 0;
}
void printText(long val)
{
	for(int k=0;k<8;k++)
	{
		putc(k+'0', stdout);
		if(k!=7)
			putc(':', stdout);
	}
	putc('\n', stdout);
	for(int i=0;i<8;i++)
	{
		if(val&(1<<i))
		{
			putc('O', stdout);
			if(i!=7)
			putc(':', stdout);
		}
		else
		{
			putc('X', stdout);
			if(i!=7)
			putc(':', stdout);
		}
	}
		putc('\n', stdout);
		putc('\n', stdout);
}
