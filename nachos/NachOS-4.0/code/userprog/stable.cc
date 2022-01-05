#include "stable.h"
#include "main.h"

STable::STable()
{
    this->bm = new Bitmap(MAX_SEMAPHORE);

    for(int i=0; i<MAX_SEMAPHORE; ++i)
    {
        this->semTab[i] = NULL;
    }
}

STable::~STable(){
    if(this->bm){
        delete this->bm;
        this->bm = NULL;
    }
}

int STable::Create(char* name, int init){
    for(int i = 0; i < MAX_SEMAPHORE; i++){
        if (bm->Test(i)){
            if (strcmp(name, semTab[i]->GetName()) == 0){
                return -1;
            }
        }
    }
    int id = this->FindFreeSlot();
    if (id < 0){
        return -1;
    }
    this->semTab[id] = new Sem(name, init);
    return 0;
}

int STable::Wait(char* name){
    for(int i = 0; i < MAX_SEMAPHORE; i++){
        if (bm->Test(i)){
            if (strcmp(name, semTab[i]->GetName()) == 0){
                semTab[i]->wait();
                return 0;
            }
        }
    }
    return -1;
}

int STable::Signal(char* name){
    for(int i = 0; i < MAX_SEMAPHORE; ++i){
        if (bm->Test(i)){
            if (strcmp(name, semTab[i]->GetName()) == 0){
                semTab[i]->signal();
                return 0;
            }
        }
    }
    return -1;
}

int STable::FindFreeSlot(){
    return this->bm->FindAndSet();
}