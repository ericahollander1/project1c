#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "pstat.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_nice(void)
{
  int niceValue;
  if(argint(0, &niceValue) < 0) {
    return -1;
  }
  if (niceValue < -20 || niceValue > 19) {
    niceValue = 10;
  }
  myproc()->nice = niceValue;
  myproc()->stride = 1000000 / nice_to_tickets[niceValue + 20];//update stride with nice value
  return 0;
}

uint64
sys_getpstat(void)
{
  uint64 result = 0;
  struct proc *p = myproc();
  uint64 upstat; // the virtual (user) address of the passed argument struct pstat
  struct pstat kpstat; // a struct pstat in kernel memory
  // get the system call argument passed by the user program
  if (argaddr(0, &upstat) < 0)
    return -1;

  for (int i=0; i<NPROC; i++) {
    kpstat.inuse[i] = proc[i].state != UNUSED;
    kpstat.pid[i] = proc[i].pid;
    kpstat.nice[i] = proc[i].nice;
  }

  // copy pstat from kernel memory to user memory
  if (copyout(p->pagetable, upstat, (char *)&kpstat, sizeof(kpstat)) < 0)
    return -1;
  return result;
}
