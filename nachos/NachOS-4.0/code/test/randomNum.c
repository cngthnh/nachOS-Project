#include "syscall.h"
int main()
{
	int a;
	a = RandomNum();
	PrintString("So nguyen duong ngau nhien la: ");
	PrintNum(a);
	Halt();
}
