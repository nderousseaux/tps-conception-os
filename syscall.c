#include "types.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"
#include "traps.h"
#include "syscall.h"

/*
 * User code makes a system call with INT T_SYSCALL.
 * System call number in %eax.
 * Arguments on the stack, from the user call to the C
 * library system call function. The saved user %esp points
 * to a saved frame pointer, a program counter, and then
 * the first argument.
 *
 * Return value? Error indication? Errno?
 */

/*
 * fetch 32 bits from a user-supplied pointer.
 * returns 1 if addr was OK, 0 if illegal.
 */
int
fetchint(struct proc *p, unsigned addr, int *ip)
{
  *ip = 0;

  if(addr > p->sz - 4)
    return 0;
  memcpy(ip, p->mem + addr, 4);
  return 1;
}

int
fetcharg(int argno, int *ip)
{
  unsigned esp;

  esp = (unsigned) curproc[cpu()]->tf->tf_esp;
  return fetchint(curproc[cpu()], esp + 8 + 4*argno, ip);
}

int
sys_fork()
{
  struct proc *np;

  np = newproc();
  return np->pid;
}

int
sys_exit()
{
  struct proc *p;
  struct proc *cp = curproc[cpu()];

  cp->state = ZOMBIE;

  // wake up parent
  for(p = proc; p < &proc[NPROC]; p++)
    if(p->pid == cp->ppid)
      wakeup(p);

  // abandon children
  for(p = proc; p < &proc[NPROC]; p++)
    if(p->ppid == cp->pid)
      p->pid = 1;

  swtch();

  return 0;
}

int
sys_wait()
{
  struct proc *p;
  struct proc *cp = curproc[cpu()];
  int any, pid;

  cprintf("waid pid %d ppid %d\n", cp->pid, cp->ppid);

  while(1){
    any = 0;
    for(p = proc; p < &proc[NPROC]; p++){
      if(p->state == ZOMBIE && p->ppid == cp->pid){
        kfree(p->mem, p->sz);
        kfree(p->kstack, KSTACKSIZE);
        pid = p->pid;
        p->state = UNUSED;
        cprintf("%x collected %x\n", cp, p);
        return pid;
      }
      if(p->state != UNUSED && p->ppid == cp->pid)
        any = 1;
    }
    if(any == 0){
      cprintf("%x nothing to wait for\n", cp);
      return -1;
    }
    sleep(cp);
  }
}

int
sys_cons_putc()
{
  int c;

  fetcharg(0, &c);
  cons_putc(c & 0xff);
  return 0;
}

void
syscall()
{
  struct proc *cp = curproc[cpu()];
  int num = cp->tf->tf_regs.reg_eax;
  int ret = -1;

  cprintf("%x sys %d\n", cp, num);
  switch(num){
  case SYS_fork:
    ret = sys_fork();
    break;
  case SYS_exit:
    ret = sys_exit();
    break;
  case SYS_wait:
    ret = sys_wait();
    break;
  case SYS_cons_putc:
    ret = sys_cons_putc();
    break;
  default:
    cprintf("unknown sys call %d\n", num);
    // XXX fault
    break;
  }
  cp->tf->tf_regs.reg_eax = ret;
}
