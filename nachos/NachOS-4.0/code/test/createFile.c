#include "syscall.h"

int main()
{
	int result = Create("testfile");
	Halt();
}
