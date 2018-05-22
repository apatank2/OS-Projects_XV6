#ifndef _PROCTHREAD_H
#define _PROCTHREAD_H


int procThread_create(void);
void procThread_exit(int ret_val);
void procThread_join(int tid, int * ret_val_p);

#endif
