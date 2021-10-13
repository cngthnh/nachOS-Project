#include "syscall.h"
int main()
{
	int a;
	char buffer[21];
	PrintString("Input a number: ");
	a = ReadNum();
	PrintString("Num = ");
	PrintNum(a);
	PrintString("\n");
	PrintString("Input a string: ");
	ReadString(buffer, 21);
	PrintString("String = \"");
	PrintString(buffer);
	PrintString("\"");
	Halt();
}
