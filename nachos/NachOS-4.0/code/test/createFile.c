#include "syscall.h"

int main()
{
	int result, actualRead, actualWrite;
	char myStr[11];
	result = CreateFile("testfile");
	PrintNum(result);
	result = Open("testfile", 0);
	Open("testfile", 0);
	actualWrite = Write("123456789", 9, result);
	PrintNum(result);
	PrintNum(Close(result));
	result = Open("testfile", 0);
	actualRead = Read(myStr, 5, result);
	PrintNum(actualRead);
	actualWrite = Write("54321", 5, result);
	PrintNum(result);
	PrintNum(Close(result));
	result = Open("testfile", 0);
	actualRead = Read(myStr, 10, result);
	PrintString(myStr);
	PrintNum(Close(result));

	// int result;
	// char myStr[10];
	// result = Read(myStr, 5, STDIN);
	// PrintNum(result);
	// result = Write(myStr, 10, STDOUT);
	// PrintNum(result);
	Halt();
}
