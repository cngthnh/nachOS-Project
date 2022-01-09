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

// needs to be removed later
#define FILESYS_STUB
#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "synchconsole.h"
#include "sysdep.h"
#include <limits.h>

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
#define BUFFER_MAX_LENGTH 255 //bytes
#define FILENAME_MAX_LENGTH 32 //bytes
#define INT_MAX_DIGIT 12 // Number with max characters: -2147483648, null-terminated
#define ARG_1 4
#define ARG_2 5
#define ARG_3 6
#define ARG_4 7
#define SYSCALL_CODE 2
#define OUTPUT_REG 2


void Flush()
{
	char byte;
	do
	{
		byte = kernel->synchConsoleIn->GetChar();
	} while (byte!=0 && byte!='\n');
}

bool IntRangeCheck(char* number)
{
	char max_Int[INT_MAX_DIGIT] = "2147483647";
	char min_Int[INT_MAX_DIGIT] = "-2147483648";
	bool isNegative = false;
	if (number[0]=='-') isNegative = true;
	if (isNegative)
	{
		if (strlen(number)>strlen(min_Int))
			return false;
		if (strlen(number)<strlen(min_Int))
			return true;
		if (strcmp(number, min_Int)>0)
			return false;
		return true;
	}
	else
	{
		if (strlen(number)>strlen(max_Int))
			return false;
		if (strlen(number)<strlen(max_Int))
			return true;
		if (strcmp(number, max_Int)>0)
			return false;
		return true;
	}
}

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

void Syscall_Halt()
{
	DEBUG(dbgSys, "Called SC_Halt\n");
	DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
	SysHalt();

	ASSERTNOTREACHED();
}

void Syscall_Add()
{

	DEBUG(dbgFile, "Called SC_Add\n");
	DEBUG(dbgFile, "Reading virtual 2 args to be added\n");

	// Read 2 numbers from register
	int arg1 = kernel->machine->ReadRegister(ARG_1);
	int arg2 = kernel->machine->ReadRegister(ARG_2);

	DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(ARG_1) << " + " << kernel->machine->ReadRegister(ARG_2) << "\n");
			
	/* Process SysAdd Systemcall*/
	int result = SysAdd(arg1, arg2);

	DEBUG(dbgSys, "Add returning with " << result << "\n");
	/* Prepare Result */
	kernel->machine->WriteRegister(OUTPUT_REG, result);
}

void Syscall_CreateFile()
{
	int virtualAddress;
			
	char* filename;

	DEBUG(dbgFile, "Reading virtual address of filename\n");
	
	// Get the address of the char array containing the file name
	virtualAddress = kernel->machine->ReadRegister(ARG_1);
	
	DEBUG(dbgFile, "Copying filename to system memory space\n");
	
	// Copy the file name to the system space
	filename = User2System(virtualAddress, FILENAME_MAX_LENGTH+1);
	
	if (filename==NULL)
	{
		DEBUG(dbgAddr, "Insufficient system memory\n");
		kernel->machine->WriteRegister(OUTPUT_REG,-1);
		return;
	} else if (strlen(filename) < 1) 
	{
		DEBUG(dbgFile, "Invalid filename!\n");
		kernel->machine->WriteRegister(OUTPUT_REG,-1);
		delete[] filename;
		return;
	}
	DEBUG(dbgFile, "Filename read successfully\n");
	if (!kernel->fileSystem->Create(filename))
	{
		DEBUG(dbgFile, "Unexpected error while creating file\n");
		kernel->machine->WriteRegister(OUTPUT_REG,-1);
		delete[] filename;
		return;
	}

	DEBUG(dbgFile, "File created successfully\n");
	kernel->machine->WriteRegister(OUTPUT_REG,0);
	delete[] filename;
}

