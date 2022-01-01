#include "ptable.h"

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