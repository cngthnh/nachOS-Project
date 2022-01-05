#include "ptable.h"
#include "main.h"

PTable::PTable(int size)
{
    if (size < 0)
        return;

    psize = size;
    bm = new Bitmap(size);
    bmsem = new Semaphore("bmsem",1);

    for (int i=0; i < MAXPROCESS; ++i) {
		pcb[i] = NULL;
    }

    int newProcessIndex = this->GetFreeSlot();
    if (newProcessIndex < 0)
    {
        DEBUG(dbgThread, "No thread slot available");
    } 
    else 
    {
        pcb[newProcessIndex] = new PCB(newProcessIndex);
        pcb[newProcessIndex]->SetFileName(""); // first program to run on nachOS
        pcb[newProcessIndex]->parentID = -1;
    }
}

int PTable::GetFreeSlot()
{
    return bm->FindAndSet();
}

int PTable::ExecUpdate(char *fileName)
{
    bmsem->P();

    if (fileName == NULL)
    {
        DEBUG(dbgFile, "Invalid filename");
        bmsem->V();
        return -1;
    }

    if (strcmp(fileName, kernel->currentThread->getName()) == 0)
    {
        DEBUG(dbgThread, "Can't self exec");
        bmsem->V();
        return -1;
    }

    int newProcessIndex = this->GetFreeSlot();
    if (newProcessIndex < 0)
    {
        DEBUG(dbgThread, "No thread slot available");
        bmsem->V();
        return -1;
    }

    pcb[newProcessIndex] = new PCB(newProcessIndex);
    pcb[newProcessIndex]->SetFileName(fileName);
    pcb[newProcessIndex]->parentID = kernel->currentThread->GetProcessID();

    int pid = pcb[newProcessIndex]->Exec(fileName, newProcessIndex);
    if (pid < 0)
    {
        bm->Clear(newProcessIndex);
    }
    bmsem->V();

    return pid;
}

int PTable::JoinUpdate(int id)
{
    if (id < 0) 
    {    
        printf("\nId = %d invalid\n", id);
		return -1;
    }
    if (kernel->currentThread->processID != pcb[id]->parentID){
        printf("\nId = %d is not it's parent process\n", id);
        return -1;
    }
    pcb[pcb[id]->parentID]->IncNumWait();
    pcb[id]->JoinWait();
    int exitCode = pcb[id]->GetExitCode();
    pcb[id]->ExitRelease();
	return exitCode;
}

int PTable::ExitUpdate(int ec)
{
    int id = kernel->currentThread->processID;
	if(id == 0)
	{	
		kernel->currentThread->FreeSpace();		
		kernel->interrupt->Halt();
        return 0;
	}
    if (IsExist(id)==false)
    {
        printf("\nThis %d is not exist\n", id);
        return -1;
    }  	
	pcb[id]->SetExitCode(ec);
	pcb[pcb[id]->parentID]->DecNumWait();
	pcb[id]->JoinRelease();
	pcb[id]->ExitWait();
    Remove(id);
	
	return ec;
}

bool PTable::IsExist(int pid)
{
    return this->bm->Test(pid);
}

void PTable::Remove(int pid)
{
    this->bm->Clear(pid);
    if(this->pcb[pid] != 0){
        delete this->pcb[pid];
        this->pcb[pid] = NULL;
    }
}

char* PTable::GetFileName(int id)
{
    return this->pcb[id]->GetFileName();
}