void Syscall_ReadNum()
{
	int number = 0;
	char digit;
	char* numString = new char[INT_MAX_DIGIT]; // Number with max characters: -2147483648, null-terminated

	if (numString == NULL)
	{
		DEBUG(dbgAddr, "Insufficient system memory\n");
		SysHalt();
		return;
	}
	
	int idx = 0;

	// Read every single digit from console
	do
	{
		digit = kernel->synchConsoleIn->GetChar();

		// Terminate the string
		if (digit=='\n') 
		{
			numString[idx]='\0';
			break;
			
		}

		// If not LF, digits, '-' 
		// => Terminate the string and then flush
		if ((digit<'0' || digit>'9') && digit!='-')
		{
			numString[idx]='\0';
			Flush();
			break;
		}

		numString[idx++] = digit;
		
		if (idx==INT_MAX_DIGIT)
		{
			Flush();
			DEBUG(dbgOverflow, "Int: Out of range\n");
			delete[] numString;
			kernel->machine->WriteRegister(OUTPUT_REG, 0); // returns 0
			return;
		}
	} while (idx<INT_MAX_DIGIT);

	// Integer range check
	if (!IntRangeCheck(numString))
	{
		DEBUG(dbgOverflow, "Int: Out of range\n");
		delete[] numString;
		kernel->machine->WriteRegister(OUTPUT_REG, 0); // returns 0
		return;
	}
	
	int i;
	// If the number is negative, we just get digits from index = 1
	if (numString[0]=='-') 
		i = 1;
	else 
		i = 0;

	// Positive => add
	// Negative => sub
	for (; i < idx; ++i)
	{
		number *= 10;
		if (numString[0]=='-')
			number -= int(numString[i])-48;
		else
			number += int(numString[i])-48;
	}
	
	kernel->machine->WriteRegister(OUTPUT_REG, number);

	delete[] numString;

}

void Syscall_PrintNum()
{
	// Read the number from register
	bool isNegative = false;
	int number = kernel->machine->ReadRegister(ARG_1);
	if (number<0) 
	{
		isNegative = true;
	}

	if (isNegative)
	{
		kernel->synchConsoleOut->PutChar('-');
	}

	// create a stack to put the number in (int -> char array)
	char* stack = new char[INT_MAX_DIGIT];

	if (stack == NULL)
	{
		DEBUG(dbgAddr, "Insufficient system memory\n");
		SysHalt();
		return;
	}

	memset(stack, 0, INT_MAX_DIGIT);

	int idx = INT_MAX_DIGIT;

	// Put digits into the stack so that it can be printed out in the right order
	do
	{
		stack[--idx] = abs(number % 10) + 48;
		number/=10;
	}
	while (number != 0);

	// print out digits in the stack
	for (; idx<INT_MAX_DIGIT; ++idx)
	{
		kernel->synchConsoleOut->PutChar(stack[idx]);
	}

	delete[] stack;
}

void Syscall_PrintString()
{
	// Load char array address from register
	int address = kernel->machine->ReadRegister(ARG_1);

	// copy char array from user space to system space
	char* buffer = User2System(address, BUFFER_MAX_LENGTH);

	if (buffer==NULL)
	{
		DEBUG(dbgAddr, "Insufficient system memory\n");
		SysHalt();
		return;
	}

	int idx=0;

	while (buffer[idx]!='\0') // not null
	{
		kernel->synchConsoleOut->PutChar(buffer[idx]);
		++idx;
	}

	delete[] buffer;
}

void Syscall_ReadString()
{
	// Get char array address and array length from register
	int address = kernel->machine->ReadRegister(ARG_1);
	int len = kernel->machine->ReadRegister(ARG_2);
	--len; // reserve the last byte for terminating the string

	// system space buffer
	char *buffer = new char[len];
	char key;

	int i;

	// Get chars from console and then append it to the buffer in the system space
	for (i=0; i<len; ++i)
	{
		key = kernel->synchConsoleIn->GetChar();
		if (key=='\n' || key=='\0')
		{
			buffer[i]=0; // null-terminated string
			break;
		}
		buffer[i]=key;
	}

	if (i==len) 
	{
		// Reach the end of string => Terminate the string and then flush remaining characters
		buffer[i]=0;
		Flush();
	}

	System2User(address, i, buffer);
	delete[] buffer;
}

void Syscall_ReadChar()
{
	// Read a single char from the console
	// Flush all other characters because we just need 1 char
	char key = kernel->synchConsoleIn->GetChar();
	Flush();
	kernel->machine->WriteRegister(OUTPUT_REG, key);
}

void Syscall_PrintChar()
{
	// Get the char from register
	// Print it to the console
	char key = kernel->machine->ReadRegister(ARG_1);
	kernel->synchConsoleOut->PutChar(key);
}

void Syscall_RandomNum()
{
	RandomInit(time(NULL));
	int num = (RandomNumber()%(INT_MAX-1))+1; //Random positive number
	kernel->machine->WriteRegister(OUTPUT_REG, num); 
}

