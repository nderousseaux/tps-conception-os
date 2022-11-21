#include "types.h"
#include "user.h"

void
mknod_prog(char* filename, int major, int minor)
{
  if(mknod(filename, major, minor) < 0)
    printf(2, "mknod: cannot create %s\n", filename);
}

int
main(int argc, char *argv[])
{

  if(argc != 4){
    printf(3, "Usage: mknod_prog <file> <major> <minor>");
    exit(0);
  }
  
  mknod_prog(argv[1], atoi(argv[2]), atoi(argv[3]));
  exit(0);

}
