// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "synchconsole.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	is in machine.h.
//----------------------------------------------------------------------

// Define some constants
#define FILENAME_MAX_LENGTH 32 //bytes
#define ARG_1 4
#define ARG_2 5
#define ARG_3 6
#define ARG_4 7
#define SYSCALL_CODE 2
#define OUTPUT_REG 2

// returns system memory buffer
char* User2System(int virtualAddress, int limit)
{
	int byte = 0;
	char* kernelBuffer = NULL;

	kernelBuffer = new char[limit+1]; // null-terminated string
	if (kernelBuffer==NULL)
		return kernelBuffer;

	// set all bits in the buffer to 0
	memset(kernelBuffer, 0, limit+1);

	for (int i=0; i<limit; ++i)
	{
		// read i-th byte
		kernel->machine->ReadMem(virtualAddress+i, 1, &byte);
		kernelBuffer[i] = (char)byte;
		if (byte == 0)
			break;
	}

	return kernelBuffer;
}

// returns the number of bytes copied
int System2User(int virtualAddress, int len, char* buffer)
{
	if (len < 0) return -1;
	if (len == 0) return len;
	int byte = 0;
	int i = 0;

	for (; i < len; ++i)
	{
		byte = (int) buffer[i];
		kernel->machine->WriteMem(virtualAddress+i, 1, byte);
		if (byte == 0)
			break;
	}

	return i;
}

void IncreasePC()
{
	// increase next PC
	int nextPC = kernel->machine->ReadRegister(NextPCReg) + 4;
	kernel->machine->WriteRegister(PrevPCReg, kernel->machine-> ReadRegister(PCReg));
	kernel->machine->WriteRegister(PCReg, kernel->machine-> ReadRegister(NextPCReg));
	kernel->machine->WriteRegister(NextPCReg, nextPC);
}

void
ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(SYSCALL_CODE);

    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) 
	{

    case SyscallException:
		switch(type) 
		{
		case SC_Halt:
		{
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
			printf("\nShutdown, initiated by user program.\n");
			SysHalt();

			ASSERTNOTREACHED();
			break;
		}

		case SC_Exit:
		{
			// needs definition
			IncreasePC();
			break;
		}

		case SC_Add:
		{
			DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(ARG_1) << " + " << kernel->machine->ReadRegister(ARG_2) << "\n");
			
			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(ARG_1),
					/* int op2 */(int)kernel->machine->ReadRegister(ARG_2));

			DEBUG(dbgSys, "Add returning with " << result << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(OUTPUT_REG, (int)result);

			IncreasePC();
			break;
		}

		case SC_Create:
		{
			int virtualAddress;
			
			char* filename;

			DEBUG(dbgFile, "Called SC_Create\n");
			DEBUG(dbgFile, "Reading virtual address of filename\n");
			
			virtualAddress = kernel->machine->ReadRegister(ARG_1);
			
			DEBUG(dbgFile, "Copying filename to system memory space\n");
			
			filename = User2System(virtualAddress, FILENAME_MAX_LENGTH+1);
			
			if (filename==NULL)
			{
				printf("\nInsufficient system memory\n");
				DEBUG(dbgAddr, "Insufficient system memory\n");
				kernel->machine->WriteRegister(OUTPUT_REG,-1);
				delete[] filename;
				return;
			}
			DEBUG(dbgFile, "Filename read successfully");
			if (!kernel->fileSystem->Create(filename, 0))
			{
				printf("\nUnexpected error while creating file: %s", filename);
				kernel->machine->WriteRegister(OUTPUT_REG,-1);
				delete[] filename;
				return;
			}

			DEBUG(dbgFile, "File created successfully\n");
			printf("\nCreated successfully: %s\n", filename);
			kernel->machine->WriteRegister(OUTPUT_REG,0);
			delete[] filename;

			IncreasePC();
			break;
		}

		case SC_ReadNum:
		{
			int number = 0;
			char digit;
			char* numString = new char[12]; // Number with max characters: -2147483648, null-terminated
			int idx = 0;
			do
			{
				digit = kernel->synchConsoleIn->GetChar();
				if (digit=='\n') 
				{
					numString[idx]='\0';
					break;
				}
				numString[idx] = digit;
				idx++;
			} while (1);

			int i;
			if (numString[0]=='-') 
				i = 1;
			else 
				i = 0;

			for (; i < idx; ++i)
			{
				number *= 10;
				number += int(numString[i])-48;
			}

			if (numString[0]=='-')
				kernel->machine->WriteRegister(OUTPUT_REG, -number);
			else
				kernel->machine->WriteRegister(OUTPUT_REG, number);

			IncreasePC();
			break;
		}

		case SC_PrintNum:
		{
			bool isNegative = false;
			int number = kernel->machine->ReadRegister(ARG_1);
			if (number<0) 
			{
				isNegative = true;
				number = abs(number);
			}

			if (isNegative)
			{
				kernel->synchConsoleOut->PutChar('-');
			}

			// create a stack to put the number in (int -> char array)
			char* stack = new char[12]; // Number with max characters: -2147483648, null-terminated
			memset(stack, 0, 12);

			int idx = 12;

			do
			{
				idx--;
				stack[idx] = number % 10 + 48;
				number/=10;
			}
			while (number != 0);

			// print out digits in the stack
			for (; idx<12; idx++)
			{
				kernel->synchConsoleOut->PutChar(stack[idx]);
			}

			IncreasePC();
			break;
		}

		default:
			cerr << "Unexpected system call " << type << "\n";
			ASSERTNOTREACHED();
			break;
		}
		break;

	case PageFaultException:
		DEBUG(dbgAddr, "No valid translation found\n");
		printf("\nNo valid translation found\n");
		SysHalt();
		break;

	case ReadOnlyException:
		DEBUG(dbgAddr, "Write attempted to page marked \"read-only\"\n");
		printf("\nWrite attempted to page marked \"read-only\"\n");
		SysHalt();
		break;

	case BusErrorException:
		DEBUG(dbgAddr, "Translation resulted in an invalid physical address\n");
		printf("\nTranslation resulted in an invalid physical address\n");
		SysHalt();
		break;
	
	case AddressErrorException:
		DEBUG(dbgAddr, "Unaligned reference or one that was beyond the end of the address space\n");
		printf("\nUnaligned reference or one that was beyond the end of the address space\n");
		SysHalt();
		break;

	case OverflowException:
		DEBUG(dbgAddr, "Integer overflow\n");
		printf("\nInteger overflow\n");
		SysHalt();
		break;

	case IllegalInstrException:
		DEBUG(dbgAddr, "Unimplemented or reserved instruction\n");
		printf("\nUnimplemented or reserved instruction\n");
		SysHalt();
		break;

	case NumExceptionTypes:
		DEBUG(dbgAddr, "NumExceptionTypes raised\n");
		printf("\nNumExceptionTypes raised\n");
		SysHalt();
		break;

	case NoException:
		return;
		break;
	
    default:
		cerr << "Unexpected user mode exception" << (int)which << "\n";
		break;
    }
}
