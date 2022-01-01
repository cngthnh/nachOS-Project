#ifndef STABLE_H 
#define STABLE_H 
#include "synch.h" 
#include "bitmap.h" 
#define MAX_LOCK 10 
class STable { 
    private: 
        BitMap* bm; 
        Lock* lockTab[MAX_LOCK]; 
    public: 
        STable(); 
        ~STable(); 
        int create(char* name); 
        int wait(char *name); 
        int signal(char *name); 
};
#endif