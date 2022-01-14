#include "pcb.h"
#include "main.h"

PCB::PCB(int id)
{
    if (id == 0)
        parentID = -1;
    else
        parentID = kernel->currentThread->GetProcessID();

	this->numwait = this->exitcode = 0;
	this->thread = NULL;

	this->joinsem = new Semaphore("joinsem",0);
	this->exitsem = new Semaphore("exitsem",0);
	this->mutex = new Semaphore("mutex",1);
}

PCB::~PCB()
{
    if (joinsem != NULL)
        delete joinsem;
    if (exitsem != NULL)
        delete exitsem;
    if (mutex != NULL)
        delete mutex;
    if (thread != NULL)
    {
        thread->FreeSpace();
        thread->Finish();
    }
    if (fileName != NULL)
    {
        delete[] fileName;
    }
}

void StartProcess(int id)
{
    char* fileName = kernel->processTab->GetFileName(id);

    AddrSpace *space;
    space = new AddrSpace();
	if(space == NULL)
	{
		printf("\nPCB::Exec : Can't create AddSpace.");
		return;
	}
    if (space->Load(fileName))
    {
        space->Execute();
    }
    
    ASSERT(FALSE);
    
}

int PCB::Exec(char* filename, int id)
{  
	mutex->P();

	this->thread = new Thread(filename);

	if(this->thread == NULL){
		printf("\nPCB::Exec:: Not enough memory..!\n");
        	mutex->V();
		return -1;
	}

	this->thread->SetProcessID(id);
	this->parentID = kernel->currentThread->GetProcessID();
 	this->thread->Fork((VoidFunctionPtr)&StartProcess, (void*)id);

    mutex->V();
	return id;

}

void PCB::SetFileName(char* fileName)
{
    this->fileName = new char[strlen(fileName)+1];
    strcpy(this->fileName, fileName);
}

char* PCB::GetFileName()
{
    return fileName;
}

void PCB::JoinWait()
{
    this->joinsem->P();
}

void PCB::ExitRelease()
{
    this->exitsem->V();
}

int PCB::GetExitCode()
{
    return this->exitcode;
}

void PCB::IncNumWait()
{
    this->mutex->P();
	this->numwait++;
	this->mutex->V();
}

void PCB::JoinRelease()
{
    this->joinsem->V();
}

void PCB::ExitWait()
{
    this->exitsem->V();
}

void PCB::SetExitCode(int ec)
{
    this->exitcode = ec;
}

void PCB::DecNumWait()
{
    this->mutex->P();
	if (numwait > 0)
        --numwait;
	this->mutex->V();
}