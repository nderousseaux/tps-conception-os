#include "types.h"
#include "param.h"
#include "x86.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "spinlock.h"
#include "buf.h"

struct buf buf[NBUF];
struct spinlock buf_table_lock;

void
binit(void)
{
  initlock(&buf_table_lock, "buf_table");
}

struct buf *
getblk(uint dev, uint sector)
{
  struct buf *b;
  static struct buf *scan = buf;
  int i;

  acquire(&buf_table_lock);

  while(1){
    for(b = buf; b < buf+NBUF; b++)
      if((b->flags & (B_BUSY|B_VALID)) && b->dev == dev && b->sector == sector)
        break;

    if(b < buf+NBUF){
      if(b->flags & B_BUSY){
        sleep(buf, &buf_table_lock);
      } else {
        b->flags |= B_BUSY;
        release(&buf_table_lock);
        return b;
      }
    } else {
      for(i = 0; i < NBUF; i++){
        b = scan++;
        if(scan >= buf+NBUF)
          scan = buf;
        if((b->flags & B_BUSY) == 0){
          b->flags = B_BUSY;
          b->dev = dev;
          b->sector = sector;
          release(&buf_table_lock);
          return b;
        }
      }
      panic("getblk: no buffers");
    }
  }
}

struct buf *
bread(uint dev, uint sector)
{
  void *c;
  struct buf *b;
  extern struct spinlock ide_lock;

  b = getblk(dev, sector);
  if(b->flags & B_VALID)
    return b;

  acquire(&ide_lock);
  c = ide_start_rw(dev & 0xff, sector, b->data, 1, 1);
  sleep (c, &ide_lock);
  ide_finish(c);
  b->flags |= B_VALID;
  release(&ide_lock);

  return b;
}

void
bwrite(uint dev, struct buf *b, uint sector)
{
  void *c;
  extern struct spinlock ide_lock;

  acquire(&ide_lock);
  c = ide_start_rw(dev & 0xff, sector, b->data, 1, 0);
  sleep (c, &ide_lock);
  ide_finish(c);
  b->flags |= B_VALID;
  release(&ide_lock);
}

void
brelse(struct buf *b)
{
  if((b->flags & B_BUSY) == 0)
    panic("brelse");
  
  acquire(&buf_table_lock);

  b->flags &= ~B_BUSY;
  wakeup(buf);

  release(&buf_table_lock);
}

