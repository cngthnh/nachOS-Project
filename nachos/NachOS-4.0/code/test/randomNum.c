#include "syscall.h"
int main()
{
	int a;
	a = RandomNum();
	PrintString("Positive Number random is: ");
	PrintNum(a);
	return 0;
}
