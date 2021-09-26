#include "syscall.h"
int main()
{
	int i;
	PrintString("Decimal\tCharacter\n");
	for (i=32; i<128; ++i)
	{
		PrintNum(i);
		PrintChar('\t');
		PrintChar(i);
		PrintChar('\n');
	}
	return 0;
}
