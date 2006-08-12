struct buf {
  int flags;
  uint dev;
  uint sector;
  struct buf *prev;
  struct buf *next;
  uchar data[512];
};
#define B_BUSY 0x1
#define B_VALID 0x2
