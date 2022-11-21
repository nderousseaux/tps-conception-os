#include "types.h"
#include "fcntl.h"
#include "user.h"

uint
hex2off(char *h)
{
  uint off;

  off=0;
  for(;*h != '\0'; h++)
  {
    int d;

    if (*h >= '0' && *h <= '9')
      d = *h - '0';
    else if (*h >= 'a' && *h <= 'f')
      d = *h - 'a' + 10;
    else if (*h >= 'A' && *h <= 'F')
      d = *h - 'A' + 10;
    else
    {
      printf(2, "illegal hex digit\n") ;
      exit (0) ;
    }
    off = (off << 4) + d;
  }
  return off;
}

int
main(int argc, char *argv[])
{
  uint off;
  int n;
  int i, nr, fd;
  uchar buf [100];

  if(argc != 4){
    printf(2, "Usage: testread dev off n\n");
    exit(0);
  }

  off = hex2off (argv [2]);

  n = atoi (argv [3]);
  if (n < 0) n = 0;
  if (n > sizeof buf) n = sizeof buf;

  printf(1, "off=%x, n=%d\n", off, n);

  if ((fd = open (argv[1], O_RDONLY)) == -1) {
    printf(2, "cannot open %d\n", argv[1]);
    exit(0);
  }
  if (lseek(fd, off, SEEK_SET) == -1) {
    printf(2, "cannot lseek to %d\n", off);
    exit(0);
  }
  nr = read(fd, buf, n);
  printf (1, "read %d bytes\n", nr) ;
  if (nr == -1) {
    printf(2, "cannot read %d bytes", n);
    exit(0);
  }
  if (close(fd) == -1) {
    printf(2, "cannot close %s\n", argv[1]);
    exit(0);
  }

  for (i = 0 ; i < nr ; i++)
  {
    if (i > 0 && i % 16 == 0)
	printf (1, "\n") ;
    printf (1, " %x%x", buf[i] >> 4, buf[i] & 0xf) ;
  }
  printf (1,"\n") ;

  exit(0);
}