void Syscall_Open()
{
	// Read virtual address of filename
	int virtualAddress = kernel->machine->ReadRegister(ARG_1);
	int fileType = kernel->machine->ReadRegister(ARG_2);

	char* filename;

	DEBUG(dbgFile, "Reading virtual address of filename\n");
	
	// Get the address of the char array containing the file name
	virtualAddress = kernel->machine->ReadRegister(ARG_1);
	
	DEBUG(dbgFile, "Copying filename to system memory space\n");
	
	// Copy the file name to the system space
	filename = User2System(virtualAddress, FILENAME_MAX_LENGTH+1);
	
	if (filename==NULL)
	{
		DEBUG(dbgAddr, "Insufficient system memory\n");
		kernel->machine->WriteRegister(OUTPUT_REG,-1);
		return;
	} else if (strlen(filename) < 1) 
	{
		DEBUG(dbgFile, "Invalid filename!\n");
		kernel->machine->WriteRegister(OUTPUT_REG,-1);
		delete[] filename;
		return;
	}

	if (kernel->fileSystem->GetFileSpace(filename) != NULL)
	{
		DEBUG(dbgFile, "File already opened by another program!\n");
		kernel->machine->WriteRegister(OUTPUT_REG,-1);
		delete[] filename;
		return;
	}

	int fileSpace = kernel->fileSystem->GetFileSpace();
	
	if (fileSpace == -1) 
	{
		DEBUG(dbgFile, "Can't find a space to open the file\n");
		kernel->machine->WriteRegister(OUTPUT_REG, -1);
		delete[] filename;
		return;
	}

	switch (fileType)
	{
		// 0: read and write, 1: read-only
		case 0:
		case 1:
		{
			if (kernel->fileSystem->AssignFileSpace(fileSpace, filename, fileType) != NULL) 
			{
				DEBUG(dbgFile, "File opened successfully!\n");
				kernel->machine->WriteRegister(OUTPUT_REG, fileSpace);
			}
			break;
		}
		default:
		{
			DEBUG(dbgFile, "Invalid file type\n");
			kernel->machine->WriteRegister(OUTPUT_REG, -1);
		}
	}
	delete[] filename;
}

void Syscall_Close()
{
	OpenFileId fileId = kernel->machine->ReadRegister(ARG_1);
	if (fileId >= 0 && fileId < MAX_FILE_NUM)
	{
		// If the file is already opened
		if (kernel->fileSystem->FreeUpFileSpace(fileId))
		{
			kernel->machine->WriteRegister(OUTPUT_REG, 0);
			DEBUG(dbgFile, "File closed successfully!\n");
			return;
		}
	}
	kernel->machine->WriteRegister(OUTPUT_REG, -1);
	DEBUG(dbgFile, "Invalid File ID or the file hadn't been opened yet!\n");
}

void Syscall_Read()
{
	// user space buffer address
	int bufferAddress = kernel->machine->ReadRegister(ARG_1);
	int charCount = kernel->machine->ReadRegister(ARG_2);
	OpenFileId fileId = kernel->machine->ReadRegister(ARG_3);
	switch (fileId) 
	{
		// stdin
		case 0:
		{
			// system space buffer
			char *buffer = new char[charCount + 1]; // for terminating
			char key;

			int i;

			// Get chars from console and then append it to the buffer in the system space
			for (i=0; i<charCount; ++i)
			{
				key = kernel->synchConsoleIn->GetChar();
				if (key == 0)
				{
					buffer[i]=0; // null-terminated string
					break;
				}
				buffer[i]=key;
			}

			if (i == charCount)
			{
				buffer[i] = 0;
			}

			if (i < charCount)
			{
				delete[] buffer;
				DEBUG(dbgFile, "Reached EOF\n");
				kernel->machine->WriteRegister(OUTPUT_REG, -2);
			}

			System2User(bufferAddress, i, buffer);

			// return the actual size
			kernel->machine->WriteRegister(OUTPUT_REG, i);
			delete[] buffer;
			break;
		}
		// stdout
		case 1:
		{
			DEBUG(dbgFile, "stdout can't be read!\n");
			kernel->machine->WriteRegister(OUTPUT_REG, -1);
			break;
		}
		default:
		{
			if (fileId > 1 && fileId < MAX_FILE_NUM) 
			{
				char *buffer = new char[charCount + 1]; // for terminating
				int actuallyRead = kernel->fileSystem->GetFileSpace(fileId)->Read(buffer, charCount);

				if (actuallyRead > 0)
				{
					buffer[actuallyRead] = 0;

					System2User(bufferAddress, actuallyRead, buffer);

					// return the actual size
					kernel->machine->WriteRegister(OUTPUT_REG, actuallyRead);
					delete[] buffer;
				}

				else
				{
					delete[] buffer;
					DEBUG(dbgFile, "Reached EOF\n");
					kernel->machine->WriteRegister(OUTPUT_REG, -2);
				}

			}
			else 
			{
				DEBUG(dbgFile, "Invalid File ID\n");
				kernel->machine->WriteRegister(OUTPUT_REG, -1);
			}
		}
	}
}

