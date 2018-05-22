#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
struct proc *q0[NPROC];
struct proc *q1[NPROC];
int counterq0 = -1;
int counterq1 = -1;
int sched_policy = 1;
int queue_switch = 0;
int RUNNING_THRESHOLD = 2;
int WAITING_THRESHOLD = 4;
struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;

int sched_trace_enabled = 1; // for CS550 CPU/process project

extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;
  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  p->running_tick = 0;
  p->waiting_tick = 0;
  p->priority = 0;
  counterq0++;
  q0[counterq0] = p;
  if(p->pid != 1)
  {
  if(p->pid != 2)
  {
 /// cprintf("In alloc proc process %d, Q0=%d \n", q0[counterq0]->pid,counterq0);
  queue_switch++;
  }
  }
  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;
  
  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;
  
  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

int setpriority(int pid, int priority)
{
  struct proc *p;
  
  acquire(&ptable.lock);
  for (p= ptable.proc; p < &ptable.proc[NPROC]; p++)
  {
  if (p->pid == pid)
  {
  if (priority == 0)
  {
  p->priority  = 1;
  }
  else 
  {
  p->priority = 0;
  }
  break;
  }
  }
  release(&ptable.lock);
  return 0;
}

int setwaitingticks(int waiting_thres)
{
  WAITING_THRESHOLD = waiting_thres;
  return 0;
}

