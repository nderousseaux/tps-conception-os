
#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

static struct {
  struct spinlock lock;
  int locking;
} drvmem;

int
drvmemread(struct inode *ip, char *dst, uint off, int n)
{
    // Adaptez votre fonction drvmemread pour agir en fonction du numéro de mineur (localisé dans l’inode) : 0 pour /dev/null et 1 pour /dev/zero.
    //Vous pouvez utiliser la fonction memset (définie dans string.c) pour implémenter le pilote.
    //Retourne le nombre demandé d’octets nuls
    switch (ip->minor)
    {
    case DEVNULL:
        // retourne 0
        return 0;
        break;
    case DEVZERO:
        // Retourne le nombre demandé d’octets nuls
        memset(dst, 0, n);
        return n;
        break;
    case DEVMEM:
        // Retourne le nombre d’octets lus
        if (off > PHYSTOP || off + n > PHYSTOP)
            return -1;
        memmove(dst, (char *)P2V(off), n);
        return n;
        break;
    case DEVKMEM:
        return kmemread(dst, off, n);
        break;
    default:
        //??
        break;
    }


    return 0;
}
int 
drvmemwrite(struct inode *ip, char *buf, uint off, int n)
{
    return 1;
}


void drvmeminit(void)
{
  initlock(&drvmem.lock, "drvmem");

  devsw[DEV].write = drvmemwrite;
  devsw[DEV].read = drvmemread;
  drvmem.locking = 1;

//   ioapicenable(IRQ_KBD, 0);
}