void Syscall_Write()
{
	int bufferAddress = kernel->machine->ReadRegister(ARG_1);
	int charCount = kernel->machine->ReadRegister(ARG_2);
	OpenFileId fileId = kernel->machine->ReadRegister(ARG_3);
	switch (fileId) 
	{
		// stdin
		case 0:
		{
			DEBUG(dbgFile, "stdout can't be written!\n");
			kernel->machine->WriteRegister(OUTPUT_REG, -1);
			break;
		}
		// stdout
		case 1:
		{
			// system space buffer
			char *buffer = User2System(bufferAddress, charCount);
			char key;

			// Get chars from console and then append it to the buffer in the system space
			if (buffer==NULL)
			{
				DEBUG(dbgAddr, "Insufficient system memory\n");
				SysHalt();
				return;
			}

			int idx=0;

			while (buffer[idx] != 0) // not null
			{
				kernel->synchConsoleOut->PutChar(buffer[idx++]);
			}

			// return the actual size
			kernel->machine->WriteRegister(OUTPUT_REG, idx);
			delete[] buffer;
			break;
		}
		default:
		{
			if (fileId > 1 && fileId < MAX_FILE_NUM) 
			{
				char *buffer = User2System(bufferAddress, charCount);

				OpenFile* currentFile = kernel->fileSystem->GetFileSpace(fileId);

				if (currentFile->GetFileType() != 0)
				{
					DEBUG(dbgFile, "Read-only files can't be written\n");
					kernel->machine->WriteRegister(OUTPUT_REG, -1);
					delete[] buffer;
					return;
				}

				int actuallyWrite = currentFile->Write(buffer, charCount);

				if (actuallyWrite > 0)
				{
					buffer[actuallyWrite] = 0;

					// return the actual size
					kernel->machine->WriteRegister(OUTPUT_REG, actuallyWrite);
					delete[] buffer;
				}

				else
				{
					delete[] buffer;
					DEBUG(dbgFile, "Reached EOF\n");
					kernel->machine->WriteRegister(OUTPUT_REG, -2);
				}

			}
			else 
			{
				DEBUG(dbgFile, "Invalid File ID\n");
				kernel->machine->WriteRegister(OUTPUT_REG, -1);
			}
		}
	}
}

void Syscall_Exec()
{
	int virtualAddress = kernel->machine->ReadRegister(ARG_1);
	char *name = User2System(virtualAddress, FILENAME_MAX_LENGTH);
	if (name == NULL) 
	{
		DEBUG(dbgAddr, "Insufficient memory space to load filename");
		kernel->machine->WriteRegister(OUTPUT_REG, -1);
		return;
	}

	OpenFile *openFile = kernel->fileSystem->Open(name);
	if (openFile == NULL) 
	{
		DEBUG(dbgFile, "File can't be opened");
		kernel->machine->WriteRegister(OUTPUT_REG, -1);
		return;
	}
	delete openFile;

	int pid = kernel->processTab->ExecUpdate(name);
	delete[] name;
	if (pid < 0)
	{
		DEBUG(dbgThread, "Can't start a new process");
		kernel->machine->WriteRegister(OUTPUT_REG, -1);
		return;
	}

	kernel->machine->WriteRegister(OUTPUT_REG, pid);
}

void Syscall_Join()
{
	int id = kernel->machine->ReadRegister(ARG_1);
	if (id < 0)
	{
		DEBUG(dbgFile, "Invalid File ID\n");
		kernel->machine->WriteRegister(OUTPUT_REG, -1);
		return;
	}		
	int result = kernel->processTab->JoinUpdate(id);
			
	kernel->machine->WriteRegister(OUTPUT_REG, result);
}