int setrunningticks(int time_allotment)
{
  RUNNING_THRESHOLD = time_allotment;
  return 0;
}
//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];
  
  p = allocproc();
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  
  sz = proc->sz;
  if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  proc->sz = sz;
  switchuvm(proc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;

  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;

  // Copy process state from p.
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);

  safestrcpy(np->name, proc->name, sizeof(proc->name));
 
  pid = np->pid;

  // lock to force the compiler to emit the np->state write last.
  acquire(&ptable.lock);
  np->state = RUNNABLE;
  release(&ptable.lock);
  
  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *p;
  int fd;

  if(proc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(proc->cwd);
  end_op();
  proc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(proc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == proc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  proc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  int ran = 0; // CS550: to solve the 100%-CPU-utilization-when-idling problem
  int i;
  int j;
  int k;
  int g;
  int h;
  int e;
  //int m;
int t;
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    ran = 0;
    if (sched_policy == 0) {
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;
	   
      ran = 1;
      
      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      proc = p;
      switchuvm(p);
      p->state = RUNNING;
      swtch(&cpu->scheduler, proc->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      proc = 0;
    }}
    else
    {//cprintf("counterq0 value: %d \n", counterq0);
     Queue0:
     if (counterq0 != -1) {
     //cprintf("PRINT11111111111111111111111111111111111111111111111111111111111111");
     
     for (i=0;i<=counterq0 ;i++)
     {
      //cprintf("Hello \n");
       if(q0[i]->state != RUNNABLE)
          { //cprintf("{{%d}}", q0[i]->pid);

	continue;}
	//cprintf("PRINT11111111111111111111111111111111111111111111111111111111111111");
       //Promotion From Queue 0
	  for (g=0;g<=counterq1 ;g++)
       {
       q1[g]->waiting_tick++;
       ////cprintf("Waiting Tick Incremented From Queue 0: %d \n", q1[g]->waiting_tick);
       }
	   for (h=0;h<=counterq1 ;h++)
       {
       if (WAITING_THRESHOLD <= q1[h]->waiting_tick)
       {
       counterq0++;
       q0[counterq0] = q1[h];
       q0[counterq0]->running_tick = 0;
       q0[counterq0]->waiting_tick = 0;
       q1[h]= 0;
       //cprintf("Assigned \n");
         for(e=h;e<=counterq1;e++)
         q1[e] = q1[e+1];
         q1[counterq1] = 0;
         counterq1--;
        /// cprintf("Shifted \n");
         h=0;
       }
       } 
       ran = 1;
       ////cprintf("Inside Queue 0 %d %d %d\n",q0[i]->pid,RUNNING_THRESHOLD,WAITING_THRESHOLD);
       q0[i]->running_tick++;
       p = q0[i];
       proc = q0[i];
       //cprintf("In Loop [[%d]] \n", q0[i]->pid);
       switchuvm(p);
       p->state = RUNNING;
       swtch(&cpu->scheduler, proc->context);
       switchkvm();
       if(q0[i]->running_tick >= RUNNING_THRESHOLD)
	{
         if (q0[i]->pid != 1)
	{
        if (q0[i]->pid != 2)
        {
         if (q0[i]->priority != 1)
         {
         //cprintf("Inside Threshold");
         counterq1++;
         q1[counterq1] = proc;
         q0[i]= 0;
         for(j=i;j<=counterq0;j++)
         q0[j] = q0[j+1];
         q0[counterq0] = 0;
         proc->running_tick = 0;
	 counterq0--;
         queue_switch--;
 	 //cprintf("End Of Queue 0\n");
         }
	}
	}
	}
       proc = 0;
     }
     }

     for (t=0;t<=counterq0 ;t++)
     {
      if(q0[t]->state == RUNNABLE && q0[t]->pid != 1 && q0[t]->pid != 2)
      {
      goto Queue0;
      }
     }


	//if (queue_switch == 0 )
    //{
     if (counterq1 != -1) {
     //cprintf("PRINT11111111111111111111111111111111111111111111111111111111111111");
     int m;
     int c;
     int temp;
     int tick;
     int q;
     int r;
	 int s;
     for (k=0;k<=counterq1 ;k++)
     {
      //cprintf("Hello \n");
       if(q1[k]->state != RUNNABLE)
          { //cprintf("{{%d}}", q0[i]->pid);

	   continue;}
       if(q1[k]->priority == 1)
          { //cprintf("{{%d}}", q0[i]->pid);
		  //// cprintf("Priority Chaged");
       counterq0++;
       q0[counterq0] = q1[k];
       q0[counterq0]->running_tick = 0;
       q0[counterq0]->waiting_tick = 0;
       q1[k]= 0;
       //cprintf("Assigned \n");
         for(s=k;s<=counterq1;s++)
         q1[s] = q1[s+1];
         q1[counterq1] = 0;
         counterq1--;
	   continue;}
       
       temp = k;
       tick = q1[k]->waiting_tick;
       
      /// cprintf("Waiting tick of Process %d is %d \n", q1[k]->pid ,q1[k]->waiting_tick);
       for (m=0;m<=counterq1 ;m++)
       {
      //cprintf("Hello \n");
       if(q1[m]->state != RUNNABLE)
          { //cprintf("{{%d}}", q0[i]->pid);

	   continue;}
      //// cprintf("Waiting tick of Process %d is %d = %d \n", q1[m]->pid ,q1[m]->waiting_tick,WAITING_THRESHOLD);
       if (tick < q1[m]->waiting_tick){
       temp = m;
       //cprintf("Selected Process is %d and Waiting tick is %d \n", q1[m]->pid ,q1[m]->waiting_tick);
       }
       }
       for (c=0;c<=counterq1 ;c++)
       {
      //cprintf("Hello \n");
       if (c !=  temp){
       q1[c]->waiting_tick++;
       //cprintf("Waiting tick of Process %d is %d \n", q1[k]->pid ,q1[k]->waiting_tick);
       }
       }
       //Promotion Code
             

       //cprintf("Completed %d \n",counterq1);

       //cprintf("Inside Queue 1 %d \n",q1[k]->pid);
        ////cprintf("Selected Process is %d and Waiting tick is %d %d\n", q1[temp]->pid ,q1[temp]->waiting_tick,WAITING_THRESHOLD);
       //q1[temp]->waiting_tick = 0;
       //cprintf("Selected Process is %d and Waiting tick is %d \n", q1[temp]->pid ,q1[temp]->waiting_tick);
       if (temp != -1)
       {
       ran = 1;
       //q1[k]->timer++;
       p = q1[temp];
       proc = q1[temp];
       //cprintf("In Loop [[%d]] \n", q0[i]->pid);
       switchuvm(p);
       p->state = RUNNING;
       swtch(&cpu->scheduler, proc->context);
       switchkvm();
       proc = 0;
       }
       for (q=0;q<=counterq1 ;q++)
       {
       if (WAITING_THRESHOLD <= q1[q]->waiting_tick)
       {
       counterq0++;
       q0[counterq0] = q1[q];
       q0[counterq0]->running_tick = 0;
       q0[counterq0]->waiting_tick = 0;
       q1[q]= 0;
       //cprintf("Assigned \n");
         for(r=q;r<=counterq1;r++)
         q1[r] = q1[r+1];
         q1[counterq1] = 0;
         counterq1--;
        /// cprintf("Shifted \n");
         q=0;
       }
       } 
      //cprintf("Completed %d \n",counterq1);
     }
     }
    }
    //}
    
    release(&ptable.lock);

    if (ran == 0){
        halt();
    }
  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state.
void
sched(void)
{
  int intena;

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;

  // CS550: print previously running process
  // We skip process 1 (init) and 2 (shell)
  if ( sched_trace_enabled &&
	proc && proc->pid != 1 && proc->pid != 2)
        cprintf("[%d]", proc->pid);

  swtch(&proc->context, cpu->scheduler);
  cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  proc->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot 
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }
  
  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  proc->chan = chan;
  proc->state = SLEEPING;
  sched();

  // Tidy up.
  proc->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];
  
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}
