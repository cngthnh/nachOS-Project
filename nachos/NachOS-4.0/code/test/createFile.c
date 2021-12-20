#include "syscall.h"

int main()
{
	int result;
	result = CreateFile("testfile");
	PrintNum(result);
	result = Open("testfile");
	PrintNum(result);
	PrintNum(Close(result));
	Halt();
}
