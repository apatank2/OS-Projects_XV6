#include "types.h"
#include "user.h"
#include "procThread.h"


int procThread_create(void)
{
    // add your implementation here ...
    void *stack = malloc(4096);  
    int tid = clone(stack);
    return tid ;
}


void procThread_exit(int ret_val)
{
    // add your implementation here ...
    thread_exit(ret_val);
    
}


void procThread_join(int tid, int *ret_val_p)
{
    // add your implementation here ...
    void *stack;
    join(tid,ret_val_p,&stack);
}

