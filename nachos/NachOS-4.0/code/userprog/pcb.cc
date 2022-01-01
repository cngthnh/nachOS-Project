#include "pcb.h"
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
}

void StartProcess(void* id)
{
    char* fileName = kernel->processTab->GetFileName((*(int*)id));

    AddrSpace *space;
    space = new AddrSpace(fileName);

	if(space == NULL)
	{
		printf("\nPCB::Exec : Can't create AddSpace.");
		return;
	}

    kernel->currentThread->space = space;

    space->InitRegisters();		
    space->RestoreState();		

    kernel->machine->Run();		
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
 	this->thread->Fork(StartProcess, &id);

    mutex->V();
	return id;

}