void Syscall_Exit()
{
	int exitStatus = kernel->machine->ReadRegister(ARG_1);		
	int result = kernel->processTab->ExitUpdate(exitStatus);
	kernel->machine->WriteRegister(OUTPUT_REG, result);
	kernel->currentThread->FreeSpace();
	kernel->currentThread->Finish();
}

void Syscall_CreateSemaphore()
{
	int virtualAddress = kernel->machine->ReadRegister(ARG_1);
	int semval = kernel->machine->ReadRegister(ARG_2);

	char* name = User2System(virtualAddress, FILENAME_MAX_LENGTH);
    if (name == NULL)
	{
        DEBUG(dbgAddr, "Insufficient memory space to load name");
		kernel->machine->WriteRegister(OUTPUT_REG, -1);
		return;
    }
	
	int result = kernel->semTab->Create(name, semval);
	delete[] name;

	if (result == -1)
	{
		DEBUG(dbgAddr, "Can't Create Semaphore");
	}

	kernel->machine->WriteRegister(OUTPUT_REG, result);

}

void Syscall_Signal()
{
	int virtualAddress = kernel->machine->ReadRegister(ARG_1);
	char* name = User2System(virtualAddress, BUFFER_MAX_LENGTH);
    if (name == NULL || strlen(name) == 0)
	{
        DEBUG(dbgAddr, "Insufficient memory space to load name");
		kernel->machine->WriteRegister(OUTPUT_REG, -1);
		return;
    }

	int result = kernel->semTab->Signal(name);
	delete[] name;

	if (result == -1)
	{
		DEBUG(dbgAddr, "Semaphore is not exist!");
	}
	kernel->machine->WriteRegister(OUTPUT_REG, result);
}

void Syscall_Wait()
{
	int virtualAddress = kernel->machine->ReadRegister(ARG_1);
	char* name = User2System(virtualAddress, BUFFER_MAX_LENGTH);
    if (name == NULL || strlen(name) == 0)
	{
        DEBUG(dbgAddr, "Insufficient memory space to load name");
		kernel->machine->WriteRegister(OUTPUT_REG, -1);
		return;
    }

	int result = kernel->semTab->Wait(name);
	delete[] name;

	if (result == -1)
	{
		DEBUG(dbgAddr, "Semaphore is not exist!");
	}
	kernel->machine->WriteRegister(OUTPUT_REG, result);
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
			Syscall_Halt();
			break;
		}

		case SC_Exit:
		{
			Syscall_Exit();
			IncreasePC();
			break;
		}

		case SC_Add:
		{
			Syscall_Add();
			IncreasePC();
			break;
		}

		case SC_ReadNum:
		{
			Syscall_ReadNum();
			IncreasePC();
			break;
		}

		case SC_PrintNum:
		{
			Syscall_PrintNum();
			IncreasePC();
			break;
		}

		case SC_PrintString:
		{
			Syscall_PrintString();
			IncreasePC();
			break;
		}

		case SC_ReadString:
		{
			Syscall_ReadString();
			IncreasePC();
			break;
		}

		case SC_ReadChar:
		{
			Syscall_ReadChar();
			IncreasePC();
			break;
		}

		case SC_PrintChar:
		{
			Syscall_PrintChar();
			IncreasePC();
			break;
		}

		case SC_RandomNum:
		{
			Syscall_RandomNum();
			IncreasePC();
			break;
		}

		case SC_CreateFile:
		{
			Syscall_CreateFile();
			IncreasePC();
			break;
		}

		case SC_Open:
		{
			Syscall_Open();
			IncreasePC();
			break;
		}

		case SC_Close:
		{
			Syscall_Close();
			IncreasePC();
			break;
		}

		case SC_Read:
		{
			Syscall_Read();
			IncreasePC();
			break;
		}

		case SC_Write:
		{
			Syscall_Write();
			IncreasePC();
			break;
		}

		case SC_Exec:
		{
			Syscall_Exec();
			IncreasePC();
			break;
		}

		case SC_Join:
		{
			Syscall_Join();
			IncreasePC();
			break;
		}

		case SC_CreateSemaphore:
		{
			Syscall_CreateSemaphore();
			IncreasePC();
			break;
		}

		case SC_Signal:
		{
			Syscall_Signal();
			IncreasePC();
			break;
		}

		case SC_Wait:
		{
			Syscall_Wait();
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
